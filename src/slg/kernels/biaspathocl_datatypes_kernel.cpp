#include <string>
namespace slg { namespace ocl {
std::string KernelSource_biaspathocl_datatypes = 
"#line 2 \"biaspathocl_datatypes.cl\"\n"
"\n"
"/***************************************************************************\n"
" *   Copyright (C) 1998-2013 by authors (see AUTHORS.txt)                  *\n"
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
"#define NEXT_VERTEX_TRACE_RAY (1<<0)\n"
"#define NEXT_VERTEX_GENERATE_RAY (1<<1)\n"
"#define DIRECT_LIGHT_TRACE_RAY (1<<2)\n"
"#define DIRECT_LIGHT_GENERATE_RAY (1<<3)\n"
"\n"
"#define PATH_VERTEX_1 (1<<16)\n"
"#define ADD_SAMPLE (1<<17)\n"
"\n"
"// This is defined only under OpenCL because of variable size structures\n"
"#if defined(SLG_OPENCL_KERNEL)\n"
"\n"
"typedef struct {\n"
"	// The task seed\n"
"	Seed seed;\n"
"\n"
"	Spectrum throughputPathVertex1;\n"
"	BSDF bsdfPathVertex1;\n"
"\n"
"	// Direct light sampling. Radiance to add to the result\n"
"	// if light source is visible.\n"
"	Spectrum lightRadiance;\n"
"	uint lightID;\n"
"\n"
"	BSDF tmpBSDF;\n"
"	Spectrum tmpThroughput;\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"	float tmpPassThroughEvent;\n"
"#endif\n"
"\n"
"#if (PARAM_DL_LIGHT_COUNT > 0)\n"
"	// This is used by TriangleLight_Illuminate() to temporary store the\n"
"	// point on the light sources\n"
"	HitPoint tmpHitPoint;\n"
"#endif\n"
"} GPUTask;\n"
"\n"
"#endif\n"
"\n"
"typedef struct {\n"
"	unsigned int raysCount;\n"
"} GPUTaskStats;\n"
; } }
