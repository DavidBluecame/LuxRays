#include <string>
namespace slg { namespace ocl {
std::string KernelSource_pathoclbase_funcs = 
"#line 2 \"patchoclbase_funcs.cl\"\n"
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
"//  PARAM_RAY_EPSILON_MIN\n"
"//  PARAM_RAY_EPSILON_MAX\n"
"//  PARAM_HAS_IMAGEMAPS\n"
"//  PARAM_HAS_PASSTHROUGH\n"
"//  PARAM_USE_PIXEL_ATOMICS\n"
"//  PARAM_HAS_BUMPMAPS\n"
"//  PARAM_ACCEL_BVH or PARAM_ACCEL_MBVH or PARAM_ACCEL_QBVH or PARAM_ACCEL_MQBVH\n"
"//  PARAM_DEVICE_INDEX\n"
"//  PARAM_DEVICE_COUNT\n"
"//  PARAM_LIGHT_WORLD_RADIUS_SCALE\n"
"//  PARAM_TRIANGLE_LIGHT_COUNT\n"
"//  PARAM_LIGHT_COUNT\n"
"//  PARAM_HAS_VOLUMEs (and SCENE_DEFAULT_VOLUME_INDEX)\n"
"\n"
"// To enable single material support\n"
"//  PARAM_ENABLE_MAT_MATTE\n"
"//  PARAM_ENABLE_MAT_MIRROR\n"
"//  PARAM_ENABLE_MAT_GLASS\n"
"//  PARAM_ENABLE_MAT_ARCHGLASS\n"
"//  PARAM_ENABLE_MAT_MIX\n"
"//  PARAM_ENABLE_MAT_NULL\n"
"//  PARAM_ENABLE_MAT_MATTETRANSLUCENT\n"
"//  PARAM_ENABLE_MAT_GLOSSY2\n"
"//  PARAM_ENABLE_MAT_METAL2\n"
"//  PARAM_ENABLE_MAT_ROUGHGLASS\n"
"//  PARAM_ENABLE_MAT_CLOTH\n"
"//  PARAM_ENABLE_MAT_CARPAINT\n"
"//  PARAM_ENABLE_MAT_CLEAR_VOL\n"
"\n"
"// To enable single texture support\n"
"//  PARAM_ENABLE_TEX_CONST_FLOAT\n"
"//  PARAM_ENABLE_TEX_CONST_FLOAT3\n"
"//  PARAM_ENABLE_TEX_CONST_FLOAT4\n"
"//  PARAM_ENABLE_TEX_IMAGEMAP\n"
"//  PARAM_ENABLE_TEX_SCALE\n"
"//  etc.\n"
"\n"
"// Film related parameters:\n"
"//  PARAM_FILM_RADIANCE_GROUP_COUNT\n"
"//  PARAM_FILM_CHANNELS_HAS_ALPHA\n"
"//  PARAM_FILM_CHANNELS_HAS_DEPTH\n"
"//  PARAM_FILM_CHANNELS_HAS_POSITION\n"
"//  PARAM_FILM_CHANNELS_HAS_GEOMETRY_NORMAL\n"
"//  PARAM_FILM_CHANNELS_HAS_SHADING_NORMAL\n"
"//  PARAM_FILM_CHANNELS_HAS_MATERIAL_ID\n"
"//  PARAM_FILM_CHANNELS_HAS_DIRECT_DIFFUSE\n"
"//  PARAM_FILM_CHANNELS_HAS_DIRECT_GLOSSY\n"
"//  PARAM_FILM_CHANNELS_HAS_EMISSION\n"
"//  PARAM_FILM_CHANNELS_HAS_INDIRECT_DIFFUSE\n"
"//  PARAM_FILM_CHANNELS_HAS_INDIRECT_GLOSSY\n"
"//  PARAM_FILM_CHANNELS_HAS_INDIRECT_SPECULAR\n"
"//  PARAM_FILM_CHANNELS_HAS_MATERIAL_ID_MASK (and PARAM_FILM_MASK_MATERIAL_ID)\n"
"//  PARAM_FILM_CHANNELS_HAS_BY_MATERIAL_ID (and PARAM_FILM_BY_MATERIAL_ID)\n"
"//  PARAM_FILM_CHANNELS_HAS_DIRECT_SHADOW_MASK\n"
"//  PARAM_FILM_CHANNELS_HAS_INDIRECT_SHADOW_MASK\n"
"//  PARAM_FILM_CHANNELS_HAS_UV\n"
"//  PARAM_FILM_CHANNELS_HAS_RAYCOUNT\n"
"\n"
"// (optional)\n"
"//  PARAM_CAMERA_HAS_DOF\n"
"\n"
"// (optional)\n"
"//  PARAM_HAS_INFINITELIGHT\n"
"//  PARAM_HAS_SUNLIGHT\n"
"//  PARAM_HAS_SKYLIGHT\n"
"//  PARAM_HAS_SKYLIGHT2\n"
"//  PARAM_HAS_POINTLIGHT\n"
"//  PARAM_HAS_MAPPOINTLIGHT\n"
"//  PARAM_HAS_SPOTLIGHT\n"
"//  PARAM_HAS_PROJECTIONLIGHT\n"
"//  PARAM_HAS_CONSTANTINFINITELIGHT\n"
"//  PARAM_HAS_SHARPDISTANTLIGHT\n"
"//  PARAM_HAS_DISTANTLIGHT\n"
"//  PARAM_HAS_LASERLIGHT\n"
"//  PARAM_HAS_INFINITELIGHTS (if it has any infinite light)\n"
"//  PARAM_HAS_ENVLIGHTS (if it has any env. light)\n"
"\n"
"// (optional)\n"
"//  PARAM_HAS_NORMALS_BUFFER\n"
"//  PARAM_HAS_UVS_BUFFER\n"
"//  PARAM_HAS_COLS_BUFFER\n"
"//  PARAM_HAS_ALPHAS_BUFFER\n"
"\n"
"void MangleMemory(__global unsigned char *ptr, const size_t size) {\n"
"	Seed seed;\n"
"	Rnd_Init(7 + get_global_id(0), &seed);\n"
"\n"
"	for (uint i = 0; i < size; ++i)\n"
"		*ptr++ = (unsigned char)(Rnd_UintValue(&seed) & 0xff);\n"
"}\n"
"\n"
"bool Scene_Intersect(\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"		__global PathVolumeInfo *volInfo,\n"
"		__global HitPoint *tmpHitPoint,\n"
"#endif\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"		const float passThrough,\n"
"#endif\n"
"#if !defined(RENDER_ENGINE_BIASPATHOCL) && !defined(RENDER_ENGINE_RTBIASPATHOCL)\n"
"		__global\n"
"#endif\n"
"		Ray *ray,\n"
"#if !defined(RENDER_ENGINE_BIASPATHOCL) && !defined(RENDER_ENGINE_RTBIASPATHOCL)\n"
"		__global\n"
"#endif\n"
"		RayHit *rayHit,\n"
"		__global BSDF *bsdf,\n"
"#if !defined(RENDER_ENGINE_BIASPATHOCL) && !defined(RENDER_ENGINE_RTBIASPATHOCL)\n"
"		__global Spectrum *pathThroughput,\n"
"#else\n"
"		float3 *pathThroughput,\n"
"#endif\n"
"		\n"
"		__global SampleResult *sampleResult,\n"
"		// BSDF_Init parameters\n"
"		__global Mesh *meshDescs,\n"
"		__global uint *meshMats,\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"		__global uint *meshTriLightDefsOffset,\n"
"#endif\n"
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
"		__global Triangle *triangles\n"
"		MATERIALS_PARAM_DECL\n"
"		) {\n"
"	const bool hit = (rayHit->meshIndex != NULL_INDEX);\n"
"\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"	uint rayVolumeIndex = volInfo->currentVolumeIndex;\n"
"#endif\n"
"	if (hit) {\n"
"		// Initialize the BSDF of the hit point\n"
"		BSDF_Init(bsdf,\n"
"				meshDescs,\n"
"				meshMats,\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"				meshTriLightDefsOffset,\n"
"#endif\n"
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
"				triangles, ray, rayHit\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"				, passThrough\n"
"#endif\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"				, volInfo\n"
"#endif\n"
"				MATERIALS_PARAM\n"
"				);\n"
"\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"		rayVolumeIndex = bsdf->hitPoint.intoObject ? bsdf->hitPoint.exteriorVolumeIndex : bsdf->hitPoint.interiorVolumeIndex;\n"
"#endif\n"
"	}\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"	else if (rayVolumeIndex == NULL_INDEX) {\n"
"		// No volume information, I use the default volume\n"
"		rayVolumeIndex = SCENE_DEFAULT_VOLUME_INDEX;\n"
"	}\n"
"#endif\n"
"\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"	// Check if there is volume scatter event\n"
"	if (rayVolumeIndex != NULL_INDEX) {\n"
"		// This applies volume transmittance too\n"
"		// Note: by using passThrough here, I introduce subtle correlation\n"
"		// between scattering events and pass-through events\n"
"		float3 connectionThroughput = WHITE;\n"
"		float3 connectionEmission = BLACK;\n"
"\n"
"		const float t = Volume_Scatter(&mats[rayVolumeIndex], ray,\n"
"				hit ? rayHit->t : ray->maxt,\n"
"				passThrough, volInfo->scatteredStart,\n"
"				&connectionThroughput, &connectionEmission,\n"
"				tmpHitPoint\n"
"				TEXTURES_PARAM);\n"
"\n"
"#if !defined(RENDER_ENGINE_BIASPATHOCL) && !defined(RENDER_ENGINE_RTBIASPATHOCL)\n"
"		VSTORE3F(VLOAD3F(pathThroughput->c) * connectionThroughput, pathThroughput->c);\n"
"#else\n"
"		*pathThroughput *= connectionThroughput;\n"
"#endif\n"
"\n"
"		// Add the volume emitted light to the appropriate light group\n"
"		if (!Spectrum_IsBlack(connectionEmission) && sampleResult)\n"
"			SampleResult_AddEmission(sampleResult, BSDF_GetLightID(bsdf\n"
"				MATERIALS_PARAM), connectionEmission);\n"
"\n"
"		if (t > 0.f) {\n"
"			// There was a volume scatter event\n"
"\n"
"			// I have to set RayHit fields even if there wasn't a real\n"
"			// ray hit\n"
"			rayHit->t = t;\n"
"			// This is a trick in order to have RayHit::Miss() return\n"
"			// false. I assume 0xfffffffeu will trigger a memory fault if\n"
"			// used (and the bug will be noticed)\n"
"			rayHit->meshIndex = 0xfffffffeu;\n"
"\n"
"			BSDF_InitVolume(bsdf, ray, rayVolumeIndex, t, passThrough);\n"
"			volInfo->scatteredStart = true;\n"
"\n"
"			return false;\n"
"		}\n"
"	}\n"
"#endif\n"
"\n"
"	if (hit) {\n"
"		// Check if the volume priority system tells me to continue to trace the ray\n"
"		bool continueToTrace =\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"			PathVolumeInfo_ContinueToTrace(volInfo, bsdf\n"
"				MATERIALS_PARAM);\n"
"#else\n"
"		false;\n"
"#endif\n"
"\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"		// Check if it is a pass through point\n"
"		if (!continueToTrace) {\n"
"			const float3 passThroughTrans = BSDF_GetPassThroughTransparency(bsdf\n"
"				MATERIALS_PARAM);\n"
"			if (!Spectrum_IsBlack(passThroughTrans)) {\n"
"#if !defined(RENDER_ENGINE_BIASPATHOCL) && !defined(RENDER_ENGINE_RTBIASPATHOCL)\n"
"				VSTORE3F(VLOAD3F(pathThroughput->c) * passThroughTrans, pathThroughput->c);\n"
"#else\n"
"				*pathThroughput *= passThroughTrans;\n"
"#endif\n"
"				// It is a pass through point, continue to trace the ray\n"
"				continueToTrace = true;\n"
"			}\n"
"		}\n"
"#endif\n"
"\n"
"		if (continueToTrace) {\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"			// Update volume information\n"
"			const BSDFEvent eventTypes = BSDF_GetEventTypes(bsdf\n"
"						MATERIALS_PARAM);\n"
"			PathVolumeInfo_Update(volInfo, eventTypes, bsdf\n"
"					MATERIALS_PARAM);\n"
"#endif\n"
"\n"
"			// It is a transparent material, continue to trace the ray\n"
"			ray->mint = rayHit->t + MachineEpsilon_E(rayHit->t);\n"
"\n"
"			// A safety check\n"
"			if (ray->mint >= ray->maxt)\n"
"				return false;\n"
"			else\n"
"				return true;\n"
"		} else\n"
"			return false;\n"
"	} else {\n"
"		// Nothing was hit, stop tracing the ray\n"
"		return false;\n"
"	}\n"
"}\n"
; } }
