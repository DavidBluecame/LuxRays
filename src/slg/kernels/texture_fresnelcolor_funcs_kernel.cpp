#include <string>
namespace slg { namespace ocl {
std::string KernelSource_texture_fresnelcolor_funcs = 
"#line 2 \"texture_funcs.cl\"\n"
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
"float FresnelApproxN(const float Fr) {\n"
"	const float sqrtReflectance = sqrt(clamp(Fr, 0.f, .999f));\n"
"\n"
"	return (1.f + sqrtReflectance) /\n"
"		(1.f - sqrtReflectance);\n"
"}\n"
"\n"
"float3 FresnelApproxN3(const float3 Fr) {\n"
"	const float3 sqrtReflectance = Spectrum_Sqrt(clamp(Fr, 0.f, .999f));\n"
"\n"
"	return (WHITE + sqrtReflectance) /\n"
"		(WHITE - sqrtReflectance);\n"
"}\n"
"\n"
"float FresnelApproxK(const float Fr) {\n"
"	const float reflectance = clamp(Fr, 0.f, .999f);\n"
"\n"
"	return 2.f * sqrt(reflectance /\n"
"		(1.f - reflectance));\n"
"}\n"
"\n"
"float3 FresnelApproxK3(const float3 Fr) {\n"
"	const float3 reflectance = clamp(Fr, 0.f, .999f);\n"
"\n"
"	return 2.f * Spectrum_Sqrt(reflectance /\n"
"		(WHITE - reflectance));\n"
"}\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// FresnelColor texture\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_ENABLE_TEX_FRESNELCOLOR)\n"
"\n"
"float FresnelColorTexture_ConstEvaluateFloat(__global HitPoint *hitPoint,\n"
"		const float3 kr) {\n"
"	return Spectrum_Y(kr);\n"
"}\n"
"\n"
"float3 FresnelColorTexture_ConstEvaluateSpectrum(__global HitPoint *hitPoint,\n"
"		const float3 kr) {\n"
"	return kr;\n"
"}\n"
"\n"
"#if defined(PARAM_DISABLE_TEX_DYNAMIC_EVALUATION)\n"
"void FresnelColorTexture_EvaluateFloat(__global Texture *texture, __global HitPoint *hitPoint,\n"
"		float texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {\n"
"	const float value = texValues[--(*texValuesSize)];\n"
"\n"
"	texValues[(*texValuesSize)++] = FresnelColorTexture_ConstEvaluateFloat(hitPoint, value);\n"
"}\n"
"\n"
"void FresnelColorTexture_EvaluateSpectrum(__global Texture *texture, __global HitPoint *hitPoint,\n"
"		float3 texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {\n"
"	const float3 value = texValues[--(*texValuesSize)];\n"
"\n"
"	texValues[(*texValuesSize)++] = FresnelColorTexture_ConstEvaluateSpectrum(hitPoint, value);\n"
"}\n"
"#endif\n"
"\n"
"#endif\n"
; } }
