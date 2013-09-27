#include <string>
namespace luxrays { namespace ocl {
std::string KernelSource_luxrays_types = 
"#line 2 \"luxrays_types.cl\"\n"
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
"#define NULL_INDEX (0xffffffffu)\n"
"\n"
"#if defined(LUXRAYS_OPENCL_KERNEL)\n"
"\n"
"#define NULL 0\n"
"\n"
"#if defined(__APPLE_CL__)\n"
"float3 __OVERLOAD__ mix(float3 a, float3 b, float t)\n"
"{\n"
"	return a + ( b - a ) * t;\n"
"}\n"
"#endif\n"
"\n"
"#if defined(__APPLE_FIX__)\n"
"\n"
"float2 VLOAD2F(const __global float *p) {\n"
"	return (float2)(p[0], p[1]);\n"
"}\n"
"\n"
"void VSTORE2F(const float2 v, __global float *p) {\n"
"	p[0] = v.x;\n"
"	p[1] = v.y;\n"
"}\n"
"\n"
"float3 VLOAD3F(const __global float *p) {\n"
"	return (float3)(p[0], p[1], p[2]);\n"
"}\n"
"\n"
"void VSTORE3F(const float3 v, __global float *p) {\n"
"	p[0] = v.x;\n"
"	p[1] = v.y;\n"
"	p[2] = v.z;\n"
"}\n"
"\n"
"float4 VLOAD4F(const __global float *p) {\n"
"	return (float4)(p[0], p[1], p[2], p[3]);\n"
"}\n"
"\n"
"void VSTORE4F(const float4 v, __global float *p) {\n"
"	p[0] = v.x;\n"
"	p[1] = v.y;\n"
"	p[2] = v.z;\n"
"	p[3] = v.w;\n"
"}\n"
"\n"
"#else\n"
"\n"
"float2 VLOAD2F(const __global float *p) {\n"
"	return vload2(0, p);\n"
"}\n"
"\n"
"void VSTORE2F(const float2 v, __global float *p) {\n"
"	vstore2(v, 0, p);\n"
"}\n"
"\n"
"float3 VLOAD3F(const __global float *p) {\n"
"	return vload3(0, p);\n"
"}\n"
"\n"
"void VSTORE3F(const float3 v, __global float *p) {\n"
"	vstore3(v, 0, p);\n"
"}\n"
"\n"
"float4 VLOAD4F(const __global float *p) {\n"
"	return vload4(0, p);\n"
"}\n"
"\n"
"void VSTORE4F(const float4 v, __global float *p) {\n"
"	vstore4(v, 0, p);\n"
"}\n"
"\n"
"#endif\n"
"\n"
"void VADD3F(__global float *p, const float3 v) {\n"
"	VSTORE3F(VLOAD3F(p) + v, p);\n"
"}\n"
"\n"
"#endif\n"
; } }
