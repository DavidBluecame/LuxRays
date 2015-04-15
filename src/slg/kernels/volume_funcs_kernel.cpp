#include <string>
namespace slg { namespace ocl {
std::string KernelSource_volume_funcs = 
"#line 2 \"volume_funcs.cl\"\n"
"\n"
"/***************************************************************************\n"
" * Copyright 1998-2015 by authors (see AUTHORS.txt)                        *\n"
" *                                                                         *\n"
" *   This file is part of LuxRender.                                       *\n"
" *                                                                         *\n"
" * Licensed under the Apache License, Version 2.0 (the \"License\");         *\n"
" * you may not use this file except in compliance with the License.        *\n"
" * You may obtain a copy of the License at                                 *\n"
" *                                                                         *\n"
" *     http://www.apache.org/licenses/LICENSE-2.0                          *\n"
" *                                                                         *\n"
" * Unless required by applicable law or agreed to in writing, software     *\n"
" * distributed under the License is distributed on an \"AS IS\" BASIS,       *\n"
" * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*\n"
" * See the License for the specific language governing permissions and     *\n"
" * limitations under the License.                                          *\n"
" ***************************************************************************/\n"
"\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// ClearVolume scatter\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined (PARAM_ENABLE_MAT_CLEAR_VOL)\n"
"float3 ClearVolume_SigmaA(__global const Volume *vol, __global HitPoint *hitPoint\n"
"	TEXTURES_PARAM_DECL) {\n"
"	const float3 sigmaA = Texture_GetSpectrumValue(vol->volume.clear.sigmaATexIndex, hitPoint\n"
"		TEXTURES_PARAM);\n"
"			\n"
"	return clamp(sigmaA, 0.f, INFINITY);\n"
"}\n"
"\n"
"float3 ClearVolume_SigmaS(__global const Volume *vol, __global HitPoint *hitPoint\n"
"	TEXTURES_PARAM_DECL) {\n"
"	return BLACK;\n"
"}\n"
"\n"
"float3 ClearVolume_SigmaT(__global const Volume *vol, __global HitPoint *hitPoint\n"
"	TEXTURES_PARAM_DECL) {\n"
"	return\n"
"			ClearVolume_SigmaA(vol, hitPoint\n"
"				TEXTURES_PARAM) +\n"
"			ClearVolume_SigmaS(vol, hitPoint\n"
"				TEXTURES_PARAM);\n"
"}\n"
"\n"
"float ClearVolume_Scatter(__global const Volume *vol,\n"
"#if !defined(RENDER_ENGINE_BIASPATHOCL) && !defined(RENDER_ENGINE_RTBIASPATHOCL)\n"
"		__global\n"
"#endif\n"
"		Ray *ray, const float hitT,\n"
"		const float passThroughEvent,\n"
"		const bool scatteredStart, float3 *connectionThroughput,\n"
"		float3 *connectionEmission, __global HitPoint *tmpHitPoint\n"
"		TEXTURES_PARAM_DECL) {\n"
"	// Initialize tmpHitPoint\n"
"#if !defined(RENDER_ENGINE_BIASPATHOCL) && !defined(RENDER_ENGINE_RTBIASPATHOCL)\n"
"	const float3 rayOrig = VLOAD3F(&ray->o.x);\n"
"	const float3 rayDir = VLOAD3F(&ray->d.x);\n"
"#else\n"
"	const float3 rayOrig = (float3)(ray->o.x, ray->o.y, ray->o.z);\n"
"	const float3 rayDir = (float3)(ray->d.x, ray->d.y, ray->d.z);\n"
"#endif\n"
"	VSTORE3F(rayDir, &tmpHitPoint->fixedDir.x);\n"
"	VSTORE3F(rayOrig, &tmpHitPoint->p.x);\n"
"	VSTORE2F((float2)(0.f, 0.f), &tmpHitPoint->uv.u);\n"
"	VSTORE3F(-rayDir, &tmpHitPoint->geometryN.x);\n"
"	VSTORE3F(-rayDir, &tmpHitPoint->shadeN.x);\n"
"#if defined(PARAM_ENABLE_TEX_HITPOINTCOLOR) || defined(PARAM_ENABLE_TEX_HITPOINTGREY) || defined(PARAM_TRIANGLE_LIGHT_HAS_VERTEX_COLOR)\n"
"	VSTORE3F(WHITE, tmpHitPoint->color.c);\n"
"#endif\n"
"#if defined(PARAM_ENABLE_TEX_HITPOINTALPHA)\n"
"	VSTORE2F(1.f, &tmpHitPoint->alpha);\n"
"#endif\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"	tmpHitPoint->passThroughEvent = passThroughEvent;\n"
"#endif\n"
"	tmpHitPoint->interiorVolumeIndex = NULL_INDEX;\n"
"	tmpHitPoint->exteriorVolumeIndex = NULL_INDEX;\n"
"	tmpHitPoint->intoObject = true;\n"
"\n"
"	const float distance = hitT - ray->mint;	\n"
"	float3 transmittance = WHITE;\n"
"\n"
"	const float3 sigmaT = ClearVolume_SigmaT(vol, tmpHitPoint\n"
"			TEXTURES_PARAM);\n"
"	if (!Spectrum_IsBlack(sigmaT)) {\n"
"		const float3 tau = clamp(distance * sigmaT, 0.f, INFINITY);\n"
"		transmittance = Spectrum_Exp(-tau);\n"
"	}\n"
"\n"
"	// Apply volume transmittance\n"
"	*connectionThroughput *= transmittance;\n"
"\n"
"	// Apply volume emission\n"
"	const uint emiTexIndex = vol->volume.volumeEmissionTexIndex;\n"
"	if (emiTexIndex != NULL_INDEX) {\n"
"		const float3 emiTex = Texture_GetSpectrumValue(emiTexIndex, tmpHitPoint\n"
"			TEXTURES_PARAM);\n"
"		*connectionEmission += *connectionThroughput * distance * clamp(emiTex, 0.f, INFINITY);\n"
"	}\n"
"\n"
"	return -1.f;\n"
"}\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// HomogeneousVolume scatter\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined (PARAM_ENABLE_MAT_HOMOGENEOUS_VOL)\n"
"float3 HomogeneousVolume_SigmaA(__global const Volume *vol, __global HitPoint *hitPoint\n"
"	TEXTURES_PARAM_DECL) {\n"
"	const float3 sigmaA = Texture_GetSpectrumValue(vol->volume.homogenous.sigmaATexIndex, hitPoint\n"
"		TEXTURES_PARAM);\n"
"			\n"
"	return clamp(sigmaA, 0.f, INFINITY);\n"
"}\n"
"\n"
"float3 HomogeneousVolume_SigmaS(__global const Volume *vol, __global HitPoint *hitPoint\n"
"	TEXTURES_PARAM_DECL) {\n"
"	const float3 sigmaS = Texture_GetSpectrumValue(vol->volume.homogenous.sigmaSTexIndex, hitPoint\n"
"		TEXTURES_PARAM);\n"
"			\n"
"	return clamp(sigmaS, 0.f, INFINITY);\n"
"}\n"
"\n"
"float HomogeneousVolume_Scatter(__global const Volume *vol,\n"
"#if !defined(RENDER_ENGINE_BIASPATHOCL) && !defined(RENDER_ENGINE_RTBIASPATHOCL)\n"
"		__global\n"
"#endif\n"
"		Ray *ray, const float hitT,\n"
"		const float passThroughEvent,\n"
"		const bool scatteredStart, float3 *connectionThroughput,\n"
"		float3 *connectionEmission, __global HitPoint *tmpHitPoint\n"
"		TEXTURES_PARAM_DECL) {\n"
"	// Initialize tmpHitPoint\n"
"#if !defined(RENDER_ENGINE_BIASPATHOCL) && !defined(RENDER_ENGINE_RTBIASPATHOCL)\n"
"	const float3 rayOrig = VLOAD3F(&ray->o.x);\n"
"	const float3 rayDir = VLOAD3F(&ray->d.x);\n"
"#else\n"
"	const float3 rayOrig = (float3)(ray->o.x, ray->o.y, ray->o.z);\n"
"	const float3 rayDir = (float3)(ray->d.x, ray->d.y, ray->d.z);\n"
"#endif\n"
"	VSTORE3F(rayDir, &tmpHitPoint->fixedDir.x);\n"
"	VSTORE3F(rayOrig, &tmpHitPoint->p.x);\n"
"	VSTORE2F((float2)(0.f, 0.f), &tmpHitPoint->uv.u);\n"
"	VSTORE3F(-rayDir, &tmpHitPoint->geometryN.x);\n"
"	VSTORE3F(-rayDir, &tmpHitPoint->shadeN.x);\n"
"#if defined(PARAM_ENABLE_TEX_HITPOINTCOLOR) || defined(PARAM_ENABLE_TEX_HITPOINTGREY) || defined(PARAM_TRIANGLE_LIGHT_HAS_VERTEX_COLOR)\n"
"	VSTORE3F(WHITE, tmpHitPoint->color.c);\n"
"#endif\n"
"#if defined(PARAM_ENABLE_TEX_HITPOINTALPHA)\n"
"	VSTORE2F(1.f, &tmpHitPoint->alpha);\n"
"#endif\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"	tmpHitPoint->passThroughEvent = passThroughEvent;\n"
"#endif\n"
"	tmpHitPoint->interiorVolumeIndex = NULL_INDEX;\n"
"	tmpHitPoint->exteriorVolumeIndex = NULL_INDEX;\n"
"	tmpHitPoint->intoObject = true;\n"
"\n"
"	const float maxDistance = hitT - ray->mint;\n"
"\n"
"	// Check if I have to support multi-scattering\n"
"	const bool scatterAllowed = (!scatteredStart || vol->volume.homogenous.multiScattering);\n"
"\n"
"	bool scatter = false;\n"
"	float distance = maxDistance;\n"
"	// I'm missing Texture::Filter() in OpenCL\n"
"	//const float k = sigmaS->Filter();\n"
"	const float3 sigmaS = HomogeneousVolume_SigmaS(vol, tmpHitPoint\n"
"			TEXTURES_PARAM);\n"
"	const float k = Spectrum_Filter(sigmaS);\n"
"	if (scatterAllowed && (k > 0.f)) {\n"
"		// Determine scattering distance\n"
"		const float scatterDistance = -log(1.f - passThroughEvent) / k;\n"
"\n"
"		scatter = scatterAllowed && (scatterDistance < maxDistance);\n"
"		distance = scatter ? scatterDistance : maxDistance;\n"
"\n"
"		// Note: distance can not be infinity because otherwise there would\n"
"		// have been a scatter event before.\n"
"		const float pdf = exp(-distance * k) * (scatter ? k : 1.f);\n"
"		*connectionThroughput /= pdf;\n"
"	}\n"
"\n"
"	const float3 sigmaT = HomogeneousVolume_SigmaA(vol, tmpHitPoint\n"
"			TEXTURES_PARAM) + sigmaS;\n"
"	if (!Spectrum_IsBlack(sigmaT)) {\n"
"		const float3 tau = clamp(distance * sigmaT, 0.f, INFINITY);\n"
"		const float3 transmittance = Spectrum_Exp(-tau);\n"
"\n"
"		// Apply volume transmittance\n"
"		*connectionThroughput *= transmittance * (scatter ? sigmaT : WHITE);\n"
"	}\n"
"\n"
"	// Apply volume emission\n"
"	const uint emiTexIndex = vol->volume.volumeEmissionTexIndex;\n"
"	if (emiTexIndex != NULL_INDEX) {\n"
"		const float3 emiTex = Texture_GetSpectrumValue(emiTexIndex, tmpHitPoint\n"
"			TEXTURES_PARAM);\n"
"		*connectionEmission += *connectionThroughput * distance * clamp(emiTex, 0.f, INFINITY);\n"
"	}\n"
"\n"
"	return scatter ? (ray->mint + distance) : -1.f;\n"
"}\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// HomogeneousVolume scatter\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined (PARAM_ENABLE_MAT_HETEROGENEOUS_VOL)\n"
"float3 HeterogeneousVolume_SigmaA(__global const Volume *vol, __global HitPoint *hitPoint\n"
"	TEXTURES_PARAM_DECL) {\n"
"	const float3 sigmaA = Texture_GetSpectrumValue(vol->volume.heterogenous.sigmaATexIndex, hitPoint\n"
"		TEXTURES_PARAM);\n"
"			\n"
"	return clamp(sigmaA, 0.f, INFINITY);\n"
"}\n"
"\n"
"float3 HeterogeneousVolume_SigmaS(__global const Volume *vol, __global HitPoint *hitPoint\n"
"	TEXTURES_PARAM_DECL) {\n"
"	const float3 sigmaS = Texture_GetSpectrumValue(vol->volume.heterogenous.sigmaSTexIndex, hitPoint\n"
"		TEXTURES_PARAM);\n"
"			\n"
"	return clamp(sigmaS, 0.f, INFINITY);\n"
"}\n"
"\n"
"float3 HeterogeneousVolume_SigmaT(__global const Volume *vol, __global HitPoint *hitPoint\n"
"	TEXTURES_PARAM_DECL) {\n"
"	return HeterogeneousVolume_SigmaA(vol, hitPoint TEXTURES_PARAM) +\n"
"			HeterogeneousVolume_SigmaS(vol, hitPoint TEXTURES_PARAM);\n"
"}\n"
"\n"
"float HeterogeneousVolume_Scatter(__global const Volume *vol,\n"
"#if !defined(RENDER_ENGINE_BIASPATHOCL) && !defined(RENDER_ENGINE_RTBIASPATHOCL)\n"
"		__global\n"
"#endif\n"
"		Ray *ray, const float hitT,\n"
"		const float passThroughEvent,\n"
"		const bool scatteredStart, float3 *connectionThroughput,\n"
"		float3 *connectionEmission, __global HitPoint *tmpHitPoint\n"
"		TEXTURES_PARAM_DECL) {\n"
"	// Compute the number of steps to evaluate the volume\n"
"	// Integrates in steps of at most stepSize\n"
"	// unless stepSize is too small compared to the total length\n"
"	const float mint = ray->mint;\n"
"	const float rayLen = hitT - mint;\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Handle the case when hitT is infinity or a very large number\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	const float stepSize = vol->volume.heterogenous.stepSize;\n"
"	const uint maxStepsCount = vol->volume.heterogenous.maxStepsCount;\n"
"	uint steps;\n"
"	float ss;\n"
"	if (rayLen == INFINITY) {\n"
"		steps = maxStepsCount;\n"
"		ss = stepSize;\n"
"	} else {\n"
"		// Note: Ceil2UInt() of an out of range number is 0\n"
"		const float fsteps = rayLen / fmax(MachineEpsilon_E(rayLen), stepSize);\n"
"		if (fsteps >= maxStepsCount)\n"
"			steps = maxStepsCount;\n"
"		else\n"
"			steps = Ceil2UInt(fsteps);\n"
"\n"
"		ss = rayLen / steps; // Effective step size\n"
"	}\n"
"\n"
"	const float totalDistance = ss * steps;\n"
"\n"
"	// Evaluate the scattering at the path origin\n"
"\n"
"	// Initialize tmpHitPoint\n"
"#if !defined(RENDER_ENGINE_BIASPATHOCL) && !defined(RENDER_ENGINE_RTBIASPATHOCL)\n"
"	const float3 rayOrig = VLOAD3F(&ray->o.x);\n"
"	const float3 rayDir = VLOAD3F(&ray->d.x);\n"
"#else\n"
"	const float3 rayOrig = (float3)(ray->o.x, ray->o.y, ray->o.z);\n"
"	const float3 rayDir = (float3)(ray->d.x, ray->d.y, ray->d.z);\n"
"#endif\n"
"	VSTORE3F(rayDir, &tmpHitPoint->fixedDir.x);\n"
"	VSTORE3F(rayOrig + mint * rayDir, &tmpHitPoint->p.x);\n"
"	VSTORE2F((float2)(0.f, 0.f), &tmpHitPoint->uv.u);\n"
"	VSTORE3F(-rayDir, &tmpHitPoint->geometryN.x);\n"
"	VSTORE3F(-rayDir, &tmpHitPoint->shadeN.x);\n"
"#if defined(PARAM_ENABLE_TEX_HITPOINTCOLOR) || defined(PARAM_ENABLE_TEX_HITPOINTGREY) || defined(PARAM_TRIANGLE_LIGHT_HAS_VERTEX_COLOR)\n"
"	VSTORE3F(WHITE, tmpHitPoint->color.c);\n"
"#endif\n"
"#if defined(PARAM_ENABLE_TEX_HITPOINTALPHA)\n"
"	VSTORE2F(1.f, &tmpHitPoint->alpha);\n"
"#endif\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"	tmpHitPoint->passThroughEvent = passThroughEvent;\n"
"#endif\n"
"	tmpHitPoint->interiorVolumeIndex = NULL_INDEX;\n"
"	tmpHitPoint->exteriorVolumeIndex = NULL_INDEX;\n"
"	tmpHitPoint->intoObject = true;\n"
"\n"
"	const bool scatterAllowed = (!scatteredStart || vol->volume.heterogenous.multiScattering);\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Find the scattering point if there is one\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	float oldSigmaS = Spectrum_Filter(HeterogeneousVolume_SigmaS(vol, tmpHitPoint\n"
"			TEXTURES_PARAM));\n"
"	float u = passThroughEvent;\n"
"	float scatterDistance = totalDistance;\n"
"	float t = -1.f;\n"
"	float pdf = 1.f;\n"
"	for (uint s = 1; s <= steps; ++s) {\n"
"		// Compute the mean scattering over the current step\n"
"		VSTORE3F(rayOrig + (mint + s * ss) * rayDir, &tmpHitPoint->p.x);\n"
"\n"
"		// Check if there is a scattering event\n"
"		const float newSigmaS = Spectrum_Filter(HeterogeneousVolume_SigmaS(vol, tmpHitPoint\n"
"			TEXTURES_PARAM));\n"
"		const float halfWaySigmaS = (oldSigmaS + newSigmaS) * .5f;\n"
"		oldSigmaS = newSigmaS;\n"
"\n"
"		// Skip the step if no scattering can occur\n"
"		if (halfWaySigmaS <= 0.f)\n"
"			continue;\n"
"\n"
"		// Determine scattering distance\n"
"		const float d = log(1.f - u) / halfWaySigmaS; // The real distance is ray.mint-d\n"
"		const bool scatter = scatterAllowed && (d > (s - 1U) * ss - totalDistance);\n"
"		if (!scatter) {\n"
"			if (scatterAllowed)\n"
"				pdf *= exp(-ss * halfWaySigmaS);\n"
"\n"
"			// Update the random variable to account for\n"
"			// the current step\n"
"			u -= (1.f - u) * (exp(oldSigmaS * ss) - 1.f);\n"
"			continue;\n"
"		}\n"
"\n"
"		// The ray is scattered\n"
"		scatterDistance = (s - 1U) * ss - d;\n"
"		t = mint + scatterDistance;\n"
"		pdf *= exp(d * halfWaySigmaS) * oldSigmaS;\n"
"\n"
"		VSTORE3F(rayOrig + t * rayDir, &tmpHitPoint->p.x);\n"
"		*connectionThroughput *= HeterogeneousVolume_SigmaT(vol, tmpHitPoint\n"
"				TEXTURES_PARAM);\n"
"		break;\n"
"	}\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Now I know the distance of the scattering point (if there is one) and\n"
"	// I can calculate transmittance and emission\n"
"	//--------------------------------------------------------------------------\n"
"	\n"
"	steps = Ceil2UInt(scatterDistance / fmax(MachineEpsilon_E(scatterDistance), stepSize));\n"
"	ss = scatterDistance / steps;\n"
"\n"
"	float3 tau = BLACK;\n"
"	float3 emission = BLACK;\n"
"	VSTORE3F(rayOrig + mint * rayDir, &tmpHitPoint->p.x);\n"
"	float3 oldSigmaT = HeterogeneousVolume_SigmaT(vol, tmpHitPoint\n"
"			TEXTURES_PARAM);\n"
"	const uint emiTexIndex = vol->volume.volumeEmissionTexIndex;\n"
"	for (uint s = 1; s <= steps; ++s) {\n"
"		VSTORE3F(rayOrig + (mint + s * ss) * rayDir, &tmpHitPoint->p.x);\n"
"\n"
"		// Accumulate tau values\n"
"		const float3 newSigmaT = HeterogeneousVolume_SigmaT(vol, tmpHitPoint\n"
"				TEXTURES_PARAM);\n"
"		const float3 halfWaySigmaT = (oldSigmaT + newSigmaT) * .5f;\n"
"		tau += clamp(ss * halfWaySigmaT, 0.f, INFINITY);\n"
"		oldSigmaT = newSigmaT;\n"
"\n"
"		// Accumulate volume emission\n"
"		if (emiTexIndex != NULL_INDEX) {\n"
"			const float3 emiTex = Texture_GetSpectrumValue(emiTexIndex, tmpHitPoint\n"
"				TEXTURES_PARAM);\n"
"			emission += Spectrum_Exp(-tau) * (ss * clamp(emiTex, 0.f, INFINITY));\n"
"		}\n"
"	}\n"
"	\n"
"	// Apply volume transmittance\n"
"	const float3 transmittance = Spectrum_Exp(-tau);\n"
"	*connectionThroughput *= transmittance / pdf;\n"
"\n"
"	// Add volume emission\n"
"	*connectionEmission += *connectionThroughput * emission;\n"
"\n"
"	return t;\n"
"}\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// Volume scatter\n"
"//------------------------------------------------------------------------------\n"
"\n"
"float Volume_Scatter(__global const Volume *vol,\n"
"#if !defined(RENDER_ENGINE_BIASPATHOCL) && !defined(RENDER_ENGINE_RTBIASPATHOCL)\n"
"		__global\n"
"#endif\n"
"		Ray *ray, const float hitT,\n"
"		const float passThrough,\n"
"		const bool scatteredStart, float3 *connectionThroughput,\n"
"		float3 *connectionEmission, __global HitPoint *tmpHitPoint\n"
"		TEXTURES_PARAM_DECL) {\n"
"	switch (vol->type) {\n"
"#if defined (PARAM_ENABLE_MAT_CLEAR_VOL)\n"
"		case CLEAR_VOL:\n"
"			return ClearVolume_Scatter(vol, ray, hitT,\n"
"					passThrough, scatteredStart,\n"
"					connectionThroughput, connectionEmission, tmpHitPoint\n"
"					TEXTURES_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_HOMOGENEOUS_VOL)\n"
"		case HOMOGENEOUS_VOL:\n"
"			return HomogeneousVolume_Scatter(vol, ray, hitT,\n"
"					passThrough, scatteredStart,\n"
"					connectionThroughput, connectionEmission, tmpHitPoint\n"
"					TEXTURES_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_HETEROGENEOUS_VOL)\n"
"		case HETEROGENEOUS_VOL:\n"
"			return HeterogeneousVolume_Scatter(vol, ray, hitT,\n"
"					passThrough, scatteredStart,\n"
"					connectionThroughput, connectionEmission, tmpHitPoint\n"
"					TEXTURES_PARAM);\n"
"#endif\n"
"		default:\n"
"			return -1.f;\n"
"	}\n"
"}\n"
"\n"
"#endif\n"
; } }
