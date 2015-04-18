#include <string>
namespace slg { namespace ocl {
std::string KernelSource_materialdefs_template_mix = 
"#line 2 \"materialdefs_funcs_mix.cl\"\n"
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
"// Mix material\n"
"//\n"
"// One instance of this file for each Mix material is used in Compiled scene\n"
"// class after having expanded the following parameters.\n"
"//\n"
"// Preprocessing parameters:\n"
"//  <<CS_MIX_MATERIAL_INDEX>>\n"
"//  <<CS_MAT_A_MATERIAL_INDEX>>\n"
"//  <<CS_MAT_B_MATERIAL_INDEX>>\n"
"//  <<CS_FACTOR_TEXTURE_INDEX>>\n"
"//------------------------------------------------------------------------------\n"
"\n"
"BSDFEvent Material_Index<<CS_MIX_MATERIAL_INDEX>>_GetEventTypes(__global const Material *material\n"
"		MATERIALS_PARAM_DECL) {\n"
"	return\n"
"			Material_Index<<CS_MAT_A_MATERIAL_INDEX>>_GetEventTypes(&mats[<<CS_MAT_A_MATERIAL_INDEX>>]\n"
"				MATERIALS_PARAM) |\n"
"			Material_Index<<CS_MAT_B_MATERIAL_INDEX>>_GetEventTypes(&mats[<<CS_MAT_B_MATERIAL_INDEX>>]\n"
"				MATERIALS_PARAM);\n"
"}\n"
"\n"
"bool Material_Index<<CS_MIX_MATERIAL_INDEX>>_IsDelta(__global const Material *material\n"
"		MATERIALS_PARAM_DECL) {\n"
"	return\n"
"			Material_Index<<CS_MAT_A_MATERIAL_INDEX>>_IsDelta(&mats[<<CS_MAT_A_MATERIAL_INDEX>>]\n"
"				MATERIALS_PARAM) &&\n"
"			Material_Index<<CS_MAT_B_MATERIAL_INDEX>>_IsDelta(&mats[<<CS_MAT_B_MATERIAL_INDEX>>]\n"
"				MATERIALS_PARAM);\n"
"}\n"
"\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"float3 Material_Index<<CS_MIX_MATERIAL_INDEX>>_GetPassThroughTransparency(__global const Material *material,\n"
"		__global HitPoint *hitPoint, const float3 localFixedDir, const float passThroughEvent\n"
"		MATERIALS_PARAM_DECL) {\n"
"	const float factor = Texture_Index<<CS_FACTOR_TEXTURE_INDEX>>_EvaluateFloat(\n"
"			&texs[<<CS_FACTOR_TEXTURE_INDEX>>],\n"
"			hitPoint\n"
"			TEXTURES_PARAM);\n"
"	const float weight2 = clamp(factor, 0.f, 1.f);\n"
"	const float weight1 = 1.f - weight2;\n"
"\n"
"	if (passThroughEvent < weight1)\n"
"		return Material_Index<<CS_MAT_A_MATERIAL_INDEX>>_GetPassThroughTransparency(&mats[<<CS_MAT_A_MATERIAL_INDEX>>],\n"
"			hitPoint, localFixedDir, passThroughEvent / weight1 MATERIALS_PARAM);\n"
"	else\n"
"		return Material_Index<<CS_MAT_B_MATERIAL_INDEX>>_GetPassThroughTransparency(&mats[<<CS_MAT_B_MATERIAL_INDEX>>],\n"
"			hitPoint, localFixedDir, (passThroughEvent - weight1) / weight2 MATERIALS_PARAM);\n"
"}\n"
"#endif\n"
"\n"
"float3 Material_Index<<CS_MIX_MATERIAL_INDEX>>_Evaluate(__global const Material *material,\n"
"		__global HitPoint *hitPoint, const float3 lightDir, const float3 eyeDir,\n"
"		BSDFEvent *event, float *directPdfW\n"
"		MATERIALS_PARAM_DECL) {\n"
"#if defined(PARAM_HAS_BUMPMAPS)\n"
"	const float3 shadeN = VLOAD3F(&hitPoint->shadeN.x);\n"
"	const float3 dpdu = VLOAD3F(&hitPoint->dpdu.x);\n"
"	const float3 dpdv = VLOAD3F(&hitPoint->dpdv.x);\n"
"	Frame frame;\n"
"	ExtMesh_GetFrame_Private(shadeN, dpdu, dpdv, &frame);\n"
"#endif\n"
"\n"
"	float3 result = BLACK;\n"
"	const float factor = Texture_Index<<CS_FACTOR_TEXTURE_INDEX>>_EvaluateFloat(\n"
"			&texs[<<CS_FACTOR_TEXTURE_INDEX>>],\n"
"			hitPoint\n"
"			TEXTURES_PARAM);\n"
"	const float weight2 = clamp(factor, 0.f, 1.f);\n"
"	const float weight1 = 1.f - weight2;\n"
"\n"
"	if (directPdfW)\n"
"		*directPdfW = 0.f;\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Evaluate material A\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	BSDFEvent eventMatA = NONE;\n"
"	if (weight1 > 0.f) {\n"
"#if defined(PARAM_HAS_BUMPMAPS)\n"
"		Material_Index<<CS_MAT_A_MATERIAL_INDEX>>_Bump(&mats[<<CS_MAT_A_MATERIAL_INDEX>>],\n"
"				hitPoint, 1.f\n"
"				MATERIALS_PARAM);\n"
"\n"
"		const float3 shadeNA = VLOAD3F(&hitPoint->shadeN.x);\n"
"		const float3 dpduA = VLOAD3F(&hitPoint->dpdu.x);\n"
"		const float3 dpdvA = VLOAD3F(&hitPoint->dpdv.x);\n"
"\n"
"		Frame frameA;\n"
"		ExtMesh_GetFrame_Private(shadeNA, dpduA, dpdvA, &frameA);\n"
"\n"
"		const float3 lightDirA = Frame_ToLocal_Private(&frameA, Frame_ToWorld_Private(&frame, lightDir));\n"
"		const float3 eyeDirA = Frame_ToLocal_Private(&frameA, Frame_ToWorld_Private(&frame, eyeDir));\n"
"#else\n"
"		const float3 lightDirA = lightDir;\n"
"		const float3 eyeDirA = eyeDir;\n"
"#endif\n"
"		float directPdfWMatA;\n"
"		const float3 matAResult = Material_Index<<CS_MAT_A_MATERIAL_INDEX>>_Evaluate(&mats[<<CS_MAT_A_MATERIAL_INDEX>>],\n"
"				hitPoint, lightDirA, eyeDirA, &eventMatA, &directPdfWMatA\n"
"				MATERIALS_PARAM);\n"
"		if (!Spectrum_IsBlack(matAResult)) {\n"
"			result += weight1 * matAResult;\n"
"			if (directPdfW)\n"
"				*directPdfW += weight1 * directPdfWMatA;\n"
"		}\n"
"\n"
"#if defined(PARAM_HAS_BUMPMAPS)\n"
"		VSTORE3F(shadeN, &hitPoint->shadeN.x);\n"
"		VSTORE3F(dpdu, &hitPoint->dpdu.x);\n"
"		VSTORE3F(dpdv, &hitPoint->dpdv.x);\n"
"#endif\n"
"	}\n"
"	\n"
"	//--------------------------------------------------------------------------\n"
"	// Evaluate material B\n"
"	//--------------------------------------------------------------------------\n"
"	\n"
"	BSDFEvent eventMatB = NONE;\n"
"	if (weight2 > 0.f) {\n"
"#if defined(PARAM_HAS_BUMPMAPS)\n"
"		Material_Index<<CS_MAT_B_MATERIAL_INDEX>>_Bump(&mats[<<CS_MAT_B_MATERIAL_INDEX>>],\n"
"				hitPoint, 1.f\n"
"				MATERIALS_PARAM);\n"
"\n"
"		const float3 shadeNB = VLOAD3F(&hitPoint->shadeN.x);\n"
"		const float3 dpduB = VLOAD3F(&hitPoint->dpdu.x);\n"
"		const float3 dpdvB = VLOAD3F(&hitPoint->dpdv.x);\n"
"\n"
"		Frame frameB;\n"
"		ExtMesh_GetFrame_Private(shadeNB, dpduB, dpdvB, &frameB);\n"
"\n"
"		const float3 lightDirB = Frame_ToLocal_Private(&frameB, Frame_ToWorld_Private(&frame, lightDir));\n"
"		const float3 eyeDirB = Frame_ToLocal_Private(&frameB, Frame_ToWorld_Private(&frame, eyeDir));\n"
"#else\n"
"		const float3 lightDirB = lightDir;\n"
"		const float3 eyeDirB = eyeDir;\n"
"#endif\n"
"		float directPdfWMatB;\n"
"		const float3 matBResult = Material_Index<<CS_MAT_B_MATERIAL_INDEX>>_Evaluate(&mats[<<CS_MAT_B_MATERIAL_INDEX>>],\n"
"				hitPoint, lightDirB, eyeDirB, &eventMatB, &directPdfWMatB\n"
"				MATERIALS_PARAM);\n"
"		if (!Spectrum_IsBlack(matBResult)) {\n"
"			result += weight2 * matBResult;\n"
"			if (directPdfW)\n"
"				*directPdfW += weight2 * directPdfWMatB;\n"
"		}\n"
"#if defined(PARAM_HAS_BUMPMAPS)\n"
"		VSTORE3F(shadeN, &hitPoint->shadeN.x);\n"
"		VSTORE3F(dpdu, &hitPoint->dpdu.x);\n"
"		VSTORE3F(dpdv, &hitPoint->dpdv.x);\n"
"#endif\n"
"	}\n"
"\n"
"	*event = eventMatA | eventMatB;\n"
"\n"
"	return result;\n"
"}\n"
"\n"
"float3 Material_Index<<CS_MIX_MATERIAL_INDEX>>_Sample(__global const Material *material,\n"
"		__global HitPoint *hitPoint, const float3 fixedDir, float3 *sampledDir, const float u0, const float u1,\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"		const float passThroughEvent,\n"
"#endif\n"
"		float *pdfW, float *cosSampledDir, BSDFEvent *event, const BSDFEvent requestedEvent\n"
"		MATERIALS_PARAM_DECL) {\n"
"	const float factor = Texture_Index<<CS_FACTOR_TEXTURE_INDEX>>_EvaluateFloat(\n"
"			&texs[<<CS_FACTOR_TEXTURE_INDEX>>],\n"
"			hitPoint\n"
"			TEXTURES_PARAM);\n"
"	const float weight2 = clamp(factor, 0.f, 1.f);\n"
"	const float weight1 = 1.f - weight2;\n"
"\n"
"	const bool sampleMatA = (passThroughEvent < weight1);\n"
"	const float weightFirst = sampleMatA ? weight1 : weight2;\n"
"	const float weightSecond = sampleMatA ? weight2 : weight1;\n"
"\n"
"	const float passThroughEventFirst = sampleMatA ? (passThroughEvent / weight1) : (passThroughEvent - weight1) / weight2;\n"
"\n"
"	__global const Material *matA = &mats[<<CS_MAT_A_MATERIAL_INDEX>>];\n"
"	__global const Material *matB = &mats[<<CS_MAT_B_MATERIAL_INDEX>>];\n"
"\n"
"#if defined(PARAM_HAS_BUMPMAPS)\n"
"	const float3 shadeN = VLOAD3F(&hitPoint->shadeN.x);\n"
"	const float3 dpdu = VLOAD3F(&hitPoint->dpdu.x);\n"
"	const float3 dpdv = VLOAD3F(&hitPoint->dpdv.x);\n"
"\n"
"	Frame frame;\n"
"	ExtMesh_GetFrame_Private(shadeN, dpdu, dpdv, &frame);\n"
"\n"
"	Frame frameFirst;\n"
"	if (sampleMatA) {\n"
"		Material_Index<<CS_MAT_A_MATERIAL_INDEX>>_Bump(matA, hitPoint, 1.f\n"
"				MATERIALS_PARAM);\n"
"\n"
"		const float3 shadeNA = VLOAD3F(&hitPoint->shadeN.x);\n"
"		const float3 dpduA = VLOAD3F(&hitPoint->dpdu.x);\n"
"		const float3 dpdvA = VLOAD3F(&hitPoint->dpdv.x);\n"
"		ExtMesh_GetFrame_Private(shadeNA, dpduA, dpdvA, &frameFirst);\n"
"	} else {\n"
"\n"
"		Material_Index<<CS_MAT_B_MATERIAL_INDEX>>_Bump(matB, hitPoint, 1.f\n"
"				MATERIALS_PARAM);\n"
"		const float3 shadeNB = VLOAD3F(&hitPoint->shadeN.x);\n"
"		const float3 dpduB = VLOAD3F(&hitPoint->dpdu.x);\n"
"		const float3 dpdvB = VLOAD3F(&hitPoint->dpdv.x);\n"
"		ExtMesh_GetFrame_Private(shadeNB, dpduB, dpdvB, &frameFirst);\n"
"	}\n"
"\n"
"	const float3 fixedDirFirst = Frame_ToLocal_Private(&frameFirst, Frame_ToWorld_Private(&frame, fixedDir));\n"
"#else\n"
"	const float3 fixedDirFirst = fixedDir;\n"
"#endif\n"
"\n"
"	float3 result = sampleMatA ?\n"
"			Material_Index<<CS_MAT_A_MATERIAL_INDEX>>_Sample(matA, hitPoint, fixedDirFirst, sampledDir,\n"
"				u0, u1,\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"				passThroughEventFirst,\n"
"#endif\n"
"				pdfW, cosSampledDir, event, requestedEvent MATERIALS_PARAM):\n"
"			Material_Index<<CS_MAT_B_MATERIAL_INDEX>>_Sample(matB, hitPoint, fixedDirFirst, sampledDir,\n"
"				u0, u1,\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"				passThroughEventFirst,\n"
"#endif\n"
"				pdfW, cosSampledDir, event, requestedEvent MATERIALS_PARAM);\n"
"\n"
"#if defined(PARAM_HAS_BUMPMAPS)\n"
"	VSTORE3F(shadeN, &hitPoint->shadeN.x);\n"
"	VSTORE3F(dpdu, &hitPoint->dpdu.x);\n"
"	VSTORE3F(dpdv, &hitPoint->dpdv.x);\n"
"#endif\n"
"\n"
"	if (Spectrum_IsBlack(result))\n"
"		return BLACK;\n"
"\n"
"	*pdfW *= weightFirst;\n"
"	result *= *pdfW;\n"
"\n"
"	BSDFEvent eventSecond;\n"
"	float pdfWSecond;\n"
"#if defined(PARAM_HAS_BUMPMAPS)\n"
"	Frame frameSecond;\n"
"	if (sampleMatA) {\n"
"		Material_Index<<CS_MAT_B_MATERIAL_INDEX>>_Bump(matB, hitPoint, 1.f\n"
"				MATERIALS_PARAM);\n"
"\n"
"		const float3 shadeNB = VLOAD3F(&hitPoint->shadeN.x);\n"
"		const float3 dpduB = VLOAD3F(&hitPoint->dpdu.x);\n"
"		const float3 dpdvB = VLOAD3F(&hitPoint->dpdv.x);\n"
"		ExtMesh_GetFrame_Private(shadeNB, dpduB, dpdvB, &frameSecond);\n"
"	} else {\n"
"		Material_Index<<CS_MAT_A_MATERIAL_INDEX>>_Bump(matA, hitPoint, 1.f\n"
"				MATERIALS_PARAM);\n"
"\n"
"		const float3 shadeNA = VLOAD3F(&hitPoint->shadeN.x);\n"
"		const float3 dpduA = VLOAD3F(&hitPoint->dpdu.x);\n"
"		const float3 dpdvA = VLOAD3F(&hitPoint->dpdv.x);\n"
"		ExtMesh_GetFrame_Private(shadeNA, dpduA, dpdvA, &frameSecond);\n"
"	}\n"
"\n"
"	const float3 fixedDirSecond = Frame_ToLocal_Private(&frameSecond, Frame_ToWorld_Private(&frame, fixedDir));\n"
"	*sampledDir = Frame_ToWorld_Private(&frameFirst, *sampledDir);\n"
"	const float3 sampledDirSecond = Frame_ToLocal_Private(&frameSecond, *sampledDir);\n"
"	*sampledDir = Frame_ToLocal_Private(&frame, *sampledDir);\n"
"#else\n"
"	const float3 fixedDirSecond = fixedDir;\n"
"	const float3 sampledDirSecond = *sampledDir;\n"
"#endif\n"
"\n"
"	float3 evalSecond = sampleMatA ?\n"
"			Material_Index<<CS_MAT_B_MATERIAL_INDEX>>_Evaluate(matB, hitPoint,\n"
"					sampledDirSecond, fixedDirSecond, &eventSecond, &pdfWSecond\n"
"					MATERIALS_PARAM) :\n"
"			Material_Index<<CS_MAT_A_MATERIAL_INDEX>>_Evaluate(matA, hitPoint,\n"
"					sampledDirSecond, fixedDirSecond, &eventSecond, &pdfWSecond\n"
"					MATERIALS_PARAM);\n"
"	if (!Spectrum_IsBlack(evalSecond)) {\n"
"		result += weightSecond * evalSecond;\n"
"		*pdfW += weightSecond * pdfWSecond;\n"
"	}\n"
"\n"
"#if defined(PARAM_HAS_BUMPMAPS)\n"
"	VSTORE3F(shadeN, &hitPoint->shadeN.x);\n"
"	VSTORE3F(dpdu, &hitPoint->dpdu.x);\n"
"	VSTORE3F(dpdv, &hitPoint->dpdv.x);\n"
"#endif\n"
"\n"
"	return result / *pdfW;\n"
"}\n"
"\n"
"float3 Material_Index<<CS_MIX_MATERIAL_INDEX>>_GetEmittedRadiance(__global const Material *material,\n"
"		__global HitPoint *hitPoint, const float oneOverPrimitiveArea\n"
"		MATERIALS_PARAM_DECL) {\n"
"	if (material->emitTexIndex != NULL_INDEX)\n"
"		return Material_GetEmittedRadianceNoMix(material, hitPoint TEXTURES_PARAM);\n"
"	else {\n"
"		float3 result = BLACK;\n"
"		const float factor = Texture_Index<<CS_FACTOR_TEXTURE_INDEX>>_EvaluateFloat(\n"
"			&texs[<<CS_FACTOR_TEXTURE_INDEX>>],\n"
"			hitPoint\n"
"			TEXTURES_PARAM);\n"
"		const float weight2 = clamp(factor, 0.f, 1.f);\n"
"		const float weight1 = 1.f - weight2;\n"
"\n"
"		if (weight1 > 0.f)\n"
"		   result += weight1 * Material_Index<<CS_MAT_A_MATERIAL_INDEX>>_GetEmittedRadiance(&mats[<<CS_MAT_A_MATERIAL_INDEX>>],\n"
"				   hitPoint, oneOverPrimitiveArea\n"
"				   MATERIALS_PARAM);\n"
"		if (weight2 > 0.f)\n"
"		   result += weight2 * Material_Index<<CS_MAT_B_MATERIAL_INDEX>>_GetEmittedRadiance(&mats[<<CS_MAT_B_MATERIAL_INDEX>>],\n"
"				   hitPoint, oneOverPrimitiveArea\n"
"				   MATERIALS_PARAM);\n"
"\n"
"		return result;\n"
"	}\n"
"}\n"
"\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"uint Material_Index<<CS_MIX_MATERIAL_INDEX>>_GetInteriorVolume(__global const Material *material,\n"
"		__global HitPoint *hitPoint, const float passThroughEvent\n"
"		MATERIALS_PARAM_DECL) {\n"
"		if (material->interiorVolumeIndex != NULL_INDEX)\n"
"			return material->interiorVolumeIndex;\n"
"\n"
"		const float factor = Texture_Index<<CS_FACTOR_TEXTURE_INDEX>>_EvaluateFloat(\n"
"			&texs[<<CS_FACTOR_TEXTURE_INDEX>>],\n"
"			hitPoint\n"
"			TEXTURES_PARAM);\n"
"		const float weight2 = clamp(factor, 0.f, 1.f);\n"
"		const float weight1 = 1.f - weight2;\n"
"		if (passThroughEvent < weight1)\n"
"			return Material_Index<<CS_MAT_A_MATERIAL_INDEX>>_GetInteriorVolume(&mats[<<CS_MAT_A_MATERIAL_INDEX>>],\n"
"					hitPoint, passThroughEvent / weight1\n"
"					MATERIALS_PARAM);\n"
"		else\n"
"			return Material_Index<<CS_MAT_B_MATERIAL_INDEX>>_GetInteriorVolume(&mats[<<CS_MAT_B_MATERIAL_INDEX>>],\n"
"					hitPoint, (passThroughEvent - weight1) / weight2\n"
"					MATERIALS_PARAM);\n"
"}\n"
"\n"
"uint Material_Index<<CS_MIX_MATERIAL_INDEX>>_GetExteriorVolume(__global const Material *material,\n"
"		__global HitPoint *hitPoint, const float passThroughEvent\n"
"		MATERIALS_PARAM_DECL) {\n"
"		if (material->exteriorVolumeIndex != NULL_INDEX)\n"
"			return material->exteriorVolumeIndex;\n"
"\n"
"		const float factor = Texture_Index<<CS_FACTOR_TEXTURE_INDEX>>_EvaluateFloat(\n"
"			&texs[<<CS_FACTOR_TEXTURE_INDEX>>],\n"
"			hitPoint\n"
"			TEXTURES_PARAM);\n"
"		const float weight2 = clamp(factor, 0.f, 1.f);\n"
"		const float weight1 = 1.f - weight2;\n"
"\n"
"		if (passThroughEvent < weight1)\n"
"			return Material_Index<<CS_MAT_A_MATERIAL_INDEX>>_GetExteriorVolume(&mats[<<CS_MAT_A_MATERIAL_INDEX>>],\n"
"					hitPoint, passThroughEvent / weight1\n"
"					MATERIALS_PARAM);\n"
"		else\n"
"			return Material_Index<<CS_MAT_B_MATERIAL_INDEX>>_GetExteriorVolume(&mats[<<CS_MAT_B_MATERIAL_INDEX>>],\n"
"					hitPoint, (passThroughEvent - weight1) / weight2\n"
"					MATERIALS_PARAM);\n"
"}\n"
"#endif\n"
; } }
