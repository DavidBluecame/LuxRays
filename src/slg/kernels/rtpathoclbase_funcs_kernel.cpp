#include <string>
namespace slg { namespace ocl {
std::string KernelSource_rtpathoclbase_funcs = 
"#line 2 \"rtpatchocl_kernels.cl\"\n"
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
"// List of symbols defined at compile time:\n"
"//  PARAM_GHOSTEFFECT_INTENSITY\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// ClearFrameBuffer Kernel\n"
"//------------------------------------------------------------------------------\n"
"\n"
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void ClearFrameBuffer(\n"
"		const uint filmWidth, const uint filmHeight, \n"
"		__global Pixel *frameBuffer) {\n"
"	const size_t gid = get_global_id(0);\n"
"	if (gid >= filmWidth * filmHeight)\n"
"		return;\n"
"\n"
"	VSTORE4F(0.f, &frameBuffer[gid].c.r);\n"
"}\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// ClearScreenBuffer Kernel\n"
"//------------------------------------------------------------------------------\n"
"\n"
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void ClearScreenBuffer(\n"
"		const uint filmWidth, const uint filmHeight,\n"
"		__global Spectrum *screenBuffer) {\n"
"	const size_t gid = get_global_id(0);\n"
"	if (gid >= filmWidth * filmHeight)\n"
"		return;\n"
"\n"
"	VSTORE3F(0.f, &screenBuffer[gid].r);\n"
"}\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// NormalizeFrameBuffer Kernel\n"
"//------------------------------------------------------------------------------\n"
"\n"
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void NormalizeFrameBuffer(\n"
"		const uint filmWidth, const uint filmHeight,\n"
"		__global Pixel *frameBuffer) {\n"
"	const size_t gid = get_global_id(0);\n"
"	if (gid >= filmWidth * filmHeight)\n"
"		return;\n"
"\n"
"	float4 rgbc = VLOAD4F(&frameBuffer[gid].c.r);\n"
"\n"
"	if (rgbc.w > 0.f) {\n"
"		const float k = 1.f / rgbc.w;\n"
"		rgbc.s0 *= k;\n"
"		rgbc.s1 *= k;\n"
"		rgbc.s2 *= k;\n"
"\n"
"		VSTORE4F(rgbc, &frameBuffer[gid].c.r);\n"
"	} else\n"
"		VSTORE4F(0.f, &frameBuffer[gid].c.r);\n"
"}\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// MergeFrameBuffer Kernel\n"
"//------------------------------------------------------------------------------\n"
"\n"
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void MergeFrameBuffer(\n"
"		const uint filmWidth, const uint filmHeight,\n"
"		__global Pixel *srcFrameBuffer,\n"
"		__global Pixel *dstFrameBuffer) {\n"
"	const size_t gid = get_global_id(0);\n"
"	if (gid >= filmWidth * filmHeight)\n"
"		return;\n"
"\n"
"	float4 srcRGBC = VLOAD4F(&srcFrameBuffer[gid].c.r);\n"
"\n"
"	if (srcRGBC.w > 0.f) {\n"
"		const float4 dstRGBC = VLOAD4F(&dstFrameBuffer[gid].c.r);\n"
"		VSTORE4F(srcRGBC + dstRGBC, &dstFrameBuffer[gid].c.r);\n"
"	}\n"
"}\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// Image filtering kernels\n"
"//------------------------------------------------------------------------------\n"
"\n"
"void ApplyBlurFilterXR1(\n"
"		const uint filmWidth, const uint filmHeight,\n"
"		__global Spectrum *src,\n"
"		__global Spectrum *dst,\n"
"		const float aF,\n"
"		const float bF,\n"
"		const float cF\n"
"		) {\n"
"	// Do left edge\n"
"	float3 a;\n"
"	float3 b = VLOAD3F(&src[0].r);\n"
"	float3 c = VLOAD3F(&src[1].r);\n"
"\n"
"	const float leftTotF = bF + cF;\n"
"	const float3 bLeftK = bF / leftTotF;\n"
"	const float3 cLeftK = cF / leftTotF;\n"
"	VSTORE3F(bLeftK  * b + cLeftK * c, &dst[0].r);\n"
"\n"
"    // Main loop\n"
"	const float totF = aF + bF + cF;\n"
"	const float3 aK = aF / totF;\n"
"	const float3 bK = bF / totF;\n"
"	const float3 cK = cF / totF;\n"
"\n"
"	for (unsigned int x = 1; x < filmWidth - 1; ++x) {\n"
"		a = b;\n"
"		b = c;\n"
"		c = VLOAD3F(&src[x + 1].r);\n"
"\n"
"		VSTORE3F(aK * a + bK  * b + cK * c, &dst[x].r);\n"
"    }\n"
"\n"
"    // Do right edge\n"
"	const float rightTotF = aF + bF;\n"
"	const float3 aRightK = aF / rightTotF;\n"
"	const float3 bRightK = bF / rightTotF;\n"
"	a = b;\n"
"	b = c;\n"
"	VSTORE3F(aRightK  * a + bRightK * b, &dst[filmWidth - 1].r);\n"
"}\n"
"\n"
"void ApplyBlurFilterYR1(\n"
"		const uint filmWidth, const uint filmHeight,\n"
"		__global Spectrum *src,\n"
"		__global Spectrum *dst,\n"
"		const float aF,\n"
"		const float bF,\n"
"		const float cF\n"
"		) {\n"
"	// Do left edge\n"
"	float3 a;\n"
"	float3 b = VLOAD3F(&src[0].r);\n"
"	float3 c = VLOAD3F(&src[filmWidth].r);\n"
"\n"
"	const float leftTotF = bF + cF;\n"
"	const float3 bLeftK = bF / leftTotF;\n"
"	const float3 cLeftK = cF / leftTotF;\n"
"	VSTORE3F(bLeftK  * b + cLeftK * c, &dst[0].r);\n"
"\n"
"    // Main loop\n"
"	const float totF = aF + bF + cF;\n"
"	const float3 aK = aF / totF;\n"
"	const float3 bK = bF / totF;\n"
"	const float3 cK = cF / totF;\n"
"\n"
"    for (unsigned int y = 1; y < filmHeight - 1; ++y) {\n"
"		const unsigned index = y * filmWidth;\n"
"\n"
"		a = b;\n"
"		b = c;\n"
"		c = VLOAD3F(&src[index + filmWidth].r);\n"
"\n"
"		VSTORE3F(aK * a + bK  * b + cK * c, &dst[index].r);\n"
"    }\n"
"\n"
"    // Do right edge\n"
"	const float rightTotF = aF + bF;\n"
"	const float3 aRightK = aF / rightTotF;\n"
"	const float3 bRightK = bF / rightTotF;\n"
"	a = b;\n"
"	b = c;\n"
"	VSTORE3F(aRightK  * a + bRightK * b, &dst[(filmHeight - 1) * filmWidth].r);\n"
"}\n"
"\n"
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void ApplyGaussianBlurFilterXR1(\n"
"		const uint filmWidth, const uint filmHeight,\n"
"		__global Spectrum *src,\n"
"		__global Spectrum *dst,\n"
"		const float weight\n"
"		) {\n"
"	const size_t gid = get_global_id(0);\n"
"	if (gid >= filmHeight)\n"
"		return;\n"
"\n"
"	src += gid * filmWidth;\n"
"	dst += gid * filmWidth;\n"
"\n"
"	const float aF = .15f;\n"
"	const float bF = 1.f;\n"
"	const float cF = .15f;\n"
"\n"
"	ApplyBlurFilterXR1(filmWidth, filmHeight, src, dst, aF, bF, cF);\n"
"}\n"
"\n"
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void ApplyGaussianBlurFilterYR1(\n"
"		const uint filmWidth, const uint filmHeight,\n"
"		__global Spectrum *src,\n"
"		__global Spectrum *dst,\n"
"		const float weight\n"
"		) {\n"
"	const size_t gid = get_global_id(0);\n"
"	if (gid >= filmWidth)\n"
"		return;\n"
"\n"
"	src += gid;\n"
"	dst += gid;\n"
"\n"
"	const float aF = weight;\n"
"	const float bF = 1.f;\n"
"	const float cF = weight;\n"
"\n"
"	ApplyBlurFilterYR1(filmWidth, filmHeight, src, dst, aF, bF, cF);\n"
"}\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// Linear Tone Map Kernel\n"
"//------------------------------------------------------------------------------\n"
"\n"
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void ToneMapLinear(\n"
"		const uint filmWidth, const uint filmHeight,\n"
"		__global Pixel *pixels) {\n"
"	const int gid = get_global_id(0);\n"
"	if (gid >= filmWidth * filmHeight)\n"
"		return;\n"
"\n"
"	const float4 k = (float4)(PARAM_TONEMAP_LINEAR_SCALE, PARAM_TONEMAP_LINEAR_SCALE, PARAM_TONEMAP_LINEAR_SCALE, 1.f);\n"
"	const float4 sp = VLOAD4F(&pixels[gid].c.r);\n"
"\n"
"	VSTORE4F(k * sp, &pixels[gid].c.r);\n"
"}\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// UpdateScreenBuffer Kernel\n"
"//------------------------------------------------------------------------------\n"
"\n"
"float Radiance2PixelFloat(const float x) {\n"
"	return pow(clamp(x, 0.f, 1.f), 1.f / PARAM_GAMMA);\n"
"}\n"
"\n"
"__kernel void UpdateScreenBuffer(\n"
"		const uint filmWidth, const uint filmHeight,\n"
"		__global Pixel *srcFrameBuffer,\n"
"		__global Spectrum *screenBuffer) {\n"
"	const int gid = get_global_id(0);\n"
"	if (gid >= filmWidth * filmHeight)\n"
"		return;\n"
"\n"
"	const int x = gid % filmWidth;\n"
"	const int y = gid / filmWidth;\n"
"\n"
"	float4 newRgbc = VLOAD4F(&srcFrameBuffer[gid].c.r);\n"
"\n"
"	if (newRgbc.s3 > 0.f) {\n"
"		float3 newRgb = (float3)(\n"
"				newRgb.s0 = Radiance2PixelFloat(newRgbc.s0),\n"
"				newRgb.s1 = Radiance2PixelFloat(newRgbc.s1),\n"
"				newRgb.s2 = Radiance2PixelFloat(newRgbc.s2));\n"
"\n"
"		const float3 oldRgb = VLOAD3F(&screenBuffer[(x + y * filmWidth)].r);\n"
"\n"
"		// Blend old and new RGB value in for ghost effect\n"
"		VSTORE3F(mix(oldRgb, newRgb, PARAM_GHOSTEFFECT_INTENSITY), &screenBuffer[(x + y * filmWidth)].r);\n"
"	}\n"
"}\n"
; } }
