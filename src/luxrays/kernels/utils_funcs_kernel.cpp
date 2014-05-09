#include <string>
namespace luxrays { namespace ocl {
std::string KernelSource_utils_funcs = 
"#line 2 \"utils_funcs.cl\"\n"
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
"int Mod(int a, int b) {\n"
"	if (b == 0)\n"
"		b = 1;\n"
"\n"
"	a %= b;\n"
"	if (a < 0)\n"
"		a += b;\n"
"\n"
"	return a;\n"
"}\n"
"\n"
"float Radians(float deg) {\n"
"	return (M_PI_F / 180.f) * deg;\n"
"}\n"
"\n"
"float Degrees(float rad) {\n"
"	return (180.f / M_PI_F) * rad;\n"
"}\n"
"\n"
"int Ceil2Int(float val) {\n"
"	return (int)ceil(val);\n"
"}\n"
"\n"
"int Floor2Int(const float val) {\n"
"	return (int)floor(val);\n"
"}\n"
"\n"
"unsigned int Floor2UInt(const float val) {\n"
"	return (val > 0.f) ? ((unsigned int)floor(val)) : 0;\n"
"}\n"
"\n"
"float Lerp(const float t, const float v1, const float v2) {\n"
"	return mix(v1, v2, t);\n"
"}\n"
"\n"
"float3 Lerp3(const float t, const float3 v1, const float3 v2) {\n"
"	return mix(v1, v2, t);\n"
"}\n"
"\n"
"float SmoothStep(const float min, const float max, const float value) {\n"
"	const float v = clamp((value - min) / (max - min), 0.f, 1.f);\n"
"	return v * v * (-2.f * v  + 3.f);\n"
"}\n"
"\n"
"float CosTheta(const float3 v) {\n"
"	return v.z;\n"
"}\n"
"\n"
"float SinTheta2(const float3 w) {\n"
"	return fmax(0.f, 1.f - CosTheta(w) * CosTheta(w));\n"
"}\n"
"\n"
"float SinTheta(const float3 w) {\n"
"	return sqrt(SinTheta2(w));\n"
"}\n"
"\n"
"float CosPhi(const float3 w) {\n"
"	const float sinTheta = SinTheta(w);\n"
"	return sinTheta > 0.f ? clamp(w.x / sinTheta, -1.f, 1.f) : 1.f;\n"
"}\n"
"\n"
"float SinPhi(const float3 w) {\n"
"	const float sinTheta = SinTheta(w);\n"
"	return sinTheta > 0.f ? clamp(w.y / sinTheta, -1.f, 1.f) : 0.f;\n"
"}\n"
"\n"
"float3 SphericalDirection(float sintheta, float costheta, float phi) {\n"
"	return (float3)(sintheta * cos(phi), sintheta * sin(phi), costheta);\n"
"}\n"
; } }
