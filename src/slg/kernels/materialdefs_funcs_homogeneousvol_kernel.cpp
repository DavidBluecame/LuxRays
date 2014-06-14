#include <string>
namespace slg { namespace ocl {
std::string KernelSource_materialdefs_funcs_homogeneousvol = 
"#line 2 \"materialdefs_funcs_homogenousvol.cl\"\n"
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
"//------------------------------------------------------------------------------\n"
"// HomogeneousVol material\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined (PARAM_ENABLE_MAT_HOMOGENEOUS_VOL)\n"
"\n"
"BSDFEvent HomogeneousVolMaterial_GetEventTypes() {\n"
"	return DIFFUSE | REFLECT;\n"
"}\n"
"\n"
"bool HomogeneousVolMaterial_IsDelta() {\n"
"	return false;\n"
"}\n"
"\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"float3 HomogeneousVolMaterial_GetPassThroughTransparency(__global Material *material,\n"
"		__global HitPoint *hitPoint, const float3 localFixedDir, const float passThroughEvent\n"
"		TEXTURES_PARAM_DECL) {\n"
"	return BLACK;\n"
"}\n"
"#endif\n"
"\n"
"float3 HomogeneousVolMaterial_ConstEvaluate(\n"
"		__global HitPoint *hitPoint, const float3 lightDir, const float3 eyeDir,\n"
"		BSDFEvent *event, float *directPdfW,\n"
"		const float3 sigmaSTexVal, const float3 sigmaATexVal, const float3 gTexVal) {\n"
"	return SchlickScatter_ConstEvaluate(\n"
"			hitPoint, eyeDir, lightDir,\n"
"			event, directPdfW,\n"
"			clamp(sigmaSTexVal, 0.f, INFINITY), clamp(sigmaATexVal, 0.f, INFINITY), gTexVal);\n"
"}\n"
"\n"
"float3 HomogeneousVolMaterial_ConstSample(\n"
"		__global HitPoint *hitPoint, const float3 fixedDir, float3 *sampledDir,\n"
"		const float u0, const float u1, \n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"		const float passThroughEvent,\n"
"#endif\n"
"		float *pdfW, float *cosSampledDir, BSDFEvent *event,\n"
"		const BSDFEvent requestedEvent,\n"
"		const float3 sigmaSTexVal, const float3 sigmaATexVal, const float3 gTexVal) {\n"
"	return SchlickScatter_ConstSample(\n"
"			hitPoint, fixedDir, sampledDir,\n"
"			u0, u1, \n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"			passThroughEvent,\n"
"#endif\n"
"			pdfW, cosSampledDir, event,\n"
"			requestedEvent,\n"
"			clamp(sigmaSTexVal, 0.f, INFINITY), clamp(sigmaATexVal, 0.f, INFINITY), gTexVal);\n"
"}\n"
"\n"
"#if defined(PARAM_DIASBLE_MAT_DYNAMIC_EVALUATION)\n"
"float3 HomogeneousVolMaterial_Evaluate(__global Material *material,\n"
"		__global HitPoint *hitPoint, const float3 lightDir, const float3 eyeDir,\n"
"		BSDFEvent *event, float *directPdfW\n"
"		TEXTURES_PARAM_DECL) {\n"
"	const float3 sigmaSTexVal = Texture_GetSpectrumValue(material->volume.homogenous.sigmaSTexIndex, hitPoint\n"
"			TEXTURES_PARAM);\n"
"	const float3 sigmaATexVal = Texture_GetSpectrumValue(material->volume.homogenous.sigmaATexIndex, hitPoint\n"
"			TEXTURES_PARAM);\n"
"	const float3 gTexVal = Texture_GetSpectrumValue(material->volume.homogenous.gTexIndex, hitPoint\n"
"			TEXTURES_PARAM);\n"
"	\n"
"	return HomogeneousVolMaterial_ConstEvaluate(hitPoint, lightDir, eyeDir,\n"
"			event, directPdfW,\n"
"			clamp(sigmaSTexVal, 0.f, INFINITY), clamp(sigmaATexVal, 0.f, INFINITY), gTexVal);\n"
"}\n"
"\n"
"float3 HomogeneousVolMaterial_Sample(__global Material *material,\n"
"		__global HitPoint *hitPoint, const float3 fixedDir, float3 *sampledDir,\n"
"		const float u0, const float u1, \n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"		const float passThroughEvent,\n"
"#endif\n"
"		float *pdfW, float *cosSampledDir, BSDFEvent *event,\n"
"		const BSDFEvent requestedEvent\n"
"		TEXTURES_PARAM_DECL) {\n"
"	const float3 sigmaSTexVal = Texture_GetSpectrumValue(material->volume.homogenous.sigmaSTexIndex, hitPoint\n"
"			TEXTURES_PARAM);\n"
"	const float3 sigmaATexVal = Texture_GetSpectrumValue(material->volume.homogenous.sigmaATexIndex, hitPoint\n"
"			TEXTURES_PARAM);\n"
"	const float3 gTexVal = Texture_GetSpectrumValue(material->volume.homogenous.gTexIndex, hitPoint\n"
"			TEXTURES_PARAM);\n"
"\n"
"	return SchlickScatter_ConstSample(\n"
"			hitPoint, fixedDir, sampledDir,\n"
"			u0, u1, \n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"			passThroughEvent,\n"
"#endif\n"
"			pdfW, cosSampledDir, event,\n"
"			requestedEvent,\n"
"			clamp(sigmaSTexVal, 0.f, INFINITY), clamp(sigmaATexVal, 0.f, INFINITY), gTexVal);\n"
"}\n"
"#endif\n"
"\n"
"#endif\n"
; } }