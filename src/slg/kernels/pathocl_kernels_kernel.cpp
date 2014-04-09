#include <string>
namespace slg { namespace ocl {
std::string KernelSource_pathocl_kernels = 
"#line 2 \"patchocl_kernels.cl\"\n"
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
"// List of symbols defined at compile time:\n"
"//  PARAM_MAX_PATH_DEPTH\n"
"//  PARAM_RR_DEPTH\n"
"//  PARAM_RR_CAP\n"
"\n"
"// (optional)\n"
"//  PARAM_IMAGE_FILTER_TYPE (0 = No filter, 1 = Box, 2 = Gaussian, 3 = Mitchell, 4 = Blackman-Harris)\n"
"//  PARAM_IMAGE_FILTER_WIDTH_X\n"
"//  PARAM_IMAGE_FILTER_WIDTH_Y\n"
"//  PARAM_IMAGE_FILTER_PIXEL_WIDTH_X\n"
"//  PARAM_IMAGE_FILTER_PIXEL_WIDTH_Y\n"
"// (Box filter)\n"
"// (Gaussian filter)\n"
"//  PARAM_IMAGE_FILTER_GAUSSIAN_ALPHA\n"
"// (Mitchell filter)\n"
"//  PARAM_IMAGE_FILTER_MITCHELL_B\n"
"//  PARAM_IMAGE_FILTER_MITCHELL_C\n"
"\n"
"// (optional)\n"
"//  PARAM_SAMPLER_TYPE (0 = Inlined Random, 1 = Metropolis, 2 = Sobol)\n"
"// (Metropolis)\n"
"//  PARAM_SAMPLER_METROPOLIS_LARGE_STEP_RATE\n"
"//  PARAM_SAMPLER_METROPOLIS_MAX_CONSECUTIVE_REJECT\n"
"//  PARAM_SAMPLER_METROPOLIS_IMAGE_MUTATION_RANGE\n"
"// (Sobol)\n"
"//  PARAM_SAMPLER_SOBOL_STARTOFFSET\n"
"//  PARAM_SAMPLER_SOBOL_MAXDEPTH\n"
"\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// Init Kernel\n"
"//------------------------------------------------------------------------------\n"
"\n"
"void GenerateCameraPath(\n"
"		__global GPUTask *task,\n"
"		__global Sample *sample,\n"
"		__global float *sampleData,\n"
"		__global Camera *camera,\n"
"		const uint filmWidth,\n"
"		const uint filmHeight,\n"
"		__global Ray *ray,\n"
"		Seed *seed) {\n"
"#if (PARAM_SAMPLER_TYPE == 0)\n"
"\n"
"#if defined(PARAM_CAMERA_HAS_DOF)\n"
"	const float dofSampleX = Rnd_FloatValue(seed);\n"
"	const float dofSampleY = Rnd_FloatValue(seed);\n"
"#endif\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"	const float eyePassthrough = Rnd_FloatValue(seed);\n"
"#endif\n"
"#endif\n"
"\n"
"#if (PARAM_SAMPLER_TYPE == 1)\n"
"	__global float *sampleDataPathBase = Sampler_GetSampleDataPathBase(sample, sampleData);\n"
"#if defined(PARAM_CAMERA_HAS_DOF)\n"
"	const float dofSampleX = Sampler_GetSamplePath(IDX_DOF_X);\n"
"	const float dofSampleY = Sampler_GetSamplePath(IDX_DOF_Y);\n"
"#endif\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"	const float eyePassthrough = Sampler_GetSamplePath(IDX_EYE_PASSTHROUGH);\n"
"#endif\n"
"#endif\n"
"\n"
"#if (PARAM_SAMPLER_TYPE == 2)\n"
"#if defined(PARAM_CAMERA_HAS_DOF)\n"
"	const float dofSampleX = Sampler_GetSamplePath(IDX_DOF_X);\n"
"	const float dofSampleY = Sampler_GetSamplePath(IDX_DOF_Y);\n"
"#endif\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"	const float eyePassthrough = Sampler_GetSamplePath(IDX_EYE_PASSTHROUGH);\n"
"#endif\n"
"#endif\n"
"\n"
"	Camera_GenerateRay(camera, filmWidth, filmHeight, ray,\n"
"			sample->result.filmX, sample->result.filmY\n"
"#if defined(PARAM_CAMERA_HAS_DOF)\n"
"			, dofSampleX, dofSampleY\n"
"#endif\n"
"			);\n"
"\n"
"	// Initialize the path state\n"
"	task->pathStateBase.state = RT_NEXT_VERTEX;\n"
"	task->pathStateBase.depth = 1;\n"
"	VSTORE3F(WHITE, task->pathStateBase.throughput.c);\n"
"	task->directLightState.pathBSDFEvent = NONE;\n"
"	task->directLightState.lastBSDFEvent = SPECULAR; // SPECULAR is required to avoid MIS\n"
"	task->directLightState.lastPdfW = 1.f;\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"	// This is a bit tricky. I store the passThroughEvent in the BSDF\n"
"	// before of the initialization because it can be used during the\n"
"	// tracing of next path vertex ray.\n"
"\n"
"	task->pathStateBase.bsdf.hitPoint.passThroughEvent = eyePassthrough;\n"
"#endif\n"
"\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_SHADOW_MASK)\n"
"	sample->result.directShadowMask = 1.f;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SHADOW_MASK)\n"
"	sample->result.indirectShadowMask = 1.f;\n"
"#endif\n"
"}\n"
"\n"
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void Init(\n"
"		uint seedBase,\n"
"		__global GPUTask *tasks,\n"
"		__global GPUTaskStats *taskStats,\n"
"		__global Sample *samples,\n"
"		__global float *samplesData,\n"
"		__global Ray *rays,\n"
"		__global Camera *camera,\n"
"		const uint filmWidth,\n"
"		const uint filmHeight\n"
"		) {\n"
"	const size_t gid = get_global_id(0);\n"
"	if (gid >= PARAM_TASK_COUNT)\n"
"		return;\n"
"\n"
"	// Initialize the task\n"
"	__global GPUTask *task = &tasks[gid];\n"
"\n"
"	// Initialize random number generator\n"
"	Seed seed;\n"
"	Rnd_Init(seedBase + gid, &seed);\n"
"\n"
"	// Initialize the sample and path\n"
"	__global Sample *sample = &samples[gid];\n"
"	__global float *sampleData = Sampler_GetSampleData(sample, samplesData);\n"
"	Sampler_Init(&seed, sample, sampleData, filmWidth, filmHeight);\n"
"	GenerateCameraPath(task, sample, sampleData, camera, filmWidth, filmHeight, &rays[gid], &seed);\n"
"\n"
"	// Save the seed\n"
"	task->seed.s1 = seed.s1;\n"
"	task->seed.s2 = seed.s2;\n"
"	task->seed.s3 = seed.s3;\n"
"\n"
"	__global GPUTaskStats *taskStat = &taskStats[gid];\n"
"	taskStat->sampleCount = 0;\n"
"}\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// AdvancePaths Kernel\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_HAS_ENVLIGHTS)\n"
"void DirectHitInfiniteLight(\n"
"		const bool firstPathVertex,\n"
"		const BSDFEvent lastBSDFEvent,\n"
"		const BSDFEvent pathBSDFEvent,\n"
"		__global const Spectrum *pathThroughput,\n"
"		const float3 eyeDir, const float lastPdfW,\n"
"		__global SampleResult *sampleResult\n"
"		LIGHTS_PARAM_DECL) {\n"
"	const float3 throughput = VLOAD3F(pathThroughput->c);\n"
"\n"
"	for (uint i = 0; i < envLightCount; ++i) {\n"
"		__global LightSource *light = &lights[envLightIndices[i]];\n"
"\n"
"		float directPdfW;\n"
"		const float3 lightRadiance = EnvLight_GetRadiance(light, eyeDir, &directPdfW\n"
"				LIGHTS_PARAM);\n"
"\n"
"		if (!Spectrum_IsBlack(lightRadiance)) {\n"
"			// MIS between BSDF sampling and direct light sampling\n"
"			const float lightPickProb = Scene_SampleAllLightPdf(lightsDistribution, light->lightSceneIndex);\n"
"			const float weight = ((lastBSDFEvent & SPECULAR) ? 1.f : PowerHeuristic(lastPdfW, directPdfW * lightPickProb));\n"
"			const float3 radiance = weight * throughput * lightRadiance;\n"
"\n"
"			const uint lightID = min(light->lightID, PARAM_FILM_RADIANCE_GROUP_COUNT - 1u);\n"
"			AddEmission(firstPathVertex, pathBSDFEvent, lightID, sampleResult, radiance);\n"
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
"	float directPdfA;\n"
"	const float3 emittedRadiance = BSDF_GetEmittedRadiance(bsdf, &directPdfA\n"
"			LIGHTS_PARAM);\n"
"\n"
"	if (!Spectrum_IsBlack(emittedRadiance)) {\n"
"		// Add emitted radiance\n"
"		float weight = 1.f;\n"
"		if (!(lastBSDFEvent & SPECULAR)) {\n"
"			const float lightPickProb = Scene_SampleAllLightPdf(lightsDistribution,\n"
"					lights[bsdf->triangleLightSourceIndex].lightSceneIndex);\n"
"			const float directPdfW = PdfAtoW(directPdfA, distance,\n"
"				fabs(dot(VLOAD3F(&bsdf->hitPoint.fixedDir.x), VLOAD3F(&bsdf->hitPoint.shadeN.x))));\n"
"\n"
"			// MIS between BSDF sampling and direct light sampling\n"
"			weight = PowerHeuristic(lastPdfW, directPdfW * lightPickProb);\n"
"		}\n"
"		const float3 lightRadiance = weight * VLOAD3F(pathThroughput->c) * emittedRadiance;\n"
"\n"
"		const uint lightID =  min(BSDF_GetLightID(bsdf\n"
"				MATERIALS_PARAM), PARAM_FILM_RADIANCE_GROUP_COUNT - 1u);\n"
"		AddEmission(firstPathVertex, pathBSDFEvent, lightID, sampleResult, lightRadiance);\n"
"	}\n"
"}\n"
"#endif\n"
"\n"
"float RussianRouletteProb(const float3 color) {\n"
"	return clamp(Spectrum_Filter(color), PARAM_RR_CAP, 1.f);\n"
"}\n"
"\n"
"bool DirectLightSampling(\n"
"		__global LightSource *light,\n"
"		const float lightPickPdf,\n"
"#if defined(PARAM_HAS_INFINITELIGHTS)\n"
"		const float worldCenterX,\n"
"		const float worldCenterY,\n"
"		const float worldCenterZ,\n"
"		const float worldRadius,\n"
"#endif\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"		__global HitPoint *tmpHitPoint,\n"
"#endif\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"		const float u3,\n"
"		__global float *shadowPassThrought,\n"
"#endif\n"
"		const float u0, const float u1, const float u2,\n"
"		const uint depth,\n"
"		__global const Spectrum *pathThroughput, __global BSDF *bsdf,\n"
"		__global Ray *shadowRay, __global Spectrum *radiance, __global uint *ID\n"
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
"#if defined(PARAM_HAS_INFINITELIGHTS)\n"
"			worldCenterX, worldCenterY, worldCenterZ, worldRadius,\n"
"#endif\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"			tmpHitPoint,\n"
"#endif		\n"
"			&lightRayDir, &distance, &directPdfW\n"
"			LIGHTS_PARAM);\n"
"\n"
"	// Setup the shadow ray\n"
"	if (!Spectrum_IsBlack(lightRadiance)) {\n"
"		BSDFEvent event;\n"
"		float bsdfPdfW;\n"
"		const float3 bsdfEval = BSDF_Evaluate(bsdf,\n"
"				lightRayDir, &event, &bsdfPdfW\n"
"				MATERIALS_PARAM);\n"
"\n"
"		if (!Spectrum_IsBlack(bsdfEval)) {\n"
"			const float cosThetaToLight = fabs(dot(lightRayDir, VLOAD3F(&bsdf->hitPoint.shadeN.x)));\n"
"			const float directLightSamplingPdfW = directPdfW * lightPickPdf;\n"
"			const float factor = 1.f / directLightSamplingPdfW;\n"
"\n"
"			// Russian Roulette\n"
"			bsdfPdfW *= (depth >= PARAM_RR_DEPTH) ? RussianRouletteProb(bsdfEval) : 1.f;\n"
"\n"
"			// MIS between direct light sampling and BSDF sampling\n"
"			const float weight = Light_IsEnvOrIntersectable(light) ?\n"
"				PowerHeuristic(directLightSamplingPdfW, bsdfPdfW) : 1.f;\n"
"\n"
"			VSTORE3F((weight * factor) * VLOAD3F(pathThroughput->c) * bsdfEval * lightRadiance, radiance->c);\n"
"			*ID = min(light->lightID, PARAM_FILM_RADIANCE_GROUP_COUNT - 1u);\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"			*shadowPassThrought = u3;\n"
"#endif\n"
"\n"
"			// Setup the shadow ray\n"
"			const float3 hitPoint = VLOAD3F(&bsdf->hitPoint.p.x);\n"
"			const float epsilon = fmax(MachineEpsilon_E_Float3(hitPoint), MachineEpsilon_E(distance));\n"
"\n"
"			Ray_Init4(shadowRay, hitPoint, lightRayDir,\n"
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
"#if defined(PARAM_HAS_INFINITELIGHTS)\n"
"		const float worldCenterX,\n"
"		const float worldCenterY,\n"
"		const float worldCenterZ,\n"
"		const float worldRadius,\n"
"#endif\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"		__global HitPoint *tmpHitPoint,\n"
"#endif\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"		const float u4,\n"
"		__global float *shadowPassThrought,\n"
"#endif\n"
"		const float u0, const float u1, const float u2, const float u3,\n"
"		const uint depth,\n"
"		__global const Spectrum *pathThroughput, __global BSDF *bsdf,\n"
"		__global Ray *shadowRay, __global Spectrum *radiance, __global uint *ID\n"
"		LIGHTS_PARAM_DECL) {\n"
"	// Pick a light source to sample\n"
"	float lightPickPdf;\n"
"	const uint lightIndex = Scene_SampleAllLights(lightsDistribution, u0, &lightPickPdf);\n"
"\n"
"	return DirectLightSampling(\n"
"		&lights[lightIndex],\n"
"		lightPickPdf,\n"
"#if defined(PARAM_HAS_INFINITELIGHTS)\n"
"		worldCenterX,\n"
"		worldCenterY,\n"
"		worldCenterZ,\n"
"		worldRadius,\n"
"#endif\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"		tmpHitPoint,\n"
"#endif\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"		u4,\n"
"		shadowPassThrought,\n"
"#endif\n"
"		u1, u2, u3,\n"
"		depth,\n"
"		pathThroughput, bsdf,\n"
"		shadowRay, radiance, ID\n"
"		LIGHTS_PARAM);\n"
"}\n"
"\n"
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void AdvancePaths(\n"
"		__global GPUTask *tasks,\n"
"		__global GPUTaskStats *taskStats,\n"
"		__global Sample *samples,\n"
"		__global float *samplesData,\n"
"		__global Ray *rays,\n"
"		__global RayHit *rayHits,\n"
"		// Film parameters\n"
"		const uint filmWidth, const uint filmHeight\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_0)\n"
"		, __global float *filmRadianceGroup0\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_1)\n"
"		, __global float *filmRadianceGroup1\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_2)\n"
"		, __global float *filmRadianceGroup2\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_3)\n"
"		, __global float *filmRadianceGroup3\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_4)\n"
"		, __global float *filmRadianceGroup4\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_5)\n"
"		, __global float *filmRadianceGroup5\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_6)\n"
"		, __global float *filmRadianceGroup6\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_7)\n"
"		, __global float *filmRadianceGroup7\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_ALPHA)\n"
"		, __global float *filmAlpha\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DEPTH)\n"
"		, __global float *filmDepth\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_POSITION)\n"
"		, __global float *filmPosition\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_GEOMETRY_NORMAL)\n"
"		, __global float *filmGeometryNormal\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_SHADING_NORMAL)\n"
"		, __global float *filmShadingNormal\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_MATERIAL_ID)\n"
"		, __global uint *filmMaterialID\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_DIFFUSE)\n"
"		, __global float *filmDirectDiffuse\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_GLOSSY)\n"
"		, __global float *filmDirectGlossy\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_EMISSION)\n"
"		, __global float *filmEmission\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_DIFFUSE)\n"
"		, __global float *filmIndirectDiffuse\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_GLOSSY)\n"
"		, __global float *filmIndirectGlossy\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SPECULAR)\n"
"		, __global float *filmIndirectSpecular\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_MATERIAL_ID_MASK)\n"
"		, __global float *filmMaterialIDMask\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_SHADOW_MASK)\n"
"		, __global float *filmDirectShadowMask\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SHADOW_MASK)\n"
"		, __global float *filmIndirectShadowMask\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_UV)\n"
"		, __global float *filmUV\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_RAYCOUNT)\n"
"		, __global float *filmRayCount\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_BY_MATERIAL_ID)\n"
"		, __global float *filmByMaterialID\n"
"#endif\n"
"		,\n"
"		// Scene parameters\n"
"#if defined(PARAM_HAS_INFINITELIGHTS)\n"
"		const float worldCenterX,\n"
"		const float worldCenterY,\n"
"		const float worldCenterZ,\n"
"		const float worldRadius,\n"
"#endif\n"
"		__global Material *mats,\n"
"		__global Texture *texs,\n"
"		__global uint *meshMats,\n"
"		__global Mesh *meshDescs,\n"
"		__global Point *vertices,\n"
"#if defined(PARAM_HAS_NORMALS_BUFFER)\n"
"		__global Vector *vertNormals,\n"
"#endif\n"
"#if defined(PARAM_HAS_UVS_BUFFER)\n"
"		__global UV *vertUVs,\n"
"#endif\n"
"#if defined(PARAM_HAS_COLS_BUFFER)\n"
"		__global Spectrum *vertCols,\n"
"#endif\n"
"#if defined(PARAM_HAS_ALPHAS_BUFFER)\n"
"		__global float *vertAlphas,\n"
"#endif\n"
"		__global Triangle *triangles,\n"
"		__global Camera *camera,\n"
"		// Lights\n"
"		__global LightSource *lights,\n"
"#if defined(PARAM_HAS_ENVLIGHTS)\n"
"		__global uint *envLightIndices,\n"
"		const uint envLightCount,\n"
"#endif\n"
"		__global uint *meshTriLightDefsOffset,\n"
"#if defined(PARAM_HAS_INFINITELIGHT)\n"
"		__global float *infiniteLightDistribution,\n"
"#endif\n"
"		__global float *lightsDistribution\n"
"		// Images\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_0)\n"
"		, __global ImageMap *imageMapDescs, __global float *imageMapBuff0\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_1)\n"
"		, __global float *imageMapBuff1\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_2)\n"
"		, __global float *imageMapBuff2\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_3)\n"
"		, __global float *imageMapBuff3\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_4)\n"
"		, __global float *imageMapBuff4\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_5)\n"
"		, __global float *imageMapBuff5\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_6)\n"
"		, __global float *imageMapBuff6\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_7)\n"
"		, __global float *imageMapBuff7\n"
"#endif\n"
"		) {\n"
"	const size_t gid = get_global_id(0);\n"
"	if (gid >= PARAM_TASK_COUNT)\n"
"		return;\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Advance the finite state machine step\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	__global GPUTask *task = &tasks[gid];\n"
"\n"
"	// Read the path state\n"
"	PathState pathState = task->pathStateBase.state;\n"
"	const uint depth = task->pathStateBase.depth;\n"
"	__global BSDF *bsdf = &task->pathStateBase.bsdf;\n"
"\n"
"	__global Sample *sample = &samples[gid];\n"
"	__global float *sampleData = Sampler_GetSampleData(sample, samplesData);\n"
"	__global float *sampleDataPathBase = Sampler_GetSampleDataPathBase(sample, sampleData);\n"
"#if (PARAM_SAMPLER_TYPE != 0)\n"
"	// Used by Sampler_GetSamplePathVertex() macro\n"
"	__global float *sampleDataPathVertexBase = Sampler_GetSampleDataPathVertex(\n"
"			sample, sampleDataPathBase, depth);\n"
"#endif\n"
"\n"
"	// Read the seed\n"
"	Seed seedValue;\n"
"	seedValue.s1 = task->seed.s1;\n"
"	seedValue.s2 = task->seed.s2;\n"
"	seedValue.s3 = task->seed.s3;\n"
"	// This trick is required by Sampler_GetSample() macro\n"
"	Seed *seed = &seedValue;\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Initialize image maps page pointer table\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_HAS_IMAGEMAPS)\n"
"	__global float *imageMapBuff[PARAM_IMAGEMAPS_COUNT];\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_0)\n"
"	imageMapBuff[0] = imageMapBuff0;\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_1)\n"
"	imageMapBuff[1] = imageMapBuff1;\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_2)\n"
"	imageMapBuff[2] = imageMapBuff2;\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_3)\n"
"	imageMapBuff[3] = imageMapBuff3;\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_4)\n"
"	imageMapBuff[4] = imageMapBuff4;\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_5)\n"
"	imageMapBuff[5] = imageMapBuff5;\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_6)\n"
"	imageMapBuff[6] = imageMapBuff6;\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_7)\n"
"	imageMapBuff[7] = imageMapBuff7;\n"
"#endif\n"
"#endif\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	__global Ray *ray = &rays[gid];\n"
"	__global RayHit *rayHit = &rayHits[gid];\n"
"	const bool rayMiss = (rayHit->meshIndex == NULL_INDEX);\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_RAYCOUNT)\n"
"	sample->result.rayCount += 1;\n"
"#endif\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Evaluation of the Path finite state machine.\n"
"	//\n"
"	// From: RT_NEXT_VERTEX\n"
"	// To: SPLAT_SAMPLE or GENERATE_DL_RAY\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	if (pathState == RT_NEXT_VERTEX) {\n"
"		if (!rayMiss) {\n"
"			//------------------------------------------------------------------\n"
"			// Something was hit\n"
"			//------------------------------------------------------------------\n"
"\n"
"			BSDF_Init(bsdf,\n"
"					meshDescs,\n"
"					meshMats,\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"					meshTriLightDefsOffset,\n"
"#endif\n"
"					vertices,\n"
"#if defined(PARAM_HAS_NORMALS_BUFFER)\n"
"					vertNormals,\n"
"#endif\n"
"#if defined(PARAM_HAS_UVS_BUFFER)\n"
"					vertUVs,\n"
"#endif\n"
"#if defined(PARAM_HAS_COLS_BUFFER)\n"
"					vertCols,\n"
"#endif\n"
"#if defined(PARAM_HAS_ALPHAS_BUFFER)\n"
"					vertAlphas,\n"
"#endif\n"
"					triangles, ray, rayHit\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"					, task->pathStateBase.bsdf.hitPoint.passThroughEvent\n"
"#endif\n"
"#if defined(PARAM_HAS_BUMPMAPS)\n"
"					MATERIALS_PARAM\n"
"#endif\n"
"					);\n"
"\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"			const float3 passThroughTrans = BSDF_GetPassThroughTransparency(bsdf\n"
"					MATERIALS_PARAM);\n"
"			if (!Spectrum_IsBlack(passThroughTrans)) {\n"
"				const float3 pathThroughput = VLOAD3F(task->pathStateBase.throughput.c) * passThroughTrans;\n"
"				VSTORE3F(pathThroughput, task->pathStateBase.throughput.c);\n"
"\n"
"				// It is a pass through point, continue to trace the ray\n"
"				ray->mint = rayHit->t + MachineEpsilon_E(rayHit->t);\n"
"\n"
"				// Keep the same path state\n"
"			} else\n"
"#endif\n"
"			{\n"
"				if (depth == 1) {\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_ALPHA)\n"
"					sample->result.alpha = 1.f;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DEPTH)\n"
"					sample->result.depth = rayHit->t;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_POSITION)\n"
"					sample->result.position = bsdf->hitPoint.p;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_GEOMETRY_NORMAL)\n"
"					sample->result.geometryNormal = bsdf->hitPoint.geometryN;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_SHADING_NORMAL)\n"
"					sample->result.shadingNormal = bsdf->hitPoint.shadeN;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_MATERIAL_ID)\n"
"					sample->result.materialID = BSDF_GetMaterialID(bsdf\n"
"							MATERIALS_PARAM);\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_UV)\n"
"					sample->result.uv = bsdf->hitPoint.uv;\n"
"#endif\n"
"				}\n"
"\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"				// Check if it is a light source (note: I can hit only triangle area light sources)\n"
"				if (BSDF_IsLightSource(bsdf)) {\n"
"					DirectHitFiniteLight((depth == 1),\n"
"							task->directLightState.lastBSDFEvent,\n"
"							task->directLightState.pathBSDFEvent,\n"
"							&task->pathStateBase.throughput,\n"
"							rayHit->t, bsdf, task->directLightState.lastPdfW,\n"
"							&sample->result\n"
"							LIGHTS_PARAM);\n"
"				}\n"
"#endif\n"
"\n"
"				// Direct light sampling\n"
"				pathState = GENERATE_DL_RAY;\n"
"			}\n"
"		} else {\n"
"			//------------------------------------------------------------------\n"
"			// Nothing was hit, add environmental lights radiance\n"
"			//------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_HAS_ENVLIGHTS)\n"
"			DirectHitInfiniteLight(\n"
"					(depth == 1),\n"
"					task->directLightState.lastBSDFEvent,\n"
"					task->directLightState.pathBSDFEvent,\n"
"					&task->pathStateBase.throughput,\n"
"					-VLOAD3F(&ray->d.x), task->directLightState.lastPdfW,\n"
"					&sample->result\n"
"					LIGHTS_PARAM);\n"
"#endif\n"
"\n"
"			if (depth == 1) {\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_ALPHA)\n"
"				sample->result.alpha = 0.f;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DEPTH)\n"
"				sample->result.depth = INFINITY;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_POSITION)\n"
"				sample->result.position.x = INFINITY;\n"
"				sample->result.position.y = INFINITY;\n"
"				sample->result.position.z = INFINITY;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_GEOMETRY_NORMAL)\n"
"				sample->result.geometryNormal.x = INFINITY;\n"
"				sample->result.geometryNormal.y = INFINITY;\n"
"				sample->result.geometryNormal.z = INFINITY;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_SHADING_NORMAL)\n"
"				sample->result.shadingNormal.x = INFINITY;\n"
"				sample->result.shadingNormal.y = INFINITY;\n"
"				sample->result.shadingNormal.z = INFINITY;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_MATERIAL_ID)\n"
"				sample->result.materialID = NULL_INDEX;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_SHADOW_MASK)\n"
"				sample->result.directShadowMask = 0.f;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SHADOW_MASK)\n"
"				sample->result.indirectShadowMask = 0.f;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_UV)\n"
"				sample->result.uv.u = INFINITY;\n"
"				sample->result.uv.v = INFINITY;\n"
"#endif\n"
"			}\n"
"\n"
"			pathState = SPLAT_SAMPLE;\n"
"		}\n"
"	}\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Evaluation of the Path finite state machine.\n"
"	//\n"
"	// From: RT_DL\n"
"	// To: GENERATE_NEXT_VERTEX_RAY\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	if (pathState == RT_DL) {\n"
"		pathState = GENERATE_NEXT_VERTEX_RAY;\n"
"\n"
"		if (rayMiss) {\n"
"			// Nothing was hit, the light source is visible\n"
"			const float3 lightRadiance = VLOAD3F(task->directLightState.lightRadiance.c);\n"
"\n"
"			const uint lightID = task->directLightState.lightID;\n"
"			VADD3F(sample->result.radiancePerPixelNormalized[lightID].c, lightRadiance);\n"
"\n"
"			if (depth == 1) {\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_SHADOW_MASK)\n"
"				sample->result.directShadowMask = 0.f;\n"
"#endif\n"
"				if (BSDF_GetEventTypes(bsdf\n"
"						MATERIALS_PARAM) & DIFFUSE) {\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_DIFFUSE)\n"
"					VADD3F(sample->result.directDiffuse.c, lightRadiance);\n"
"#endif\n"
"				} else {\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_GLOSSY)\n"
"					VADD3F(sample->result.directGlossy.c, lightRadiance);\n"
"#endif\n"
"				}\n"
"			} else {\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SHADOW_MASK)\n"
"				sample->result.indirectShadowMask = 0.f;\n"
"#endif\n"
"\n"
"				const BSDFEvent pathBSDFEvent = task->directLightState.pathBSDFEvent;\n"
"				if (pathBSDFEvent & DIFFUSE) {\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_DIFFUSE)\n"
"					VADD3F(sample->result.indirectDiffuse.c, lightRadiance);\n"
"#endif\n"
"				} else if (pathBSDFEvent & GLOSSY) {\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_GLOSSY)\n"
"					VADD3F(sample->result.indirectGlossy.c, lightRadiance);\n"
"#endif\n"
"				} else if (pathBSDFEvent & SPECULAR) {\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SPECULAR)\n"
"					VADD3F(sample->result.indirectSpecular.c, lightRadiance);\n"
"#endif\n"
"				}\n"
"			}\n"
"		}\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"		else {\n"
"			BSDF_Init(&task->passThroughState.passThroughBsdf,\n"
"					meshDescs,\n"
"					meshMats,\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"					meshTriLightDefsOffset,\n"
"#endif\n"
"					vertices,\n"
"#if defined(PARAM_HAS_NORMALS_BUFFER)\n"
"					vertNormals,\n"
"#endif\n"
"#if defined(PARAM_HAS_UVS_BUFFER)\n"
"					vertUVs,\n"
"#endif\n"
"#if defined(PARAM_HAS_COLS_BUFFER)\n"
"					vertCols,\n"
"#endif\n"
"#if defined(PARAM_HAS_ALPHAS_BUFFER)\n"
"					vertAlphas,\n"
"#endif\n"
"					triangles, ray, rayHit,\n"
"					task->passThroughState.passThroughEvent\n"
"#if defined(PARAM_HAS_BUMPMAPS)\n"
"					MATERIALS_PARAM\n"
"#endif\n"
"					);\n"
"\n"
"			const float3 passthroughTrans = BSDF_GetPassThroughTransparency(&task->passThroughState.passThroughBsdf\n"
"					MATERIALS_PARAM);\n"
"			if (!Spectrum_IsBlack(passthroughTrans)) {\n"
"				const float3 lightRadiance = VLOAD3F(task->directLightState.lightRadiance.c) * passthroughTrans;\n"
"				VSTORE3F(lightRadiance, task->directLightState.lightRadiance.c);\n"
"\n"
"				// It is a pass through point, continue to trace the ray\n"
"				ray->mint = rayHit->t + MachineEpsilon_E(rayHit->t);\n"
"				pathState = RT_DL;\n"
"			}\n"
"		}\n"
"#endif\n"
"	}\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Evaluation of the Path finite state machine.\n"
"	//\n"
"	// From: GENERATE_DL_RAY\n"
"	// To: GENERATE_NEXT_VERTEX_RAY or RT_DL or SPLAT_SAMPLE\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	if (pathState == GENERATE_DL_RAY) {\n"
"		if (depth > PARAM_MAX_PATH_DEPTH) {\n"
"			pathState = SPLAT_SAMPLE;\n"
"		} else {\n"
"			// No shadow ray to trace, move to the next vertex ray\n"
"			pathState = GENERATE_NEXT_VERTEX_RAY;\n"
"\n"
"			if (BSDF_IsDelta(bsdf\n"
"				MATERIALS_PARAM)) {\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_SHADOW_MASK)\n"
"				if (depth == 1)\n"
"					sample->result.directShadowMask = 0.f;\n"
"#endif\n"
"			} else {\n"
"				if (DirectLightSampling_ONE(\n"
"#if defined(PARAM_HAS_INFINITELIGHTS)\n"
"						worldCenterX, worldCenterY, worldCenterZ, worldRadius,\n"
"#endif\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"						&task->directLightState.tmpHitPoint,\n"
"#endif\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"						Sampler_GetSamplePathVertex(depth, IDX_DIRECTLIGHT_A),\n"
"						&task->passThroughState.passThroughEvent,\n"
"#endif\n"
"						Sampler_GetSamplePathVertex(depth, IDX_DIRECTLIGHT_X),\n"
"						Sampler_GetSamplePathVertex(depth, IDX_DIRECTLIGHT_Y),\n"
"						Sampler_GetSamplePathVertex(depth, IDX_DIRECTLIGHT_Z),\n"
"						Sampler_GetSamplePathVertex(depth, IDX_DIRECTLIGHT_W),\n"
"						depth, &task->pathStateBase.throughput, bsdf,\n"
"						ray, &task->directLightState.lightRadiance, &task->directLightState.lightID\n"
"						LIGHTS_PARAM)) {\n"
"					// I have to trace the shadow ray\n"
"					pathState = RT_DL;\n"
"				}\n"
"			}\n"
"		}\n"
"	}\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Evaluation of the Path finite state machine.\n"
"	//\n"
"	// From: GENERATE_NEXT_VERTEX_RAY\n"
"	// To: SPLAT_SAMPLE or RT_NEXT_VERTEX\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	if (pathState == GENERATE_NEXT_VERTEX_RAY) {\n"
"		// Sample the BSDF\n"
"		__global BSDF *bsdf = &task->pathStateBase.bsdf;\n"
"		float3 sampledDir;\n"
"		float lastPdfW;\n"
"		float cosSampledDir;\n"
"		BSDFEvent event;\n"
"\n"
"		const float3 bsdfSample = BSDF_Sample(bsdf,\n"
"				Sampler_GetSamplePathVertex(depth, IDX_BSDF_X),\n"
"				Sampler_GetSamplePathVertex(depth, IDX_BSDF_Y),\n"
"				&sampledDir, &lastPdfW, &cosSampledDir, &event, ALL\n"
"				MATERIALS_PARAM);\n"
"\n"
"		// Russian Roulette\n"
"		const float rrProb = RussianRouletteProb(bsdfSample);\n"
"		const bool rrEnabled = (depth >= PARAM_RR_DEPTH) && !(event & SPECULAR);\n"
"		const bool rrContinuePath = !rrEnabled || (Sampler_GetSamplePathVertex(depth, IDX_RR) < rrProb);\n"
"\n"
"		const bool continuePath = !Spectrum_IsBlack(bsdfSample) && rrContinuePath;\n"
"		if (continuePath) {\n"
"			float3 throughput = VLOAD3F(task->pathStateBase.throughput.c);\n"
"			throughput *= bsdfSample;\n"
"			if (rrEnabled)\n"
"				throughput /= rrProb; // Russian Roulette\n"
"\n"
"			VSTORE3F(throughput, task->pathStateBase.throughput.c);\n"
"\n"
"			Ray_Init2(ray, VLOAD3F(&bsdf->hitPoint.p.x), sampledDir);\n"
"\n"
"			task->pathStateBase.depth = depth + 1;\n"
"			if (depth == 1)\n"
"				task->directLightState.pathBSDFEvent = event;\n"
"			task->directLightState.lastBSDFEvent = event;\n"
"			task->directLightState.lastPdfW = lastPdfW;\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"			// This is a bit tricky. I store the passThroughEvent in the BSDF\n"
"			// before of the initialization because it can be use during the\n"
"			// tracing of next path vertex ray.\n"
"\n"
"			// This sampleDataPathVertexBase is used inside Sampler_GetSamplePathVertex() macro\n"
"			__global float *sampleDataPathVertexBase = Sampler_GetSampleDataPathVertex(\n"
"				sample, sampleDataPathBase, depth + 1);\n"
"			task->pathStateBase.bsdf.hitPoint.passThroughEvent = Sampler_GetSamplePathVertex(depth + 1, IDX_PASSTHROUGH);\n"
"#endif\n"
"			pathState = RT_NEXT_VERTEX;\n"
"		} else\n"
"			pathState = SPLAT_SAMPLE;\n"
"	}\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Evaluation of the Path finite state machine.\n"
"	//\n"
"	// From: SPLAT_SAMPLE\n"
"	// To: RT_NEXT_VERTEX\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	if (pathState == SPLAT_SAMPLE) {	\n"
"		// Initialize Film radiance group pointer table\n"
"		__global float *filmRadianceGroup[PARAM_FILM_RADIANCE_GROUP_COUNT];\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_0)\n"
"		filmRadianceGroup[0] = filmRadianceGroup0;\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_1)\n"
"		filmRadianceGroup[1] = filmRadianceGroup1;\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_2)\n"
"		filmRadianceGroup[2] = filmRadianceGroup2;\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_3)\n"
"		filmRadianceGroup[3] = filmRadianceGroup3;\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_4)\n"
"		filmRadianceGroup[3] = filmRadianceGroup4;\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_5)\n"
"		filmRadianceGroup[3] = filmRadianceGroup5;\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_6)\n"
"		filmRadianceGroup[3] = filmRadianceGroup6;\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_7)\n"
"		filmRadianceGroup[3] = filmRadianceGroup7;\n"
"#endif\n"
"\n"
"		Sampler_NextSample(seed, sample, sampleData\n"
"				FILM_PARAM);\n"
"		taskStats[gid].sampleCount += 1;\n"
"\n"
"		GenerateCameraPath(task, sample, sampleData, camera, filmWidth, filmHeight, ray, seed);\n"
"		// task->pathStateBase.state is set to RT_NEXT_VERTEX inside Sampler_NextSample() => GenerateCameraPath()\n"
"	} else {\n"
"		// Save the state\n"
"		task->pathStateBase.state = pathState;\n"
"	}\n"
"		\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	// Save the seed\n"
"	task->seed.s1 = seed->s1;\n"
"	task->seed.s2 = seed->s2;\n"
"	task->seed.s3 = seed->s3;\n"
"}\n"
; } }
