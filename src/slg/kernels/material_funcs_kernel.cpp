#include <string>
namespace slg { namespace ocl {
std::string KernelSource_material_funcs = 
"#line 2 \"material_funcs.cl\"\n"
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
"// Generic material functions\n"
"//\n"
"// They include the support for all material but Mix\n"
"// (because OpenCL doesn't support recursion)\n"
"//------------------------------------------------------------------------------\n"
"\n"
"bool Material_IsDeltaNoMix(__global Material *material) {\n"
"	switch (material->type) {\n"
"		//----------------------------------------------------------------------\n"
"		// Non Specular materials\n"
"#if defined (PARAM_ENABLE_MAT_CARPAINT)\n"
"		case CARPAINT:\n"
"			return false;\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_CLOTH)\n"
"		case CLOTH:\n"
"			return false;\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_ROUGHGLASS)\n"
"		case ROUGHGLASS:\n"
"			return false;\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_METAL2)\n"
"		case METAL2:\n"
"			return false;\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_GLOSSY2)\n"
"		case GLOSSY2:\n"
"			return false;\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_MATTETRANSLUCENT)\n"
"		case MATTETRANSLUCENT:\n"
"			return false;\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_VELVET)\n"
"		case VELVET:\n"
"			return false;\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_MATTE)\n"
"		case MATTE:\n"
"			return false;\n"
"#endif\n"
"		//----------------------------------------------------------------------\n"
"		// Specular materials\n"
"#if defined (PARAM_ENABLE_MAT_MIRROR)\n"
"		case MIRROR:\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_GLASS)\n"
"		case GLASS:\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_ARCHGLASS)\n"
"		case ARCHGLASS:\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_NULL)\n"
"		case NULLMAT:\n"
"#endif\n"
"		default:\n"
"			return true;\n"
"	}\n"
"}\n"
"\n"
"BSDFEvent Material_GetEventTypesNoMix(__global Material *mat) {\n"
"	switch (mat->type) {\n"
"#if defined (PARAM_ENABLE_MAT_MATTE)\n"
"		case MATTE:\n"
"			return DIFFUSE | REFLECT;\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_VELVET)\n"
"		case VELVET:\n"
"			return DIFFUSE | REFLECT;\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_MIRROR)\n"
"		case MIRROR:\n"
"			return SPECULAR | REFLECT;\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_GLASS)\n"
"		case GLASS:\n"
"			return SPECULAR | REFLECT | TRANSMIT;\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_ARCHGLASS)\n"
"		case ARCHGLASS:\n"
"			return SPECULAR | REFLECT | TRANSMIT;\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_NULL)\n"
"		case NULLMAT:\n"
"			return SPECULAR | TRANSMIT;\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_MATTETRANSLUCENT)\n"
"		case MATTETRANSLUCENT:\n"
"			return DIFFUSE | REFLECT | TRANSMIT;\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_GLOSSY2)\n"
"		case GLOSSY2:\n"
"			return DIFFUSE | GLOSSY | REFLECT;\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_METAL2)\n"
"		case METAL2:\n"
"			return GLOSSY | REFLECT;\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_ROUGHGLASS)\n"
"		case ROUGHGLASS:\n"
"			return GLOSSY | REFLECT | TRANSMIT;\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_CLOTH)\n"
"		case CLOTH:\n"
"			return GLOSSY | REFLECT;\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_CARPAINT)\n"
"		case CARPAINT:\n"
"			return GLOSSY | REFLECT;\n"
"#endif\n"
"		default:\n"
"			return NONE;\n"
"	}\n"
"}\n"
"\n"
"float3 Material_SampleNoMix(__global Material *material,\n"
"		__global HitPoint *hitPoint, const float3 fixedDir, float3 *sampledDir,\n"
"		const float u0, const float u1,\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"		const float passThroughEvent,\n"
"#endif\n"
"		float *pdfW, float *cosSampledDir, BSDFEvent *event,\n"
"		const BSDFEvent requestedEvent\n"
"		TEXTURES_PARAM_DECL) {\n"
"	switch (material->type) {\n"
"#if defined (PARAM_ENABLE_MAT_MATTE)\n"
"		case MATTE:\n"
"			return MatteMaterial_Sample(material, hitPoint, fixedDir, sampledDir,\n"
"					u0, u1,	pdfW, cosSampledDir, event, requestedEvent\n"
"					TEXTURES_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_VELVET)\n"
"		case VELVET:\n"
"			return VelvetMaterial_Sample(material, hitPoint, fixedDir, sampledDir,\n"
"					u0, u1,	pdfW, cosSampledDir, event, requestedEvent\n"
"					TEXTURES_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_MIRROR)\n"
"		case MIRROR:\n"
"			return MirrorMaterial_Sample(material, hitPoint, fixedDir, sampledDir,\n"
"					u0, u1, pdfW, cosSampledDir, event, requestedEvent\n"
"					TEXTURES_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_GLASS)\n"
"		case GLASS:\n"
"			return GlassMaterial_Sample(material, hitPoint, fixedDir, sampledDir,\n"
"					u0, u1,	passThroughEvent, pdfW, cosSampledDir, event, requestedEvent\n"
"					TEXTURES_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_ARCHGLASS)\n"
"		case ARCHGLASS:\n"
"			return ArchGlassMaterial_Sample(material, hitPoint, fixedDir, sampledDir,\n"
"					u0, u1,	passThroughEvent, pdfW, cosSampledDir, event, requestedEvent\n"
"					TEXTURES_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_NULL)\n"
"		case NULLMAT:\n"
"			return NullMaterial_Sample(material, hitPoint, fixedDir, sampledDir,\n"
"					u0, u1, pdfW, cosSampledDir, event, requestedEvent\n"
"					TEXTURES_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_MATTETRANSLUCENT)\n"
"		case MATTETRANSLUCENT:\n"
"			return MatteTranslucentMaterial_Sample(material, hitPoint, fixedDir, sampledDir,\n"
"					u0, u1,	passThroughEvent, pdfW, cosSampledDir, event, requestedEvent\n"
"					TEXTURES_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_GLOSSY2)\n"
"		case GLOSSY2:\n"
"			return Glossy2Material_Sample(material, hitPoint, fixedDir, sampledDir,\n"
"					u0, u1,	passThroughEvent, pdfW, cosSampledDir, event, requestedEvent\n"
"					TEXTURES_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_METAL2)\n"
"		case METAL2:\n"
"			return Metal2Material_Sample(material, hitPoint, fixedDir, sampledDir,\n"
"					u0, u1,	pdfW, cosSampledDir, event, requestedEvent\n"
"					TEXTURES_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_ROUGHGLASS)\n"
"		case ROUGHGLASS:\n"
"			return RoughGlassMaterial_Sample(material, hitPoint, fixedDir, sampledDir,\n"
"					u0, u1,	passThroughEvent, pdfW, cosSampledDir, event, requestedEvent\n"
"					TEXTURES_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_CLOTH)\n"
"		case CLOTH:\n"
"			return ClothMaterial_Sample(material, hitPoint, fixedDir, sampledDir,\n"
"					u0, u1,	pdfW, cosSampledDir, event, requestedEvent\n"
"					TEXTURES_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_CARPAINT)\n"
"		case CARPAINT:\n"
"			return CarpaintMaterial_Sample(material, hitPoint, fixedDir, sampledDir,\n"
"					u0, u1, passThroughEvent, pdfW, cosSampledDir, event, requestedEvent\n"
"					TEXTURES_PARAM);\n"
"#endif\n"
"		default:\n"
"			return BLACK;\n"
"	}\n"
"}\n"
"\n"
"float3 Material_EvaluateNoMix(__global Material *material,\n"
"		__global HitPoint *hitPoint, const float3 lightDir, const float3 eyeDir,\n"
"		BSDFEvent *event, float *directPdfW\n"
"		TEXTURES_PARAM_DECL) {\n"
"	switch (material->type) {\n"
"#if defined (PARAM_ENABLE_MAT_MATTE)\n"
"		case MATTE:\n"
"			return MatteMaterial_Evaluate(material, hitPoint, lightDir, eyeDir, event, directPdfW\n"
"					TEXTURES_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_VELVET)\n"
"		case VELVET:\n"
"			return VelvetMaterial_Evaluate(material, hitPoint, lightDir, eyeDir, event, directPdfW\n"
"					TEXTURES_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_MATTETRANSLUCENT)\n"
"		case MATTETRANSLUCENT:\n"
"			return MatteTranslucentMaterial_Evaluate(material, hitPoint, lightDir, eyeDir, event, directPdfW\n"
"					TEXTURES_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_GLOSSY2)\n"
"		case GLOSSY2:\n"
"			return Glossy2Material_Evaluate(material, hitPoint, lightDir, eyeDir, event, directPdfW\n"
"					TEXTURES_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_METAL2)\n"
"		case METAL2:\n"
"			return Metal2Material_Evaluate(material, hitPoint, lightDir, eyeDir, event, directPdfW\n"
"					TEXTURES_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_ROUGHGLASS)\n"
"		case ROUGHGLASS:\n"
"			return RoughGlassMaterial_Evaluate(material, hitPoint, lightDir, eyeDir, event, directPdfW\n"
"					TEXTURES_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_CLOTH)\n"
"		case CLOTH:\n"
"			return ClothMaterial_Evaluate(material, hitPoint, lightDir, eyeDir, event, directPdfW\n"
"					TEXTURES_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_CARPAINT)\n"
"		case CARPAINT:\n"
"			return CarpaintMaterial_Evaluate(material, hitPoint, lightDir, eyeDir, event, directPdfW\n"
"					TEXTURES_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_MIRROR)\n"
"		case MIRROR:\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_GLASS)\n"
"		case GLASS:\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_ARCHGLASS)\n"
"		case ARCHGLASS:\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_NULL)\n"
"		case NULLMAT:\n"
"#endif\n"
"		default:\n"
"			return BLACK;\n"
"	}\n"
"}\n"
"\n"
"float3 Material_GetEmittedRadianceNoMix(__global Material *material, __global HitPoint *hitPoint\n"
"		TEXTURES_PARAM_DECL) {\n"
"	const uint emitTexIndex = material->emitTexIndex;\n"
"	if (emitTexIndex == NULL_INDEX)\n"
"		return BLACK;\n"
"\n"
"	return Texture_GetSpectrumValue(emitTexIndex, hitPoint\n"
"				TEXTURES_PARAM);\n"
"}\n"
"\n"
"#if defined(PARAM_HAS_BUMPMAPS)\n"
"void Material_BumpNoMix(__global Material *material, __global HitPoint *hitPoint,\n"
"        const float3 dpdu, const float3 dpdv,\n"
"        const float3 dndu, const float3 dndv, const float weight\n"
"        MATERIALS_PARAM_DECL) {\n"
"    if ((material->bumpTexIndex != NULL_INDEX) && (weight > 0.f)) {\n"
"        const float2 duv = weight * \n"
"#if defined(PARAM_ENABLE_TEX_NORMALMAP)\n"
"            ((texs[material->bumpTexIndex].type == NORMALMAP_TEX) ?\n"
"                NormalMapTexture_GetDuv(material->bumpTexIndex,\n"
"                    hitPoint, dpdu, dpdv, dndu, dndv, material->bumpSampleDistance\n"
"                    TEXTURES_PARAM) :\n"
"                Texture_GetDuv(material->bumpTexIndex,\n"
"                    hitPoint, dpdu, dpdv, dndu, dndv, material->bumpSampleDistance\n"
"                    TEXTURES_PARAM));\n"
"#else\n"
"            Texture_GetDuv(material->bumpTexIndex,\n"
"                hitPoint, dpdu, dpdv, dndu, dndv, material->bumpSampleDistance\n"
"                TEXTURES_PARAM);\n"
"#endif\n"
"\n"
"        const float3 oldShadeN = VLOAD3F(&hitPoint->shadeN.x);\n"
"        const float3 bumpDpdu = dpdu + duv.s0 * oldShadeN;\n"
"        const float3 bumpDpdv = dpdv + duv.s1 * oldShadeN;\n"
"        float3 newShadeN = normalize(cross(bumpDpdu, bumpDpdv));\n"
"\n"
"        // The above transform keeps the normal in the original normal\n"
"        // hemisphere. If they are opposed, it means UVN was indirect and\n"
"        // the normal needs to be reversed\n"
"        newShadeN *= (dot(oldShadeN, newShadeN) < 0.f) ? -1.f : 1.f;\n"
"\n"
"        VSTORE3F(newShadeN, &hitPoint->shadeN.x);\n"
"    }\n"
"}\n"
"#endif\n"
"\n"
"float3 Material_GetPassThroughTransparencyNoMix(__global Material *material,\n"
"		__global HitPoint *hitPoint, const float3 fixedDir, const float passThroughEvent\n"
"		TEXTURES_PARAM_DECL) {\n"
"	switch (material->type) {\n"
"#if defined (PARAM_ENABLE_MAT_ARCHGLASS)\n"
"		case ARCHGLASS:\n"
"			return ArchGlassMaterial_GetPassThroughTransparency(material,\n"
"					hitPoint, fixedDir, passThroughEvent\n"
"					TEXTURES_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_NULL)\n"
"		case NULLMAT:\n"
"			return WHITE;\n"
"#endif\n"
"		default:\n"
"			return BLACK;\n"
"	}\n"
"}\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// Mix material\n"
"//\n"
"// This requires a quite complex implementation because OpenCL doesn't support\n"
"// recursion.\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined (PARAM_ENABLE_MAT_MIX)\n"
"\n"
"#define MIX_STACK_SIZE 16\n"
"\n"
"BSDFEvent MixMaterial_IsDelta(__global Material *material\n"
"		MATERIALS_PARAM_DECL) {\n"
"	__global Material *materialStack[MIX_STACK_SIZE];\n"
"	materialStack[0] = material;\n"
"	int stackIndex = 0;\n"
"\n"
"	while (stackIndex >= 0) {\n"
"		// Extract a material from the stack\n"
"		__global Material *m = materialStack[stackIndex--];\n"
"\n"
"		// Check if it is a Mix material too\n"
"		if (m->type == MIX) {\n"
"			// Add both material to the stack\n"
"			materialStack[++stackIndex] = &mats[m->mix.matAIndex];\n"
"			materialStack[++stackIndex] = &mats[m->mix.matBIndex];\n"
"		} else {\n"
"			// Normal GetEventTypes() evaluation\n"
"			if (!Material_IsDeltaNoMix(m))\n"
"				return false;\n"
"		}\n"
"	}\n"
"\n"
"	return true;\n"
"}\n"
"\n"
"BSDFEvent MixMaterial_GetEventTypes(__global Material *material\n"
"		MATERIALS_PARAM_DECL) {\n"
"	BSDFEvent event = NONE;\n"
"	__global Material *materialStack[MIX_STACK_SIZE];\n"
"	materialStack[0] = material;\n"
"	int stackIndex = 0;\n"
"\n"
"	while (stackIndex >= 0) {\n"
"		// Extract a material from the stack\n"
"		__global Material *m = materialStack[stackIndex--];\n"
"\n"
"		// Check if it is a Mix material too\n"
"		if (m->type == MIX) {\n"
"			// Add both material to the stack\n"
"			materialStack[++stackIndex] = &mats[m->mix.matAIndex];\n"
"			materialStack[++stackIndex] = &mats[m->mix.matBIndex];\n"
"		} else {\n"
"			// Normal GetEventTypes() evaluation\n"
"			event |= Material_GetEventTypesNoMix(m);\n"
"		}\n"
"	}\n"
"\n"
"	return event;\n"
"}\n"
"\n"
"float3 MixMaterial_Evaluate(__global Material *material,\n"
"		__global HitPoint *hitPoint, const float3 lightDir, const float3 eyeDir,\n"
"		BSDFEvent *event, float *directPdfW\n"
"		MATERIALS_PARAM_DECL) {\n"
"	__global Material *materialStack[MIX_STACK_SIZE];\n"
"	float totalWeightStack[MIX_STACK_SIZE];\n"
"\n"
"	// Push the root Mix material\n"
"	materialStack[0] = material;\n"
"	totalWeightStack[0] = 1.f;\n"
"	int stackIndex = 0;\n"
"\n"
"	// Setup the results\n"
"	float3 result = BLACK;\n"
"	*event = NONE;\n"
"	if (directPdfW)\n"
"		*directPdfW = 0.f;\n"
"\n"
"	while (stackIndex >= 0) {\n"
"		// Extract a material from the stack\n"
"		__global Material *m = materialStack[stackIndex];\n"
"		float totalWeight = totalWeightStack[stackIndex--];\n"
"\n"
"		// Check if it is a Mix material too\n"
"		if (m->type == MIX) {\n"
"			// Add both material to the stack\n"
"			const float factor = Texture_GetFloatValue(m->mix.mixFactorTexIndex, hitPoint\n"
"					TEXTURES_PARAM);\n"
"			const float weight2 = clamp(factor, 0.f, 1.f);\n"
"			const float weight1 = 1.f - weight2;\n"
"\n"
"			materialStack[++stackIndex] = &mats[m->mix.matAIndex];\n"
"			totalWeightStack[stackIndex] = totalWeight * weight1;\n"
"\n"
"			materialStack[++stackIndex] = &mats[m->mix.matBIndex];			\n"
"			totalWeightStack[stackIndex] = totalWeight * weight2;\n"
"		} else {\n"
"			// Normal Evaluate() evaluation\n"
"			if (totalWeight > 0.f) {\n"
"				BSDFEvent eventMat;\n"
"				float directPdfWMat;\n"
"				const float3 resultMat = Material_EvaluateNoMix(m, hitPoint, lightDir, eyeDir, &eventMat, &directPdfWMat\n"
"						TEXTURES_PARAM);\n"
"\n"
"				if (!Spectrum_IsBlack(resultMat)) {\n"
"					*event |= eventMat;\n"
"					result += totalWeight * resultMat;\n"
"\n"
"					if (directPdfW)\n"
"						*directPdfW += totalWeight * directPdfWMat;\n"
"				}\n"
"			}\n"
"		}\n"
"	}\n"
"\n"
"	return result;\n"
"}\n"
"\n"
"float3 MixMaterial_Sample(__global Material *material,\n"
"		__global HitPoint *hitPoint, const float3 fixedDir, float3 *sampledDir,\n"
"		const float u0, const float u1, const float passEvent,\n"
"		float *pdfW, float *cosSampledDir, BSDFEvent *event,\n"
"		const BSDFEvent requestedEvent\n"
"		MATERIALS_PARAM_DECL) {\n"
"	__global Material *evaluationMatList[MIX_STACK_SIZE];\n"
"	float parentWeightList[MIX_STACK_SIZE];\n"
"	int evaluationListSize = 0;\n"
"\n"
"	// Setup the results\n"
"	float3 result = BLACK;\n"
"	*pdfW = 0.f;\n"
"\n"
"	// Look for a no Mix material to sample\n"
"	__global Material *currentMixMat = material;\n"
"	float passThroughEvent = passEvent;\n"
"	float parentWeight = 1.f;\n"
"	for (;;) {\n"
"		const float factor = Texture_GetFloatValue(currentMixMat->mix.mixFactorTexIndex, hitPoint\n"
"			TEXTURES_PARAM);\n"
"		const float weight2 = clamp(factor, 0.f, 1.f);\n"
"		const float weight1 = 1.f - weight2;\n"
"\n"
"		const bool sampleMatA = (passThroughEvent < weight1);\n"
"\n"
"		const float weightFirst = sampleMatA ? weight1 : weight2;\n"
"		const float weightSecond = sampleMatA ? weight2 : weight1;\n"
"\n"
"		const float passThroughEventFirst = sampleMatA ? (passThroughEvent / weight1) : (passThroughEvent - weight1) / weight2;\n"
"\n"
"		const uint matIndexFirst = sampleMatA ? currentMixMat->mix.matAIndex : currentMixMat->mix.matBIndex;\n"
"		const uint matIndexSecond = sampleMatA ? currentMixMat->mix.matBIndex : currentMixMat->mix.matAIndex;\n"
"\n"
"		// Sample the first material, evaluate the second\n"
"		__global Material *matFirst = &mats[matIndexFirst];\n"
"		__global Material *matSecond = &mats[matIndexSecond];\n"
"\n"
"		//----------------------------------------------------------------------\n"
"		// Add the second material to the evaluation list\n"
"		//----------------------------------------------------------------------\n"
"\n"
"		if (weightSecond > 0.f) {\n"
"			evaluationMatList[evaluationListSize] = matSecond;\n"
"			parentWeightList[evaluationListSize++] = parentWeight * weightSecond;\n"
"		}\n"
"\n"
"		//----------------------------------------------------------------------\n"
"		// Sample the first material\n"
"		//----------------------------------------------------------------------\n"
"\n"
"		// Check if it is a Mix material too\n"
"		if (matFirst->type == MIX) {\n"
"			// Make the first material the current\n"
"			currentMixMat = matFirst;\n"
"			passThroughEvent = passThroughEventFirst;\n"
"			parentWeight *= weightFirst;\n"
"		} else {\n"
"			// Sample the first material\n"
"			float pdfWMatFirst;\n"
"			const float3 sampleResult = Material_SampleNoMix(matFirst, hitPoint,\n"
"					fixedDir, sampledDir,\n"
"					u0, u1, passThroughEventFirst,\n"
"					&pdfWMatFirst, cosSampledDir, event,\n"
"					requestedEvent\n"
"					TEXTURES_PARAM);\n"
"\n"
"			if (all(isequal(sampleResult, BLACK)))\n"
"				return BLACK;\n"
"\n"
"			const float weight = parentWeight * weightFirst;\n"
"			*pdfW += weight * pdfWMatFirst;\n"
"			result += weight * sampleResult * pdfWMatFirst;\n"
"\n"
"			// I can stop now\n"
"			break;\n"
"		}\n"
"	}\n"
"\n"
"	while (evaluationListSize > 0) {\n"
"		// Extract the material to evaluate\n"
"		__global Material *evalMat = evaluationMatList[--evaluationListSize];\n"
"		const float evalWeight = parentWeightList[evaluationListSize];\n"
"\n"
"		// Evaluate the material\n"
"\n"
"		// Check if it is a Mix material too\n"
"		BSDFEvent eventMat;\n"
"		float pdfWMat;\n"
"		float3 eval;\n"
"		if (evalMat->type == MIX) {\n"
"			eval = MixMaterial_Evaluate(evalMat, hitPoint, *sampledDir, fixedDir,\n"
"					&eventMat, &pdfWMat\n"
"					MATERIALS_PARAM);\n"
"		} else {\n"
"			eval = Material_EvaluateNoMix(evalMat, hitPoint, *sampledDir, fixedDir,\n"
"					&eventMat, &pdfWMat\n"
"					TEXTURES_PARAM);\n"
"		}\n"
"		if (!Spectrum_IsBlack(eval)) {\n"
"			result += evalWeight * eval;\n"
"			*pdfW += evalWeight * pdfWMat;\n"
"		}\n"
"	}\n"
"\n"
"	return result / *pdfW;\n"
"}\n"
"\n"
"float3 MixMaterial_GetEmittedRadiance(__global Material *material, __global HitPoint *hitPoint\n"
"		MATERIALS_PARAM_DECL) {\n"
"	__global Material *materialStack[MIX_STACK_SIZE];\n"
"	float totalWeightStack[MIX_STACK_SIZE];\n"
"\n"
"	// Push the root Mix material\n"
"	materialStack[0] = material;\n"
"	totalWeightStack[0] = 1.f;\n"
"	int stackIndex = 0;\n"
"\n"
"	// Setup the results\n"
"	float3 result = BLACK;\n"
"\n"
"	while (stackIndex >= 0) {\n"
"		// Extract a material from the stack\n"
"		__global Material *m = materialStack[stackIndex];\n"
"		float totalWeight = totalWeightStack[stackIndex--];\n"
"\n"
"		if (m->type == MIX) {\n"
"			const float factor = Texture_GetFloatValue(m->mix.mixFactorTexIndex, hitPoint\n"
"					TEXTURES_PARAM);\n"
"			const float weight2 = clamp(factor, 0.f, 1.f);\n"
"			const float weight1 = 1.f - weight2;\n"
"\n"
"			if (weight1 > 0.f) {\n"
"				materialStack[++stackIndex] = &mats[m->mix.matAIndex];\n"
"				totalWeightStack[stackIndex] = totalWeight * weight1;\n"
"			}\n"
"\n"
"			if (weight2 > 0.f) {\n"
"				materialStack[++stackIndex] = &mats[m->mix.matBIndex];\n"
"				totalWeightStack[stackIndex] = totalWeight * weight2;\n"
"			}\n"
"		} else {\n"
"			const float3 emitRad = Material_GetEmittedRadianceNoMix(m, hitPoint\n"
"				TEXTURES_PARAM);\n"
"			if (!Spectrum_IsBlack(emitRad))\n"
"				result += totalWeight * emitRad;\n"
"		}\n"
"	}\n"
"	\n"
"	return result;\n"
"}\n"
"\n"
"#if defined(PARAM_HAS_BUMPMAPS)\n"
"void MixMaterial_Bump(__global Material *material, __global HitPoint *hitPoint,\n"
"        const float3 dpdu, const float3 dpdv,\n"
"        const float3 dndu, const float3 dndv, const float weight\n"
"        MATERIALS_PARAM_DECL) {\n"
"    if (weight == 0.f)\n"
"        return;\n"
"\n"
"    if (material->bumpTexIndex != NULL_INDEX) {\n"
"        // Use this mix node bump mapping\n"
"        Material_BumpNoMix(material, hitPoint,\n"
"                dpdu, dpdv, dndu, dndv, weight\n"
"                MATERIALS_PARAM);\n"
"    } else {\n"
"        // Mix the child bump mapping\n"
"        __global Material *materialStack[MIX_STACK_SIZE];\n"
"        float totalWeightStack[MIX_STACK_SIZE];\n"
"\n"
"        // Push the root Mix material\n"
"        materialStack[0] = material;\n"
"        totalWeightStack[0] = weight;\n"
"        int stackIndex = 0;\n"
"\n"
"        while (stackIndex >= 0) {\n"
"            // Extract a material from the stack\n"
"            __global Material *m = materialStack[stackIndex];\n"
"            float totalWeight = totalWeightStack[stackIndex--];\n"
"\n"
"            if (m->type == MIX) {\n"
"                const float factor = Texture_GetFloatValue(m->mix.mixFactorTexIndex, hitPoint\n"
"                        TEXTURES_PARAM);\n"
"                const float weight2 = clamp(factor, 0.f, 1.f);\n"
"                const float weight1 = 1.f - weight2;\n"
"\n"
"                if (weight1 > 0.f) {\n"
"                    materialStack[++stackIndex] = &mats[m->mix.matAIndex];\n"
"                    totalWeightStack[stackIndex] = totalWeight * weight1;\n"
"                }\n"
"\n"
"                if (weight2 > 0.f) {\n"
"                    materialStack[++stackIndex] = &mats[m->mix.matBIndex];\n"
"                    totalWeightStack[stackIndex] = totalWeight * weight2;\n"
"                }\n"
"            } else {\n"
"                Material_BumpNoMix(m, hitPoint,\n"
"                        dpdu, dpdv, dndu, dndv, totalWeight\n"
"                        MATERIALS_PARAM);\n"
"            }\n"
"        }\n"
"    }\n"
"}\n"
"#endif\n"
"\n"
"float3 MixMaterial_GetPassThroughTransparency(__global Material *material,\n"
"		__global HitPoint *hitPoint, const float3 fixedDir, const float passEvent\n"
"		MATERIALS_PARAM_DECL) {\n"
"	__global Material *currentMixMat = material;\n"
"	float passThroughEvent = passEvent;\n"
"	for (;;) {\n"
"		const float factor = Texture_GetFloatValue(currentMixMat->mix.mixFactorTexIndex, hitPoint\n"
"				TEXTURES_PARAM);\n"
"		const float weight2 = clamp(factor, 0.f, 1.f);\n"
"		const float weight1 = 1.f - weight2;\n"
"\n"
"		const bool sampleMatA = (passThroughEvent < weight1);\n"
"		passThroughEvent = sampleMatA ? (passThroughEvent / weight1) : (passThroughEvent - weight1) / weight2;\n"
"		const uint matIndex = sampleMatA ? currentMixMat->mix.matAIndex : currentMixMat->mix.matBIndex;\n"
"		__global Material *mat = &mats[matIndex];\n"
"\n"
"		if (mat->type == MIX) {\n"
"			currentMixMat = mat;\n"
"		} else\n"
"			return Material_GetPassThroughTransparencyNoMix(mat, hitPoint, fixedDir, passThroughEvent\n"
"					TEXTURES_PARAM);\n"
"	}\n"
"}\n"
"\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"uint MixMaterial_GetInteriorVolume(__global Material *material, \n"
"		__global HitPoint *hitPoint\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"		, const float passEvent\n"
"#endif\n"
"		MATERIALS_PARAM_DECL) {\n"
"	if (material->interiorVolumeIndex != NULL_INDEX)\n"
"		return material->interiorVolumeIndex;\n"
"\n"
"	__global Material *currentMixMat = material;\n"
"	float passThroughEvent = passEvent;\n"
"	for (;;) {\n"
"		const float factor = Texture_GetFloatValue(currentMixMat->mix.mixFactorTexIndex, hitPoint\n"
"				TEXTURES_PARAM);\n"
"		const float weight2 = clamp(factor, 0.f, 1.f);\n"
"		const float weight1 = 1.f - weight2;\n"
"\n"
"		const bool sampleMatA = (passThroughEvent < weight1);\n"
"		passThroughEvent = sampleMatA ? (passThroughEvent / weight1) : (passThroughEvent - weight1) / weight2;\n"
"		const uint matIndex = sampleMatA ? currentMixMat->mix.matAIndex : currentMixMat->mix.matBIndex;\n"
"		__global Material *mat = &mats[matIndex];\n"
"\n"
"		if (mat->type == MIX) {\n"
"			if (mat->interiorVolumeIndex != NULL_INDEX)\n"
"				return mat->interiorVolumeIndex;\n"
"			else\n"
"				currentMixMat = mat;\n"
"		} else\n"
"			return mat->interiorVolumeIndex;\n"
"	}\n"
"}\n"
"#endif\n"
"\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"uint MixMaterial_GetExteriorVolume(__global Material *material, \n"
"		__global HitPoint *hitPoint\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"		, const float passEvent\n"
"#endif\n"
"		MATERIALS_PARAM_DECL) {\n"
"	if (material->exteriorVolumeIndex != NULL_INDEX)\n"
"		return material->exteriorVolumeIndex;\n"
"\n"
"	__global Material *currentMixMat = material;\n"
"	float passThroughEvent = passEvent;\n"
"	for (;;) {\n"
"		const float factor = Texture_GetFloatValue(currentMixMat->mix.mixFactorTexIndex, hitPoint\n"
"				TEXTURES_PARAM);\n"
"		const float weight2 = clamp(factor, 0.f, 1.f);\n"
"		const float weight1 = 1.f - weight2;\n"
"\n"
"		const bool sampleMatA = (passThroughEvent < weight1);\n"
"		passThroughEvent = sampleMatA ? (passThroughEvent / weight1) : (passThroughEvent - weight1) / weight2;\n"
"		const uint matIndex = sampleMatA ? currentMixMat->mix.matAIndex : currentMixMat->mix.matBIndex;\n"
"		__global Material *mat = &mats[matIndex];\n"
"\n"
"		if (mat->type == MIX) {\n"
"			if (mat->exteriorVolumeIndex != NULL_INDEX)\n"
"				return mat->exteriorVolumeIndex;\n"
"			else\n"
"				currentMixMat = mat;\n"
"		} else\n"
"			return mat->exteriorVolumeIndex;\n"
"	}\n"
"}\n"
"#endif\n"
"\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// Generic material functions with Mix support\n"
"//------------------------------------------------------------------------------\n"
"\n"
"BSDFEvent Material_GetEventTypes(__global Material *material\n"
"		MATERIALS_PARAM_DECL) {\n"
"#if defined (PARAM_ENABLE_MAT_MIX)\n"
"	if (material->type == MIX)\n"
"		return MixMaterial_GetEventTypes(material\n"
"				MATERIALS_PARAM);\n"
"	else\n"
"#endif\n"
"		return Material_GetEventTypesNoMix(material);\n"
"}\n"
"\n"
"bool Material_IsDelta(__global Material *material\n"
"		MATERIALS_PARAM_DECL) {\n"
"#if defined (PARAM_ENABLE_MAT_MIX)\n"
"	if (material->type == MIX)\n"
"		return MixMaterial_IsDelta(material\n"
"				MATERIALS_PARAM);\n"
"	else\n"
"#endif\n"
"		return Material_IsDeltaNoMix(material);\n"
"}\n"
"\n"
"float3 Material_Evaluate(__global Material *material,\n"
"		__global HitPoint *hitPoint, const float3 lightDir, const float3 eyeDir,\n"
"		BSDFEvent *event, float *directPdfW\n"
"		MATERIALS_PARAM_DECL) {\n"
"#if defined (PARAM_ENABLE_MAT_MIX)\n"
"	if (material->type == MIX)\n"
"		return MixMaterial_Evaluate(material, hitPoint, lightDir, eyeDir,\n"
"				event, directPdfW\n"
"				MATERIALS_PARAM);\n"
"	else\n"
"#endif\n"
"		return Material_EvaluateNoMix(material, hitPoint, lightDir, eyeDir,\n"
"				event, directPdfW\n"
"				TEXTURES_PARAM);\n"
"}\n"
"\n"
"float3 Material_Sample(__global Material *material,	__global HitPoint *hitPoint,\n"
"		const float3 fixedDir, float3 *sampledDir,\n"
"		const float u0, const float u1,\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"		const float passThroughEvent,\n"
"#endif\n"
"		float *pdfW, float *cosSampledDir, BSDFEvent *event,\n"
"		const BSDFEvent requestedEvent\n"
"		MATERIALS_PARAM_DECL) {\n"
"#if defined (PARAM_ENABLE_MAT_MIX)\n"
"	if (material->type == MIX)\n"
"		return MixMaterial_Sample(material, hitPoint,\n"
"				fixedDir, sampledDir,\n"
"				u0, u1,\n"
"				passThroughEvent,\n"
"				pdfW, cosSampledDir, event, requestedEvent\n"
"				MATERIALS_PARAM);\n"
"	else\n"
"#endif\n"
"		return Material_SampleNoMix(material, hitPoint,\n"
"				fixedDir, sampledDir,\n"
"				u0, u1,\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"				passThroughEvent,\n"
"#endif\n"
"				pdfW, cosSampledDir, event, requestedEvent\n"
"				TEXTURES_PARAM);\n"
"}\n"
"\n"
"float3 Material_GetEmittedRadiance(__global Material *material,\n"
"		__global HitPoint *hitPoint, const float oneOverPrimitiveArea\n"
"		MATERIALS_PARAM_DECL) {\n"
"	float3 result;\n"
"#if defined (PARAM_ENABLE_MAT_MIX)\n"
"	if (material->type == MIX)\n"
"		result = MixMaterial_GetEmittedRadiance(material, hitPoint\n"
"				MATERIALS_PARAM);\n"
"	else\n"
"#endif\n"
"		result = Material_GetEmittedRadianceNoMix(material, hitPoint\n"
"				TEXTURES_PARAM);\n"
"\n"
"	return 	VLOAD3F(material->emittedFactor.c) * (material->usePrimitiveArea ? oneOverPrimitiveArea : 1.f) * result;\n"
"}\n"
"\n"
"#if defined(PARAM_HAS_BUMPMAPS)\n"
"void Material_Bump(__global Material *material, __global HitPoint *hitPoint,\n"
"        const float3 dpdu, const float3 dpdv,\n"
"        const float3 dndu, const float3 dndv, const float weight\n"
"        MATERIALS_PARAM_DECL) {\n"
"#if defined (PARAM_ENABLE_MAT_MIX)\n"
"	if (material->type == MIX)\n"
"		MixMaterial_Bump(material, hitPoint,\n"
"                dpdu, dpdv, dndu, dndv, weight\n"
"                MATERIALS_PARAM);\n"
"	else\n"
"#endif\n"
"		Material_BumpNoMix(material, hitPoint,\n"
"                dpdu, dpdv, dndu, dndv, weight\n"
"                MATERIALS_PARAM);\n"
"}\n"
"#endif\n"
"\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"float3 Material_GetPassThroughTransparency(__global Material *material,\n"
"		__global HitPoint *hitPoint, const float3 fixedDir, const float passThroughEvent\n"
"		MATERIALS_PARAM_DECL) {\n"
"#if defined (PARAM_ENABLE_MAT_MIX)\n"
"	if (material->type == MIX)\n"
"		return MixMaterial_GetPassThroughTransparency(material,\n"
"				hitPoint, fixedDir, passThroughEvent\n"
"				MATERIALS_PARAM);\n"
"	else\n"
"#endif\n"
"		return Material_GetPassThroughTransparencyNoMix(material,\n"
"				hitPoint, fixedDir, passThroughEvent\n"
"				TEXTURES_PARAM);\n"
"}\n"
"#endif\n"
"\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"uint Material_GetInteriorVolume(__global Material *material, \n"
"		__global HitPoint *hitPoint\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"		, const float passThroughEvent\n"
"#endif\n"
"		MATERIALS_PARAM_DECL) {\n"
"#if defined (PARAM_ENABLE_MAT_MIX)\n"
"	if (material->type == MIX)\n"
"		return MixMaterial_GetInteriorVolume(material, hitPoint\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"			, passThroughEvent\n"
"#endif\n"
"			MATERIALS_PARAM);\n"
"	else\n"
"#endif\n"
"		return material->interiorVolumeIndex;\n"
"}\n"
"\n"
"uint Material_GetExteriorVolume(__global Material *material, \n"
"		__global HitPoint *hitPoint\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"		, const float passThroughEvent\n"
"#endif\n"
"		MATERIALS_PARAM_DECL) {\n"
"#if defined (PARAM_ENABLE_MAT_MIX)\n"
"	if (material->type == MIX)\n"
"		return MixMaterial_GetExteriorVolume(material, hitPoint\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"			, passThroughEvent\n"
"#endif\n"
"			MATERIALS_PARAM);\n"
"	else\n"
"#endif\n"
"		return material->exteriorVolumeIndex;\n"
"}\n"
"#endif\n"
; } }
