#line 2 "light_funcs.cl"

/***************************************************************************
 * Copyright 1998-2013 by authors (see AUTHORS.txt)                        *
 *                                                                         *
 *   This file is part of LuxRender.                                       *
 *                                                                         *
 * Licensed under the Apache License, Version 2.0 (the "License");         *
 * you may not use this file except in compliance with the License.        *
 * You may obtain a copy of the License at                                 *
 *                                                                         *
 *     http://www.apache.org/licenses/LICENSE-2.0                          *
 *                                                                         *
 * Unless required by applicable law or agreed to in writing, software     *
 * distributed under the License is distributed on an "AS IS" BASIS,       *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*
 * See the License for the specific language governing permissions and     *
 * limitations under the License.                                          *
 ***************************************************************************/

//------------------------------------------------------------------------------
// InfiniteLight
//------------------------------------------------------------------------------

#if defined(PARAM_HAS_INFINITELIGHT)

float3 InfiniteLight_GetRadiance(__global InfiniteLight *infiniteLight,
		__global float *infiniteLightDistirbution,
		const float3 dir, float *directPdfA
	IMAGEMAPS_PARAM_DECL) {
	__global ImageMap *imageMap = &imageMapDescs[infiniteLight->imageMapIndex];
	__global float *pixels = ImageMap_GetPixelsAddress(
			imageMapBuff, imageMap->pageIndex, imageMap->pixelsIndex);

	const float3 localDir = normalize(Transform_InvApplyVector(&infiniteLight->light2World, -dir));
	const float2 uv = (float2)(
		SphericalPhi(localDir) * (1.f / (2.f * M_PI_F)),
		SphericalTheta(localDir) * M_1_PI_F);

	// TextureMapping2D_Map() is expended here
	const float2 scale = VLOAD2F(&infiniteLight->mapping.uvMapping2D.uScale);
	const float2 delta = VLOAD2F(&infiniteLight->mapping.uvMapping2D.uDelta);
	const float2 mapUV = uv * scale + delta;

	const float distPdf = Distribution2D_Pdf(infiniteLightDistirbution, mapUV.s0, mapUV.s1);
	*directPdfA = distPdf / (4.f * M_PI_F);

	return VLOAD3F(&infiniteLight->gain.r) * ImageMap_GetSpectrum(
			pixels,
			imageMap->width, imageMap->height, imageMap->channelCount,
			mapUV.s0, mapUV.s1);
}

float3 InfiniteLight_Illuminate(__global InfiniteLight *infiniteLight,
		__global float *infiniteLightDistirbution,
		const float worldCenterX, const float worldCenterY, const float worldCenterZ,
		const float sceneRadius,
		const float u0, const float u1, const float3 p,
		float3 *dir, float *distance, float *directPdfW
		IMAGEMAPS_PARAM_DECL) {
	float2 sampleUV;
	float distPdf;
	Distribution2D_SampleContinuous(infiniteLightDistirbution, u0, u1, &sampleUV, &distPdf);

	const float phi = sampleUV.s0 * 2.f * M_PI_F;
	const float theta = sampleUV.s1 * M_PI_F;
	*dir = normalize(Transform_ApplyVector(&infiniteLight->light2World,
			SphericalDirection(sin(theta), cos(theta), phi)));

	const float3 worldCenter = (float3)(worldCenterX, worldCenterY, worldCenterZ);
	const float worldRadius = PARAM_LIGHT_WORLD_RADIUS_SCALE * sceneRadius * 1.01f;

	const float3 toCenter = worldCenter - p;
	const float centerDistance = dot(toCenter, toCenter);
	const float approach = dot(toCenter, *dir);
	*distance = approach + sqrt(max(0.f, worldRadius * worldRadius -
		centerDistance + approach * approach));

	const float3 emisPoint = p + (*distance) * (*dir);
	const float3 emisNormal = normalize(worldCenter - emisPoint);

	const float cosAtLight = dot(emisNormal, -(*dir));
	if (cosAtLight < DEFAULT_COS_EPSILON_STATIC)
		return BLACK;

	*directPdfW = distPdf / (4.f * M_PI_F);

	// InfiniteLight_GetRadiance  is expended here
	__global ImageMap *imageMap = &imageMapDescs[infiniteLight->imageMapIndex];
	__global float *pixels = ImageMap_GetPixelsAddress(
			imageMapBuff, imageMap->pageIndex, imageMap->pixelsIndex);

	const float2 uv = (float2)(sampleUV.s0, sampleUV.s1);

	// TextureMapping2D_Map() is expended here
	const float2 scale = VLOAD2F(&infiniteLight->mapping.uvMapping2D.uScale);
	const float2 delta = VLOAD2F(&infiniteLight->mapping.uvMapping2D.uDelta);
	const float2 mapUV = uv * scale + delta;
	
	return VLOAD3F(&infiniteLight->gain.r) * ImageMap_GetSpectrum(
			pixels,
			imageMap->width, imageMap->height, imageMap->channelCount,
			mapUV.s0, mapUV.s1);
}

#endif

//------------------------------------------------------------------------------
// SktLight
//------------------------------------------------------------------------------

#if defined(PARAM_HAS_SKYLIGHT)

float SkyLight_PerezBase(__global float *lam, const float theta, const float gamma) {
	return (1.f + lam[1] * exp(lam[2] / cos(theta))) *
		(1.f + lam[3] * exp(lam[4] * gamma)  + lam[5] * cos(gamma) * cos(gamma));
}

float SkyLight_RiAngleBetween(const float thetav, const float phiv, const float theta, const float phi) {
	const float cospsi = sin(thetav) * sin(theta) * cos(phi - phiv) + cos(thetav) * cos(theta);
	if (cospsi >= 1.f)
		return 0.f;
	if (cospsi <= -1.f)
		return M_PI_F;
	return acos(cospsi);
}

float3 SkyLight_ChromaticityToSpectrum(float Y, float x, float y) {
	float X, Z;
	
	if (y != 0.f)
		X = (x / y) * Y;
	else
		X = 0.f;
	
	if (y != 0.f && Y != 0.f)
		Z = (1.f - x - y) / y * Y;
	else
		Z = 0.f;

	// Assuming sRGB (D65 illuminant)
	return (float3)(3.2410f * X - 1.5374f * Y - 0.4986f * Z,
			-0.9692f * X + 1.8760f * Y + 0.0416f * Z,
			0.0556f * X - 0.2040f * Y + 1.0570f * Z);
}

float3 SkyLight_GetSkySpectralRadiance(__global SkyLight *skyLight,
		const float theta, const float phi) {
	// Add bottom half of hemisphere with horizon colour
	const float theta_fin = fmin(theta, (M_PI_F * .5f) - .001f);
	const float gamma = SkyLight_RiAngleBetween(theta, phi, skyLight->thetaS, skyLight->phiS);

	// Compute xyY values
	const float x = skyLight->zenith_x * SkyLight_PerezBase(skyLight->perez_x, theta_fin, gamma);
	const float y = skyLight->zenith_y * SkyLight_PerezBase(skyLight->perez_y, theta_fin, gamma);
	const float Y = skyLight->zenith_Y * SkyLight_PerezBase(skyLight->perez_Y, theta_fin, gamma);

	return SkyLight_ChromaticityToSpectrum(Y, x, y);
}

float3 SkyLight_GetRadiance(__global SkyLight *skyLight, const float3 dir,
		float *directPdfA) {
	*directPdfA = 1.f / (4.f * M_PI_F);

	const float3 localDir = normalize(Transform_InvApplyVector(&skyLight->light2World, -dir));
	const float theta = SphericalTheta(localDir);
	const float phi = SphericalPhi(localDir);
	const float3 s = SkyLight_GetSkySpectralRadiance(skyLight, theta, phi);

	return VLOAD3F(&skyLight->gain.r) * s;
}

float3 SkyLight_Illuminate(__global SkyLight *skyLight,
		const float worldCenterX, const float worldCenterY, const float worldCenterZ,
		const float sceneRadius,
		const float u0, const float u1, const float3 p,
		float3 *dir, float *distance, float *directPdfW) {
	const float3 worldCenter = (float3)(worldCenterX, worldCenterY, worldCenterZ);
	const float worldRadius = PARAM_LIGHT_WORLD_RADIUS_SCALE * sceneRadius * 1.01f;

	const float3 localDir = normalize(Transform_ApplyVector(&skyLight->light2World, -(*dir)));
	*dir = normalize(Transform_ApplyVector(&skyLight->light2World,  UniformSampleSphere(u0, u1)));

	const float3 toCenter = worldCenter - p;
	const float centerDistance = dot(toCenter, toCenter);
	const float approach = dot(toCenter, *dir);
	*distance = approach + sqrt(max(0.f, worldRadius * worldRadius -
		centerDistance + approach * approach));

	const float3 emisPoint = p + (*distance) * (*dir);
	const float3 emisNormal = normalize(worldCenter - emisPoint);

	const float cosAtLight = dot(emisNormal, -(*dir));
	if (cosAtLight < DEFAULT_COS_EPSILON_STATIC)
		return BLACK;

	return SkyLight_GetRadiance(skyLight, -(*dir), directPdfW);
}

#endif

//------------------------------------------------------------------------------
// SunLight
//------------------------------------------------------------------------------

#if defined(PARAM_HAS_SUNLIGHT)

float3 SunLight_Illuminate(__global SunLight *sunLight,
		const float u0, const float u1,
		float3 *dir, float *distance, float *directPdfW) {
	const float cosThetaMax = sunLight->cosThetaMax;
	const float3 sunDir = VLOAD3F(&sunLight->sunDir.x);
	*dir = UniformSampleCone(u0, u1, cosThetaMax, VLOAD3F(&sunLight->x.x), VLOAD3F(&sunLight->y.x), sunDir);

	// Check if the point can be inside the sun cone of light
	const float cosAtLight = dot(sunDir, *dir);
	if (cosAtLight <= cosThetaMax)
		return BLACK;

	*distance = INFINITY;
	*directPdfW = UniformConePdf(cosThetaMax);

	return VLOAD3F(&sunLight->sunColor.r);
}

float3 SunLight_GetRadiance(__global SunLight *sunLight, const float3 dir, float *directPdfA) {
	const float cosThetaMax = sunLight->cosThetaMax;
	const float3 sunDir = VLOAD3F(&sunLight->sunDir.x);

	if ((cosThetaMax < 1.f) && (dot(-dir, sunDir) > cosThetaMax)) {
		if (directPdfA)
			*directPdfA = UniformConePdf(cosThetaMax);

		return VLOAD3F(&sunLight->sunColor.r);
	} else
		return BLACK;
}

#endif

//------------------------------------------------------------------------------
// TriangleLight
//------------------------------------------------------------------------------

float3 TriangleLight_Illuminate(__global TriangleLight *triLight, __global HitPoint *tmpHitPoint,
		const float3 p, const float u0, const float u1, const float passThroughEvent,
		float3 *dir, float *distance, float *directPdfW
		MATERIALS_PARAM_DECL) {
	const float3 p0 = VLOAD3F(&triLight->v0.x);
	const float3 p1 = VLOAD3F(&triLight->v1.x);
	const float3 p2 = VLOAD3F(&triLight->v2.x);
	float b0, b1, b2;
	float3 samplePoint = Triangle_Sample(
			p0, p1, p2,
			u0, u1,
			&b0, &b1, &b2);

	const float3 sampleN = Triangle_GetGeometryNormal(p0, p1, p2); // Light sources are supposed to be flat

	*dir = samplePoint - p;
	const float distanceSquared = dot(*dir, *dir);;
	*distance = sqrt(distanceSquared);
	*dir /= (*distance);

	const float cosAtLight = dot(sampleN, -(*dir));
	if (cosAtLight < DEFAULT_COS_EPSILON_STATIC)
		return BLACK;

	*directPdfW = triLight->invArea * distanceSquared / cosAtLight;

	const float2 uv0 = VLOAD2F(&triLight->uv0.u);
	const float2 uv1 = VLOAD2F(&triLight->uv1.u);
	const float2 uv2 = VLOAD2F(&triLight->uv2.u);
	const float2 triUV = Triangle_InterpolateUV(uv0, uv1, uv2, b0, b1, b2);

	VSTORE3F(-sampleN, &tmpHitPoint->fixedDir.x);
	VSTORE3F(samplePoint, &tmpHitPoint->p.x);
	VSTORE2F(triUV, &tmpHitPoint->uv.u);
	VSTORE3F(sampleN, &tmpHitPoint->geometryN.x);
	VSTORE3F(sampleN, &tmpHitPoint->shadeN.x);
#if defined(PARAM_HAS_PASSTHROUGH)
	tmpHitPoint->passThroughEvent = passThroughEvent;
#endif

	return Material_GetEmittedRadiance(&mats[triLight->materialIndex], tmpHitPoint
			MATERIALS_PARAM);
}

float3 TriangleLight_GetRadiance(__global TriangleLight *triLight,
		 __global HitPoint *hitPoint, float *directPdfA
		MATERIALS_PARAM_DECL) {
	const float3 dir = VLOAD3F(&hitPoint->fixedDir.x);
	const float3 hitPointNormal = VLOAD3F(&hitPoint->geometryN.x);
	const float cosOutLight = dot(hitPointNormal, dir);
	if (cosOutLight <= 0.f)
		return BLACK;

	if (directPdfA)
		*directPdfA = triLight->invArea;

	return Material_GetEmittedRadiance(&mats[triLight->materialIndex], hitPoint
			MATERIALS_PARAM);
}
