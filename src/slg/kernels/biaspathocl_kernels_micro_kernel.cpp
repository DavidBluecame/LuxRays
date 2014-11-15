#include <string>
namespace slg { namespace ocl {
std::string KernelSource_biaspathocl_kernels_micro = 
"#line 2 \"biaspatchocl_kernels_micro.cl\"\n"
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
"//  PARAM_TASK_COUNT\n"
"//  PARAM_TILE_SIZE\n"
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
"		KERNEL_ARGS\n"
"		) {\n"
"	const size_t gid = get_global_id(0);\n"
"	__global GPUTask *task = &tasks[gid];\n"
"\n"
"	const uint sampleIndex = gid % (PARAM_AA_SAMPLES * PARAM_AA_SAMPLES);\n"
"	const uint samplePixelIndex = gid / (PARAM_AA_SAMPLES * PARAM_AA_SAMPLES);\n"
"	const uint samplePixelX = samplePixelIndex % PARAM_TILE_SIZE;\n"
"	const uint samplePixelY = samplePixelIndex / PARAM_TILE_SIZE;\n"
"\n"
"	if ((gid >= PARAM_TILE_SIZE * PARAM_TILE_SIZE * PARAM_AA_SAMPLES * PARAM_AA_SAMPLES) ||\n"
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
"	__global GPUTaskStats *taskStat = &taskStats[gid];\n"
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
"	taskStat->raysCount += BIASPATHOCL_Scene_Intersect(\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"		&task->volInfoPathVertex1,\n"
"		&task->tmpHitPoint,\n"
"#endif\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"		Rnd_FloatValue(&seed),\n"
"#endif\n"
"		&ray, &rayHit,\n"
"		&task->bsdfPathVertex1,\n"
"		&throughputPathVertex1,\n"
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
"	RayHit rayHit = task->tmpRayHit;\n"
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
"	}\n"
"}\n"
; } }
