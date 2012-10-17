#include "pathgpu2/kernels/kernels.h"
std::string luxrays::KernelSource_PathGPU2_kernels = 
"/***************************************************************************\n"
" *   Copyright (C) 1998-2010 by authors (see AUTHORS.txt )                 *\n"
" *                                                                         *\n"
" *   This file is part of LuxRays.                                         *\n"
" *                                                                         *\n"
" *   LuxRays is free software; you can redistribute it and/or modify       *\n"
" *   it under the terms of the GNU General Public License as published by  *\n"
" *   the Free Software Foundation; either version 3 of the License, or     *\n"
" *   (at your option) any later version.                                   *\n"
" *                                                                         *\n"
" *   LuxRays is distributed in the hope that it will be useful,            *\n"
" *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *\n"
" *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *\n"
" *   GNU General Public License for more details.                          *\n"
" *                                                                         *\n"
" *   You should have received a copy of the GNU General Public License     *\n"
" *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *\n"
" *                                                                         *\n"
" *   LuxRays website: http://www.luxrender.net                             *\n"
" ***************************************************************************/\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// Init Kernel\n"
"//------------------------------------------------------------------------------\n"
"\n"
"__kernel void Init(\n"
"		__global GPUTask *tasks,\n"
"		__global GPUTaskStats *taskStats,\n"
"		__global Ray *rays,\n"
"		__global Camera *camera\n"
"#if (PARAM_SAMPLER_TYPE == 3)\n"
"		, __local float *localMemTempBuff\n"
"#endif\n"
"		) {\n"
"	const size_t gid = get_global_id(0);\n"
"\n"
"	//if (gid == 0)\n"
"	//	printf(\"GPUTask: %d\\n\", sizeof(GPUTask));\n"
"\n"
"	// Initialize the task\n"
"	__global GPUTask *task = &tasks[gid];\n"
"\n"
"	// Initialize random number generator\n"
"	Seed seed;\n"
"	InitRandomGenerator(PARAM_SEED + gid, &seed);\n"
"\n"
"	// Initialize the sample\n"
"	Sampler_Init(gid,\n"
"#if (PARAM_SAMPLER_TYPE == 3)\n"
"			localMemTempBuff,\n"
"#endif\n"
"			&seed, &task->sample);\n"
"\n"
"	// Initialize the path\n"
"	GenerateCameraPath(task, &rays[gid], &seed, camera);\n"
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
"// InitFrameBuffer Kernel\n"
"//------------------------------------------------------------------------------\n"
"\n"
"__kernel void InitFrameBuffer(\n"
"		__global Pixel *frameBuffer\n"
"		) {\n"
"	const size_t gid = get_global_id(0);\n"
"	if (gid >= (PARAM_IMAGE_WIDTH + 2) * (PARAM_IMAGE_HEIGHT + 2))\n"
"		return;\n"
"\n"
"	__global Pixel *p = &frameBuffer[gid];\n"
"	p->c.r = 0.f;\n"
"	p->c.g = 0.f;\n"
"	p->c.b = 0.f;\n"
"	p->count = 0.f;\n"
"}\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// AdvancePaths Kernel\n"
"//------------------------------------------------------------------------------\n"
"\n"
"__kernel void AdvancePaths(\n"
"		__global GPUTask *tasks,\n"
"		__global Ray *rays,\n"
"		__global RayHit *rayHits,\n"
"		__global Pixel *frameBuffer,\n"
"		__global Material *mats,\n"
"		__global uint *meshMats,\n"
"		__global uint *meshIDs,\n"
"#if defined(PARAM_ACCEL_MQBVH)\n"
"		__global uint *triangleIDs,\n"
"		__global Mesh *meshDescs,\n"
"#endif\n"
"		__global Spectrum *vertColors,\n"
"		__global Vector *vertNormals,\n"
"		__global Point *vertices,\n"
"		__global Triangle *triangles,\n"
"		__global Camera *camera\n"
"#if defined(PARAM_HAS_INFINITELIGHT)\n"
"		, __global InfiniteLight *infiniteLight\n"
"		, __global Spectrum *infiniteLightMap\n"
"#endif\n"
"#if defined(PARAM_HAS_SUNLIGHT)\n"
"		, __global SunLight *sunLight\n"
"#endif\n"
"#if defined(PARAM_HAS_SKYLIGHT)\n"
"		, __global SkyLight *skyLight\n"
"#endif\n"
"#if (PARAM_DL_LIGHT_COUNT > 0)\n"
"		, __global TriangleLight *triLights\n"
"#endif\n"
"#if defined(PARAM_HAS_TEXTUREMAPS)\n"
"        , __global Spectrum *texMapRGBBuff\n"
"#if defined(PARAM_HAS_ALPHA_TEXTUREMAPS)\n"
"		, __global float *texMapAlphaBuff\n"
"#endif\n"
"        , __global TexMap *texMapDescBuff\n"
"        , __global unsigned int *meshTexsBuff\n"
"#if defined(PARAM_HAS_BUMPMAPS)\n"
"        , __global unsigned int *meshBumpsBuff\n"
"		, __global float *meshBumpsScaleBuff\n"
"#endif\n"
"        , __global UV *vertUVs\n"
"#endif\n"
"		) {\n"
"	const size_t gid = get_global_id(0);\n"
"\n"
"	__global GPUTask *task = &tasks[gid];\n"
"	uint pathState = task->pathState.state;\n"
"\n"
"#if (PARAM_SAMPLER_TYPE == 0)\n"
"	// Read the seed\n"
"	Seed seed;\n"
"	seed.s1 = task->seed.s1;\n"
"	seed.s2 = task->seed.s2;\n"
"	seed.s3 = task->seed.s3;\n"
"#endif\n"
"\n"
"	__global Sample *sample = &task->sample;\n"
"\n"
"	__global Ray *ray = &rays[gid];\n"
"	__global RayHit *rayHit = &rayHits[gid];\n"
"	const uint currentTriangleIndex = rayHit->index;\n"
"\n"
"	const float hitPointT = rayHit->t;\n"
"    const float hitPointB1 = rayHit->b1;\n"
"    const float hitPointB2 = rayHit->b2;\n"
"\n"
"    Vector rayDir = ray->d;\n"
"\n"
"	Point hitPoint;\n"
"    hitPoint.x = ray->o.x + rayDir.x * hitPointT;\n"
"    hitPoint.y = ray->o.y + rayDir.y * hitPointT;\n"
"    hitPoint.z = ray->o.z + rayDir.z * hitPointT;\n"
"\n"
"	Spectrum throughput = task->pathState.throughput;\n"
"	const Spectrum prevThroughput = throughput;\n"
"\n"
"	switch (pathState) {\n"
"		case PATH_STATE_NEXT_VERTEX: {\n"
"			if (currentTriangleIndex != 0xffffffffu) {\n"
"				// Something was hit\n"
"\n"
"				uint pathDepth = task->pathState.depth;\n"
"#if (PARAM_SAMPLER_TYPE == 1) || (PARAM_SAMPLER_TYPE == 3)\n"
"				__global float *sampleData = &sample->u[IDX_BSDF_OFFSET + SAMPLE_SIZE * pathDepth];\n"
"#elif (PARAM_SAMPLER_TYPE == 2)\n"
"				__global float *sampleData = &sample->u[sample->proposed][IDX_BSDF_OFFSET + SAMPLE_SIZE * pathDepth];\n"
"#endif\n"
"\n"
"				const uint meshIndex = meshIDs[currentTriangleIndex];\n"
"#if defined(PARAM_ACCEL_MQBVH)\n"
"				__global Mesh *meshDesc = &meshDescs[meshIndex];\n"
"				__global Point *iVertices = &vertices[meshDesc->vertsOffset];\n"
"				__global Spectrum *iVertColors = &vertColors[meshDesc->vertsOffset];\n"
"				__global Vector *iVertNormals = &vertNormals[meshDesc->vertsOffset];\n"
"#if defined(PARAM_HAS_TEXTUREMAPS)\n"
"				__global UV *iVertUVs = &vertUVs[meshDesc->vertsOffset];\n"
"#endif\n"
"				__global Triangle *iTriangles = &triangles[meshDesc->trisOffset];\n"
"				const uint triangleID = triangleIDs[currentTriangleIndex];\n"
"#endif\n"
"				__global Material *hitPointMat = &mats[meshMats[meshIndex]];\n"
"				uint matType = hitPointMat->type;\n"
"\n"
"				// Interpolate Color\n"
"				Spectrum shadeColor;\n"
"#if defined(PARAM_ACCEL_MQBVH)\n"
"				Mesh_InterpolateColor(iVertColors, iTriangles, triangleID, hitPointB1, hitPointB2, &shadeColor);\n"
"#else\n"
"				Mesh_InterpolateColor(vertColors, triangles, currentTriangleIndex, hitPointB1, hitPointB2, &shadeColor);\n"
"#endif\n"
"\n"
"#if defined(PARAM_HAS_TEXTUREMAPS)\n"
"				// Interpolate UV coordinates\n"
"				UV uv;\n"
"#if defined(PARAM_ACCEL_MQBVH)\n"
"				Mesh_InterpolateUV(iVertUVs, iTriangles, triangleID, hitPointB1, hitPointB2, &uv);\n"
"#else\n"
"				Mesh_InterpolateUV(vertUVs, triangles, currentTriangleIndex, hitPointB1, hitPointB2, &uv);\n"
"#endif\n"
"				// Check it the mesh has a texture map\n"
"				unsigned int texIndex = meshTexsBuff[meshIndex];\n"
"				if (texIndex != 0xffffffffu) {\n"
"					__global TexMap *texMap = &texMapDescBuff[texIndex];\n"
"\n"
"#if defined(PARAM_HAS_ALPHA_TEXTUREMAPS)\n"
"					// Check if it has an alpha channel\n"
"					if (texMap->alphaOffset != 0xffffffffu) {\n"
"						const float alpha = TexMap_GetAlpha(&texMapAlphaBuff[texMap->alphaOffset], texMap->width, texMap->height, uv.u, uv.v);\n"
"\n"
"#if (PARAM_SAMPLER_TYPE == 0)\n"
"						const float texAlphaSample = RndFloatValue(&seed);\n"
"#elif (PARAM_SAMPLER_TYPE == 1) || (PARAM_SAMPLER_TYPE == 2) || (PARAM_SAMPLER_TYPE == 3)\n"
"						const float texAlphaSample = sampleData[IDX_TEX_ALPHA];\n"
"#endif\n"
"\n"
"						if ((alpha == 0.0f) || ((alpha < 1.f) && (texAlphaSample > alpha))) {\n"
"							// Continue to trace the ray\n"
"							matType = MAT_NULL;\n"
"						}\n"
"					}\n"
"#endif\n"
"\n"
"					Spectrum texColor;\n"
"					TexMap_GetColor(&texMapRGBBuff[texMap->rgbOffset], texMap->width, texMap->height, uv.u, uv.v, &texColor);\n"
"\n"
"					shadeColor.r *= texColor.r;\n"
"					shadeColor.g *= texColor.g;\n"
"					shadeColor.b *= texColor.b;\n"
"				}\n"
"#endif\n"
"\n"
"				throughput.r *= shadeColor.r;\n"
"				throughput.g *= shadeColor.g;\n"
"				throughput.b *= shadeColor.b;\n"
"\n"
"				// Interpolate the normal\n"
"				Vector N;\n"
"#if defined(PARAM_ACCEL_MQBVH)\n"
"				Mesh_InterpolateNormal(iVertNormals, iTriangles, triangleID, hitPointB1, hitPointB2, &N);\n"
"				// (__global float (*)[4]) seems required by Apple OpenCL\n"
"				TransformNormal((__global float (*)[4])(meshDesc->invTrans), &N);\n"
"#else\n"
"				Mesh_InterpolateNormal(vertNormals, triangles, currentTriangleIndex, hitPointB1, hitPointB2, &N);\n"
"#endif\n"
"\n"
"#if defined(PARAM_HAS_BUMPMAPS)\n"
"				// Check it the mesh has a bump map\n"
"				unsigned int bumpIndex = meshBumpsBuff[meshIndex];\n"
"				if (bumpIndex != 0xffffffffu) {\n"
"					// Apply bump mapping\n"
"					__global TexMap *texMap = &texMapDescBuff[bumpIndex];\n"
"					const uint texWidth = texMap->width;\n"
"					const uint texHeight = texMap->height;\n"
"\n"
"					UV dudv;\n"
"					dudv.u = 1.f / texWidth;\n"
"					dudv.v = 1.f / texHeight;\n"
"\n"
"					Spectrum texColor;\n"
"					TexMap_GetColor(&texMapRGBBuff[texMap->rgbOffset], texWidth, texHeight, uv.u, uv.v, &texColor);\n"
"					const float b0 = Spectrum_Y(&texColor);\n"
"\n"
"					TexMap_GetColor(&texMapRGBBuff[texMap->rgbOffset], texWidth, texHeight, uv.u + dudv.u, uv.v, &texColor);\n"
"					const float bu = Spectrum_Y(&texColor);\n"
"\n"
"					TexMap_GetColor(&texMapRGBBuff[texMap->rgbOffset], texWidth, texHeight, uv.u, uv.v + dudv.v, &texColor);\n"
"					const float bv = Spectrum_Y(&texColor);\n"
"\n"
"					const float scale = meshBumpsScaleBuff[meshIndex];\n"
"					Vector bump;\n"
"					bump.x = scale * (bu - b0);\n"
"					bump.y = scale * (bv - b0);\n"
"					bump.z = 1.f;\n"
"\n"
"					Vector v1, v2;\n"
"					CoordinateSystem(&N, &v1, &v2);\n"
"					N.x = v1.x * bump.x + v2.x * bump.y + N.x * bump.z;\n"
"					N.y = v1.y * bump.x + v2.y * bump.y + N.y * bump.z;\n"
"					N.z = v1.z * bump.x + v2.z * bump.y + N.z * bump.z;\n"
"					Normalize(&N);\n"
"				}\n"
"#endif\n"
"\n"
"				// Flip the normal if required\n"
"				Vector shadeN;\n"
"				const float nFlip = (Dot(&rayDir, &N) > 0.f) ? -1.f : 1.f;\n"
"				shadeN.x = nFlip * N.x;\n"
"				shadeN.y = nFlip * N.y;\n"
"				shadeN.z = nFlip * N.z;\n"
"\n"
"#if (PARAM_SAMPLER_TYPE == 0)\n"
"				const float u0 = RndFloatValue(&seed);\n"
"				const float u1 = RndFloatValue(&seed);\n"
"#elif (PARAM_SAMPLER_TYPE == 1) ||(PARAM_SAMPLER_TYPE == 2) || (PARAM_SAMPLER_TYPE == 3)\n"
"				const float u0 = sampleData[IDX_BSDF_X];\n"
"				const float u1 = sampleData[IDX_BSDF_Y];\n"
"#endif\n"
"\n"
"#if defined(PARAM_ENABLE_MAT_MATTEMIRROR) || defined(PARAM_ENABLE_MAT_MATTEMETAL) || defined(PARAM_ENABLE_MAT_ALLOY)\n"
"#if (PARAM_SAMPLER_TYPE == 0)\n"
"				const float u2 = RndFloatValue(&seed);\n"
"#elif (PARAM_SAMPLER_TYPE == 1) ||(PARAM_SAMPLER_TYPE == 2) || (PARAM_SAMPLER_TYPE == 3)\n"
"				const float u2 = sampleData[IDX_BSDF_Z];\n"
"#endif\n"
"#endif\n"
"\n"
"				Vector wo;\n"
"				wo.x = -rayDir.x;\n"
"				wo.y = -rayDir.y;\n"
"				wo.z = -rayDir.z;\n"
"\n"
"				Vector wi;\n"
"				Spectrum f;\n"
"				f.r = 1.f;\n"
"				f.g = 1.f;\n"
"				f.b = 1.f;\n"
"				float materialPdf;\n"
"#if defined(PARAM_DIRECT_LIGHT_SAMPLING)\n"
"				int specularMaterial;\n"
"#endif\n"
"\n"
"				switch (matType) {\n"
"\n"
"#if defined(PARAM_ENABLE_MAT_MATTE)\n"
"					case MAT_MATTE:\n"
"						Matte_Sample_f(&hitPointMat->param.matte, &wo, &wi, &materialPdf, &f, &shadeN, u0, u1\n"
"#if defined(PARAM_DIRECT_LIGHT_SAMPLING)\n"
"								, &specularMaterial\n"
"#endif\n"
"								);\n"
"						break;\n"
"#endif\n"
"\n"
"#if (PARAM_DL_LIGHT_COUNT > 0)\n"
"					case MAT_AREALIGHT: {\n"
"						Spectrum Le;\n"
"						AreaLight_Le(&hitPointMat->param.areaLight, &wo, &N, &Le);\n"
"\n"
"#if defined(PARAM_DIRECT_LIGHT_SAMPLING)\n"
"						if (!task->pathState.specularBounce) {\n"
"#if defined(PARAM_HAS_SUNLIGHT)\n"
"							const uint lightSourceCount = PARAM_DL_LIGHT_COUNT + 1;\n"
"#else\n"
"							const uint lightSourceCount = PARAM_DL_LIGHT_COUNT;\n"
"#endif\n"
"#if defined(PARAM_ACCEL_MQBVH)\n"
"							// (__global float (*)[4]) seems required by Apple OpenCL\n"
"							const float area = InstanceMesh_Area((__global float (*)[4])(meshDesc->trans), iVertices, iTriangles, triangleID);\n"
"#else\n"
"							const float area = Mesh_Area(vertices, triangles, currentTriangleIndex);\n"
"#endif\n"
"							const float lpdf = lightSourceCount / area;\n"
"							const float ph = PowerHeuristic(1, task->pathState.bouncePdf, 1, lpdf);\n"
"\n"
"							Le.r *= ph;\n"
"							Le.g *= ph;\n"
"							Le.b *= ph;\n"
"						}\n"
"#endif\n"
"\n"
"						sample->radiance.r += throughput.r * Le.r;\n"
"						sample->radiance.g += throughput.g * Le.g;\n"
"						sample->radiance.b += throughput.b * Le.b;\n"
"\n"
"						materialPdf = 0.f;\n"
"						break;\n"
"					}\n"
"#endif\n"
"\n"
"#if defined(PARAM_ENABLE_MAT_MIRROR)\n"
"					case MAT_MIRROR:\n"
"						Mirror_Sample_f(&hitPointMat->param.mirror, &wo, &wi, &materialPdf, &f, &shadeN\n"
"#if defined(PARAM_DIRECT_LIGHT_SAMPLING)\n"
"								, &specularMaterial\n"
"#endif\n"
"								);\n"
"						break;\n"
"#endif\n"
"\n"
"#if defined(PARAM_ENABLE_MAT_GLASS)\n"
"					case MAT_GLASS:\n"
"						Glass_Sample_f(&hitPointMat->param.glass, &wo, &wi, &materialPdf, &f, &N, &shadeN, u0\n"
"#if defined(PARAM_DIRECT_LIGHT_SAMPLING)\n"
"								, &specularMaterial\n"
"#endif\n"
"								);\n"
"						break;\n"
"#endif\n"
"\n"
"#if defined(PARAM_ENABLE_MAT_MATTEMIRROR)\n"
"					case MAT_MATTEMIRROR:\n"
"						MatteMirror_Sample_f(&hitPointMat->param.matteMirror, &wo, &wi, &materialPdf, &f, &shadeN, u0, u1, u2\n"
"#if defined(PARAM_DIRECT_LIGHT_SAMPLING)\n"
"								, &specularMaterial\n"
"#endif\n"
"								);\n"
"						break;\n"
"#endif\n"
"\n"
"#if defined(PARAM_ENABLE_MAT_METAL)\n"
"					case MAT_METAL:\n"
"						Metal_Sample_f(&hitPointMat->param.metal, &wo, &wi, &materialPdf, &f, &shadeN, u0, u1\n"
"#if defined(PARAM_DIRECT_LIGHT_SAMPLING)\n"
"								, &specularMaterial\n"
"#endif\n"
"								);\n"
"						break;\n"
"#endif\n"
"\n"
"#if defined(PARAM_ENABLE_MAT_MATTEMETAL)\n"
"					case MAT_MATTEMETAL:\n"
"						MatteMetal_Sample_f(&hitPointMat->param.matteMetal, &wo, &wi, &materialPdf, &f, &shadeN, u0, u1, u2\n"
"#if defined(PARAM_DIRECT_LIGHT_SAMPLING)\n"
"								, &specularMaterial\n"
"#endif\n"
"								);\n"
"						break;\n"
"#endif\n"
"\n"
"#if defined(PARAM_ENABLE_MAT_ALLOY)\n"
"					case MAT_ALLOY:\n"
"						Alloy_Sample_f(&hitPointMat->param.alloy, &wo, &wi, &materialPdf, &f, &shadeN, u0, u1, u2\n"
"#if defined(PARAM_DIRECT_LIGHT_SAMPLING)\n"
"								, &specularMaterial\n"
"#endif\n"
"								);\n"
"						break;\n"
"#endif\n"
"\n"
"#if defined(PARAM_ENABLE_MAT_ARCHGLASS)\n"
"					case MAT_ARCHGLASS:\n"
"						ArchGlass_Sample_f(&hitPointMat->param.archGlass, &wo, &wi, &materialPdf, &f, &N, &shadeN, u0\n"
"#if defined(PARAM_DIRECT_LIGHT_SAMPLING)\n"
"								, &specularMaterial\n"
"#endif\n"
"								);\n"
"						break;\n"
"#endif\n"
"\n"
"					case MAT_NULL:\n"
"						wi = rayDir;\n"
"						materialPdf = 1.f;\n"
"\n"
"						// I have also to restore the original throughput\n"
"						throughput = prevThroughput;\n"
"						break;\n"
"\n"
"					default:\n"
"						// Huston, we have a problem...\n"
"						materialPdf = 0.f;\n"
"						break;\n"
"				}\n"
"\n"
"				// Russian roulette\n"
"\n"
"#if (PARAM_SAMPLER_TYPE == 0)\n"
"				const float rrSample = RndFloatValue(&seed);\n"
"#elif (PARAM_SAMPLER_TYPE == 1) || (PARAM_SAMPLER_TYPE == 2) || (PARAM_SAMPLER_TYPE == 3)\n"
"				const float rrSample = sampleData[IDX_RR];\n"
"#endif\n"
"\n"
"				const float rrProb = max(max(throughput.r, max(throughput.g, throughput.b)), (float) PARAM_RR_CAP);\n"
"				pathDepth += 1;\n"
"				float invRRProb = (pathDepth > PARAM_RR_DEPTH) ? ((rrProb < rrSample) ? 0.f : (1.f / rrProb)) : 1.f;\n"
"				invRRProb = ((materialPdf <= 0.f) || (pathDepth >= PARAM_MAX_PATH_DEPTH)) ? 0.f : invRRProb;\n"
"				throughput.r *= f.r * invRRProb;\n"
"				throughput.g *= f.g * invRRProb;\n"
"				throughput.b *= f.b * invRRProb;\n"
"\n"
"				//if (pathDepth > 2)\n"
"				//	printf(\"Depth: %d Throughput: (%f, %f, %f)\\n\", pathDepth, throughput.r, throughput.g, throughput.b);\n"
"\n"
"#if defined(PARAM_DIRECT_LIGHT_SAMPLING)\n"
"				float directLightPdf;\n"
"				switch (matType) {\n"
"					case MAT_MATTE:\n"
"						directLightPdf = 1.f;\n"
"						break;\n"
"					case MAT_MATTEMIRROR:\n"
"						directLightPdf = hitPointMat->param.matteMirror.mattePdf;\n"
"						break;\n"
"					case MAT_MATTEMETAL:\n"
"						directLightPdf = hitPointMat->param.matteMetal.mattePdf;\n"
"						break;\n"
"					case MAT_ALLOY: {\n"
"						// Schilick's approximation\n"
"						const float c = 1.f + Dot(&rayDir, &shadeN);\n"
"						const float R0 = hitPointMat->param.alloy.R0;\n"
"						const float Re = R0 + (1.f - R0) * c * c * c * c * c;\n"
"\n"
"						const float P = .25f + .5f * Re;\n"
"\n"
"						directLightPdf = 1.f - P;\n"
"						break;\n"
"					}\n"
"					default:\n"
"						directLightPdf = 0.f;\n"
"						break;\n"
"				}\n"
"\n"
"				if (directLightPdf > 0.f) {\n"
"#if (PARAM_SAMPLER_TYPE == 0)\n"
"					const float ul0 = RndFloatValue(&seed);\n"
"					const float ul1 = RndFloatValue(&seed);\n"
"					const float ul2 = RndFloatValue(&seed);\n"
"#elif (PARAM_SAMPLER_TYPE == 1) || (PARAM_SAMPLER_TYPE == 2) || (PARAM_SAMPLER_TYPE == 3)\n"
"					const float ul1 = sampleData[IDX_DIRECTLIGHT_X];\n"
"					const float ul2 = sampleData[IDX_DIRECTLIGHT_Y];\n"
"					const float ul0 = sampleData[IDX_DIRECTLIGHT_Z];\n"
"#endif\n"
"\n"
"					// Select a light source to sample\n"
"\n"
"					// Setup the shadow ray\n"
"					Spectrum Le;\n"
"					uint lightSourceCount;\n"
"					float lightPdf;\n"
"					float lPdf; // pdf used for MIS\n"
"					Ray shadowRay;\n"
"\n"
"#if defined(PARAM_HAS_SUNLIGHT) && (PARAM_DL_LIGHT_COUNT == 0)\n"
"					//----------------------------------------------------------\n"
"					// This is the case with only the sun light\n"
"					//----------------------------------------------------------\n"
"\n"
"					SunLight_Sample_L(sunLight, &hitPoint, &lightPdf, &Le, &shadowRay, ul1, ul2);\n"
"					lPdf = lightPdf;\n"
"					lightSourceCount = 1;\n"
"\n"
"					//----------------------------------------------------------\n"
"#elif defined(PARAM_HAS_SUNLIGHT) && (PARAM_DL_LIGHT_COUNT > 0)\n"
"					//----------------------------------------------------------\n"
"					// This is the case with sun light and area lights\n"
"					//----------------------------------------------------------\n"
"\n"
"					// Select one of the lights\n"
"					const uint lightIndex = min((uint)floor((PARAM_DL_LIGHT_COUNT + 1)* ul0), (uint)(PARAM_DL_LIGHT_COUNT));\n"
"					lightSourceCount = PARAM_DL_LIGHT_COUNT + 1;\n"
"\n"
"					if (lightIndex == PARAM_DL_LIGHT_COUNT) {\n"
"						// The sun light was selected\n"
"\n"
"						SunLight_Sample_L(sunLight, &hitPoint, &lightPdf, &Le, &shadowRay, ul1, ul2);\n"
"						lPdf = lightPdf;\n"
"					} else {\n"
"						// An area light was selected\n"
"\n"
"						__global TriangleLight *l = &triLights[lightIndex];\n"
"						TriangleLight_Sample_L(l, &hitPoint, &lightPdf, &Le, &shadowRay, ul1, ul2);\n"
"						lPdf = PARAM_DL_LIGHT_COUNT / l->area;\n"
"					}\n"
"\n"
"					//----------------------------------------------------------\n"
"#elif !defined(PARAM_HAS_SUNLIGHT) && (PARAM_DL_LIGHT_COUNT > 0)\n"
"					//----------------------------------------------------------\n"
"					// This is the case without sun light and with area lights\n"
"					//----------------------------------------------------------\n"
"\n"
"					// Select one of the area lights\n"
"					const uint lightIndex = min((uint)floor(PARAM_DL_LIGHT_COUNT * ul0), (uint)(PARAM_DL_LIGHT_COUNT - 1));\n"
"					__global TriangleLight *l = &triLights[lightIndex];\n"
"\n"
"					TriangleLight_Sample_L(l, &hitPoint, &lightPdf, &Le, &shadowRay, ul1, ul2);\n"
"					lPdf = PARAM_DL_LIGHT_COUNT / l->area;\n"
"					lightSourceCount = PARAM_DL_LIGHT_COUNT;\n"
"\n"
"					//----------------------------------------------------------\n"
"#else\n"
"Error: Huston, we have a problem !\n"
"#endif\n"
"\n"
"					const float dp = Dot(&shadeN, &shadowRay.d);\n"
"					const float matPdf = M_PI;\n"
"\n"
"					const float mPdf = directLightPdf * dp * INV_PI;\n"
"					const float pdf = (dp <= 0.f) ? 0.f :\n"
"						(PowerHeuristic(1, lPdf, 1, mPdf) * lightPdf * directLightPdf * matPdf / (dp * lightSourceCount));\n"
"					if (pdf > 0.f) {\n"
"						Spectrum throughputLightDir = prevThroughput;\n"
"						throughputLightDir.r *= shadeColor.r;\n"
"						throughputLightDir.g *= shadeColor.g;\n"
"						throughputLightDir.b *= shadeColor.b;\n"
"\n"
"						const float k = 1.f / pdf;\n"
"						// NOTE: I assume all matte mixed material have a MatteParam as first field\n"
"						task->pathState.lightRadiance.r = throughputLightDir.r * hitPointMat->param.matte.r * Le.r * k;\n"
"						task->pathState.lightRadiance.g = throughputLightDir.g * hitPointMat->param.matte.g * Le.g * k;\n"
"						task->pathState.lightRadiance.b = throughputLightDir.b * hitPointMat->param.matte.b * Le.b * k;\n"
"\n"
"						*ray = shadowRay;\n"
"\n"
"						// Save data for next path vertex\n"
"						task->pathState.nextPathRay.o = hitPoint;\n"
"						task->pathState.nextPathRay.d = wi;\n"
"						task->pathState.nextPathRay.mint = PARAM_RAY_EPSILON;\n"
"						task->pathState.nextPathRay.maxt = FLT_MAX;\n"
"\n"
"						task->pathState.bouncePdf = materialPdf;\n"
"						task->pathState.specularBounce = specularMaterial;\n"
"						task->pathState.nextThroughput = throughput;\n"
"\n"
"						pathState = PATH_STATE_SAMPLE_LIGHT;\n"
"					} else {\n"
"						// Skip the shadow ray tracing step\n"
"\n"
"						if ((throughput.r <= 0.f) && (throughput.g <= 0.f) && (throughput.b <= 0.f))\n"
"							pathState = PATH_STATE_DONE;\n"
"						else {\n"
"							ray->o = hitPoint;\n"
"							ray->d = wi;\n"
"							ray->mint = PARAM_RAY_EPSILON;\n"
"							ray->maxt = FLT_MAX;\n"
"\n"
"							task->pathState.bouncePdf = materialPdf;\n"
"							task->pathState.specularBounce = specularMaterial;\n"
"							task->pathState.throughput = throughput;\n"
"							task->pathState.depth = pathDepth;\n"
"\n"
"							pathState = PATH_STATE_NEXT_VERTEX;\n"
"						}\n"
"					}\n"
"				} else {\n"
"					// Skip the shadow ray tracing step\n"
"\n"
"					if ((throughput.r <= 0.f) && (throughput.g <= 0.f) && (throughput.b <= 0.f))\n"
"						pathState = PATH_STATE_DONE;\n"
"					else {\n"
"						ray->o = hitPoint;\n"
"						ray->d = wi;\n"
"						ray->mint = PARAM_RAY_EPSILON;\n"
"						ray->maxt = FLT_MAX;\n"
"\n"
"						task->pathState.bouncePdf = materialPdf;\n"
"						task->pathState.specularBounce = specularMaterial;\n"
"						task->pathState.throughput = throughput;\n"
"						task->pathState.depth = pathDepth;\n"
"\n"
"						pathState = PATH_STATE_NEXT_VERTEX;\n"
"					}\n"
"				}\n"
"\n"
"#else\n"
"\n"
"				if ((throughput.r <= 0.f) && (throughput.g <= 0.f) && (throughput.b <= 0.f))\n"
"					pathState = PATH_STATE_DONE;\n"
"				else {\n"
"					// Setup next ray\n"
"					ray->o = hitPoint;\n"
"					ray->d = wi;\n"
"					ray->mint = PARAM_RAY_EPSILON;\n"
"					ray->maxt = FLT_MAX;\n"
"\n"
"					task->pathState.throughput = throughput;\n"
"					task->pathState.depth = pathDepth;\n"
"\n"
"					pathState = PATH_STATE_NEXT_VERTEX;\n"
"				}\n"
"#endif\n"
"\n"
"			} else {\n"
"#if defined(PARAM_HAS_INFINITELIGHT)\n"
"				Spectrum iLe;\n"
"				InfiniteLight_Le(infiniteLight, infiniteLightMap, &iLe, &rayDir);\n"
"\n"
"				sample->radiance.r += throughput.r * iLe.r;\n"
"				sample->radiance.g += throughput.g * iLe.g;\n"
"				sample->radiance.b += throughput.b * iLe.b;\n"
"#endif\n"
"\n"
"#if defined(PARAM_HAS_SUNLIGHT)\n"
"				// Make the sun visible only if relsize has been changed (in order\n"
"				// to avoid fireflies).\n"
"				if (sunLight->relSize > 5.f) {\n"
"					Spectrum sLe;\n"
"					SunLight_Le(sunLight, &sLe, &rayDir);\n"
"\n"
"					if (!task->pathState.specularBounce) {\n"
"						const float lpdf = UniformConePdf(sunLight->cosThetaMax);\n"
"						const float ph = PowerHeuristic(1, task->pathState.bouncePdf, 1, lpdf);\n"
"\n"
"						sLe.r *= ph;\n"
"						sLe.g *= ph;\n"
"						sLe.b *= ph;\n"
"					}\n"
"\n"
"					sample->radiance.r += throughput.r * sLe.r;\n"
"					sample->radiance.g += throughput.g * sLe.g;\n"
"					sample->radiance.b += throughput.b * sLe.b;\n"
"				}\n"
"#endif\n"
"\n"
"#if defined(PARAM_HAS_SKYLIGHT)\n"
"				Spectrum skLe;\n"
"				SkyLight_Le(skyLight, &skLe, &rayDir);\n"
"\n"
"				sample->radiance.r += throughput.r * skLe.r;\n"
"				sample->radiance.g += throughput.g * skLe.g;\n"
"				sample->radiance.b += throughput.b * skLe.b;\n"
"#endif\n"
"\n"
"				pathState = PATH_STATE_DONE;\n"
"			}\n"
"			break;\n"
"		}\n"
"\n"
"#if defined(PARAM_DIRECT_LIGHT_SAMPLING)\n"
"		case PATH_STATE_SAMPLE_LIGHT: {\n"
"			if (currentTriangleIndex != 0xffffffffu) {\n"
"				// The shadow ray has hit something\n"
"\n"
"#if defined(PARAM_HAS_TEXTUREMAPS) && defined(PARAM_HAS_ALPHA_TEXTUREMAPS)\n"
"				// Check if I have to continue to trace the shadow ray\n"
"\n"
"				const uint pathDepth = task->pathState.depth;\n"
"#if (PARAM_SAMPLER_TYPE == 1) || (PARAM_SAMPLER_TYPE == 3)\n"
"				__global float *sampleData = &sample->u[IDX_BSDF_OFFSET + SAMPLE_SIZE * pathDepth];\n"
"#elif (PARAM_SAMPLER_TYPE == 2)\n"
"				__global float *sampleData = &sample->u[sample->proposed][IDX_BSDF_OFFSET + SAMPLE_SIZE * pathDepth];\n"
"#endif\n"
"\n"
"				const uint meshIndex = meshIDs[currentTriangleIndex];\n"
"				__global Material *hitPointMat = &mats[meshMats[meshIndex]];\n"
"				uint matType = hitPointMat->type;\n"
"\n"
"				// Interpolate UV coordinates\n"
"				UV uv;\n"
"#if defined(PARAM_ACCEL_MQBVH)\n"
"				__global Mesh *meshDesc = &meshDescs[meshIndex];\n"
"				__global UV *iVertUVs = &vertUVs[meshDesc->vertsOffset];\n"
"				__global Triangle *iTriangles = &triangles[meshDesc->trisOffset];\n"
"				const uint triangleID = triangleIDs[currentTriangleIndex];\n"
"				Mesh_InterpolateUV(iVertUVs, iTriangles, triangleID, hitPointB1, hitPointB2, &uv);\n"
"#else\n"
"				Mesh_InterpolateUV(vertUVs, triangles, currentTriangleIndex, hitPointB1, hitPointB2, &uv);\n"
"#endif\n"
"\n"
"				// Check it the mesh has a texture map\n"
"				unsigned int texIndex = meshTexsBuff[meshIndex];\n"
"				if (texIndex != 0xffffffffu) {\n"
"					__global TexMap *texMap = &texMapDescBuff[texIndex];\n"
"\n"
"					// Check if it has an alpha channel\n"
"					if (texMap->alphaOffset != 0xffffffffu) {\n"
"						const float alpha = TexMap_GetAlpha(&texMapAlphaBuff[texMap->alphaOffset], texMap->width, texMap->height, uv.u, uv.v);\n"
"\n"
"#if (PARAM_SAMPLER_TYPE == 0)\n"
"						const float texAlphaSample = RndFloatValue(&seed);\n"
"#elif (PARAM_SAMPLER_TYPE == 1) || (PARAM_SAMPLER_TYPE == 2) || (PARAM_SAMPLER_TYPE == 3)\n"
"						const float texAlphaSample = sampleData[IDX_TEX_ALPHA];\n"
"#endif\n"
"\n"
"						if ((alpha == 0.0f) || ((alpha < 1.f) && (texAlphaSample > alpha))) {\n"
"							// Continue to trace the ray\n"
"							matType = MAT_NULL;\n"
"						}\n"
"					}\n"
"				}\n"
"\n"
"				if (matType == MAT_ARCHGLASS) {\n"
"					task->pathState.lightRadiance.r *= hitPointMat->param.archGlass.refrct_r;\n"
"					task->pathState.lightRadiance.g *= hitPointMat->param.archGlass.refrct_g;\n"
"					task->pathState.lightRadiance.b *= hitPointMat->param.archGlass.refrct_b;\n"
"				}\n"
"\n"
"				if ((matType == MAT_ARCHGLASS) || (matType == MAT_NULL)) {\n"
"					const float hitPointT = rayHit->t;\n"
"\n"
"					Point hitPoint;\n"
"					hitPoint.x = ray->o.x + rayDir.x * hitPointT;\n"
"					hitPoint.y = ray->o.y + rayDir.y * hitPointT;\n"
"					hitPoint.z = ray->o.z + rayDir.z * hitPointT;\n"
"\n"
"					// Continue to trace the ray\n"
"					ray->o = hitPoint;\n"
"					ray->maxt -= hitPointT;\n"
"				} else\n"
"					pathState = PATH_STATE_NEXT_VERTEX;\n"
"\n"
"#else\n"
"				// The light is source is not visible\n"
"\n"
"				pathState = PATH_STATE_NEXT_VERTEX;\n"
"#endif\n"
"\n"
"			} else {\n"
"				// The light source is visible\n"
"\n"
"				sample->radiance.r += task->pathState.lightRadiance.r;\n"
"				sample->radiance.g += task->pathState.lightRadiance.g;\n"
"				sample->radiance.b += task->pathState.lightRadiance.b;\n"
"\n"
"				pathState = PATH_STATE_NEXT_VERTEX;\n"
"			}\n"
"\n"
"\n"
"			if (pathState == PATH_STATE_NEXT_VERTEX) {\n"
"				Spectrum throughput = task->pathState.nextThroughput;\n"
"				if ((throughput.r <= 0.f) && (throughput.g <= 0.f) && (throughput.b <= 0.f))\n"
"					pathState = PATH_STATE_DONE;\n"
"				else {\n"
"					// Restore the ray for the next path vertex\n"
"					*ray = task->pathState.nextPathRay;\n"
"\n"
"					task->pathState.throughput = throughput;\n"
"\n"
"					// Increase path depth\n"
"					task->pathState.depth += 1;\n"
"				}\n"
"			}\n"
"			break;\n"
"		}\n"
"#endif\n"
"	}\n"
"\n"
"	if (pathState == PATH_STATE_DONE) {\n"
"#if (PARAM_SAMPLER_TYPE == 2)\n"
"\n"
"		// Read the seed\n"
"		Seed seed;\n"
"		seed.s1 = task->seed.s1;\n"
"		seed.s2 = task->seed.s2;\n"
"		seed.s3 = task->seed.s3;\n"
"\n"
"		Sampler_MLT_SplatSample(frameBuffer, &seed, sample);\n"
"\n"
"		// Save the seed\n"
"		task->seed.s1 = seed.s1;\n"
"		task->seed.s2 = seed.s2;\n"
"		task->seed.s3 = seed.s3;\n"
"\n"
"#else\n"
"\n"
"#if (PARAM_IMAGE_FILTER_TYPE == 0)\n"
"		Spectrum radiance = sample->radiance;\n"
"		SplatSample(frameBuffer, sample->pixelIndex, &radiance, 1.f);\n"
"#else\n"
"		__global float *sampleData = &sample->u[0];\n"
"		const float sx = sampleData[IDX_SCREEN_X] - .5f;\n"
"		const float sy = sampleData[IDX_SCREEN_Y] - .5f;\n"
"\n"
"		Spectrum radiance = sample->radiance;\n"
"		SplatSample(frameBuffer, sample->pixelIndex, sx, sy, &radiance, 1.f);\n"
"#endif\n"
"\n"
"#endif\n"
"	}\n"
"\n"
"	task->pathState.state = pathState;\n"
"\n"
"#if (PARAM_SAMPLER_TYPE == 0)\n"
"	// Save the seed\n"
"	task->seed.s1 = seed.s1;\n"
"	task->seed.s2 = seed.s2;\n"
"	task->seed.s3 = seed.s3;\n"
"#endif\n"
"}\n"
;
