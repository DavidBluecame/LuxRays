#include <string>
namespace slg { namespace ocl {
std::string KernelSource_biaspathocl_kernels_micro = 
"#line 2 \"biaspatchocl_kernels_micro.cl\"\n"
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
"// List of symbols defined at compile time:\n"
"//  PARAM_TASK_COUNT\n"
"//  PARAM_TILE_WIDTH\n"
"//  PARAM_TILE_HEIGHT\n"
"//  PARAM_FIRST_VERTEX_DL_COUNT\n"
"//  PARAM_RADIANCE_CLAMP_MAXVALUE\n"
"//  PARAM_PDF_CLAMP_VALUE\n"
"//  PARAM_AA_SAMPLES\n"
"//  PARAM_DIRECT_LIGHT_SAMPLES\n"
"//  PARAM_DIFFUSE_SAMPLES\n"
"//  PARAM_GLOSSY_SAMPLES\n"
"//  PARAM_SPECULAR_SAMPLES\n"
"//  PARAM_DEPTH_MAX\n"
"//  PARAM_DEPTH_DIFFUSE_MAX\n"
"//  PARAM_DEPTH_GLOSSY_MAX\n"
"//  PARAM_DEPTH_SPECULAR_MAX\n"
"//  PARAM_IMAGE_FILTER_WIDTH\n"
"//  PARAM_LOW_LIGHT_THREASHOLD\n"
"//  PARAM_NEAR_START_LIGHT\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// RenderSample (Micro-Kernels)\n"
"//------------------------------------------------------------------------------\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// RenderSample_MK_GENERATE_CAMERA_RAY\n"
"//------------------------------------------------------------------------------\n"
"\n"
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void RenderSample_MK_GENERATE_CAMERA_RAY(\n"
"		const uint tileStartX,\n"
"		const uint tileStartY,\n"
"		KERNEL_ARGS\n"
"		) {\n"
"	const size_t gid = get_global_id(0);\n"
"	__global GPUTask *task = &tasks[gid];\n"
"\n"
"	const uint sampleIndex = gid % (PARAM_AA_SAMPLES * PARAM_AA_SAMPLES);\n"
"	const uint samplePixelIndex = gid / (PARAM_AA_SAMPLES * PARAM_AA_SAMPLES);\n"
"	const uint samplePixelX = samplePixelIndex % PARAM_TILE_WIDTH;\n"
"	const uint samplePixelY = samplePixelIndex / PARAM_TILE_WIDTH;\n"
"\n"
"	if ((gid >= PARAM_TILE_WIDTH * PARAM_TILE_HEIGHT * PARAM_AA_SAMPLES * PARAM_AA_SAMPLES) ||\n"
"			(tileStartX + samplePixelX >= engineFilmWidth) ||\n"
"			(tileStartY + samplePixelY >= engineFilmHeight)) {\n"
"		task->pathState = MK_DONE;\n"
"		return;\n"
"	}\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Initialize the eye ray\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	// Read the seed\n"
"	Seed seed;\n"
"	seed.s1 = task->seed.s1;\n"
"	seed.s2 = task->seed.s2;\n"
"	seed.s3 = task->seed.s3;\n"
"\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"	// Initialize the volume information\n"
"	PathVolumeInfo_Init(&task->volInfoPathVertex1);\n"
"#endif\n"
"	\n"
"	__global SampleResult *sampleResult = &taskResults[gid];\n"
"	SampleResult_Init(sampleResult);\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_SHADOW_MASK)\n"
"	sampleResult->directShadowMask = 1.f;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SHADOW_MASK)\n"
"	sampleResult->indirectShadowMask = 1.f;\n"
"#endif\n"
"\n"
"	Ray ray;\n"
"	GenerateCameraRay(&seed, task, sampleResult,\n"
"			camera, pixelFilterDistribution,\n"
"			samplePixelX, samplePixelY, sampleIndex,\n"
"			tileStartX, tileStartY,\n"
"			engineFilmWidth, engineFilmHeight, &ray);\n"
"\n"
"	task->currentTime = ray.time;\n"
"	VSTORE3F(WHITE, task->throughputPathVertex1.c);\n"
"	task->tmpRay = ray;\n"
"\n"
"	task->pathState = MK_TRACE_EYE_RAY;\n"
"\n"
"	// Save the seed\n"
"	task->seed.s1 = seed.s1;\n"
"	task->seed.s2 = seed.s2;\n"
"	task->seed.s3 = seed.s3;\n"
"}\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// RenderSample_MK_TRACE_EYE_RAY\n"
"//------------------------------------------------------------------------------\n"
"\n"
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void RenderSample_MK_TRACE_EYE_RAY(\n"
"		KERNEL_ARGS\n"
"		) {\n"
"	const size_t gid = get_global_id(0);\n"
"\n"
"	__global GPUTask *task = &tasks[gid];\n"
"	if (task->pathState != MK_TRACE_EYE_RAY)\n"
"		return;\n"
"\n"
"	// Initialize image maps page pointer table\n"
"	INIT_IMAGEMAPS_PAGES\n"
"\n"
"	// Read the seed\n"
"	Seed seed;\n"
"	seed.s1 = task->seed.s1;\n"
"	seed.s2 = task->seed.s2;\n"
"	seed.s3 = task->seed.s3;\n"
"\n"
"	float3 throughputPathVertex1 = VLOAD3F(task->throughputPathVertex1.c);\n"
"	\n"
"	__global SampleResult *sampleResult = &taskResults[gid];\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Trace the ray\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	Ray ray = task->tmpRay;\n"
"	RayHit rayHit;\n"
"\n"
"	float3 connectionThroughput;\n"
"	taskStats[gid].raysCount += BIASPATHOCL_Scene_Intersect(\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"		&task->volInfoPathVertex1,\n"
"		&task->tmpHitPoint,\n"
"#endif\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"		Rnd_FloatValue(&seed),\n"
"#endif\n"
"		&ray, &rayHit,\n"
"		&task->bsdfPathVertex1,\n"
"		&connectionThroughput, throughputPathVertex1,\n"
"		sampleResult,\n"
"		// BSDF_Init parameters\n"
"		meshDescs,\n"
"		meshMats,\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"		meshTriLightDefsOffset,\n"
"#endif\n"
"		vertices,\n"
"#if defined(PARAM_HAS_NORMALS_BUFFER)\n"
"		vertNormals,\n"
"#endif\n"
"#if defined(PARAM_HAS_UVS_BUFFER)\n"
"		vertUVs,\n"
"#endif\n"
"#if defined(PARAM_HAS_COLS_BUFFER)\n"
"		vertCols,\n"
"#endif\n"
"#if defined(PARAM_HAS_ALPHAS_BUFFER)\n"
"		vertAlphas,\n"
"#endif\n"
"		triangles\n"
"		MATERIALS_PARAM\n"
"		// Accelerator_Intersect parameters\n"
"		ACCELERATOR_INTERSECT_PARAM);\n"
"	throughputPathVertex1 *= connectionThroughput;\n"
"\n"
"	VSTORE3F(throughputPathVertex1, task->throughputPathVertex1.c);\n"
"	task->tmpRayHit = rayHit;\n"
"\n"
"	task->pathState = MK_ILLUMINATE_EYE_MISS;\n"
"\n"
"	// Save the seed\n"
"	task->seed.s1 = seed.s1;\n"
"	task->seed.s2 = seed.s2;\n"
"	task->seed.s3 = seed.s3;\n"
"}\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// RenderSample_MK_ILLUMINATE_EYE_MISS\n"
"//------------------------------------------------------------------------------\n"
"\n"
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void RenderSample_MK_ILLUMINATE_EYE_MISS(\n"
"		KERNEL_ARGS\n"
"		) {\n"
"	const size_t gid = get_global_id(0);\n"
"\n"
"	__global GPUTask *task = &tasks[gid];\n"
"	if (task->pathState != MK_ILLUMINATE_EYE_MISS)\n"
"		return;\n"
"\n"
"	// Initialize image maps page pointer table\n"
"	INIT_IMAGEMAPS_PAGES\n"
"\n"
"	__global SampleResult *sampleResult = &taskResults[gid];\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Render the eye ray miss\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	const RayHit rayHit = task->tmpRayHit;\n"
"	if (rayHit.meshIndex == NULL_INDEX) {\n"
"		// Nothing was hit\n"
"\n"
"#if defined(PARAM_HAS_ENVLIGHTS)\n"
"		const Ray ray = task->tmpRay;\n"
"\n"
"		// Add environmental lights radiance\n"
"		const float3 rayDir = (float3)(ray.d.x, ray.d.y, ray.d.z);\n"
"		// SPECULAR is required to avoid MIS\n"
"		DirectHitInfiniteLight(\n"
"				SPECULAR,\n"
"				VLOAD3F(task->throughputPathVertex1.c),\n"
"				-rayDir, 1.f,\n"
"				sampleResult\n"
"				LIGHTS_PARAM);\n"
"#endif\n"
"\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_ALPHA)\n"
"		sampleResult->alpha = 0.f;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DEPTH)\n"
"		sampleResult->depth = INFINITY;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_POSITION)\n"
"		sampleResult->position.x = INFINITY;\n"
"		sampleResult->position.y = INFINITY;\n"
"		sampleResult->position.z = INFINITY;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_GEOMETRY_NORMAL)\n"
"		sampleResult->geometryNormal.x = INFINITY;\n"
"		sampleResult->geometryNormal.y = INFINITY;\n"
"		sampleResult->geometryNormal.z = INFINITY;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_SHADING_NORMAL)\n"
"		sampleResult->shadingNormal.x = INFINITY;\n"
"		sampleResult->shadingNormal.y = INFINITY;\n"
"		sampleResult->shadingNormal.z = INFINITY;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_MATERIAL_ID)\n"
"		sampleResult->materialID = NULL_INDEX;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_UV)\n"
"		sampleResult->uv.u = INFINITY;\n"
"		sampleResult->uv.v = INFINITY;\n"
"#endif\n"
"\n"
"		task->pathState = MK_DONE;\n"
"	} else\n"
"		task->pathState = MK_ILLUMINATE_EYE_HIT;\n"
"}\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// RenderSample_MK_ILLUMINATE_EYE_HIT\n"
"//------------------------------------------------------------------------------\n"
"\n"
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void RenderSample_MK_ILLUMINATE_EYE_HIT(\n"
"		KERNEL_ARGS\n"
"		) {\n"
"	const size_t gid = get_global_id(0);\n"
"\n"
"	__global GPUTask *task = &tasks[gid];\n"
"	if (task->pathState != MK_ILLUMINATE_EYE_HIT)\n"
"		return;\n"
"\n"
"	// Initialize image maps page pointer table\n"
"	INIT_IMAGEMAPS_PAGES\n"
"\n"
"	__global SampleResult *sampleResult = &taskResults[gid];\n"
"\n"
"	//----------------------------------------------------------------------\n"
"	// Something was hit\n"
"	//----------------------------------------------------------------------\n"
"\n"
"	const float rayHitT = task->tmpRayHit.t;\n"
"\n"
"	// Save the path first vertex information\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_ALPHA)\n"
"	sampleResult->alpha = 1.f;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DEPTH)\n"
"	sampleResult->depth = rayHitT;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_POSITION)\n"
"	sampleResult->position = task->bsdfPathVertex1.hitPoint.p;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_GEOMETRY_NORMAL)\n"
"	sampleResult->geometryNormal = task->bsdfPathVertex1.hitPoint.geometryN;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_SHADING_NORMAL)\n"
"	sampleResult->shadingNormal = task->bsdfPathVertex1.hitPoint.shadeN;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_MATERIAL_ID)\n"
"	sampleResult->materialID = BSDF_GetMaterialID(&task->bsdfPathVertex1\n"
"		MATERIALS_PARAM);\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_UV)\n"
"	sampleResult->uv = task->bsdfPathVertex1.hitPoint.uv;\n"
"#endif\n"
"\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"	// Check if it is a light source (note: I can hit only triangle area light sources)\n"
"	if (BSDF_IsLightSource(&task->bsdfPathVertex1) && (rayHitT > PARAM_NEAR_START_LIGHT)) {\n"
"		// SPECULAR is required to avoid MIS\n"
"		DirectHitFiniteLight(SPECULAR,\n"
"				VLOAD3F(task->throughputPathVertex1.c),\n"
"				rayHitT, &task->bsdfPathVertex1, 1.f,\n"
"				sampleResult\n"
"				LIGHTS_PARAM);\n"
"	}\n"
"#endif\n"
"\n"
"	task->pathState = MK_DL_VERTEX_1;\n"
"}\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// RenderSample_MK_DL_VERTEX_1\n"
"//------------------------------------------------------------------------------\n"
"\n"
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void RenderSample_MK_DL_VERTEX_1(\n"
"		KERNEL_ARGS\n"
"		) {\n"
"	const size_t gid = get_global_id(0);\n"
"\n"
"	__global GPUTask *task = &tasks[gid];\n"
"	if (task->pathState != MK_DL_VERTEX_1)\n"
"		return;\n"
"\n"
"	// Initialize image maps page pointer table\n"
"	INIT_IMAGEMAPS_PAGES\n"
"\n"
"	__global SampleResult *sampleResult = &taskResults[gid];\n"
"\n"
"	//----------------------------------------------------------------------\n"
"	// First path vertex direct light sampling\n"
"	//----------------------------------------------------------------------\n"
"\n"
"	const BSDFEvent materialEventTypes = BSDF_GetEventTypes(&task->bsdfPathVertex1\n"
"		MATERIALS_PARAM);\n"
"	sampleResult->lastPathVertex = \n"
"			(PARAM_DEPTH_MAX <= 1) ||\n"
"			(!((PARAM_DEPTH_DIFFUSE_MAX > 0) && (materialEventTypes & DIFFUSE)) &&\n"
"			!((PARAM_DEPTH_GLOSSY_MAX > 0) && (materialEventTypes & GLOSSY)) &&\n"
"			!((PARAM_DEPTH_SPECULAR_MAX > 0) && (materialEventTypes & SPECULAR)));\n"
"	task->materialEventTypesPathVertex1 = materialEventTypes;\n"
"\n"
"	// Only if it is not a SPECULAR BSDF\n"
"	if (!BSDF_IsDelta(&task->bsdfPathVertex1\n"
"			MATERIALS_PARAM)) {\n"
"		// Read the seed\n"
"		Seed seed;\n"
"		seed.s1 = task->seed.s1;\n"
"		seed.s2 = task->seed.s2;\n"
"		seed.s3 = task->seed.s3;\n"
"\n"
"		__global GPUTaskDirectLight *taskDirectLight = &tasksDirectLight[gid];\n"
"\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"		// I need to work on a copy of volume information of the first path vertex\n"
"		taskDirectLight->directLightVolInfo = task->volInfoPathVertex1;\n"
"#endif\n"
"\n"
"		taskStats[gid].raysCount +=\n"
"			DirectLightSampling_ALL(\n"
"			&seed,\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"			&taskDirectLight->directLightVolInfo,\n"
"#endif\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0) || defined(PARAM_HAS_VOLUMES)\n"
"			&task->tmpHitPoint,\n"
"#endif\n"
"#if defined(PARAM_HAS_INFINITELIGHTS)\n"
"			worldCenterX, worldCenterY, worldCenterZ, worldRadius,\n"
"#endif\n"
"			task->currentTime,\n"
"			VLOAD3F(task->throughputPathVertex1.c),\n"
"			&task->bsdfPathVertex1, &taskDirectLight->directLightBSDF,\n"
"			sampleResult,\n"
"			// BSDF_Init parameters\n"
"			meshDescs,\n"
"			meshMats,\n"
"			vertices,\n"
"#if defined(PARAM_HAS_NORMALS_BUFFER)\n"
"			vertNormals,\n"
"#endif\n"
"#if defined(PARAM_HAS_UVS_BUFFER)\n"
"			vertUVs,\n"
"#endif\n"
"#if defined(PARAM_HAS_COLS_BUFFER)\n"
"			vertCols,\n"
"#endif\n"
"#if defined(PARAM_HAS_ALPHAS_BUFFER)\n"
"			vertAlphas,\n"
"#endif\n"
"			triangles\n"
"			// Accelerator_Intersect parameters\n"
"			ACCELERATOR_INTERSECT_PARAM\n"
"			// Light related parameters\n"
"			LIGHTS_PARAM);\n"
"\n"
"		// Save the seed\n"
"		task->seed.s1 = seed.s1;\n"
"		task->seed.s2 = seed.s2;\n"
"		task->seed.s3 = seed.s3;\n"
"	}\n"
"\n"
"	task->pathState = MK_BSDF_SAMPLE;\n"
"\n"
"	sampleResult->firstPathVertex = false;\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SHADOW_MASK)\n"
"	sampleResult->indirectShadowMask = 0.f;\n"
"#endif\n"
"}\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// RenderSample_MK_BSDF_SAMPLE\n"
"//------------------------------------------------------------------------------\n"
"\n"
"void RenderSample_MK_BSDF_SAMPLE(\n"
"		const BSDFEvent vertex1SampleComponent,\n"
"		KERNEL_ARGS\n"
"		) {\n"
"	const size_t gid = get_global_id(0);\n"
"\n"
"	__global GPUTask *task = &tasks[gid];\n"
"	__global SampleResult *sampleResult = &taskResults[gid];\n"
"	if ((task->pathState != MK_BSDF_SAMPLE) ||\n"
"			(sampleResult->lastPathVertex) ||\n"
"			!(task->materialEventTypesPathVertex1 & vertex1SampleComponent) ||\n"
"			!PathDepthInfo_CheckComponentDepths(vertex1SampleComponent))\n"
"		return;\n"
"\n"
"	// Read the seed\n"
"	Seed seed;\n"
"	seed.s1 = task->seed.s1;\n"
"	seed.s2 = task->seed.s2;\n"
"	seed.s3 = task->seed.s3;\n"
"\n"
"	// Initialize image maps page pointer table\n"
"	INIT_IMAGEMAPS_PAGES\n"
"\n"
"	__global GPUTaskDirectLight *taskDirectLight = &tasksDirectLight[gid];\n"
"	__global GPUTaskPathVertexN *taskPathVertexN = &tasksPathVertexN[gid];\n"
"\n"
"	//----------------------------------------------------------------------\n"
"	// Sample the components of the BSDF of the first path vertex\n"
"	//----------------------------------------------------------------------\n"
"\n"
"	const int matSamplesCount = mats[task->bsdfPathVertex1.materialIndex].samples;\n"
"	const uint globalMatSamplesCount = ((vertex1SampleComponent == DIFFUSE) ? PARAM_DIFFUSE_SAMPLES :\n"
"		((vertex1SampleComponent == GLOSSY) ? PARAM_GLOSSY_SAMPLES :\n"
"			PARAM_SPECULAR_SAMPLES));\n"
"	const uint sampleCount = (matSamplesCount < 0) ? globalMatSamplesCount : (uint)matSamplesCount;\n"
"\n"
"	taskStats[gid].raysCount += SampleComponent(\n"
"			&seed,\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"			&task->volInfoPathVertex1,\n"
"			&taskPathVertexN->volInfoPathVertexN,\n"
"			&taskDirectLight->directLightVolInfo,\n"
"#endif\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0) || defined(PARAM_HAS_VOLUMES)\n"
"			&task->tmpHitPoint,\n"
"#endif\n"
"#if defined(PARAM_HAS_INFINITELIGHTS)\n"
"			worldCenterX, worldCenterY, worldCenterZ, worldRadius,\n"
"#endif\n"
"			task->currentTime,\n"
"			vertex1SampleComponent,\n"
"			sampleCount, VLOAD3F(task->throughputPathVertex1.c),\n"
"			&task->bsdfPathVertex1, &taskPathVertexN->bsdfPathVertexN,\n"
"			&taskDirectLight->directLightBSDF,\n"
"			sampleResult,\n"
"			// BSDF_Init parameters\n"
"			meshDescs,\n"
"			meshMats,\n"
"			vertices,\n"
"#if defined(PARAM_HAS_NORMALS_BUFFER)\n"
"			vertNormals,\n"
"#endif\n"
"#if defined(PARAM_HAS_UVS_BUFFER)\n"
"			vertUVs,\n"
"#endif\n"
"#if defined(PARAM_HAS_COLS_BUFFER)\n"
"			vertCols,\n"
"#endif\n"
"#if defined(PARAM_HAS_ALPHAS_BUFFER)\n"
"			vertAlphas,\n"
"#endif\n"
"			triangles\n"
"			// Accelerator_Intersect parameters\n"
"			ACCELERATOR_INTERSECT_PARAM\n"
"			// Light related parameters\n"
"			LIGHTS_PARAM);\n"
"\n"
"	// Save the seed\n"
"	task->seed.s1 = seed.s1;\n"
"	task->seed.s2 = seed.s2;\n"
"	task->seed.s3 = seed.s3;\n"
"}\n"
"\n"
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void RenderSample_MK_BSDF_SAMPLE_DIFFUSE(\n"
"		KERNEL_ARGS\n"
"		) {\n"
"	RenderSample_MK_BSDF_SAMPLE(\n"
"			DIFFUSE,\n"
"			engineFilmWidth,engineFilmHeight,\n"
"			tasks,\n"
"			tasksDirectLight,\n"
"			tasksPathVertexN,\n"
"			taskStats,\n"
"			taskResults,\n"
"			pixelFilterDistribution,\n"
"			// Film parameters\n"
"			filmWidth, filmHeight\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_0)\n"
"			, filmRadianceGroup0\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_1)\n"
"			, filmRadianceGroup1\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_2)\n"
"			, filmRadianceGroup2\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_3)\n"
"			, filmRadianceGroup3\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_4)\n"
"			, filmRadianceGroup4\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_5)\n"
"			, filmRadianceGroup5\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_6)\n"
"			, filmRadianceGroup6\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_7)\n"
"			, filmRadianceGroup7\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_ALPHA)\n"
"			, filmAlpha\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DEPTH)\n"
"			, filmDepth\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_POSITION)\n"
"			, filmPosition\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_GEOMETRY_NORMAL)\n"
"			, filmGeometryNormal\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_SHADING_NORMAL)\n"
"			, filmShadingNormal\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_MATERIAL_ID)\n"
"			, filmMaterialID\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_DIFFUSE)\n"
"			, filmDirectDiffuse\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_GLOSSY)\n"
"			, filmDirectGlossy\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_EMISSION)\n"
"			, filmEmission\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_DIFFUSE)\n"
"			, filmIndirectDiffuse\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_GLOSSY)\n"
"			, filmIndirectGlossy\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SPECULAR)\n"
"			, filmIndirectSpecular\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_MATERIAL_ID_MASK)\n"
"			, filmMaterialIDMask\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_SHADOW_MASK)\n"
"			, filmDirectShadowMask\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SHADOW_MASK)\n"
"			, filmIndirectShadowMask\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_UV)\n"
"			, filmUV\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_RAYCOUNT)\n"
"			, filmRayCount\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_BY_MATERIAL_ID)\n"
"			, filmByMaterialID\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_IRRADIANCE)\n"
"			, filmIrradiance\n"
"#endif\n"
"			,\n"
"			// Scene parameters\n"
"#if defined(PARAM_HAS_INFINITELIGHTS)\n"
"			worldCenterX,\n"
"			worldCenterY,\n"
"			worldCenterZ,\n"
"			worldRadius,\n"
"#endif\n"
"			mats,\n"
"			texs,\n"
"			meshMats,\n"
"			meshDescs,\n"
"			vertices,\n"
"#if defined(PARAM_HAS_NORMALS_BUFFER)\n"
"			vertNormals,\n"
"#endif\n"
"#if defined(PARAM_HAS_UVS_BUFFER)\n"
"			vertUVs,\n"
"#endif\n"
"#if defined(PARAM_HAS_COLS_BUFFER)\n"
"			vertCols,\n"
"#endif\n"
"#if defined(PARAM_HAS_ALPHAS_BUFFER)\n"
"			vertAlphas,\n"
"#endif\n"
"			triangles,\n"
"			camera,\n"
"			// Lights\n"
"			lights,\n"
"#if defined(PARAM_HAS_ENVLIGHTS)\n"
"			envLightIndices,\n"
"			envLightCount,\n"
"#endif\n"
"			meshTriLightDefsOffset,\n"
"#if defined(PARAM_HAS_INFINITELIGHT)\n"
"			infiniteLightDistribution,\n"
"#endif\n"
"			lightsDistribution\n"
"			// Images\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_0)\n"
"			, 	imageMapDescs, 	imageMapBuff0\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_1)\n"
"			imageMapBuff1\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_2)\n"
"			, imageMapBuff2\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_3)\n"
"			, imageMapBuff3\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_4)\n"
"			, imageMapBuff4\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_5)\n"
"			, imageMapBuff5\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_6)\n"
"			, imageMapBuff6\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_7)\n"
"			, imageMapBuff7\n"
"#endif\n"
"			// Ray intersection accelerator parameters\n"
"			ACCELERATOR_INTERSECT_PARAM\n"
"			);\n"
"}\n"
"\n"
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void RenderSample_MK_BSDF_SAMPLE_GLOSSY(\n"
"		KERNEL_ARGS\n"
"		) {\n"
"	RenderSample_MK_BSDF_SAMPLE(\n"
"			GLOSSY,\n"
"			engineFilmWidth,engineFilmHeight,\n"
"			tasks,\n"
"			tasksDirectLight,\n"
"			tasksPathVertexN,\n"
"			taskStats,\n"
"			taskResults,\n"
"			pixelFilterDistribution,\n"
"			// Film parameters\n"
"			filmWidth, filmHeight\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_0)\n"
"			, filmRadianceGroup0\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_1)\n"
"			, filmRadianceGroup1\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_2)\n"
"			, filmRadianceGroup2\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_3)\n"
"			, filmRadianceGroup3\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_4)\n"
"			, filmRadianceGroup4\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_5)\n"
"			, filmRadianceGroup5\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_6)\n"
"			, filmRadianceGroup6\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_7)\n"
"			, filmRadianceGroup7\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_ALPHA)\n"
"			, filmAlpha\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DEPTH)\n"
"			, filmDepth\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_POSITION)\n"
"			, filmPosition\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_GEOMETRY_NORMAL)\n"
"			, filmGeometryNormal\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_SHADING_NORMAL)\n"
"			, filmShadingNormal\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_MATERIAL_ID)\n"
"			, filmMaterialID\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_DIFFUSE)\n"
"			, filmDirectDiffuse\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_GLOSSY)\n"
"			, filmDirectGlossy\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_EMISSION)\n"
"			, filmEmission\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_DIFFUSE)\n"
"			, filmIndirectDiffuse\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_GLOSSY)\n"
"			, filmIndirectGlossy\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SPECULAR)\n"
"			, filmIndirectSpecular\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_MATERIAL_ID_MASK)\n"
"			, filmMaterialIDMask\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_SHADOW_MASK)\n"
"			, filmDirectShadowMask\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SHADOW_MASK)\n"
"			, filmIndirectShadowMask\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_UV)\n"
"			, filmUV\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_RAYCOUNT)\n"
"			, filmRayCount\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_BY_MATERIAL_ID)\n"
"			, filmByMaterialID\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_IRRADIANCE)\n"
"			, filmIrradiance\n"
"#endif\n"
"			,\n"
"			// Scene parameters\n"
"#if defined(PARAM_HAS_INFINITELIGHTS)\n"
"			worldCenterX,\n"
"			worldCenterY,\n"
"			worldCenterZ,\n"
"			worldRadius,\n"
"#endif\n"
"			mats,\n"
"			texs,\n"
"			meshMats,\n"
"			meshDescs,\n"
"			vertices,\n"
"#if defined(PARAM_HAS_NORMALS_BUFFER)\n"
"			vertNormals,\n"
"#endif\n"
"#if defined(PARAM_HAS_UVS_BUFFER)\n"
"			vertUVs,\n"
"#endif\n"
"#if defined(PARAM_HAS_COLS_BUFFER)\n"
"			vertCols,\n"
"#endif\n"
"#if defined(PARAM_HAS_ALPHAS_BUFFER)\n"
"			vertAlphas,\n"
"#endif\n"
"			triangles,\n"
"			camera,\n"
"			// Lights\n"
"			lights,\n"
"#if defined(PARAM_HAS_ENVLIGHTS)\n"
"			envLightIndices,\n"
"			envLightCount,\n"
"#endif\n"
"			meshTriLightDefsOffset,\n"
"#if defined(PARAM_HAS_INFINITELIGHT)\n"
"			infiniteLightDistribution,\n"
"#endif\n"
"			lightsDistribution\n"
"			// Images\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_0)\n"
"			, 	imageMapDescs, 	imageMapBuff0\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_1)\n"
"			imageMapBuff1\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_2)\n"
"			, imageMapBuff2\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_3)\n"
"			, imageMapBuff3\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_4)\n"
"			, imageMapBuff4\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_5)\n"
"			, imageMapBuff5\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_6)\n"
"			, imageMapBuff6\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_7)\n"
"			, imageMapBuff7\n"
"#endif\n"
"			// Ray intersection accelerator parameters\n"
"			ACCELERATOR_INTERSECT_PARAM\n"
"			);\n"
"}\n"
"\n"
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void RenderSample_MK_BSDF_SAMPLE_SPECULAR(\n"
"		KERNEL_ARGS\n"
"		) {\n"
"	RenderSample_MK_BSDF_SAMPLE(\n"
"			SPECULAR,\n"
"			engineFilmWidth,engineFilmHeight,\n"
"			tasks,\n"
"			tasksDirectLight,\n"
"			tasksPathVertexN,\n"
"			taskStats,\n"
"			taskResults,\n"
"			pixelFilterDistribution,\n"
"			// Film parameters\n"
"			filmWidth, filmHeight\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_0)\n"
"			, filmRadianceGroup0\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_1)\n"
"			, filmRadianceGroup1\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_2)\n"
"			, filmRadianceGroup2\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_3)\n"
"			, filmRadianceGroup3\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_4)\n"
"			, filmRadianceGroup4\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_5)\n"
"			, filmRadianceGroup5\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_6)\n"
"			, filmRadianceGroup6\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_7)\n"
"			, filmRadianceGroup7\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_ALPHA)\n"
"			, filmAlpha\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DEPTH)\n"
"			, filmDepth\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_POSITION)\n"
"			, filmPosition\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_GEOMETRY_NORMAL)\n"
"			, filmGeometryNormal\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_SHADING_NORMAL)\n"
"			, filmShadingNormal\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_MATERIAL_ID)\n"
"			, filmMaterialID\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_DIFFUSE)\n"
"			, filmDirectDiffuse\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_GLOSSY)\n"
"			, filmDirectGlossy\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_EMISSION)\n"
"			, filmEmission\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_DIFFUSE)\n"
"			, filmIndirectDiffuse\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_GLOSSY)\n"
"			, filmIndirectGlossy\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SPECULAR)\n"
"			, filmIndirectSpecular\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_MATERIAL_ID_MASK)\n"
"			, filmMaterialIDMask\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_SHADOW_MASK)\n"
"			, filmDirectShadowMask\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SHADOW_MASK)\n"
"			, filmIndirectShadowMask\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_UV)\n"
"			, filmUV\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_RAYCOUNT)\n"
"			, filmRayCount\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_BY_MATERIAL_ID)\n"
"			, filmByMaterialID\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_IRRADIANCE)\n"
"			, filmIrradiance\n"
"#endif\n"
"			,\n"
"			// Scene parameters\n"
"#if defined(PARAM_HAS_INFINITELIGHTS)\n"
"			worldCenterX,\n"
"			worldCenterY,\n"
"			worldCenterZ,\n"
"			worldRadius,\n"
"#endif\n"
"			mats,\n"
"			texs,\n"
"			meshMats,\n"
"			meshDescs,\n"
"			vertices,\n"
"#if defined(PARAM_HAS_NORMALS_BUFFER)\n"
"			vertNormals,\n"
"#endif\n"
"#if defined(PARAM_HAS_UVS_BUFFER)\n"
"			vertUVs,\n"
"#endif\n"
"#if defined(PARAM_HAS_COLS_BUFFER)\n"
"			vertCols,\n"
"#endif\n"
"#if defined(PARAM_HAS_ALPHAS_BUFFER)\n"
"			vertAlphas,\n"
"#endif\n"
"			triangles,\n"
"			camera,\n"
"			// Lights\n"
"			lights,\n"
"#if defined(PARAM_HAS_ENVLIGHTS)\n"
"			envLightIndices,\n"
"			envLightCount,\n"
"#endif\n"
"			meshTriLightDefsOffset,\n"
"#if defined(PARAM_HAS_INFINITELIGHT)\n"
"			infiniteLightDistribution,\n"
"#endif\n"
"			lightsDistribution\n"
"			// Images\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_0)\n"
"			, 	imageMapDescs, 	imageMapBuff0\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_1)\n"
"			imageMapBuff1\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_2)\n"
"			, imageMapBuff2\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_3)\n"
"			, imageMapBuff3\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_4)\n"
"			, imageMapBuff4\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_5)\n"
"			, imageMapBuff5\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_6)\n"
"			, imageMapBuff6\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_7)\n"
"			, imageMapBuff7\n"
"#endif\n"
"			// Ray intersection accelerator parameters\n"
"			ACCELERATOR_INTERSECT_PARAM\n"
"			);\n"
"	\n"
"	// All done\n"
"	tasks[get_global_id(0)].pathState = MK_DONE;\n"
"}\n"
; } }
