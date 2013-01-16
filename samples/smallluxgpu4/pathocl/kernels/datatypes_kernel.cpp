#include <string>
namespace slg { namespace ocl {
std::string KernelSource_datatypes = 
"#line 2 \"datatypes.cl\"\n"
"\n"
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
"// Some OpenCL specific definition\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(SLG_OPENCL_KERNEL)\n"
"\n"
"#if defined(PARAM_USE_PIXEL_ATOMICS)\n"
"#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable\n"
"#endif\n"
"\n"
"#if defined(PARAM_HAS_SUNLIGHT) & !defined(PARAM_DIRECT_LIGHT_SAMPLING)\n"
"Error: PARAM_HAS_SUNLIGHT requires PARAM_DIRECT_LIGHT_SAMPLING !\n"
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
"// Frame buffer data types\n"
"//------------------------------------------------------------------------------\n"
"\n"
"typedef struct {\n"
"	Spectrum c;\n"
"	float count;\n"
"} Pixel;\n"
"\n"
"typedef struct {\n"
"	float alpha;\n"
"} AlphaPixel;\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// Sample data types\n"
"//------------------------------------------------------------------------------\n"
"\n"
"typedef struct {\n"
"	Spectrum radiance;\n"
"	float alpha;\n"
"} RandomSampleWithAlphaChannel;\n"
"\n"
"typedef struct {\n"
"	Spectrum radiance;\n"
"} RandomSampleWithoutAlphaChannel;\n"
"\n"
"typedef struct {\n"
"	Spectrum radiance;\n"
"	float alpha;\n"
"\n"
"	float totalI;\n"
"\n"
"	// Using ushort here totally freeze the ATI driver\n"
"	unsigned int largeMutationCount, smallMutationCount;\n"
"	unsigned int current, proposed, consecutiveRejects;\n"
"\n"
"	float weight;\n"
"	Spectrum currentRadiance;\n"
"	float currentAlpha;\n"
"} MetropolisSampleWithAlphaChannel;\n"
"\n"
"typedef struct {\n"
"	Spectrum radiance;\n"
"\n"
"	float totalI;\n"
"\n"
"	// Using ushort here totally freeze the ATI driver\n"
"	unsigned int largeMutationCount, smallMutationCount;\n"
"	unsigned int current, proposed, consecutiveRejects;\n"
"\n"
"	float weight;\n"
"	Spectrum currentRadiance;\n"
"} MetropolisSampleWithoutAlphaChannel;\n"
"\n"
"#if defined(SLG_OPENCL_KERNEL)\n"
"\n"
"#if (PARAM_SAMPLER_TYPE == 0)\n"
"#if defined(PARAM_ENABLE_ALPHA_CHANNEL)\n"
"typedef RandomSampleWithAlphaChannel Sample;\n"
"#else\n"
"typedef RandomSampleWithoutAlphaChannel Sample;\n"
"#endif\n"
"#endif\n"
"\n"
"#if (PARAM_SAMPLER_TYPE == 1)\n"
"#if defined(PARAM_ENABLE_ALPHA_CHANNEL)\n"
"typedef MetropolisSampleWithAlphaChannel Sample;\n"
"#else\n"
"typedef MetropolisSampleWithoutAlphaChannel Sample;\n"
"#endif\n"
"#endif\n"
"\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// Indices of Sample related u[] array\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(SLG_OPENCL_KERNEL)\n"
"\n"
"#define IDX_SCREEN_X 0\n"
"#define IDX_SCREEN_Y 1\n"
"#if defined(PARAM_CAMERA_HAS_DOF) && defined(PARAM_HAS_PASSTHROUGH)\n"
"#define IDX_EYE_PASSTHROUGH 2\n"
"#define IDX_DOF_X 3\n"
"#define IDX_DOF_Y 4\n"
"#define IDX_BSDF_OFFSET 5\n"
"#elif defined(PARAM_CAMERA_HAS_DOF)\n"
"#define IDX_DOF_X 2\n"
"#define IDX_DOF_Y 3\n"
"#define IDX_BSDF_OFFSET 4\n"
"#elif defined(PARAM_HAS_PASSTHROUGH)\n"
"#define IDX_EYE_PASSTHROUGH 2\n"
"#define IDX_BSDF_OFFSET 3\n"
"#else\n"
"#define IDX_BSDF_OFFSET 2\n"
"#endif\n"
"\n"
"// Relative to IDX_BSDF_OFFSET + PathDepth * SAMPLE_SIZE\n"
"#if defined(PARAM_DIRECT_LIGHT_SAMPLING) && defined(PARAM_HAS_PASSTHROUGH)\n"
"\n"
"#define IDX_PASSTHROUGH 0\n"
"#define IDX_BSDF_X 1\n"
"#define IDX_BSDF_Y 2\n"
"#define IDX_DIRECTLIGHT_X 3\n"
"#define IDX_DIRECTLIGHT_Y 4\n"
"#define IDX_DIRECTLIGHT_Z 5\n"
"#define IDX_DIRECTLIGHT_W 6\n"
"#define IDX_DIRECTLIGHT_A 7\n"
"#define IDX_RR 8\n"
"\n"
"#define SAMPLE_SIZE 9\n"
"\n"
"#elif defined(PARAM_DIRECT_LIGHT_SAMPLING)\n"
"\n"
"#define IDX_BSDF_X 0\n"
"#define IDX_BSDF_Y 1\n"
"#define IDX_DIRECTLIGHT_X 2\n"
"#define IDX_DIRECTLIGHT_Y 3\n"
"#define IDX_DIRECTLIGHT_Z 4\n"
"#define IDX_DIRECTLIGHT_W 5\n"
"#define IDX_RR 6\n"
"\n"
"#define SAMPLE_SIZE 7\n"
"\n"
"#elif defined(PARAM_HAS_PASSTHROUGH)\n"
"\n"
"#define IDX_PASSTHROUGH 0\n"
"#define IDX_BSDF_X 1\n"
"#define IDX_BSDF_Y 2\n"
"#define IDX_RR 3\n"
"\n"
"#define SAMPLE_SIZE 4\n"
"\n"
"#else\n"
"\n"
"#define IDX_BSDF_X 0\n"
"#define IDX_BSDF_Y 1\n"
"#define IDX_RR 2\n"
"\n"
"#define SAMPLE_SIZE 3\n"
"\n"
"#endif\n"
"\n"
"#if (PARAM_SAMPLER_TYPE == 0)\n"
"#define TOTAL_U_SIZE 2\n"
"#endif\n"
"\n"
"#if (PARAM_SAMPLER_TYPE == 1)\n"
"#define TOTAL_U_SIZE (IDX_BSDF_OFFSET + PARAM_MAX_PATH_DEPTH * SAMPLE_SIZE)\n"
"#endif\n"
"\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// GPUTask data types\n"
"//------------------------------------------------------------------------------\n"
"\n"
"typedef enum {\n"
"	RT_NEXT_VERTEX,\n"
"	GENERATE_DL_RAY,\n"
"	RT_DL,\n"
"	GENERATE_NEXT_VERTEX_RAY,\n"
"	SPLAT_SAMPLE\n"
"} PathState;\n"
"\n"
"typedef struct {\n"
"	PathState state;\n"
"	unsigned int depth;\n"
"\n"
"	Spectrum throughput;\n"
"	BSDF bsdf;\n"
"} PathStateBase;\n"
"\n"
"typedef struct {\n"
"	// Radiance to add to the result if light source is visible\n"
"	Spectrum lightRadiance;\n"
"\n"
"	float lastPdfW;\n"
"	int lastSpecular;\n"
"} PathStateDirectLight;\n"
"\n"
"typedef struct {\n"
"	float passThroughEvent; // The passthrough sample used for the shadow ray\n"
"	BSDF passThroughBsdf;\n"
"} PathStateDirectLightPassThrough;\n"
"\n"
"// This is defined only under OpenCL\n"
"#if defined(SLG_OPENCL_KERNEL)\n"
"\n"
"typedef struct {\n"
"	// The task seed\n"
"	Seed seed;\n"
"\n"
"	// The set of Samples assigned to this task\n"
"	Sample sample;\n"
"\n"
"	// The state used to keep track of the rendered path\n"
"	PathStateBase pathStateBase;\n"
"#if defined(PARAM_DIRECT_LIGHT_SAMPLING)\n"
"	PathStateDirectLight directLightState;\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"	PathStateDirectLightPassThrough passThroughState;\n"
"#endif\n"
"#endif\n"
"} GPUTask;\n"
"\n"
"#endif\n"
"\n"
"typedef struct {\n"
"	unsigned int sampleCount;\n"
"} GPUTaskStats;\n"
; } }
