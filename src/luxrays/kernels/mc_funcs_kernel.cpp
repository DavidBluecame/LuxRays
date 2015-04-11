#include <string>
namespace luxrays { namespace ocl {
std::string KernelSource_mc_funcs = 
"#line 2 \"mc_funcs.cl\"\n"
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
"void ConcentricSampleDisk(const float u0, const float u1, float *dx, float *dy) {\n"
"	float r, theta;\n"
"	// Map uniform random numbers to $[-1,1]^2$\n"
"	const float sx = 2.f * u0 - 1.f;\n"
"	const float sy = 2.f * u1 - 1.f;\n"
"	// Map square to $(r,\\theta)$\n"
"	// Handle degeneracy at the origin\n"
"	if (sx == 0.f && sy == 0.f) {\n"
"		*dx = 0.f;\n"
"		*dy = 0.f;\n"
"		return;\n"
"	}\n"
"	if (sx >= -sy) {\n"
"		if (sx > sy) {\n"
"			// Handle first region of disk\n"
"			r = sx;\n"
"			if (sy > 0.f)\n"
"				theta = sy / r;\n"
"			else\n"
"				theta = 8.f + sy / r;\n"
"		} else {\n"
"			// Handle second region of disk\n"
"			r = sy;\n"
"			theta = 2.f - sx / r;\n"
"		}\n"
"	} else {\n"
"		if (sx <= sy) {\n"
"			// Handle third region of disk\n"
"			r = -sx;\n"
"			theta = 4.f - sy / r;\n"
"		} else {\n"
"			// Handle fourth region of disk\n"
"			r = -sy;\n"
"			theta = 6.f + sx / r;\n"
"		}\n"
"	}\n"
"	theta *= M_PI_F / 4.f;\n"
"	*dx = r * cos(theta);\n"
"	*dy = r * sin(theta);\n"
"}\n"
"\n"
"float3 CosineSampleHemisphere(const float u0, const float u1) {\n"
"	float x, y;\n"
"	ConcentricSampleDisk(u0, u1, &x, &y);\n"
"\n"
"	const float z = sqrt(fmax(0.f, 1.f - x * x - y * y));\n"
"\n"
"	return (float3)(x, y, z);\n"
"}\n"
"\n"
"float3 CosineSampleHemisphereWithPdf(const float u0, const float u1, float *pdfW) {\n"
"	float x, y;\n"
"	ConcentricSampleDisk(u0, u1, &x, &y);\n"
"\n"
"	const float z = sqrt(fmax(0.f, 1.f - x * x - y * y));\n"
"\n"
"	*pdfW = z * M_1_PI_F;\n"
"\n"
"	return (float3)(x, y, z);\n"
"}\n"
"\n"
"float3 UniformSampleSphere(const float u1, const float u2) {\n"
"	float z = 1.f - 2.f * u1;\n"
"	float r = sqrt(max(0.f, 1.f - z * z));\n"
"	float phi = 2.f * M_PI_F * u2;\n"
"	float x = r * cos(phi);\n"
"	float y = r * sin(phi);\n"
"\n"
"	return (float3)(x, y, z);\n"
"}\n"
"\n"
"float3 UniformSampleCone(const float u0, const float u1, const float costhetamax,\n"
"	const float3 x, const float3 y, const float3 z) {\n"
"	const float costheta = mix(1.f, costhetamax, u0);\n"
"	const float u0x = (1.f - costhetamax) * u0;\n"
"	const float sintheta = sqrt(u0x * (2.f - u0x));\n"
"	const float phi = u1 * 2.f * M_PI_F;\n"
"\n"
"	const float kx = cos(phi) * sintheta;\n"
"	const float ky = sin(phi) * sintheta;\n"
"	const float kz = costheta;\n"
"\n"
"	return (float3)(kx * x.x + ky * y.x + kz * z.x,\n"
"			kx * x.y + ky * y.y + kz * z.y,\n"
"			kx * x.z + ky * y.z + kz * z.z);\n"
"}\n"
"\n"
"float UniformConePdf(const float costhetamax) {\n"
"	return 1.f / (2.f * M_PI_F * (1.f - costhetamax));\n"
"}\n"
"\n"
"float PowerHeuristic(const float fPdf, const float gPdf) {\n"
"	const float f = fPdf;\n"
"	const float g = gPdf;\n"
"\n"
"	return (f * f) / (f * f + g * g);\n"
"}\n"
"\n"
"float PdfWtoA(const float pdfW, const float dist, const float cosThere) {\n"
"    return pdfW * fabs(cosThere) / (dist * dist);\n"
"}\n"
"\n"
"float PdfAtoW(const float pdfA, const float dist, const float cosThere) {\n"
"    return pdfA * dist * dist / fabs(cosThere);\n"
"}\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// Distribution1D\n"
"//------------------------------------------------------------------------------\n"
"\n"
"// Implementation of std::upper_bound()\n"
"__global const float *std_upper_bound(__global const float *first, __global const float *last, const float val) {\n"
"	__global const float *it;\n"
"	uint count = last - first;\n"
"	uint step;\n"
"\n"
"	while (count > 0) {\n"
"		it = first;\n"
"		step = count / 2;\n"
"		it += step;\n"
"\n"
"		if (!(val < *it)) {\n"
"			first = ++it;\n"
"			count -= step + 1;\n"
"		} else\n"
"			count = step;\n"
"	}\n"
"\n"
"	return first;\n"
"}\n"
"\n"
"//__global const float *std_upper_bound(__global const float *first, __global const float *last, const float val) {\n"
"//	__global const float *it = first;\n"
"//\n"
"//	while ((it <= last) && (*it <= val)) {\n"
"//		it++;\n"
"//	}\n"
"//\n"
"//	return it;\n"
"//}\n"
"\n"
"float Distribution1D_SampleContinuous(__global const float *distribution1D, const float u,\n"
"		float *pdf, uint *off) {\n"
"	const uint count = as_uint(distribution1D[0]);\n"
"	__global const float *func = &distribution1D[1];\n"
"	__global const float *cdf = &distribution1D[1 + count];\n"
"\n"
"	// Find surrounding CDF segments and _offset_\n"
"	if (u <= cdf[0]) {\n"
"		*pdf = func[0];\n"
"		if (off)\n"
"			*off = 0;\n"
"		return 0.f;\n"
"	}\n"
"	if (u >= cdf[count]) {\n"
"		*pdf = func[count - 1];\n"
"		if (off)\n"
"			*off = count - 1;\n"
"		return 1.f;\n"
"	}\n"
"\n"
"	__global const float *ptr = std_upper_bound(cdf, cdf + count + 1, u);\n"
"	const uint offset = ptr - cdf - 1;\n"
"\n"
"	// Compute offset along CDF segment\n"
"	const float du = (u - cdf[offset]) /\n"
"			(cdf[offset + 1] - cdf[offset]);\n"
"\n"
"	// Compute PDF for sampled offset\n"
"	*pdf = func[offset];\n"
"\n"
"	// Save offset\n"
"	if (off)\n"
"		*off = offset;\n"
"\n"
"	return (offset + du) / count;\n"
"}\n"
"\n"
"uint Distribution1D_SampleDiscrete(__global const float *distribution1D, const float u, float *pdf) {\n"
"	const uint count = as_uint(distribution1D[0]);\n"
"	__global const float *func = &distribution1D[1];\n"
"	__global const float *cdf = &distribution1D[1 + count];\n"
"\n"
"	// Find surrounding CDF segments and _offset_\n"
"	if (u <= cdf[0]) {\n"
"		*pdf = func[0] / count;\n"
"		return 0;\n"
"	}\n"
"	if (u >= cdf[count]) {\n"
"		*pdf = func[count - 1] / count;\n"
"		return count - 1;\n"
"	}\n"
"\n"
"	__global const float *ptr = std_upper_bound(cdf, cdf + count + 1, u);\n"
"	const uint offset = ptr - cdf - 1;\n"
"\n"
"	// Compute PDF for sampled offset\n"
"	*pdf = func[offset] / count;\n"
"\n"
"	return offset;\n"
"}\n"
"\n"
"uint Distribution1D_Offset(__global const float *distribution1D, const float u) {\n"
"	const uint count = as_uint(distribution1D[0]);\n"
"\n"
"	return min(count - 1, Floor2UInt(u * count));\n"
"}\n"
"\n"
"float Distribution1D_Pdf_UINT(__global const float *distribution1D, const uint offset) {\n"
"	const uint count = as_uint(distribution1D[0]);\n"
"	__global const float *func = &distribution1D[1];\n"
"\n"
"	return func[offset] / count;\n"
"}\n"
"\n"
"float Distribution1D_Pdf_FLOAT(__global const float *distribution1D, const float u) {\n"
"	const uint count = as_uint(distribution1D[0]);\n"
"	__global const float *func = &distribution1D[1];\n"
"\n"
"	return func[Distribution1D_Offset(distribution1D, u)];\n"
"}\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// Distribution2D\n"
"//------------------------------------------------------------------------------\n"
"\n"
"void Distribution2D_SampleContinuous(__global const float *distribution2D,\n"
"		const float u0, const float u1, float2 *uv, float *pdf) {\n"
"	const uint width = as_uint(distribution2D[0]);\n"
"	const uint height = as_uint(distribution2D[1]);\n"
"	__global const float *marginal = &distribution2D[2];\n"
"	// size of Distribution1D: size of counts + size of func + size of cdf\n"
"	const uint marginalSize = 1 + height + height + 1;\n"
"	// size of Distribution1D: size of counts + size of func + size of cdf\n"
"	const uint conditionalSize = 1 + width + width + 1;\n"
"\n"
"	float pdf1;\n"
"	uint index;\n"
"	(*uv).s1 = Distribution1D_SampleContinuous(marginal, u1, &pdf1, &index);\n"
"\n"
"	float pdf0;\n"
"	__global const float *conditional = &distribution2D[2 + marginalSize + index * conditionalSize];\n"
"	(*uv).s0 = Distribution1D_SampleContinuous(conditional, u0, &pdf0, NULL);\n"
"\n"
"	*pdf = pdf0 * pdf1;\n"
"}\n"
"\n"
"float Distribution2D_Pdf(__global const float *distribution2D, const float u0, const float u1) {\n"
"	const uint width = as_uint(distribution2D[0]);\n"
"	const uint height = as_uint(distribution2D[1]);\n"
"	__global const float *marginal = &distribution2D[2];\n"
"	// size of Distribution1D: size of counts + size of func + size of cdf\n"
"	const uint marginalSize = 1 + height + height + 1;\n"
"	// size of Distribution1D: size of counts + size of func + size of cdf\n"
"	const uint conditionalSize = 1 + width + width + 1;\n"
"\n"
"	const uint index = Distribution1D_Offset(marginal, u1);\n"
"	__global const float *conditional = &distribution2D[2 + marginalSize + index * conditionalSize];\n"
"\n"
"	return Distribution1D_Pdf_FLOAT(conditional, u0) * Distribution1D_Pdf_FLOAT(marginal, u1);\n"
"}\n"
; } }
