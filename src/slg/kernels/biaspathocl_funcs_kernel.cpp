#include <string>
namespace slg { namespace ocl {
std::string KernelSource_biaspathocl_funcs = 
"#line 2 \"biaspatchocl_funcs.cl\"\n"
"\n"
"/***************************************************************************\n"
" * Copyright 1998-2013 by authors (see AUTHORS.txt)                        *\n"
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
"void SR_RadianceClamp(__global SampleResult *sampleResult) {\n"
"	// Initialize only Spectrum fields\n"
"\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_0)\n"
"	sampleResult->radiancePerPixelNormalized[0].r = clamp(sampleResult->radiancePerPixelNormalized[0].r, 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);\n"
"	sampleResult->radiancePerPixelNormalized[0].g = clamp(sampleResult->radiancePerPixelNormalized[0].g, 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);\n"
"	sampleResult->radiancePerPixelNormalized[0].b = clamp(sampleResult->radiancePerPixelNormalized[0].b, 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_1)\n"
"	sampleResult->radiancePerPixelNormalized[1].r = clamp(sampleResult->radiancePerPixelNormalized[1].r, 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);\n"
"	sampleResult->radiancePerPixelNormalized[1].g = clamp(sampleResult->radiancePerPixelNormalized[1].g, 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);\n"
"	sampleResult->radiancePerPixelNormalized[1].b = clamp(sampleResult->radiancePerPixelNormalized[1].b, 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_2)\n"
"	sampleResult->radiancePerPixelNormalized[2].r = clamp(sampleResult->radiancePerPixelNormalized[2].r, 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);\n"
"	sampleResult->radiancePerPixelNormalized[2].g = clamp(sampleResult->radiancePerPixelNormalized[2].g, 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);\n"
"	sampleResult->radiancePerPixelNormalized[2].b = clamp(sampleResult->radiancePerPixelNormalized[2].b, 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_3)\n"
"	sampleResult->radiancePerPixelNormalized[3].r = clamp(sampleResult->radiancePerPixelNormalized[3].r, 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);\n"
"	sampleResult->radiancePerPixelNormalized[3].g = clamp(sampleResult->radiancePerPixelNormalized[3].g, 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);\n"
"	sampleResult->radiancePerPixelNormalized[3].b = clamp(sampleResult->radiancePerPixelNormalized[3].b, 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_4)\n"
"	sampleResult->radiancePerPixelNormalized[4].r = clamp(sampleResult->radiancePerPixelNormalized[4].r, 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);\n"
"	sampleResult->radiancePerPixelNormalized[4].g = clamp(sampleResult->radiancePerPixelNormalized[4].g, 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);\n"
"	sampleResult->radiancePerPixelNormalized[4].b = clamp(sampleResult->radiancePerPixelNormalized[4].b, 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_5)\n"
"	sampleResult->radiancePerPixelNormalized[5].r = clamp(sampleResult->radiancePerPixelNormalized[5].r, 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);\n"
"	sampleResult->radiancePerPixelNormalized[5].g = clamp(sampleResult->radiancePerPixelNormalized[5].g, 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);\n"
"	sampleResult->radiancePerPixelNormalized[5].b = clamp(sampleResult->radiancePerPixelNormalized[5].b, 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_6)\n"
"	sampleResult->radiancePerPixelNormalized[6].r = clamp(sampleResult->radiancePerPixelNormalized[6].r, 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);\n"
"	sampleResult->radiancePerPixelNormalized[6].g = clamp(sampleResult->radiancePerPixelNormalized[6].g, 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);\n"
"	sampleResult->radiancePerPixelNormalized[6].b = clamp(sampleResult->radiancePerPixelNormalized[6].b, 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_7)\n"
"	sampleResult->radiancePerPixelNormalized[7].r = clamp(sampleResult->radiancePerPixelNormalized[7].r, 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);\n"
"	sampleResult->radiancePerPixelNormalized[7].g = clamp(sampleResult->radiancePerPixelNormalized[7].g, 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);\n"
"	sampleResult->radiancePerPixelNormalized[7].b = clamp(sampleResult->radiancePerPixelNormalized[7].b, 0.f, PARAM_RADIANCE_CLAMP_MAXVALUE);\n"
"#endif\n"
"}\n"
"\n"
"void SR_Accumulate(__global SampleResult *src, SampleResult *dst) {\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_0)\n"
"	dst->radiancePerPixelNormalized[0].r += src->radiancePerPixelNormalized[0].r;\n"
"	dst->radiancePerPixelNormalized[0].g += src->radiancePerPixelNormalized[0].g;\n"
"	dst->radiancePerPixelNormalized[0].b += src->radiancePerPixelNormalized[0].b;\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_1)\n"
"	dst->radiancePerPixelNormalized[1].r += src->radiancePerPixelNormalized[1].r;\n"
"	dst->radiancePerPixelNormalized[1].g += src->radiancePerPixelNormalized[1].g;\n"
"	dst->radiancePerPixelNormalized[1].b += src->radiancePerPixelNormalized[1].b;\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_2)\n"
"	dst->radiancePerPixelNormalized[2].r += src->radiancePerPixelNormalized[2].r;\n"
"	dst->radiancePerPixelNormalized[2].g += src->radiancePerPixelNormalized[2].g;\n"
"	dst->radiancePerPixelNormalized[2].b += src->radiancePerPixelNormalized[2].b;\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_3)\n"
"	dst->radiancePerPixelNormalized[3].r += src->radiancePerPixelNormalized[3].r;\n"
"	dst->radiancePerPixelNormalized[3].g += src->radiancePerPixelNormalized[3].g;\n"
"	dst->radiancePerPixelNormalized[3].b += src->radiancePerPixelNormalized[3].b;\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_4)\n"
"	dst->radiancePerPixelNormalized[4].r += src->radiancePerPixelNormalized[4].r;\n"
"	dst->radiancePerPixelNormalized[4].g += src->radiancePerPixelNormalized[4].g;\n"
"	dst->radiancePerPixelNormalized[4].b += src->radiancePerPixelNormalized[4].b;\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_5)\n"
"	dst->radiancePerPixelNormalized[5].r += src->radiancePerPixelNormalized[5].r;\n"
"	dst->radiancePerPixelNormalized[5].g += src->radiancePerPixelNormalized[5].g;\n"
"	dst->radiancePerPixelNormalized[5].b += src->radiancePerPixelNormalized[5].b;\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_6)\n"
"	dst->radiancePerPixelNormalized[6].r += src->radiancePerPixelNormalized[6].r;\n"
"	dst->radiancePerPixelNormalized[6].g += src->radiancePerPixelNormalized[6].g;\n"
"	dst->radiancePerPixelNormalized[6].b += src->radiancePerPixelNormalized[6].b;\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_7)\n"
"	dst->radiancePerPixelNormalized[7].r += src->radiancePerPixelNormalized[7].r;\n"
"	dst->radiancePerPixelNormalized[7].g += src->radiancePerPixelNormalized[7].g;\n"
"	dst->radiancePerPixelNormalized[7].b += src->radiancePerPixelNormalized[7].b;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_ALPHA)\n"
"	dst->alpha += dst->alpha + src->alpha;\n"
"#endif\n"
"\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_DIFFUSE)\n"
"	dst->directDiffuse.r += src->directDiffuse.r;\n"
"	dst->directDiffuse.g += src->directDiffuse.g;\n"
"	dst->directDiffuse.b += src->directDiffuse.b;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_GLOSSY)\n"
"	dst->directGlossy.r += src->directGlossy.r;\n"
"	dst->directGlossy.g += src->directGlossy.g;\n"
"	dst->directGlossy.b += src->directGlossy.b;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_EMISSION)\n"
"	dst->emission.r += src->emission.r;\n"
"	dst->emission.g += src->emission.g;\n"
"	dst->emission.b += src->emission.b;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_DIFFUSE)\n"
"	dst->indirectDiffuse.r += src->indirectDiffuse.r;\n"
"	dst->indirectDiffuse.g += src->indirectDiffuse.g;\n"
"	dst->indirectDiffuse.b += src->indirectDiffuse.b;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_GLOSSY)\n"
"	dst->indirectGlossy.r += src->indirectGlossy.r;\n"
"	dst->indirectGlossy.g += src->indirectGlossy.g;\n"
"	dst->indirectGlossy.b += src->indirectGlossy.b;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SPECULAR)\n"
"	dst->indirectSpecular.r += src->indirectSpecular.r;\n"
"	dst->indirectSpecular.g += src->indirectSpecular.g;\n"
"	dst->indirectSpecular.b += src->indirectSpecular.b;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_SHADOW_MASK)\n"
"	dst->directShadowMask += src->directShadowMask;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SHADOW_MASK)\n"
"	dst->indirectShadowMask += src->indirectShadowMask;\n"
"#endif\n"
"\n"
"	bool depthWrite = true;\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DEPTH)\n"
"	const float srcDepthValue = src->depth;\n"
"	if (srcDepthValue <= dst->depth)\n"
"		dst->depth = srcDepthValue;\n"
"	else\n"
"		depthWrite = false;\n"
"#endif\n"
"	if (depthWrite) {\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_POSITION)\n"
"		dst->position = src->position;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_GEOMETRY_NORMAL)\n"
"		dst->geometryNormal = src->geometryNormal;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_SHADING_NORMAL)\n"
"		dst->shadingNormal = src->shadingNormal;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_MATERIAL_ID)\n"
"		// Note: MATERIAL_ID_MASK is calculated starting from materialID field\n"
"		dst->materialID = src->materialID;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_UV)\n"
"		dst->uv = src->uv;\n"
"#endif\n"
"	}\n"
"\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_RAYCOUNT)\n"
"	dst->rayCount += src->rayCount;\n"
"#endif\n"
"}\n"
"\n"
"void SR_Normalize(SampleResult *dst, const float k) {\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_0)\n"
"	dst->radiancePerPixelNormalized[0].r *= k;\n"
"	dst->radiancePerPixelNormalized[0].g *= k;\n"
"	dst->radiancePerPixelNormalized[0].b *= k;\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_1)\n"
"	dst->radiancePerPixelNormalized[1].r *= k;\n"
"	dst->radiancePerPixelNormalized[1].g *= k;\n"
"	dst->radiancePerPixelNormalized[1].b *= k;\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_2)\n"
"	dst->radiancePerPixelNormalized[2].r *= k;\n"
"	dst->radiancePerPixelNormalized[2].g *= k;\n"
"	dst->radiancePerPixelNormalized[2].b *= k;\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_3)\n"
"	dst->radiancePerPixelNormalized[3].r *= k;\n"
"	dst->radiancePerPixelNormalized[3].g *= k;\n"
"	dst->radiancePerPixelNormalized[3].b *= k;\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_4)\n"
"	dst->radiancePerPixelNormalized[4].r *= k;\n"
"	dst->radiancePerPixelNormalized[4].g *= k;\n"
"	dst->radiancePerPixelNormalized[4].b *= k;\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_5)\n"
"	dst->radiancePerPixelNormalized[5].r *= k;\n"
"	dst->radiancePerPixelNormalized[5].g *= k;\n"
"	dst->radiancePerPixelNormalized[5].b *= k;\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_6)\n"
"	dst->radiancePerPixelNormalized[6].r *= k;\n"
"	dst->radiancePerPixelNormalized[6].g *= k;\n"
"	dst->radiancePerPixelNormalized[6].b *= k;\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_7)\n"
"	dst->radiancePerPixelNormalized[7].r *= k;\n"
"	dst->radiancePerPixelNormalized[7].g *= k;\n"
"	dst->radiancePerPixelNormalized[7].b *= k;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_ALPHA)\n"
"	dst->alpha *= k;\n"
"#endif\n"
"\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_DIFFUSE)\n"
"	dst->directDiffuse.r *= k;\n"
"	dst->directDiffuse.g *= k;\n"
"	dst->directDiffuse.b *= k;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_GLOSSY)\n"
"	dst->directGlossy.r *= k;\n"
"	dst->directGlossy.g *= k;\n"
"	dst->directGlossy.b *= k;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_EMISSION)\n"
"	dst->emission.r *= k;\n"
"	dst->emission.g *= k;\n"
"	dst->emission.b *= k;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_DIFFUSE)\n"
"	dst->indirectDiffuse.r *= k;\n"
"	dst->indirectDiffuse.g *= k;\n"
"	dst->indirectDiffuse.b *= k;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_GLOSSY)\n"
"	dst->indirectGlossy.r *= k;\n"
"	dst->indirectGlossy.g *= k;\n"
"	dst->indirectGlossy.b *= k;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SPECULAR)\n"
"	dst->indirectSpecular.r *= k;\n"
"	dst->indirectSpecular.g *= k;\n"
"	dst->indirectSpecular.b *= k;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_SHADOW_MASK)\n"
"	dst->directShadowMask *= k;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SHADOW_MASK)\n"
"	dst->indirectShadowMask *= k;\n"
"#endif\n"
"}\n"
"\n"
"void SampleGrid(Seed *seed, const uint size,\n"
"		const uint ix, const uint iy, float *u0, float *u1) {\n"
"	*u0 = Rnd_FloatValue(seed);\n"
"	*u1 = Rnd_FloatValue(seed);\n"
"\n"
"	if (size > 1) {\n"
"		const float idim = 1.f / size;\n"
"		*u0 = (ix + *u0) * idim;\n"
"		*u1 = (iy + *u1) * idim;\n"
"	}\n"
"}\n"
"\n"
"typedef struct {\n"
"	uint depth, diffuseDepth, glossyDepth, specularDepth;\n"
"} PathDepthInfo;\n"
"\n"
"void PathDepthInfo_Init(PathDepthInfo *depthInfo, const uint val) {\n"
"	depthInfo->depth = val;\n"
"	depthInfo->diffuseDepth = val;\n"
"	depthInfo->glossyDepth = val;\n"
"	depthInfo->specularDepth = val;\n"
"}\n"
"\n"
"void PathDepthInfo_IncDepths(PathDepthInfo *depthInfo, const BSDFEvent event) {\n"
"	++(depthInfo->depth);\n"
"	if (event & DIFFUSE)\n"
"		++(depthInfo->diffuseDepth);\n"
"	if (event & GLOSSY)\n"
"		++(depthInfo->glossyDepth);\n"
"	if (event & SPECULAR)\n"
"		++(depthInfo->specularDepth);\n"
"}\n"
"\n"
"bool PathDepthInfo_CheckDepths(PathDepthInfo *depthInfo) {\n"
"	return ((depthInfo->depth <= PARAM_DEPTH_MAX) &&\n"
"			(depthInfo->diffuseDepth <= PARAM_DEPTH_DIFFUSE_MAX) &&\n"
"			(depthInfo->glossyDepth <= PARAM_DEPTH_GLOSSY_MAX) &&\n"
"			(depthInfo->specularDepth <= PARAM_DEPTH_SPECULAR_MAX));\n"
"}\n"
"\n"
"//------------------------------------------------------------------------------\n"
"\n"
"void GenerateCameraRay(\n"
"		Seed *seed,\n"
"		__global GPUTask *task,\n"
"		__global SampleResult *sampleResult,\n"
"		__global Camera *camera,\n"
"		__global float *pixelFilterDistribution,\n"
"		const uint sampleX, const uint sampleY, const int sampleIndex,\n"
"		const uint tileStartX, const uint tileStartY, \n"
"		const uint engineFilmWidth, const uint engineFilmHeight,\n"
"		Ray *ray) {\n"
"	//if (get_global_id(0) == 0)\n"
"	//	printf(\"tileSampleIndex: %d (%d, %d)\\n\", sampleIndex, sampleIndex % PARAM_AA_SAMPLES, sampleIndex / PARAM_AA_SAMPLES);\n"
"\n"
"	float u0, u1;\n"
"	SampleGrid(seed, PARAM_AA_SAMPLES,\n"
"			sampleIndex % PARAM_AA_SAMPLES, sampleIndex / PARAM_AA_SAMPLES,\n"
"			&u0, &u1);\n"
"\n"
"	float2 xy;\n"
"	float distPdf;\n"
"	Distribution2D_SampleContinuous(pixelFilterDistribution, u0, u1, &xy, &distPdf);\n"
"\n"
"	const float filmX = sampleX + .5f + (xy.x - .5f) * PARAM_IMAGE_FILTER_WIDTH_X;\n"
"	const float filmY = sampleY + .5f + (xy.y - .5f) * PARAM_IMAGE_FILTER_WIDTH_Y;\n"
"	sampleResult->filmX = filmX;\n"
"	sampleResult->filmY = filmY;\n"
"\n"
"#if defined(PARAM_CAMERA_HAS_DOF)\n"
"	const float dofSampleX = Rnd_FloatValue(seed);\n"
"	const float dofSampleY = Rnd_FloatValue(seed);\n"
"#endif\n"
"\n"
"	Camera_GenerateRay(camera, engineFilmWidth, engineFilmHeight, ray, tileStartX + filmX, tileStartY + filmY\n"
"#if defined(PARAM_CAMERA_HAS_DOF)\n"
"			, dofSampleX, dofSampleY\n"
"#endif\n"
"			);	\n"
"}\n"
"\n"
"#if defined(PARAM_HAS_SKYLIGHT) || defined(PARAM_HAS_INFINITELIGHT) || defined(PARAM_HAS_SUNLIGHT)\n"
"void DirectHitInfiniteLight(\n"
"		const bool firstPathVertex,\n"
"		const BSDFEvent lastBSDFEvent,\n"
"		const BSDFEvent pathBSDFEvent,\n"
"		__global const Spectrum *pathThroughput,\n"
"		const float3 eyeDir, const float lastPdfW,\n"
"		__global SampleResult *sampleResult\n"
"		LIGHTS_PARAM_DECL) {\n"
"	const float3 throughput = VLOAD3F(&pathThroughput->r);\n"
"\n"
"	for (uint i = 0; i < envLightCount; ++i) {\n"
"		__global LightSource *light = &lights[envLightIndices[i]];\n"
"\n"
"		if (firstPathVertex || (light->visibility & (pathBSDFEvent & (DIFFUSE | GLOSSY | SPECULAR)))) {\n"
"			float directPdfW;\n"
"			const float3 lightRadiance = EnvLight_GetRadiance(light, eyeDir, &directPdfW\n"
"					LIGHTS_PARAM);\n"
"\n"
"			if (!Spectrum_IsBlack(lightRadiance)) {\n"
"				// MIS between BSDF sampling and direct light sampling\n"
"				const float lightPickProb = Scene_SampleAllLightPdf(lightsDistribution, light->lightSceneIndex);\n"
"				const float weight = ((lastBSDFEvent & SPECULAR) ? 1.f : PowerHeuristic(lastPdfW, directPdfW * lightPickProb));\n"
"				const float3 radiance = weight * throughput * lightRadiance;\n"
"\n"
"				const uint lightID = min(light->lightID, PARAM_FILM_RADIANCE_GROUP_COUNT - 1u);\n"
"				AddEmission(firstPathVertex, pathBSDFEvent, lightID, sampleResult, radiance);\n"
"			}\n"
"		}\n"
"	}\n"
"}\n"
"#endif\n"
"\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"void DirectHitFiniteLight(\n"
"		const bool firstPathVertex,\n"
"		const BSDFEvent lastBSDFEvent,\n"
"		const BSDFEvent pathBSDFEvent,\n"
"		__global const Spectrum *pathThroughput, const float distance, __global BSDF *bsdf,\n"
"		const float lastPdfW, __global SampleResult *sampleResult\n"
"		LIGHTS_PARAM_DECL) {\n"
"	if (firstPathVertex || (lights[bsdf->triangleLightSourceIndex].visibility & (pathBSDFEvent & (DIFFUSE | GLOSSY | SPECULAR)))) {\n"
"		float directPdfA;\n"
"		const float3 emittedRadiance = BSDF_GetEmittedRadiance(bsdf, &directPdfA\n"
"				LIGHTS_PARAM);\n"
"\n"
"		if (!Spectrum_IsBlack(emittedRadiance)) {\n"
"			// Add emitted radiance\n"
"			float weight = 1.f;\n"
"			if (!(lastBSDFEvent & SPECULAR)) {\n"
"				const float lightPickProb = Scene_SampleAllLightPdf(lightsDistribution,\n"
"						lights[bsdf->triangleLightSourceIndex].lightSceneIndex);\n"
"				const float directPdfW = PdfAtoW(directPdfA, distance,\n"
"					fabs(dot(VLOAD3F(&bsdf->hitPoint.fixedDir.x), VLOAD3F(&bsdf->hitPoint.shadeN.x))));\n"
"\n"
"				// MIS between BSDF sampling and direct light sampling\n"
"				weight = PowerHeuristic(lastPdfW, directPdfW * lightPickProb);\n"
"			}\n"
"			const float3 lightRadiance = weight * VLOAD3F(&pathThroughput->r) * emittedRadiance;\n"
"\n"
"			const uint lightID =  min(BSDF_GetLightID(bsdf\n"
"					MATERIALS_PARAM), PARAM_FILM_RADIANCE_GROUP_COUNT - 1u);\n"
"			AddEmission(firstPathVertex, pathBSDFEvent, lightID, sampleResult, lightRadiance);\n"
"		}\n"
"	}\n"
"}\n"
"#endif\n"
"\n"
"bool DirectLightSampling(\n"
"		__global LightSource *light,\n"
"		const float lightPickPdf,\n"
"#if defined(PARAM_HAS_INFINITELIGHT) || defined(PARAM_HAS_SKYLIGHT)\n"
"		const float worldCenterX,\n"
"		const float worldCenterY,\n"
"		const float worldCenterZ,\n"
"		const float worldRadius,\n"
"#endif\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"		__global HitPoint *tmpHitPoint,\n"
"#endif\n"
"		const float u0, const float u1, const float u2,\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"		const float u3,\n"
"#endif\n"
"		__global const Spectrum *pathThroughput, __global BSDF *bsdf,\n"
"		Ray *shadowRay, __global Spectrum *radiance, __global uint *ID\n"
"		LIGHTS_PARAM_DECL) {\n"
"	float3 lightRayDir;\n"
"	float distance, directPdfW;\n"
"	const float3 lightRadiance = Light_Illuminate(\n"
"			light,\n"
"			VLOAD3F(&bsdf->hitPoint.p.x),\n"
"			u0, u1, u2,\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"			u3,\n"
"#endif\n"
"#if defined(PARAM_HAS_INFINITELIGHT) || defined(PARAM_HAS_SKYLIGHT)\n"
"			worldCenterX, worldCenterY, worldCenterZ, worldRadius,\n"
"#endif\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"			tmpHitPoint,\n"
"#endif		\n"
"			&lightRayDir, &distance, &directPdfW\n"
"			LIGHTS_PARAM);\n"
"\n"
"	// Setup the shadow ray\n"
"	const float cosThetaToLight = fabs(dot(lightRayDir, VLOAD3F(&bsdf->hitPoint.shadeN.x)));\n"
"	if (((Spectrum_Y(lightRadiance) * cosThetaToLight / directPdfW) > PARAM_LOW_LIGHT_THREASHOLD) &&\n"
"			(distance > PARAM_NEAR_START_LIGHT)) {\n"
"		BSDFEvent event;\n"
"		float bsdfPdfW;\n"
"		const float3 bsdfEval = BSDF_Evaluate(bsdf,\n"
"				lightRayDir, &event, &bsdfPdfW\n"
"				MATERIALS_PARAM);\n"
"\n"
"		if (!Spectrum_IsBlack(bsdfEval)) {\n"
"			const float directLightSamplingPdfW = directPdfW * lightPickPdf;\n"
"			const float factor = cosThetaToLight / directLightSamplingPdfW;\n"
"\n"
"			// MIS between direct light sampling and BSDF sampling\n"
"			const float weight = PowerHeuristic(directLightSamplingPdfW, bsdfPdfW);\n"
"\n"
"			VSTORE3F((weight * factor) * VLOAD3F(&pathThroughput->r) * bsdfEval * lightRadiance, &radiance->r);\n"
"			*ID = min(light->lightID, PARAM_FILM_RADIANCE_GROUP_COUNT - 1u);\n"
"\n"
"			// Setup the shadow ray\n"
"			const float3 hitPoint = VLOAD3F(&bsdf->hitPoint.p.x);\n"
"			const float epsilon = fmax(MachineEpsilon_E_Float3(hitPoint), MachineEpsilon_E(distance));\n"
"\n"
"			Ray_Init4_Private(shadowRay, hitPoint, lightRayDir,\n"
"				epsilon,\n"
"				distance - epsilon);\n"
"\n"
"			return true;\n"
"		}\n"
"	}\n"
"\n"
"	return false;\n"
"}\n"
"\n"
"bool DirectLightSampling_ONE(\n"
"		const bool firstPathVertex,\n"
"		Seed *seed,\n"
"#if defined(PARAM_HAS_INFINITELIGHT) || defined(PARAM_HAS_SKYLIGHT)\n"
"		const float worldCenterX,\n"
"		const float worldCenterY,\n"
"		const float worldCenterZ,\n"
"		const float worldRadius,\n"
"#endif\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"		__global HitPoint *tmpHitPoint,\n"
"#endif\n"
"		__global const Spectrum *pathThroughput, __global BSDF *bsdf,\n"
"		__global SampleResult *sampleResult,\n"
"		Ray *shadowRay, __global Spectrum *radiance, __global uint *ID\n"
"		LIGHTS_PARAM_DECL) {\n"
"	// Pick a light source to sample\n"
"	float lightPickPdf;\n"
"	const uint lightIndex = Scene_SampleAllLights(lightsDistribution, Rnd_FloatValue(seed), &lightPickPdf);\n"
"\n"
"	const bool illuminated = DirectLightSampling(\n"
"		&lights[lightIndex],\n"
"		lightPickPdf,\n"
"#if defined(PARAM_HAS_INFINITELIGHT) || defined(PARAM_HAS_SKYLIGHT)\n"
"		worldCenterX,\n"
"		worldCenterY,\n"
"		worldCenterZ,\n"
"		worldRadius,\n"
"#endif\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"		tmpHitPoint,\n"
"#endif\n"
"		Rnd_FloatValue(seed), Rnd_FloatValue(seed), Rnd_FloatValue(seed),\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"		Rnd_FloatValue(seed),\n"
"#endif\n"
"		pathThroughput, bsdf,\n"
"		shadowRay, radiance, ID\n"
"		LIGHTS_PARAM);\n"
"\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_SHADOW_MASK)\n"
"	if (firstPathVertex && !illuminated)\n"
"		sampleResult->directShadowMask += 1.f;\n"
"#endif\n"
"\n"
"	return illuminated;\n"
"}\n"
"\n"
"#if defined(PARAM_DIRECT_LIGHT_ALL_STRATEGY)\n"
"bool DirectLightSampling_ALL(\n"
"		__global uint *currentLightIndex,\n"
"		__global uint *currentLightSampleIndex,\n"
"		Seed *seed,\n"
"#if defined(PARAM_HAS_INFINITELIGHT) || defined(PARAM_HAS_SKYLIGHT)\n"
"		const float worldCenterX,\n"
"		const float worldCenterY,\n"
"		const float worldCenterZ,\n"
"		const float worldRadius,\n"
"#endif\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"		__global HitPoint *tmpHitPoint,\n"
"#endif\n"
"		__global const Spectrum *pathThroughput, __global BSDF *bsdf,\n"
"		__global SampleResult *sampleResult,\n"
"		Ray *shadowRay, __global Spectrum *radiance, __global uint *ID\n"
"		LIGHTS_PARAM_DECL) {\n"
"	for (; *currentLightIndex < PARAM_LIGHT_COUNT; ++(*currentLightIndex)) {\n"
"		const int lightSamplesCount = lights[*currentLightIndex].samples;\n"
"		const uint sampleCount = (lightSamplesCount < 0) ? PARAM_DIRECT_LIGHT_SAMPLES : (uint)lightSamplesCount;\n"
"		const uint sampleCount2 = sampleCount * sampleCount;\n"
"\n"
"		for (; *currentLightSampleIndex < sampleCount2; ++(*currentLightSampleIndex)) {\n"
"			//if (get_global_id(0) == 0)\n"
"			//	printf(\"DirectLightSampling_ALL() ==> currentLightIndex: %d  currentLightSampleIndex: %d\\n\", *currentLightIndex, *currentLightSampleIndex);\n"
"\n"
"			float u0, u1;\n"
"			SampleGrid(seed, sampleCount,\n"
"					(*currentLightSampleIndex) % sampleCount, (*currentLightSampleIndex) / sampleCount,\n"
"					&u0, &u1);\n"
"\n"
"			const float scaleFactor = 1.f / sampleCount2;\n"
"			const bool illuminated = DirectLightSampling(\n"
"				&lights[*currentLightIndex],\n"
"				1.f,\n"
"#if defined(PARAM_HAS_INFINITELIGHT) || defined(PARAM_HAS_SKYLIGHT)\n"
"				worldCenterX,\n"
"				worldCenterY,\n"
"				worldCenterZ,\n"
"				worldRadius,\n"
"#endif\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"				tmpHitPoint,\n"
"#endif\n"
"				u0, u1, Rnd_FloatValue(seed),\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"				Rnd_FloatValue(seed),\n"
"#endif\n"
"				pathThroughput, bsdf,\n"
"				shadowRay, radiance, ID\n"
"				LIGHTS_PARAM);\n"
"\n"
"			if (illuminated) {\n"
"				VSTORE3F(scaleFactor * VLOAD3F(&radiance->r), &radiance->r);\n"
"				return true;\n"
"			}\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_SHADOW_MASK)\n"
"			else {\n"
"				sampleResult->directShadowMask += scaleFactor;\n"
"			}\n"
"#endif\n"
"		}\n"
"\n"
"		*currentLightSampleIndex = 0;\n"
"	}\n"
"\n"
"	return false;\n"
"}\n"
"#endif\n"
; } }
