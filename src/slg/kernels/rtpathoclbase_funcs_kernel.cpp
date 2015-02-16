#include <string>
namespace slg { namespace ocl {
std::string KernelSource_rtpathoclbase_funcs = 
"#line 2 \"rtpatchocl_kernels.cl\"\n"
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
"	VSTORE4F(0.f, frameBuffer[gid].c.c);\n"
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
"	VSTORE3F(0.f, screenBuffer[gid].c);\n"
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
"	float4 rgbc = VLOAD4F(frameBuffer[gid].c.c);\n"
"\n"
"	if (rgbc.w > 0.f) {\n"
"		const float k = 1.f / rgbc.w;\n"
"		rgbc.s0 *= k;\n"
"		rgbc.s1 *= k;\n"
"		rgbc.s2 *= k;\n"
"\n"
"		VSTORE4F(rgbc, frameBuffer[gid].c.c);\n"
"	} else\n"
"		VSTORE4F(0.f, frameBuffer[gid].c.c);\n"
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
"	float4 srcRGBC = VLOAD4F(srcFrameBuffer[gid].c.c);\n"
"\n"
"	if (srcRGBC.w > 0.f) {\n"
"		const float4 dstRGBC = VLOAD4F(dstFrameBuffer[gid].c.c);\n"
"		VSTORE4F(srcRGBC + dstRGBC, dstFrameBuffer[gid].c.c);\n"
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
"	float3 b = VLOAD3F(src[0].c);\n"
"	float3 c = VLOAD3F(src[1].c);\n"
"\n"
"	const float leftTotF = bF + cF;\n"
"	const float3 bLeftK = bF / leftTotF;\n"
"	const float3 cLeftK = cF / leftTotF;\n"
"	VSTORE3F(bLeftK  * b + cLeftK * c, dst[0].c);\n"
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
"		c = VLOAD3F(src[x + 1].c);\n"
"\n"
"		VSTORE3F(aK * a + bK  * b + cK * c, dst[x].c);\n"
"    }\n"
"\n"
"    // Do right edge\n"
"	const float rightTotF = aF + bF;\n"
"	const float3 aRightK = aF / rightTotF;\n"
"	const float3 bRightK = bF / rightTotF;\n"
"	a = b;\n"
"	b = c;\n"
"	VSTORE3F(aRightK  * a + bRightK * b, dst[filmWidth - 1].c);\n"
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
"	float3 b = VLOAD3F(src[0].c);\n"
"	float3 c = VLOAD3F(src[filmWidth].c);\n"
"\n"
"	const float leftTotF = bF + cF;\n"
"	const float3 bLeftK = bF / leftTotF;\n"
"	const float3 cLeftK = cF / leftTotF;\n"
"	VSTORE3F(bLeftK  * b + cLeftK * c, dst[0].c);\n"
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
"		c = VLOAD3F(src[index + filmWidth].c);\n"
"\n"
"		VSTORE3F(aK * a + bK  * b + cK * c, dst[index].c);\n"
"    }\n"
"\n"
"    // Do right edge\n"
"	const float rightTotF = aF + bF;\n"
"	const float3 aRightK = aF / rightTotF;\n"
"	const float3 bRightK = bF / rightTotF;\n"
"	a = b;\n"
"	b = c;\n"
"	VSTORE3F(aRightK  * a + bRightK * b, dst[(filmHeight - 1) * filmWidth].c);\n"
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
"	const float aF = weight;\n"
"	const float bF = 1.f;\n"
"	const float cF = weight;\n"
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
"	const float4 sp = VLOAD4F(pixels[gid].c.c);\n"
"\n"
"	VSTORE4F(k * sp, pixels[gid].c.c);\n"
"}\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// Compute the sum of all frame buffer RGB values\n"
"//------------------------------------------------------------------------------\n"
"\n"
"__attribute__((reqd_work_group_size(64, 1, 1))) __kernel void SumRGBValuesReduce(\n"
"		const uint filmWidth, const uint filmHeight,\n"
"		__global float4 *src, __global float4 *dst) {\n"
"	// Workgroup local shared memory\n"
"	__local float4 localMemBuffer[64];\n"
"\n"
"	const uint tid = get_local_id(0);\n"
"	const uint gid = get_global_id(0);\n"
"\n"
"	const uint localSize = get_local_size(0);\n"
"	const uint stride = gid * 2;\n"
"	const uint pixelCount = filmWidth * filmHeight;\n"
"	localMemBuffer[tid] = 0.f;\n"
"	if (stride < pixelCount)\n"
"		localMemBuffer[tid] += src[stride];\n"
"	if (stride + 1 < pixelCount)\n"
"		localMemBuffer[tid] += src[stride + 1];	\n"
"\n"
"	barrier(CLK_LOCAL_MEM_FENCE);\n"
"\n"
"	// Do reduction in local memory\n"
"	for (uint s = localSize >> 1; s > 0; s >>= 1) {\n"
"		if (tid < s)\n"
"			localMemBuffer[tid] += localMemBuffer[tid + s];\n"
"\n"
"		barrier(CLK_LOCAL_MEM_FENCE);\n"
"	}\n"
"\n"
"	// Write result for this block to global memory\n"
"	if (tid == 0) {\n"
"		const uint bid = get_group_id(0);\n"
"\n"
"		dst[bid] = localMemBuffer[0];\n"
"	}\n"
"}\n"
"\n"
"__attribute__((reqd_work_group_size(64, 1, 1))) __kernel void SumRGBValueAccumulate(\n"
"		const uint size, __global float4 *buff) {\n"
"	if (get_local_id(0) == 0) {\n"
"		float4 totalRGBValue = 0.f;\n"
"		for(uint i = 0; i < size; ++i)\n"
"			totalRGBValue += buff[i];\n"
"		buff[0] = totalRGBValue;\n"
"	}\n"
"}\n"
"\n"
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void ToneMapAutoLinear(\n"
"		const uint filmWidth, const uint filmHeight,\n"
"		__global Pixel *pixels, const float gamma, __global float4 *totalRGB) {\n"
"	const int gid = get_global_id(0);\n"
"	const uint pixelCount = filmWidth * filmHeight;\n"
"	if (gid >= pixelCount)\n"
"		return;\n"
"\n"
"	const float totalLuminance = .212671f * (*totalRGB).x + .715160f * (*totalRGB).y + .072169f * (*totalRGB).z;\n"
"	const float avgLuminance = totalLuminance / pixelCount;\n"
"	const float scale = (avgLuminance > 0.f) ? (1.25f / avgLuminance * pow(118.f / 255.f, gamma)) : 1.f;\n"
"\n"
"	const float4 sp = VLOAD4F(pixels[gid].c.c);\n"
"\n"
"	VSTORE4F(scale * sp, pixels[gid].c.c);\n"
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
"	float4 newRgbc = VLOAD4F(srcFrameBuffer[gid].c.c);\n"
"\n"
"	if (newRgbc.s3 > 0.f) {\n"
"		float3 newRgb = (float3)(\n"
"				newRgb.s0 = Radiance2PixelFloat(newRgbc.s0),\n"
"				newRgb.s1 = Radiance2PixelFloat(newRgbc.s1),\n"
"				newRgb.s2 = Radiance2PixelFloat(newRgbc.s2));\n"
"\n"
"		const float3 oldRgb = VLOAD3F(screenBuffer[(x + y * filmWidth)].c);\n"
"\n"
"		// Blend old and new RGB value in for ghost effect\n"
"		VSTORE3F(mix(oldRgb, newRgb, PARAM_GHOSTEFFECT_INTENSITY), screenBuffer[(x + y * filmWidth)].c);\n"
"	}\n"
"}\n"
; } }
