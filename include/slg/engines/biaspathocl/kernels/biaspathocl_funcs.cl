#line 2 "biaspatchocl_funcs.cl"

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

void SR_RadianceClamp(__global SampleResult *sampleResult) {
	// Initialize only Spectrum fields

#if defined(PARAM_FILM_RADIANCE_GROUP_0)
	sampleResult->radiancePerPixelNormalized[0].c[0] = clamp(sampleResult->radiancePerPixelNormalized[0].c[0], 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);
	sampleResult->radiancePerPixelNormalized[0].c[1] = clamp(sampleResult->radiancePerPixelNormalized[0].c[1], 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);
	sampleResult->radiancePerPixelNormalized[0].c[2] = clamp(sampleResult->radiancePerPixelNormalized[0].c[2], 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);
#endif
#if defined(PARAM_FILM_RADIANCE_GROUP_1)
	sampleResult->radiancePerPixelNormalized[1].c[0] = clamp(sampleResult->radiancePerPixelNormalized[1].c[0], 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);
	sampleResult->radiancePerPixelNormalized[1].c[1] = clamp(sampleResult->radiancePerPixelNormalized[1].c[1], 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);
	sampleResult->radiancePerPixelNormalized[1].c[2] = clamp(sampleResult->radiancePerPixelNormalized[1].c[2], 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);
#endif
#if defined(PARAM_FILM_RADIANCE_GROUP_2)
	sampleResult->radiancePerPixelNormalized[2].c[0] = clamp(sampleResult->radiancePerPixelNormalized[2].c[0], 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);
	sampleResult->radiancePerPixelNormalized[2].c[1] = clamp(sampleResult->radiancePerPixelNormalized[2].c[1], 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);
	sampleResult->radiancePerPixelNormalized[2].c[2] = clamp(sampleResult->radiancePerPixelNormalized[2].c[2], 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);
#endif
#if defined(PARAM_FILM_RADIANCE_GROUP_3)
	sampleResult->radiancePerPixelNormalized[3].c[0] = clamp(sampleResult->radiancePerPixelNormalized[3].c[0], 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);
	sampleResult->radiancePerPixelNormalized[3].c[1] = clamp(sampleResult->radiancePerPixelNormalized[3].c[1], 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);
	sampleResult->radiancePerPixelNormalized[3].c[2] = clamp(sampleResult->radiancePerPixelNormalized[3].c[2], 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);
#endif
#if defined(PARAM_FILM_RADIANCE_GROUP_4)
	sampleResult->radiancePerPixelNormalized[4].c[0] = clamp(sampleResult->radiancePerPixelNormalized[4].c[0], 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);
	sampleResult->radiancePerPixelNormalized[4].c[1] = clamp(sampleResult->radiancePerPixelNormalized[4].c[1], 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);
	sampleResult->radiancePerPixelNormalized[4].c[2] = clamp(sampleResult->radiancePerPixelNormalized[4].c[2], 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);
#endif
#if defined(PARAM_FILM_RADIANCE_GROUP_5)
	sampleResult->radiancePerPixelNormalized[5].c[0] = clamp(sampleResult->radiancePerPixelNormalized[5].c[0], 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);
	sampleResult->radiancePerPixelNormalized[5].c[1] = clamp(sampleResult->radiancePerPixelNormalized[5].c[1], 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);
	sampleResult->radiancePerPixelNormalized[5].c[2] = clamp(sampleResult->radiancePerPixelNormalized[5].c[2], 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);
#endif
#if defined(PARAM_FILM_RADIANCE_GROUP_6)
	sampleResult->radiancePerPixelNormalized[6].c[0] = clamp(sampleResult->radiancePerPixelNormalized[6].c[0], 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);
	sampleResult->radiancePerPixelNormalized[6].c[1] = clamp(sampleResult->radiancePerPixelNormalized[6].c[1], 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);
	sampleResult->radiancePerPixelNormalized[6].c[2] = clamp(sampleResult->radiancePerPixelNormalized[6].c[2], 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);
#endif
#if defined(PARAM_FILM_RADIANCE_GROUP_7)
	sampleResult->radiancePerPixelNormalized[7].c[0] = clamp(sampleResult->radiancePerPixelNormalized[7].c[0], 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);
	sampleResult->radiancePerPixelNormalized[7].c[1] = clamp(sampleResult->radiancePerPixelNormalized[7].c[1], 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);
	sampleResult->radiancePerPixelNormalized[7].c[2] = clamp(sampleResult->radiancePerPixelNormalized[7].c[2], 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);
#endif
}

void SR_Accumulate(__global SampleResult *src, SampleResult *dst) {
#if defined(PARAM_FILM_RADIANCE_GROUP_0)
	dst->radiancePerPixelNormalized[0].c[0] += src->radiancePerPixelNormalized[0].c[0];
	dst->radiancePerPixelNormalized[0].c[1] += src->radiancePerPixelNormalized[0].c[1];
	dst->radiancePerPixelNormalized[0].c[2] += src->radiancePerPixelNormalized[0].c[2];
#endif
#if defined(PARAM_FILM_RADIANCE_GROUP_1)
	dst->radiancePerPixelNormalized[1].c[0] += src->radiancePerPixelNormalized[1].c[0];
	dst->radiancePerPixelNormalized[1].c[1] += src->radiancePerPixelNormalized[1].c[1];
	dst->radiancePerPixelNormalized[1].c[2] += src->radiancePerPixelNormalized[1].c[2];
#endif
#if defined(PARAM_FILM_RADIANCE_GROUP_2)
	dst->radiancePerPixelNormalized[2].c[0] += src->radiancePerPixelNormalized[2].c[0];
	dst->radiancePerPixelNormalized[2].c[1] += src->radiancePerPixelNormalized[2].c[1];
	dst->radiancePerPixelNormalized[2].c[2] += src->radiancePerPixelNormalized[2].c[2];
#endif
#if defined(PARAM_FILM_RADIANCE_GROUP_3)
	dst->radiancePerPixelNormalized[3].c[0] += src->radiancePerPixelNormalized[3].c[0];
	dst->radiancePerPixelNormalized[3].c[1] += src->radiancePerPixelNormalized[3].c[1];
	dst->radiancePerPixelNormalized[3].c[2] += src->radiancePerPixelNormalized[3].c[2];
#endif
#if defined(PARAM_FILM_RADIANCE_GROUP_4)
	dst->radiancePerPixelNormalized[4].c[0] += src->radiancePerPixelNormalized[4].c[0];
	dst->radiancePerPixelNormalized[4].c[1] += src->radiancePerPixelNormalized[4].c[1];
	dst->radiancePerPixelNormalized[4].c[2] += src->radiancePerPixelNormalized[4].c[2];
#endif
#if defined(PARAM_FILM_RADIANCE_GROUP_5)
	dst->radiancePerPixelNormalized[5].c[0] += src->radiancePerPixelNormalized[5].c[0];
	dst->radiancePerPixelNormalized[5].c[1] += src->radiancePerPixelNormalized[5].c[1];
	dst->radiancePerPixelNormalized[5].c[2] += src->radiancePerPixelNormalized[5].c[2];
#endif
#if defined(PARAM_FILM_RADIANCE_GROUP_6)
	dst->radiancePerPixelNormalized[6].c[0] += src->radiancePerPixelNormalized[6].c[0];
	dst->radiancePerPixelNormalized[6].c[1] += src->radiancePerPixelNormalized[6].c[1];
	dst->radiancePerPixelNormalized[6].c[2] += src->radiancePerPixelNormalized[6].c[2];
#endif
#if defined(PARAM_FILM_RADIANCE_GROUP_7)
	dst->radiancePerPixelNormalized[7].c[0] += src->radiancePerPixelNormalized[7].c[0];
	dst->radiancePerPixelNormalized[7].c[1] += src->radiancePerPixelNormalized[7].c[1];
	dst->radiancePerPixelNormalized[7].c[2] += src->radiancePerPixelNormalized[7].c[2];
#endif
#if defined(PARAM_FILM_CHANNELS_HAS_ALPHA)
	dst->alpha += dst->alpha + src->alpha;
#endif

#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_DIFFUSE)
	dst->directDiffuse.c[0] += src->directDiffuse.c[0];
	dst->directDiffuse.c[1] += src->directDiffuse.c[1];
	dst->directDiffuse.c[2] += src->directDiffuse.c[2];
#endif
#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_GLOSSY)
	dst->directGlossy.c[0] += src->directGlossy.c[0];
	dst->directGlossy.c[1] += src->directGlossy.c[1];
	dst->directGlossy.c[2] += src->directGlossy.c[2];
#endif
#if defined(PARAM_FILM_CHANNELS_HAS_EMISSION)
	dst->emission.c[0] += src->emission.c[0];
	dst->emission.c[1] += src->emission.c[1];
	dst->emission.c[2] += src->emission.c[2];
#endif
#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_DIFFUSE)
	dst->indirectDiffuse.c[0] += src->indirectDiffuse.c[0];
	dst->indirectDiffuse.c[1] += src->indirectDiffuse.c[1];
	dst->indirectDiffuse.c[2] += src->indirectDiffuse.c[2];
#endif
#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_GLOSSY)
	dst->indirectGlossy.c[0] += src->indirectGlossy.c[0];
	dst->indirectGlossy.c[1] += src->indirectGlossy.c[1];
	dst->indirectGlossy.c[2] += src->indirectGlossy.c[2];
#endif
#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SPECULAR)
	dst->indirectSpecular.c[0] += src->indirectSpecular.c[0];
	dst->indirectSpecular.c[1] += src->indirectSpecular.c[1];
	dst->indirectSpecular.c[2] += src->indirectSpecular.c[2];
#endif
#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_SHADOW_MASK)
	dst->directShadowMask += src->directShadowMask;
#endif
#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SHADOW_MASK)
	dst->indirectShadowMask += src->indirectShadowMask;
#endif

	bool depthWrite = true;
#if defined(PARAM_FILM_CHANNELS_HAS_DEPTH)
	const float srcDepthValue = src->depth;
	if (srcDepthValue <= dst->depth)
		dst->depth = srcDepthValue;
	else
		depthWrite = false;
#endif
	if (depthWrite) {
#if defined(PARAM_FILM_CHANNELS_HAS_POSITION)
		dst->position = src->position;
#endif
#if defined(PARAM_FILM_CHANNELS_HAS_GEOMETRY_NORMAL)
		dst->geometryNormal = src->geometryNormal;
#endif
#if defined(PARAM_FILM_CHANNELS_HAS_SHADING_NORMAL)
		dst->shadingNormal = src->shadingNormal;
#endif
#if defined(PARAM_FILM_CHANNELS_HAS_MATERIAL_ID)
		// Note: MATERIAL_ID_MASK and BY_MATERIAL_ID are calculated starting from materialID field
		dst->materialID = src->materialID;
#endif
#if defined(PARAM_FILM_CHANNELS_HAS_UV)
		dst->uv = src->uv;
#endif
	}

#if defined(PARAM_FILM_CHANNELS_HAS_RAYCOUNT)
	dst->rayCount += src->rayCount;
#endif
}

void SR_Normalize(SampleResult *dst, const float k) {
#if defined(PARAM_FILM_RADIANCE_GROUP_0)
	dst->radiancePerPixelNormalized[0].c[0] *= k;
	dst->radiancePerPixelNormalized[0].c[1] *= k;
	dst->radiancePerPixelNormalized[0].c[2] *= k;
#endif
#if defined(PARAM_FILM_RADIANCE_GROUP_1)
	dst->radiancePerPixelNormalized[1].c[0] *= k;
	dst->radiancePerPixelNormalized[1].c[1] *= k;
	dst->radiancePerPixelNormalized[1].c[2] *= k;
#endif
#if defined(PARAM_FILM_RADIANCE_GROUP_2)
	dst->radiancePerPixelNormalized[2].c[0] *= k;
	dst->radiancePerPixelNormalized[2].c[1] *= k;
	dst->radiancePerPixelNormalized[2].c[2] *= k;
#endif
#if defined(PARAM_FILM_RADIANCE_GROUP_3)
	dst->radiancePerPixelNormalized[3].c[0] *= k;
	dst->radiancePerPixelNormalized[3].c[1] *= k;
	dst->radiancePerPixelNormalized[3].c[2] *= k;
#endif
#if defined(PARAM_FILM_RADIANCE_GROUP_4)
	dst->radiancePerPixelNormalized[4].c[0] *= k;
	dst->radiancePerPixelNormalized[4].c[1] *= k;
	dst->radiancePerPixelNormalized[4].c[2] *= k;
#endif
#if defined(PARAM_FILM_RADIANCE_GROUP_5)
	dst->radiancePerPixelNormalized[5].c[0] *= k;
	dst->radiancePerPixelNormalized[5].c[1] *= k;
	dst->radiancePerPixelNormalized[5].c[2] *= k;
#endif
#if defined(PARAM_FILM_RADIANCE_GROUP_6)
	dst->radiancePerPixelNormalized[6].c[0] *= k;
	dst->radiancePerPixelNormalized[6].c[1] *= k;
	dst->radiancePerPixelNormalized[6].c[2] *= k;
#endif
#if defined(PARAM_FILM_RADIANCE_GROUP_7)
	dst->radiancePerPixelNormalized[7].c[0] *= k;
	dst->radiancePerPixelNormalized[7].c[1] *= k;
	dst->radiancePerPixelNormalized[7].c[2] *= k;
#endif
#if defined(PARAM_FILM_CHANNELS_HAS_ALPHA)
	dst->alpha *= k;
#endif

#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_DIFFUSE)
	dst->directDiffuse.c[0] *= k;
	dst->directDiffuse.c[1] *= k;
	dst->directDiffuse.c[2] *= k;
#endif
#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_GLOSSY)
	dst->directGlossy.c[0] *= k;
	dst->directGlossy.c[1] *= k;
	dst->directGlossy.c[2] *= k;
#endif
#if defined(PARAM_FILM_CHANNELS_HAS_EMISSION)
	dst->emission.c[0] *= k;
	dst->emission.c[1] *= k;
	dst->emission.c[2] *= k;
#endif
#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_DIFFUSE)
	dst->indirectDiffuse.c[0] *= k;
	dst->indirectDiffuse.c[1] *= k;
	dst->indirectDiffuse.c[2] *= k;
#endif
#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_GLOSSY)
	dst->indirectGlossy.c[0] *= k;
	dst->indirectGlossy.c[1] *= k;
	dst->indirectGlossy.c[2] *= k;
#endif
#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SPECULAR)
	dst->indirectSpecular.c[0] *= k;
	dst->indirectSpecular.c[1] *= k;
	dst->indirectSpecular.c[2] *= k;
#endif
#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_SHADOW_MASK)
	dst->directShadowMask *= k;
#endif
#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SHADOW_MASK)
	dst->indirectShadowMask *= k;
#endif
}

void SampleGrid(Seed *seed, const uint size,
		const uint ix, const uint iy, float *u0, float *u1) {
	*u0 = Rnd_FloatValue(seed);
	*u1 = Rnd_FloatValue(seed);

	if (size > 1) {
		const float idim = 1.f / size;
		*u0 = (ix + *u0) * idim;
		*u1 = (iy + *u1) * idim;
	}
}

typedef struct {
	uint depth, diffuseDepth, glossyDepth, specularDepth;
} PathDepthInfo;

void PathDepthInfo_Init(PathDepthInfo *depthInfo) {
	depthInfo->depth = 0;
	depthInfo->diffuseDepth = 0;
	depthInfo->glossyDepth = 0;
	depthInfo->specularDepth = 0;
}

void PathDepthInfo_IncDepths(PathDepthInfo *depthInfo, const BSDFEvent event) {
	++(depthInfo->depth);
	if (event & DIFFUSE)
		++(depthInfo->diffuseDepth);
	if (event & GLOSSY)
		++(depthInfo->glossyDepth);
	if (event & SPECULAR)
		++(depthInfo->specularDepth);
}

bool PathDepthInfo_CheckDepths(const PathDepthInfo *depthInfo) {
	return ((depthInfo->depth <= PARAM_DEPTH_MAX) &&
			(depthInfo->diffuseDepth <= PARAM_DEPTH_DIFFUSE_MAX) &&
			(depthInfo->glossyDepth <= PARAM_DEPTH_GLOSSY_MAX) &&
			(depthInfo->specularDepth <= PARAM_DEPTH_SPECULAR_MAX));
}

bool PathDepthInfo_CheckComponentDepths(const BSDFEvent component) {
	return ((component & DIFFUSE) && (PARAM_DEPTH_DIFFUSE_MAX > 0)) ||
			((component & GLOSSY) && (PARAM_DEPTH_GLOSSY_MAX > 0)) ||
			((component & SPECULAR) && (PARAM_DEPTH_SPECULAR_MAX > 0));
}

//------------------------------------------------------------------------------

void GenerateCameraRay(
		Seed *seed,
		__global GPUTask *task,
		__global SampleResult *sampleResult,
		__global Camera *camera,
		__global float *pixelFilterDistribution,
		const uint sampleX, const uint sampleY, const int sampleIndex,
		const uint tileStartX, const uint tileStartY, 
		const uint engineFilmWidth, const uint engineFilmHeight,
		Ray *ray) {
	//if (get_global_id(0) == 0)
	//	printf("tileSampleIndex: %d (%d, %d)\n", sampleIndex, sampleIndex % PARAM_AA_SAMPLES, sampleIndex / PARAM_AA_SAMPLES);

	float u0, u1;
	SampleGrid(seed, PARAM_AA_SAMPLES,
			sampleIndex % PARAM_AA_SAMPLES, sampleIndex / PARAM_AA_SAMPLES,
			&u0, &u1);

	float2 xy;
	float distPdf;
	Distribution2D_SampleContinuous(pixelFilterDistribution, u0, u1, &xy, &distPdf);

	const float filmX = sampleX + .5f + (xy.x - .5f) * PARAM_IMAGE_FILTER_WIDTH_X;
	const float filmY = sampleY + .5f + (xy.y - .5f) * PARAM_IMAGE_FILTER_WIDTH_Y;
	sampleResult->filmX = filmX;
	sampleResult->filmY = filmY;

#if defined(PARAM_CAMERA_HAS_DOF)
	const float dofSampleX = Rnd_FloatValue(seed);
	const float dofSampleY = Rnd_FloatValue(seed);
#endif

	Camera_GenerateRay(camera, engineFilmWidth, engineFilmHeight, ray, tileStartX + filmX, tileStartY + filmY
#if defined(PARAM_CAMERA_HAS_DOF)
			, dofSampleX, dofSampleY
#endif
			);	
}

//------------------------------------------------------------------------------

uint BIASPATHOCL_Scene_Intersect(
#if defined(PARAM_HAS_VOLUMES)
		__global PathVolumeInfo *volInfo,
		__global HitPoint *tmpHitPoint,
#endif
#if defined(PARAM_HAS_PASSTHROUGH)
		const float passThrough,
#endif
#if !defined(RENDER_ENGINE_BIASPATHOCL)
		__global
#endif
		Ray *ray,
#if !defined(RENDER_ENGINE_BIASPATHOCL)
		__global
#endif
		RayHit *rayHit,
		__global BSDF *bsdf,
		float3 *pathThroughput,
		__global SampleResult *sampleResult,
		// BSDF_Init parameters
		__global Mesh *meshDescs,
		__global uint *meshMats,
#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)
		__global uint *meshTriLightDefsOffset,
#endif
		__global Point *vertices,
#if defined(PARAM_HAS_NORMALS_BUFFER)
		__global Vector *vertNormals,
#endif
#if defined(PARAM_HAS_UVS_BUFFER)
		__global UV *vertUVs,
#endif
#if defined(PARAM_HAS_COLS_BUFFER)
		__global Spectrum *vertCols,
#endif
#if defined(PARAM_HAS_ALPHAS_BUFFER)
		__global float *vertAlphas,
#endif
		__global Triangle *triangles
		MATERIALS_PARAM_DECL
		// Accelerator_Intersect parameters
		ACCELERATOR_INTERSECT_PARAM_DECL
		) {
	uint tracedRaysCount = 0;

	do {
		Accelerator_Intersect(ray, rayHit
			ACCELERATOR_INTERSECT_PARAM);
		++tracedRaysCount;
	} while(
		Scene_Intersect(
#if defined(PARAM_HAS_VOLUMES)
			volInfo,
			tmpHitPoint,
#endif
#if defined(PARAM_HAS_PASSTHROUGH)
			passThrough,
#endif
			ray, rayHit, bsdf,
			pathThroughput,
			sampleResult,
			// BSDF_Init parameters
			meshDescs,
			meshMats,
#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)
			meshTriLightDefsOffset,
#endif
			vertices,
#if defined(PARAM_HAS_NORMALS_BUFFER)
			vertNormals,
#endif
#if defined(PARAM_HAS_UVS_BUFFER)
			vertUVs,
#endif
#if defined(PARAM_HAS_COLS_BUFFER)
			vertCols,
#endif
#if defined(PARAM_HAS_ALPHAS_BUFFER)
			vertAlphas,
#endif
			triangles
			MATERIALS_PARAM
			)
		);

	return tracedRaysCount;
}

//------------------------------------------------------------------------------
// Direct light sampling
//------------------------------------------------------------------------------

#if defined(PARAM_HAS_ENVLIGHTS)
void DirectHitInfiniteLight(
		const BSDFEvent lastBSDFEvent,
		const float3 pathThroughput,
		const float3 eyeDir, const float lastPdfW,
		__global SampleResult *sampleResult
		LIGHTS_PARAM_DECL) {
	for (uint i = 0; i < envLightCount; ++i) {
		__global LightSource *light = &lights[envLightIndices[i]];

		if (sampleResult->firstPathVertex || (light->visibility & (sampleResult->firstPathVertexEvent & (DIFFUSE | GLOSSY | SPECULAR)))) {
			float directPdfW;
			const float3 lightRadiance = EnvLight_GetRadiance(light, eyeDir, &directPdfW
					LIGHTS_PARAM);

			if (!Spectrum_IsBlack(lightRadiance)) {
				// MIS between BSDF sampling and direct light sampling
				const float lightPickProb = Scene_SampleAllLightPdf(lightsDistribution, light->lightSceneIndex);
				const float weight = ((lastBSDFEvent & SPECULAR) ? 1.f : PowerHeuristic(lastPdfW, directPdfW * lightPickProb));
				const float3 radiance = weight * pathThroughput * lightRadiance;

				SampleResult_AddEmission(sampleResult, light->lightID, radiance);
			}
		}
	}
}
#endif

#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)
void DirectHitFiniteLight(
		const BSDFEvent lastBSDFEvent,
		const float3 pathThroughput, const float distance, __global BSDF *bsdf,
		const float lastPdfW, __global SampleResult *sampleResult
		LIGHTS_PARAM_DECL) {
	if (sampleResult->firstPathVertex || (lights[bsdf->triangleLightSourceIndex].visibility & (sampleResult->firstPathVertexEvent & (DIFFUSE | GLOSSY | SPECULAR)))) {
		float directPdfA;
		const float3 emittedRadiance = BSDF_GetEmittedRadiance(bsdf, &directPdfA
				LIGHTS_PARAM);

		if (!Spectrum_IsBlack(emittedRadiance)) {
			// Add emitted radiance
			float weight = 1.f;
			if (!(lastBSDFEvent & SPECULAR)) {
				const float lightPickProb = Scene_SampleAllLightPdf(lightsDistribution,
						lights[bsdf->triangleLightSourceIndex].lightSceneIndex);
				const float directPdfW = PdfAtoW(directPdfA, distance,
					fabs(dot(VLOAD3F(&bsdf->hitPoint.fixedDir.x), VLOAD3F(&bsdf->hitPoint.shadeN.x))));

				// MIS between BSDF sampling and direct light sampling
				weight = PowerHeuristic(lastPdfW, directPdfW * lightPickProb);
			}
			const float3 lightRadiance = weight * pathThroughput * emittedRadiance;

			SampleResult_AddEmission(sampleResult, BSDF_GetLightID(bsdf
					MATERIALS_PARAM), lightRadiance);
		}
	}
}
#endif

bool DirectLightSamplingInit(
		__global LightSource *light,
		const float lightPickPdf,
#if defined(PARAM_HAS_INFINITELIGHTS)
		const float worldCenterX,
		const float worldCenterY,
		const float worldCenterZ,
		const float worldRadius,
#endif
#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)
		__global HitPoint *tmpHitPoint,
#endif
		const float u0, const float u1,
#if defined(PARAM_HAS_PASSTHROUGH)
		const float passThroughEvent,
#endif
		const float3 pathThroughput, __global BSDF *bsdf, BSDFEvent *event,
		Ray *shadowRay, float3 *radiance, uint *ID
		LIGHTS_PARAM_DECL) {
	float3 lightRayDir;
	float distance, directPdfW;
	const float3 lightRadiance = Light_Illuminate(
			light,
			VLOAD3F(&bsdf->hitPoint.p.x),
			u0, u1,
#if defined(PARAM_HAS_PASSTHROUGH)
			passThroughEvent,
#endif
#if defined(PARAM_HAS_INFINITELIGHTS)
			worldCenterX, worldCenterY, worldCenterZ, worldRadius,
#endif
#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)
			tmpHitPoint,
#endif		
			&lightRayDir, &distance, &directPdfW
			LIGHTS_PARAM);

	// Setup the shadow ray
	const float cosThetaToLight = fabs(dot(lightRayDir, VLOAD3F(&bsdf->hitPoint.shadeN.x)));
	if (((Spectrum_Y(lightRadiance) * cosThetaToLight / directPdfW) > PARAM_LOW_LIGHT_THREASHOLD) &&
			(distance > PARAM_NEAR_START_LIGHT)) {
		float bsdfPdfW;
		const float3 bsdfEval = BSDF_Evaluate(bsdf,
				lightRayDir, event, &bsdfPdfW
				MATERIALS_PARAM);

		if (!Spectrum_IsBlack(bsdfEval)) {
			const float directLightSamplingPdfW = directPdfW * lightPickPdf;
			const float factor = 1.f / directLightSamplingPdfW;

			// MIS between direct light sampling and BSDF sampling
			//
			// Note: applying MIS to the direct light of the last path vertex (when we
			// hit the max. path depth) is not correct because the light source
			// can be sampled only here and not with the next path vertex. The
			// value of weight should be 1 in that case. However, because of different
			// max. depths for diffuse, glossy and specular, I can not really know
			// if I'm on the last path vertex or not.
			//
			// For the moment, I'm just ignoring this error.
			const float weight = Light_IsEnvOrIntersectable(light) ?
				PowerHeuristic(directLightSamplingPdfW, bsdfPdfW) : 1.f;

			*radiance = (weight * factor) * pathThroughput * bsdfEval * lightRadiance;
			*ID = light->lightID;

			// Setup the shadow ray
			const float3 hitPoint = VLOAD3F(&bsdf->hitPoint.p.x);
			const float epsilon = fmax(MachineEpsilon_E_Float3(hitPoint), MachineEpsilon_E(distance));

			Ray_Init4_Private(shadowRay, hitPoint, lightRayDir,
				epsilon,
				distance - epsilon);

			return true;
		}
	}

	return false;
}

uint DirectLightSampling_ONE(
		Seed *seed,
#if defined(PARAM_HAS_VOLUMES)
		__global PathVolumeInfo *volInfo,
#endif
#if (PARAM_TRIANGLE_LIGHT_COUNT > 0) || defined(PARAM_HAS_VOLUMES)
		__global HitPoint *tmpHitPoint,
#endif
#if defined(PARAM_HAS_INFINITELIGHTS)
		const float worldCenterX,
		const float worldCenterY,
		const float worldCenterZ,
		const float worldRadius,
#endif
		const float3 pathThroughput,
		__global BSDF *bsdf, __global BSDF *directLightBSDF,
		__global SampleResult *sampleResult,
		// BSDF_Init parameters
		__global Mesh *meshDescs,
		__global uint *meshMats,
		__global Point *vertices,
#if defined(PARAM_HAS_NORMALS_BUFFER)
		__global Vector *vertNormals,
#endif
#if defined(PARAM_HAS_UVS_BUFFER)
		__global UV *vertUVs,
#endif
#if defined(PARAM_HAS_COLS_BUFFER)
		__global Spectrum *vertCols,
#endif
#if defined(PARAM_HAS_ALPHAS_BUFFER)
		__global float *vertAlphas,
#endif
		__global Triangle *triangles
		// Accelerator_Intersect parameters
		ACCELERATOR_INTERSECT_PARAM_DECL
		// Light related parameters
		LIGHTS_PARAM_DECL) {
	// ONE direct light sampling strategy

	// Pick a light source to sample
	float lightPickPdf;
	const uint lightIndex = Scene_SampleAllLights(lightsDistribution, Rnd_FloatValue(seed), &lightPickPdf);

	Ray shadowRay;
	uint lightID;
	BSDFEvent event;
	float3 lightRadiance;
	const bool illuminated = DirectLightSamplingInit(
		&lights[lightIndex],
		lightPickPdf,
#if defined(PARAM_HAS_INFINITELIGHTS)
		worldCenterX,
		worldCenterY,
		worldCenterZ,
		worldRadius,
#endif
#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)
		tmpHitPoint,
#endif
		Rnd_FloatValue(seed), Rnd_FloatValue(seed),
#if defined(PARAM_HAS_PASSTHROUGH)
		Rnd_FloatValue(seed),
#endif
		pathThroughput, bsdf, &event,
		&shadowRay, &lightRadiance, &lightID
		LIGHTS_PARAM);

	uint tracedRaysCount = 0;
	if (illuminated) {
		// Trace the shadow ray

		float3 directLightThroughput = WHITE;
		RayHit shadowRayHit;
		tracedRaysCount += BIASPATHOCL_Scene_Intersect(
#if defined(PARAM_HAS_VOLUMES)
				volInfo,
				tmpHitPoint,
#endif
#if defined(PARAM_HAS_PASSTHROUGH)
				Rnd_FloatValue(seed),
#endif
				&shadowRay, &shadowRayHit,
				directLightBSDF,
				&directLightThroughput,
				sampleResult,
				// BSDF_Init parameters
				meshDescs,
				meshMats,
#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)
				meshTriLightDefsOffset,
#endif
				vertices,
#if defined(PARAM_HAS_NORMALS_BUFFER)
				vertNormals,
#endif
#if defined(PARAM_HAS_UVS_BUFFER)
				vertUVs,
#endif
#if defined(PARAM_HAS_COLS_BUFFER)
				vertCols,
#endif
#if defined(PARAM_HAS_ALPHAS_BUFFER)
				vertAlphas,
#endif
				triangles
				MATERIALS_PARAM
				// Accelerator_Intersect parameters
				ACCELERATOR_INTERSECT_PARAM
				);

		if (shadowRayHit.meshIndex == NULL_INDEX) {
			// Nothing was hit, the light source is visible
			SampleResult_AddDirectLight(sampleResult, lightID, event, directLightThroughput * lightRadiance, 1.f);
		}
	}
	
	return tracedRaysCount;
}

#if defined(PARAM_DIRECT_LIGHT_ALL_STRATEGY)
uint DirectLightSampling_ALL(
		Seed *seed,
#if defined(PARAM_HAS_VOLUMES)
		__global PathVolumeInfo *volInfo,
#endif
#if (PARAM_TRIANGLE_LIGHT_COUNT > 0) || defined(PARAM_HAS_VOLUMES)
		__global HitPoint *tmpHitPoint,
#endif
#if defined(PARAM_HAS_INFINITELIGHTS)
		const float worldCenterX,
		const float worldCenterY,
		const float worldCenterZ,
		const float worldRadius,
#endif
		const float3 pathThroughput,
		__global BSDF *bsdf, __global BSDF *directLightBSDF,
		__global SampleResult *sampleResult,
		// BSDF_Init parameters
		__global Mesh *meshDescs,
		__global uint *meshMats,
		__global Point *vertices,
#if defined(PARAM_HAS_NORMALS_BUFFER)
		__global Vector *vertNormals,
#endif
#if defined(PARAM_HAS_UVS_BUFFER)
		__global UV *vertUVs,
#endif
#if defined(PARAM_HAS_COLS_BUFFER)
		__global Spectrum *vertCols,
#endif
#if defined(PARAM_HAS_ALPHAS_BUFFER)
		__global float *vertAlphas,
#endif
		__global Triangle *triangles
		// Accelerator_Intersect parameters
		ACCELERATOR_INTERSECT_PARAM_DECL
		// Light related parameters
		LIGHTS_PARAM_DECL) {
	uint tracedRaysCount = 0;

	for (uint currentLightIndex = 0; currentLightIndex < PARAM_LIGHT_COUNT; ++currentLightIndex) {
		const int lightSamplesCount = lights[currentLightIndex].samples;
		const uint sampleCount = (lightSamplesCount < 0) ? PARAM_DIRECT_LIGHT_SAMPLES : (uint)lightSamplesCount;
		const uint sampleCount2 = sampleCount * sampleCount;

		const float scaleFactor = 1.f / sampleCount2;
		for (uint currentLightSampleIndex = 0; currentLightSampleIndex < sampleCount2; ++currentLightSampleIndex) {
			float u0, u1;
			SampleGrid(seed, sampleCount,
					currentLightSampleIndex % sampleCount, currentLightSampleIndex / sampleCount,
					&u0, &u1);

			Ray shadowRay;
			float3 lightRadiance;
			uint lightID;
			BSDFEvent event;
			const bool illuminated = DirectLightSamplingInit(
				&lights[currentLightIndex],
				1.f,
#if defined(PARAM_HAS_INFINITELIGHTS)
				worldCenterX,
				worldCenterY,
				worldCenterZ,
				worldRadius,
#endif
#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)
				tmpHitPoint,
#endif
				u0, u1,
#if defined(PARAM_HAS_PASSTHROUGH)
				Rnd_FloatValue(seed),
#endif
				pathThroughput, bsdf, &event,
				&shadowRay, &lightRadiance, &lightID
				LIGHTS_PARAM);

			if (illuminated) {
				// Trace the shadow ray

				float3 directLightThroughput = WHITE;
				RayHit shadowRayHit;
				tracedRaysCount += BIASPATHOCL_Scene_Intersect(
#if defined(PARAM_HAS_VOLUMES)
						volInfo,
						tmpHitPoint,
#endif
#if defined(PARAM_HAS_PASSTHROUGH)
						Rnd_FloatValue(seed),
#endif
						&shadowRay, &shadowRayHit,
						directLightBSDF,
						&directLightThroughput,
						sampleResult,
						// BSDF_Init parameters
						meshDescs,
						meshMats,
#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)
						meshTriLightDefsOffset,
#endif
						vertices,
#if defined(PARAM_HAS_NORMALS_BUFFER)
						vertNormals,
#endif
#if defined(PARAM_HAS_UVS_BUFFER)
						vertUVs,
#endif
#if defined(PARAM_HAS_COLS_BUFFER)
						vertCols,
#endif
#if defined(PARAM_HAS_ALPHAS_BUFFER)
						vertAlphas,
#endif
						triangles
						MATERIALS_PARAM
						// Accelerator_Intersect parameters
						ACCELERATOR_INTERSECT_PARAM
						);

				if (shadowRayHit.meshIndex == NULL_INDEX) {
					// Nothing was hit, the light source is visible
					SampleResult_AddDirectLight(sampleResult, lightID, event,
							scaleFactor * directLightThroughput * lightRadiance, scaleFactor);
				}
			}
		}
	}

	return tracedRaysCount;
}
#endif

//------------------------------------------------------------------------------
// Indirect light sampling
//------------------------------------------------------------------------------

uint ContinueTracePath(
		Seed *seed,
#if defined(PARAM_HAS_VOLUMES)
		__global PathVolumeInfo *volInfoPathVertexN,
		__global PathVolumeInfo *directLightVolInfo,
#endif
#if (PARAM_TRIANGLE_LIGHT_COUNT > 0) || defined(PARAM_HAS_VOLUMES)
		__global HitPoint *tmpHitPoint,
#endif
#if defined(PARAM_HAS_INFINITELIGHTS)
		const float worldCenterX,
		const float worldCenterY,
		const float worldCenterZ,
		const float worldRadius,
#endif
		PathDepthInfo *depthInfo,
		Ray *ray,
		float3 pathThroughput,
		BSDFEvent lastBSDFEvent, float lastPdfW,
		__global BSDF *bsdfPathVertexN, __global BSDF *directLightBSDF,
		__global SampleResult *sampleResult,
		// BSDF_Init parameters
		__global Mesh *meshDescs,
		__global uint *meshMats,
		__global Point *vertices,
#if defined(PARAM_HAS_NORMALS_BUFFER)
		__global Vector *vertNormals,
#endif
#if defined(PARAM_HAS_UVS_BUFFER)
		__global UV *vertUVs,
#endif
#if defined(PARAM_HAS_COLS_BUFFER)
		__global Spectrum *vertCols,
#endif
#if defined(PARAM_HAS_ALPHAS_BUFFER)
		__global float *vertAlphas,
#endif
		__global Triangle *triangles
		// Accelerator_Intersect parameters
		ACCELERATOR_INTERSECT_PARAM_DECL
		// Light related parameters
		LIGHTS_PARAM_DECL) {
	uint tracedRaysCount = 0;

	for (;;) {
		//----------------------------------------------------------------------
		// Trace the ray
		//----------------------------------------------------------------------

		RayHit rayHit;
		tracedRaysCount += BIASPATHOCL_Scene_Intersect(
#if defined(PARAM_HAS_VOLUMES)
			volInfoPathVertexN,
			tmpHitPoint,
#endif
#if defined(PARAM_HAS_PASSTHROUGH)
			Rnd_FloatValue(seed),
#endif
			ray, &rayHit,
			bsdfPathVertexN,
			&pathThroughput,
			sampleResult,
			// BSDF_Init parameters
			meshDescs,
			meshMats,
#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)
			meshTriLightDefsOffset,
#endif
			vertices,
#if defined(PARAM_HAS_NORMALS_BUFFER)
			vertNormals,
#endif
#if defined(PARAM_HAS_UVS_BUFFER)
			vertUVs,
#endif
#if defined(PARAM_HAS_COLS_BUFFER)
			vertCols,
#endif
#if defined(PARAM_HAS_ALPHAS_BUFFER)
			vertAlphas,
#endif
			triangles
			MATERIALS_PARAM
			// Accelerator_Intersect parameters
			ACCELERATOR_INTERSECT_PARAM);
		
		if (rayHit.meshIndex == NULL_INDEX) {
			// Nothing was hit, look for env. lights

#if defined(PARAM_HAS_ENVLIGHTS)
			// Add environmental lights radiance
			const float3 rayDir = (float3)(ray->d.x, ray->d.y, ray->d.z);
			DirectHitInfiniteLight(
					lastBSDFEvent,
					pathThroughput,
					-rayDir, lastPdfW,
					sampleResult
					LIGHTS_PARAM);
#endif

			break;
		}

		// Something was hit

		// Check if it is visible in indirect paths
		if (!(mats[bsdfPathVertexN->materialIndex].visibility &
				(sampleResult->firstPathVertexEvent & (DIFFUSE | GLOSSY | SPECULAR))))
			break;

#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)
		// Check if it is a light source (note: I can hit only triangle area light sources)
		if (BSDF_IsLightSource(bsdfPathVertexN) && (rayHit.t > PARAM_NEAR_START_LIGHT)) {
			DirectHitFiniteLight(lastBSDFEvent,
					pathThroughput,
					rayHit.t, bsdfPathVertexN, lastPdfW,
					sampleResult
					LIGHTS_PARAM);
		}
#endif

		// Check the path depth
		// NOTE: before direct Light sampling in order to have a correct MIS
		if (!PathDepthInfo_CheckDepths(depthInfo))
			break;

		//------------------------------------------------------------------
		// Direct light sampling
		//------------------------------------------------------------------

		// Only if it is not a SPECULAR BSDF
		if (!BSDF_IsDelta(bsdfPathVertexN
				MATERIALS_PARAM)) {
#if defined(PARAM_HAS_VOLUMES)
			// I need to work on a copy of volume information of the path vertex
			*directLightVolInfo = *volInfoPathVertexN;
#endif

			tracedRaysCount += DirectLightSampling_ONE(
				seed,
#if defined(PARAM_HAS_VOLUMES)
				directLightVolInfo,
#endif
#if (PARAM_TRIANGLE_LIGHT_COUNT > 0) || defined(PARAM_HAS_VOLUMES)
				tmpHitPoint,
#endif
#if defined(PARAM_HAS_INFINITELIGHTS)
				worldCenterX, worldCenterY, worldCenterZ, worldRadius,
#endif
				pathThroughput,
				bsdfPathVertexN, directLightBSDF,
				sampleResult,
				// BSDF_Init parameters
				meshDescs,
				meshMats,
				vertices,
#if defined(PARAM_HAS_NORMALS_BUFFER)
				vertNormals,
#endif
#if defined(PARAM_HAS_UVS_BUFFER)
				vertUVs,
#endif
#if defined(PARAM_HAS_COLS_BUFFER)
				vertCols,
#endif
#if defined(PARAM_HAS_ALPHAS_BUFFER)
				vertAlphas,
#endif
				triangles
				// Accelerator_Intersect parameters
				ACCELERATOR_INTERSECT_PARAM
				// Light related parameters
				LIGHTS_PARAM);
		}

		//------------------------------------------------------------------
		// Build the next path vertex ray
		//------------------------------------------------------------------

		float3 sampledDir;
		float cosSampledDir;

		const float3 bsdfSample = BSDF_Sample(bsdfPathVertexN,
			Rnd_FloatValue(seed),
			Rnd_FloatValue(seed),
			&sampledDir, &lastPdfW, &cosSampledDir, &lastBSDFEvent,
			ALL
			MATERIALS_PARAM);

		if (Spectrum_IsBlack(bsdfSample))
			break;

		// Increment path depth informations
		PathDepthInfo_IncDepths(depthInfo, lastBSDFEvent);

		// Update volume information
#if defined(PARAM_HAS_VOLUMES)
		PathVolumeInfo_Update(volInfoPathVertexN, lastBSDFEvent, bsdfPathVertexN
				MATERIALS_PARAM);
#endif

		// Continue to trace the path
		pathThroughput *= bsdfSample *
			((lastBSDFEvent & SPECULAR) ? 1.f : min(1.f, lastPdfW / PARAM_PDF_CLAMP_VALUE));

		Ray_Init2_Private(ray, VLOAD3F(&bsdfPathVertexN->hitPoint.p.x), sampledDir);
	}

	return tracedRaysCount;
}

uint SampleComponent(
		Seed *seed,
#if defined(PARAM_HAS_VOLUMES)
		__global PathVolumeInfo *volInfoPathVertex1,
		__global PathVolumeInfo *volInfoPathVertexN,
		__global PathVolumeInfo *directLightVolInfo,
#endif
#if (PARAM_TRIANGLE_LIGHT_COUNT > 0) || defined(PARAM_HAS_VOLUMES)
		__global HitPoint *tmpHitPoint,
#endif
#if defined(PARAM_HAS_INFINITELIGHTS)
		const float worldCenterX,
		const float worldCenterY,
		const float worldCenterZ,
		const float worldRadius,
#endif
		const BSDFEvent requestedEventTypes,
		const uint size,
		const float3 throughputPathVertex1,
		__global BSDF *bsdfPathVertex1, __global BSDF *bsdfPathVertexN,
		__global BSDF *directLightBSDF,
		__global SampleResult *sampleResult,
		// BSDF_Init parameters
		__global Mesh *meshDescs,
		__global uint *meshMats,
		__global Point *vertices,
#if defined(PARAM_HAS_NORMALS_BUFFER)
		__global Vector *vertNormals,
#endif
#if defined(PARAM_HAS_UVS_BUFFER)
		__global UV *vertUVs,
#endif
#if defined(PARAM_HAS_COLS_BUFFER)
		__global Spectrum *vertCols,
#endif
#if defined(PARAM_HAS_ALPHAS_BUFFER)
		__global float *vertAlphas,
#endif
		__global Triangle *triangles
		// Accelerator_Intersect parameters
		ACCELERATOR_INTERSECT_PARAM_DECL
		// Light related parameters
		LIGHTS_PARAM_DECL) {
	uint tracedRaysCount = 0;

	const uint sampleCount = size * size;
	const float scaleFactor = 1.f / sampleCount;
#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SHADOW_MASK)
	float indirectShadowMask = 0.f;
#endif
	for (uint currentBSDFSampleIndex = 0; currentBSDFSampleIndex < sampleCount; ++currentBSDFSampleIndex) {
#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SHADOW_MASK)
		sampleResult->indirectShadowMask = 1.f;
#endif

		float u0, u1;
		SampleGrid(seed, size,
			currentBSDFSampleIndex % size, currentBSDFSampleIndex / size,
			&u0, &u1);

		// Sample the BSDF on the first path vertex
		float3 sampledDir;
		float pdfW, cosSampledDir;
		BSDFEvent event;

		const float3 bsdfSample = BSDF_Sample(bsdfPathVertex1,
			u0,
			u1,
			&sampledDir, &pdfW, &cosSampledDir, &event,
			requestedEventTypes | REFLECT | TRANSMIT
			MATERIALS_PARAM);

		if (!Spectrum_IsBlack(bsdfSample)) {
			PathDepthInfo depthInfo;
			PathDepthInfo_Init(&depthInfo);
			PathDepthInfo_IncDepths(&depthInfo, event);

			// Update information about the first path BSDF 
			sampleResult->firstPathVertexEvent = event;

			// Update volume information
#if defined(PARAM_HAS_VOLUMES)
			// I need to work on a copy of volume information of the first path vertex
			*volInfoPathVertexN = *volInfoPathVertex1;
			PathVolumeInfo_Update(volInfoPathVertexN, event, bsdfPathVertexN
					MATERIALS_PARAM);
#endif

			// Continue to trace the path
			const float3 continuePathThroughput = throughputPathVertex1 * bsdfSample *
				(scaleFactor * ((event & SPECULAR) ? 1.f : min(1.f, pdfW / PARAM_PDF_CLAMP_VALUE)));

			Ray continueRay;
			Ray_Init2_Private(&continueRay, VLOAD3F(&bsdfPathVertex1->hitPoint.p.x), sampledDir);

			tracedRaysCount += ContinueTracePath(
					seed,
#if defined(PARAM_HAS_VOLUMES)
					volInfoPathVertexN,
					directLightVolInfo,
#endif
#if (PARAM_TRIANGLE_LIGHT_COUNT > 0) || defined(PARAM_HAS_VOLUMES)
					tmpHitPoint,
#endif
#if defined(PARAM_HAS_INFINITELIGHTS)
					worldCenterX, worldCenterY, worldCenterZ, worldRadius,
#endif
					&depthInfo, &continueRay,
					continuePathThroughput,
					event, pdfW,
					bsdfPathVertexN, directLightBSDF,
					sampleResult,
					// BSDF_Init parameters
					meshDescs,
					meshMats,
					vertices,
#if defined(PARAM_HAS_NORMALS_BUFFER)
					vertNormals,
#endif
#if defined(PARAM_HAS_UVS_BUFFER)
					vertUVs,
#endif
#if defined(PARAM_HAS_COLS_BUFFER)
					vertCols,
#endif
#if defined(PARAM_HAS_ALPHAS_BUFFER)
					vertAlphas,
#endif
					triangles
					// Accelerator_Intersect parameters
					ACCELERATOR_INTERSECT_PARAM
					// Light related parameters
					LIGHTS_PARAM);
		}

#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SHADOW_MASK)
		// sampleResult->indirectShadowMask requires special handling: the
		// end result must be the average of all path results
		indirectShadowMask += scaleFactor * sampleResult->indirectShadowMask;
#endif
	}

#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SHADOW_MASK)
	sampleResult->indirectShadowMask = indirectShadowMask;
#endif

	return tracedRaysCount;
}
