#include <string>
namespace slg { namespace ocl {
std::string KernelSource_biaspathocl_datatypes = 
"#line 2 \"biaspathocl_datatypes.cl\"\n"
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
"//------------------------------------------------------------------------------\n"
"// Some OpenCL specific definition\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(SLG_OPENCL_KERNEL)\n"
"\n"
"#if defined(PARAM_USE_PIXEL_ATOMICS)\n"
"#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable\n"
"#endif\n"
"\n"
"#ifndef TRUE\n"
"#define TRUE 1\n"
"#endif\n"
"\n"
"#ifndef FALSE\n"
"#define FALSE 0\n"
"#endif\n"
"\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// GPUTask data types\n"
"//------------------------------------------------------------------------------\n"
"\n"
"// This is defined only under OpenCL because of variable size structures\n"
"#if defined(SLG_OPENCL_KERNEL)\n"
"\n"
"typedef enum {\n"
"	// Micro-kernel states\n"
"	MK_GENERATE_CAMERA_RAY,\n"
"	MK_TRACE_EYE_RAY,\n"
"	MK_ILLUMINATE_EYE_MISS,\n"
"	MK_ILLUMINATE_EYE_HIT,\n"
"	MK_DL_VERTEX_1,\n"
"	MK_BSDF_SAMPLE,\n"
"	MK_DONE\n"
"} PathState;\n"
"\n"
"typedef struct {\n"
"	PathState pathState;\n"
"	float currentTime;\n"
"	\n"
"	BSDFEvent materialEventTypesPathVertex1;\n"
"	Spectrum throughputPathVertex1;\n"
"\n"
"	Ray tmpRay;\n"
"	RayHit tmpRayHit;\n"
"\n"
"	// The task seed\n"
"	Seed seed;\n"
"\n"
"	BSDF bsdfPathVertex1;\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"	PathVolumeInfo volInfoPathVertex1;\n"
"#endif\n"
"\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0) || defined(PARAM_HAS_VOLUMES)\n"
"	// This is used by TriangleLight_Illuminate() to temporary store the\n"
"	// point on the light sources\n"
"	// Also used by Scene_Intersect() for evaluating volume textures.\n"
"	HitPoint tmpHitPoint;\n"
"#endif\n"
"} GPUTask;\n"
"\n"
"typedef struct {\n"
"	BSDF directLightBSDF;\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"	PathVolumeInfo directLightVolInfo;\n"
"#endif\n"
"} GPUTaskDirectLight;\n"
"\n"
"typedef struct {\n"
"	BSDF bsdfPathVertexN;\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"	PathVolumeInfo volInfoPathVertexN;\n"
"#endif\n"
"} GPUTaskPathVertexN;\n"
"\n"
"#endif\n"
"\n"
"typedef struct {\n"
"	unsigned int raysCount;\n"
"} GPUTaskStats;\n"
; } }
