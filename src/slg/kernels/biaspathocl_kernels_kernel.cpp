#include <string>
namespace slg { namespace ocl {
std::string KernelSource_biaspathocl_kernels = 
"#line 2 \"biaspatchocl_kernels.cl\"\n"
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
"//  PARAM_DIRECT_LIGHT_ONE_STRATEGY or PARAM_DIRECT_LIGHT_ALL_STRATEGY\n"
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
"// InitSeed Kernel\n"
"//------------------------------------------------------------------------------\n"
"\n"
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void InitSeed(\n"
"		uint seedBase,\n"
"		__global GPUTask *tasks) {\n"
"	//if (get_global_id(0) == 0)\n"
"	//	printf(\"sizeof(BSDF) = %dbytes\\n\", sizeof(BSDF));\n"
"	//if (get_global_id(0) == 0)\n"
"	//	printf(\"sizeof(HitPoint) = %dbytes\\n\", sizeof(HitPoint));\n"
"	//if (get_global_id(0) == 0)\n"
"	//	printf(\"sizeof(GPUTask) = %dbytes\\n\", sizeof(GPUTask));\n"
"	//if (get_global_id(0) == 0)\n"
"	//	printf(\"sizeof(SampleResult) = %dbytes\\n\", sizeof(SampleResult));\n"
"\n"
"	const size_t gid = get_global_id(0);\n"
"	if (gid >= PARAM_TASK_COUNT)\n"
"		return;\n"
"\n"
"	// Initialize the task\n"
"	__global GPUTask *task = &tasks[gid];\n"
"\n"
"	// For testing/debugging\n"
"	//MangleMemory((__global unsigned char *)task, sizeof(GPUTask));\n"
"\n"
"	// Initialize random number generator\n"
"	Seed seed;\n"
"	Rnd_Init(seedBase + gid, &seed);\n"
"\n"
"	// Save the seed\n"
"	task->seed.s1 = seed.s1;\n"
"	task->seed.s2 = seed.s2;\n"
"	task->seed.s3 = seed.s3;\n"
"}\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// InitStats Kernel\n"
"//------------------------------------------------------------------------------\n"
"\n"
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void InitStat(\n"
"		__global GPUTaskStats *taskStats) {\n"
"	const size_t gid = get_global_id(0);\n"
"	if (gid >= PARAM_TASK_COUNT)\n"
"		return;\n"
"\n"
"	__global GPUTaskStats *taskStat = &taskStats[gid];\n"
"	taskStat->raysCount = 0;\n"
"}\n"
"\n"
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void RenderSample(\n"
"		const uint tileStartX,\n"
"		const uint tileStartY,\n"
"		const int tileSampleIndex,\n"
"		const uint engineFilmWidth, const uint engineFilmHeight,\n"
"		__global GPUTask *tasks,\n"
"		__global GPUTaskDirectLight *tasksDirectLight,\n"
"		__global GPUTaskPathVertexN *tasksPathVertexN,\n"
"		__global GPUTaskStats *taskStats,\n"
"		__global SampleResult *taskResults,\n"
"		__global float *pixelFilterDistribution,\n"
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
"		// Ray intersection accelerator parameters\n"
"		ACCELERATOR_INTERSECT_PARAM_DECL\n"
"		) {\n"
"	const size_t gid = get_global_id(0);\n"
"\n"
"	const uint sampleIndex = gid % (PARAM_AA_SAMPLES * PARAM_AA_SAMPLES);\n"
"	const uint samplePixelIndex = gid / (PARAM_AA_SAMPLES * PARAM_AA_SAMPLES);\n"
"	const uint samplePixelX = samplePixelIndex % PARAM_TILE_SIZE;\n"
"	const uint samplePixelY = samplePixelIndex / PARAM_TILE_SIZE;\n"
"\n"
"	if ((gid >= PARAM_TILE_SIZE * PARAM_TILE_SIZE * PARAM_AA_SAMPLES * PARAM_AA_SAMPLES) ||\n"
"			(tileStartX + samplePixelX >= engineFilmWidth) ||\n"
"			(tileStartY + samplePixelY >= engineFilmHeight))\n"
"		return;\n"
"\n"
"	__global GPUTask *task = &tasks[gid];\n"
"	__global GPUTaskDirectLight *taskDirectLight = &tasksDirectLight[gid];\n"
"	__global GPUTaskPathVertexN *taskPathVertexN = &tasksPathVertexN[gid];\n"
"	__global GPUTaskStats *taskStat = &taskStats[gid];\n"
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
"\n"
"	uint tracedRaysCount = taskStat->raysCount;\n"
"	float3 throughputPathVertex1 = WHITE;\n"
"	\n"
"	__global SampleResult *sampleResult = &taskResults[gid];\n"
"	SampleResult_Init(sampleResult);\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_SHADOW_MASK)\n"
"	sampleResult->directShadowMask = 0.f;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SHADOW_MASK)\n"
"	sampleResult->indirectShadowMask = 0.f;\n"
"#endif\n"
"\n"
"	Ray ray;\n"
"	RayHit rayHit;\n"
"	GenerateCameraRay(&seed, task, sampleResult,\n"
"			camera, pixelFilterDistribution,\n"
"			samplePixelX, samplePixelY, sampleIndex,\n"
"			tileStartX, tileStartY,\n"
"			engineFilmWidth, engineFilmHeight, &ray);\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Trace the eye ray\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	tracedRaysCount += BIASPATHOCL_Scene_Intersect(\n"
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
"	//--------------------------------------------------------------------------\n"
"	// Render the eye ray hit point\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	if (rayHit.meshIndex != NULL_INDEX) {\n"
"		//----------------------------------------------------------------------\n"
"		// Something was hit\n"
"		//----------------------------------------------------------------------\n"
"\n"
"		// Save the path first vertex information\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_ALPHA)\n"
"		sampleResult->alpha = 1.f;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DEPTH)\n"
"		sampleResult->depth = rayHit.t;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_POSITION)\n"
"		sampleResult->position = task->bsdfPathVertex1.hitPoint.p;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_GEOMETRY_NORMAL)\n"
"		sampleResult->geometryNormal = task->bsdfPathVertex1.hitPoint.geometryN;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_SHADING_NORMAL)\n"
"		sampleResult->shadingNormal = task->bsdfPathVertex1.hitPoint.shadeN;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_MATERIAL_ID)\n"
"		sampleResult->materialID = BSDF_GetMaterialID(&task->bsdfPathVertex1\n"
"			MATERIALS_PARAM);\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_UV)\n"
"		sampleResult->uv = task->bsdfPathVertex1.hitPoint.uv;\n"
"#endif\n"
"\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"		// Check if it is a light source (note: I can hit only triangle area light sources)\n"
"		if (BSDF_IsLightSource(&task->bsdfPathVertex1) && (rayHit.t > PARAM_NEAR_START_LIGHT)) {\n"
"			// SPECULAR is required to avoid MIS\n"
"			DirectHitFiniteLight(SPECULAR,\n"
"					throughputPathVertex1,\n"
"					rayHit.t, &task->bsdfPathVertex1, 1.f,\n"
"					sampleResult\n"
"					LIGHTS_PARAM);\n"
"		}\n"
"#endif\n"
"\n"
"		//----------------------------------------------------------------------\n"
"		// First path vertex direct light sampling\n"
"		//----------------------------------------------------------------------\n"
"\n"
"		// Only if it is not a SPECULAR BSDF\n"
"		if (!BSDF_IsDelta(&task->bsdfPathVertex1\n"
"				MATERIALS_PARAM)) {\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"			// I need to work on a copy of volume information of the first path vertex\n"
"			taskDirectLight->directLightVolInfo = task->volInfoPathVertex1;\n"
"#endif\n"
"\n"
"			tracedRaysCount +=\n"
"#if defined(PARAM_DIRECT_LIGHT_ALL_STRATEGY)\n"
"				DirectLightSampling_ALL(\n"
"#else\n"
"				DirectLightSampling_ONE(\n"
"#endif\n"
"				&seed,\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"				&taskDirectLight->directLightVolInfo,\n"
"#endif\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0) || defined(PARAM_HAS_VOLUMES)\n"
"				&task->tmpHitPoint,\n"
"#endif\n"
"#if defined(PARAM_HAS_INFINITELIGHTS)\n"
"				worldCenterX, worldCenterY, worldCenterZ, worldRadius,\n"
"#endif\n"
"				throughputPathVertex1,\n"
"				&task->bsdfPathVertex1, &taskDirectLight->directLightBSDF,\n"
"				sampleResult,\n"
"				// BSDF_Init parameters\n"
"				meshDescs,\n"
"				meshMats,\n"
"				vertices,\n"
"#if defined(PARAM_HAS_NORMALS_BUFFER)\n"
"				vertNormals,\n"
"#endif\n"
"#if defined(PARAM_HAS_UVS_BUFFER)\n"
"				vertUVs,\n"
"#endif\n"
"#if defined(PARAM_HAS_COLS_BUFFER)\n"
"				vertCols,\n"
"#endif\n"
"#if defined(PARAM_HAS_ALPHAS_BUFFER)\n"
"				vertAlphas,\n"
"#endif\n"
"				triangles\n"
"				// Accelerator_Intersect parameters\n"
"				ACCELERATOR_INTERSECT_PARAM\n"
"				// Light related parameters\n"
"				LIGHTS_PARAM);\n"
"		}\n"
"		\n"
"		//----------------------------------------------------------------------\n"
"		// Sample the components of the BSDF of the first path vertex\n"
"		//----------------------------------------------------------------------\n"
"		\n"
"		const BSDFEvent materialEventTypes = BSDF_GetEventTypes(&task->bsdfPathVertex1\n"
"			MATERIALS_PARAM);\n"
"\n"
"		BSDFEvent vertex1SampleComponent = DIFFUSE;\n"
"		do {\n"
"			if (PathDepthInfo_CheckComponentDepths(vertex1SampleComponent) && (materialEventTypes & vertex1SampleComponent)) {\n"
"				const int matSamplesCount = mats[task->bsdfPathVertex1.materialIndex].samples;\n"
"				const uint globalMatSamplesCount = ((vertex1SampleComponent == DIFFUSE) ? PARAM_DIFFUSE_SAMPLES :\n"
"					((vertex1SampleComponent == GLOSSY) ? PARAM_GLOSSY_SAMPLES :\n"
"						PARAM_SPECULAR_SAMPLES));\n"
"				const uint sampleCount = (matSamplesCount < 0) ? globalMatSamplesCount : (uint)matSamplesCount;\n"
"\n"
"				tracedRaysCount += SampleComponent(\n"
"						&seed,\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"						&task->volInfoPathVertex1,\n"
"						&taskPathVertexN->volInfoPathVertexN,\n"
"						&taskDirectLight->directLightVolInfo,\n"
"#endif\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0) || defined(PARAM_HAS_VOLUMES)\n"
"						&task->tmpHitPoint,\n"
"#endif\n"
"#if defined(PARAM_HAS_INFINITELIGHTS)\n"
"						worldCenterX, worldCenterY, worldCenterZ, worldRadius,\n"
"#endif\n"
"						vertex1SampleComponent,\n"
"						sampleCount, throughputPathVertex1,\n"
"						&task->bsdfPathVertex1, &taskPathVertexN->bsdfPathVertexN,\n"
"						&taskDirectLight->directLightBSDF,\n"
"						sampleResult,\n"
"						// BSDF_Init parameters\n"
"						meshDescs,\n"
"						meshMats,\n"
"						vertices,\n"
"#if defined(PARAM_HAS_NORMALS_BUFFER)\n"
"						vertNormals,\n"
"#endif\n"
"#if defined(PARAM_HAS_UVS_BUFFER)\n"
"						vertUVs,\n"
"#endif\n"
"#if defined(PARAM_HAS_COLS_BUFFER)\n"
"						vertCols,\n"
"#endif\n"
"#if defined(PARAM_HAS_ALPHAS_BUFFER)\n"
"						vertAlphas,\n"
"#endif\n"
"						triangles\n"
"						// Accelerator_Intersect parameters\n"
"						ACCELERATOR_INTERSECT_PARAM\n"
"						// Light related parameters\n"
"						LIGHTS_PARAM);\n"
"			}\n"
"\n"
"			// Move to the next BSDF component\n"
"			vertex1SampleComponent = (vertex1SampleComponent == DIFFUSE) ? GLOSSY :\n"
"				((vertex1SampleComponent == GLOSSY) ? SPECULAR : NONE);\n"
"		} while (vertex1SampleComponent != NONE);\n"
"	} else {\n"
"		//----------------------------------------------------------------------\n"
"		// Nothing was hit\n"
"		//----------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_HAS_ENVLIGHTS)\n"
"		// Add environmental lights radiance\n"
"		const float3 rayDir = (float3)(ray.d.x, ray.d.y, ray.d.z);\n"
"		// SPECULAR is required to avoid MIS\n"
"		DirectHitInfiniteLight(\n"
"				SPECULAR,\n"
"				throughputPathVertex1,\n"
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
"	}\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Save the result\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_RAYCOUNT)\n"
"	sampleResult->rayCount = tracedRaysCount;\n"
"#endif\n"
"\n"
"	// Radiance clamping\n"
"	SR_RadianceClamp(sampleResult);\n"
"\n"
"	taskStat->raysCount = tracedRaysCount;\n"
"\n"
"	// Save the seed\n"
"	task->seed.s1 = seed.s1;\n"
"	task->seed.s2 = seed.s2;\n"
"	task->seed.s3 = seed.s3;\n"
"}\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// MergePixelSamples\n"
"//------------------------------------------------------------------------------\n"
"\n"
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void MergePixelSamples(\n"
"		const uint tileStartX,\n"
"		const uint tileStartY,\n"
"		const uint engineFilmWidth, const uint engineFilmHeight,\n"
"		__global SampleResult *taskResults,\n"
"		__global GPUTaskStats *taskStats,\n"
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
"		) {\n"
"	const size_t gid = get_global_id(0);\n"
"\n"
"	uint sampleX, sampleY;\n"
"	sampleX = gid % PARAM_TILE_SIZE;\n"
"	sampleY = gid / PARAM_TILE_SIZE;\n"
"\n"
"	if ((gid >= PARAM_TILE_SIZE * PARAM_TILE_SIZE) ||\n"
"			(tileStartX + sampleX >= engineFilmWidth) ||\n"
"			(tileStartY + sampleY >= engineFilmHeight))\n"
"		return;\n"
"\n"
"	const uint index = gid * PARAM_AA_SAMPLES * PARAM_AA_SAMPLES;\n"
"	__global SampleResult *sampleResult = &taskResults[index];\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Initialize Film radiance group pointer table\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	__global float *filmRadianceGroup[PARAM_FILM_RADIANCE_GROUP_COUNT];\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_0)\n"
"	filmRadianceGroup[0] = filmRadianceGroup0;\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_1)\n"
"	filmRadianceGroup[1] = filmRadianceGroup1;\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_2)\n"
"	filmRadianceGroup[2] = filmRadianceGroup2;\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_3)\n"
"	filmRadianceGroup[3] = filmRadianceGroup3;\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_4)\n"
"	filmRadianceGroup[3] = filmRadianceGroup4;\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_5)\n"
"	filmRadianceGroup[3] = filmRadianceGroup5;\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_6)\n"
"	filmRadianceGroup[3] = filmRadianceGroup6;\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_7)\n"
"	filmRadianceGroup[3] = filmRadianceGroup7;\n"
"#endif\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Merge all samples and accumulate statistics\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"#if (PARAM_AA_SAMPLES == 1)\n"
"	Film_AddSample(sampleX, sampleY, &sampleResult[0], PARAM_AA_SAMPLES * PARAM_AA_SAMPLES\n"
"			FILM_PARAM);\n"
"#else\n"
"	__global GPUTaskStats *taskStat = &taskStats[index];\n"
"\n"
"	SampleResult result = sampleResult[0];\n"
"	uint totalRaysCount = 0;\n"
"	for (uint i = 1; i < PARAM_AA_SAMPLES * PARAM_AA_SAMPLES; ++i) {\n"
"		SR_Accumulate(&sampleResult[i], &result);\n"
"		totalRaysCount += taskStat[i].raysCount;\n"
"	}\n"
"	SR_Normalize(&result, 1.f / (PARAM_AA_SAMPLES * PARAM_AA_SAMPLES));\n"
"\n"
"	// I have to save result in __global space in order to be able\n"
"	// to use Film_AddSample(). OpenCL can be so stupid some time...\n"
"	sampleResult[0] = result;\n"
"	Film_AddSample(sampleX, sampleY, &sampleResult[0], PARAM_AA_SAMPLES * PARAM_AA_SAMPLES\n"
"			FILM_PARAM);\n"
"\n"
"	taskStat[0].raysCount = totalRaysCount;\n"
"#endif\n"
"}\n"
; } }
