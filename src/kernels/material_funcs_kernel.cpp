#include <string>
namespace luxrays { namespace ocl {
std::string KernelSource_material_funcs = 
"#line 2 \"material_funcs.cl\"\n"
"\n"
"/***************************************************************************\n"
" *   Copyright (C) 1998-2010 by authors (see AUTHORS.txt )                 *\n"
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
"// Matte material\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined (PARAM_ENABLE_MAT_MATTE)\n"
"\n"
"float3 MatteMaterial_Evaluate(__global Material *material, __global Texture *texs,\n"
"		const float2 uv, const float3 lightDir, const float3 eyeDir,\n"
"		BSDFEvent *event, float *directPdfW\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"	if (directPdfW)\n"
"		*directPdfW = fabs(lightDir.z * M_1_PI_F);\n"
"\n"
"	*event = DIFFUSE | REFLECT;\n"
"\n"
"	const float3 kd = (float3)(1.f, 0.f, 0.f);/*Texture_GetColorValue(&texs[material->matte.kdTexIndex], uv\n"
"			IMAGEMAPS_PARAM);*/\n"
"	return M_1_PI_F * kd;\n"
"}\n"
"\n"
"float3 MatteMaterial_Sample(__global Material *material, __global Texture *texs,\n"
"		const float2 uv, const float3 fixedDir, float3 *sampledDir,\n"
"		const float u0, const float u1, \n"
"		float *pdfW, float *cosSampledDir, BSDFEvent *event\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"	if (fabs(fixedDir.z) < DEFAULT_COS_EPSILON_STATIC)\n"
"		return BLACK;\n"
"\n"
"	*sampledDir = (signbit(fixedDir.z) ? -1.f : 1.f) * CosineSampleHemisphereWithPdf(u0, u1, pdfW);\n"
"\n"
"	*cosSampledDir = fabs((*sampledDir).z);\n"
"	if (*cosSampledDir < DEFAULT_COS_EPSILON_STATIC)\n"
"		return BLACK;\n"
"\n"
"	*event = DIFFUSE | REFLECT;\n"
"\n"
"	const float3 kd = Texture_GetColorValue(&texs[material->matte.kdTexIndex], uv\n"
"			IMAGEMAPS_PARAM);\n"
"	return M_1_PI_F * kd;\n"
"}\n"
"\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// Mirror material\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined (PARAM_ENABLE_MAT_MIRROR)\n"
"\n"
"float3 MirrorMaterial_Sample(__global Material *material, __global Texture *texs,\n"
"		const float2 uv, const float3 fixedDir, float3 *sampledDir,\n"
"		const float u0, const float u1,\n"
"		float *pdfW, float *cosSampledDir, BSDFEvent *event\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"	*event = SPECULAR | REFLECT;\n"
"\n"
"	*sampledDir = (float3)(-fixedDir.x, -fixedDir.y, fixedDir.z);\n"
"	*pdfW = 1.f;\n"
"\n"
"	*cosSampledDir = fabs((*sampledDir).z);\n"
"	const float3 kr = Texture_GetColorValue(&texs[material->mirror.krTexIndex], uv\n"
"			IMAGEMAPS_PARAM);\n"
"	// The cosSampledDir is used to compensate the other one used inside the integrator\n"
"	return kr / (*cosSampledDir);\n"
"}\n"
"\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// Glass material\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined (PARAM_ENABLE_MAT_GLASS)\n"
"\n"
"float3 GlassMaterial_Sample(__global Material *material, __global Texture *texs,\n"
"		const float2 uv, const float3 fixedDir, float3 *sampledDir,\n"
"		const float u0, const float u1, const float passThroughEvent,\n"
"		float *pdfW, float *cosSampledDir, BSDFEvent *event\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"	// Ray from outside going in ?\n"
"	const bool into = (fixedDir.z > 0.f);\n"
"\n"
"	// TODO: remove\n"
"	const float3 shadeN = (float3)(0.f, 0.f, into ? 1.f : -1.f);\n"
"	const float3 N = (float3)(0.f, 0.f, 1.f);\n"
"\n"
"	const float3 rayDir = -fixedDir;\n"
"	const float3 reflDir = rayDir - (2.f * dot(N, rayDir)) * N;\n"
"\n"
"	const float nc = Texture_GetGreyValue(&texs[material->glass.ousideIorTexIndex], uv\n"
"			IMAGEMAPS_PARAM);\n"
"	const float nt = Texture_GetGreyValue(&texs[material->glass.iorTexIndex], uv\n"
"			IMAGEMAPS_PARAM);\n"
"	const float nnt = into ? (nc / nt) : (nt / nc);\n"
"	const float nnt2 = nnt * nnt;\n"
"	const float ddn = dot(rayDir, shadeN);\n"
"	const float cos2t = 1.f - nnt2 * (1.f - ddn * ddn);\n"
"\n"
"	// Total internal reflection\n"
"	if (cos2t < 0.f) {\n"
"		*event = SPECULAR | REFLECT;\n"
"		*sampledDir = reflDir;\n"
"		*cosSampledDir = fabs((*sampledDir).z);\n"
"		*pdfW = 1.f;\n"
"\n"
"		const float3 kr = Texture_GetColorValue(&texs[material->glass.krTexIndex], uv\n"
"				IMAGEMAPS_PARAM);\n"
"		// The cosSampledDir is used to compensate the other one used inside the integrator\n"
"		return kr / (*cosSampledDir);\n"
"	}\n"
"\n"
"	const float kk = (into ? 1.f : -1.f) * (ddn * nnt + sqrt(cos2t));\n"
"	const float3 nkk = kk * N;\n"
"	const float3 transDir = normalize(nnt * rayDir - nkk);\n"
"\n"
"	const float c = 1.f - (into ? -ddn : dot(transDir, N));\n"
"	const float c2 = c * c;\n"
"	const float a = nt - nc;\n"
"	const float b = nt + nc;\n"
"	const float R0 = a * a / (b * b);\n"
"	const float Re = R0 + (1.f - R0) * c2 * c2 * c;\n"
"	const float Tr = 1.f - Re;\n"
"	const float P = .25f + .5f * Re;\n"
"\n"
"	if (Tr == 0.f) {\n"
"		if (Re == 0.f)\n"
"			return BLACK;\n"
"		else {\n"
"			*event = SPECULAR | REFLECT;\n"
"			*sampledDir = reflDir;\n"
"			*cosSampledDir = fabs((*sampledDir).z);\n"
"			*pdfW = 1.f;\n"
"\n"
"			const float3 kr = Texture_GetColorValue(&texs[material->glass.krTexIndex], uv\n"
"					IMAGEMAPS_PARAM);\n"
"			// The cosSampledDir is used to compensate the other one used inside the integrator\n"
"			return kr / (*cosSampledDir);\n"
"		}\n"
"	} else if (Re == 0.f) {\n"
"		*event = SPECULAR | TRANSMIT;\n"
"		*sampledDir = transDir;\n"
"		*cosSampledDir = fabs((*sampledDir).z);\n"
"		*pdfW = 1.f;\n"
"\n"
"		// The cosSampledDir is used to compensate the other one used inside the integrator\n"
"//		if (fromLight)\n"
"//			return Kt->GetColorValue(uv) * (nnt2 / (*cosSampledDir));\n"
"//		else\n"
"//			return Kt->GetColorValue(uv) / (*cosSampledDir);\n"
"		const float3 kt = Texture_GetColorValue(&texs[material->glass.ktTexIndex], uv\n"
"				IMAGEMAPS_PARAM);\n"
"		return kt / (*cosSampledDir);\n"
"	} else if (passThroughEvent < P) {\n"
"		*event = SPECULAR | REFLECT;\n"
"		*sampledDir = reflDir;\n"
"		*cosSampledDir = fabs((*sampledDir).z);\n"
"		*pdfW = P / Re;\n"
"\n"
"		const float3 kr = Texture_GetColorValue(&texs[material->glass.krTexIndex], uv\n"
"				IMAGEMAPS_PARAM);\n"
"		// The cosSampledDir is used to compensate the other one used inside the integrator\n"
"		return kr / (*cosSampledDir);\n"
"	} else {\n"
"		*event = SPECULAR | TRANSMIT;\n"
"		*sampledDir = transDir;\n"
"		*cosSampledDir = fabs((*sampledDir).z);\n"
"		*pdfW = (1.f - P) / Tr;\n"
"\n"
"		// The cosSampledDir is used to compensate the other one used inside the integrator\n"
"//		if (fromLight)\n"
"//			return Kt->GetColorValue(uv) * (nnt2 / (*cosSampledDir));\n"
"//		else\n"
"//			return Kt->GetColorValue(uv) / (*cosSampledDir);\n"
"		const float3 kt = Texture_GetColorValue(&texs[material->glass.ktTexIndex], uv\n"
"				IMAGEMAPS_PARAM);\n"
"		return kt / (*cosSampledDir);\n"
"	}\n"
"}\n"
"\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// Metal material\n"
"//------------------------------------------------------------------------------\n"
"\n"
"float3 GlossyReflection(const float3 fixedDir, const float exponent,\n"
"		const float u0, const float u1) {\n"
"	// Ray from outside going in ?\n"
"	const bool into = (fixedDir.z > 0.f);\n"
"	const float3 shadeN = (float3)(0.f, 0.f, into ? 1.f : -1.f);\n"
"\n"
"	const float phi = 2.f * M_PI * u0;\n"
"	const float cosTheta = pow(1.f - u1, exponent);\n"
"	const float sinTheta = sqrt(fmax(0.f, 1.f - cosTheta * cosTheta));\n"
"	const float x = cos(phi) * sinTheta;\n"
"	const float y = sin(phi) * sinTheta;\n"
"	const float z = cosTheta;\n"
"\n"
"	const float3 dir = -fixedDir;\n"
"	const float dp = dot(shadeN, dir);\n"
"	const float3 w = dir - (2.f * dp) * shadeN;\n"
"\n"
"	const float3 u = normalize(cross(\n"
"			(fabs(shadeN.x) > .1f) ? ((float3)(0.f, 1.f, 0.f)) : ((float3)(1.f, 0.f, 0.f)),\n"
"			w));\n"
"	const float3 v = cross(w, u);\n"
"\n"
"	return x * u + y * v + z * w;\n"
"}\n"
"\n"
"#if defined (PARAM_ENABLE_MAT_METAL)\n"
"\n"
"float3 MetalMaterial_Sample(__global Material *material, __global Texture *texs,\n"
"		const float2 uv, const float3 fixedDir, float3 *sampledDir,\n"
"		const float u0, const float u1,\n"
"		float *pdfW, float *cosSampledDir, BSDFEvent *event\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"	const float e = 1.f / (Texture_GetGreyValue(&texs[material->metal.expTexIndex], uv\n"
"				IMAGEMAPS_PARAM) + 1.f);\n"
"	*sampledDir = GlossyReflection(fixedDir, e, u0, u1);\n"
"\n"
"	if ((*sampledDir).z * fixedDir.z > 0.f) {\n"
"		*event = SPECULAR | REFLECT;\n"
"		*pdfW = 1.f;\n"
"		*cosSampledDir = fabs((*sampledDir).z);\n"
"\n"
"		const float3 kt = Texture_GetColorValue(&texs[material->metal.krTexIndex], uv\n"
"				IMAGEMAPS_PARAM);\n"
"		// The cosSampledDir is used to compensate the other one used inside the integrator\n"
"		return kt / (*cosSampledDir);\n"
"	} else\n"
"		return BLACK;\n"
"}\n"
"\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// ArchGlass material\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined (PARAM_ENABLE_MAT_ARCHGLASS)\n"
"\n"
"float3 ArchGlassMaterial_Sample(__global Material *material, __global Texture *texs,\n"
"		const float2 uv, const float3 fixedDir, float3 *sampledDir,\n"
"		const float u0, const float u1, const float passThroughEvent,\n"
"		float *pdfW, float *cosSampledDir, BSDFEvent *event\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"	// Ray from outside going in ?\n"
"	const bool into = (fixedDir.z > 0.f);\n"
"\n"
"	// TODO: remove\n"
"	const float3 shadeN = (float3)(0.f, 0.f, into ? 1.f : -1.f);\n"
"	const float3 N = (float3)(0.f, 0.f, 1.f);\n"
"\n"
"	const float3 rayDir = -fixedDir;\n"
"	const float3 reflDir = rayDir - (2.f * dot(N, rayDir)) * N;\n"
"\n"
"	const float ddn = dot(rayDir, shadeN);\n"
"	const float cos2t = ddn * ddn;\n"
"\n"
"	// Total internal reflection is not possible\n"
"	const float kk = (into ? 1.f : -1.f) * (ddn + sqrt(cos2t));\n"
"	const float3 nkk = kk * N;\n"
"	const float3 transDir = normalize(rayDir - nkk);\n"
"\n"
"	const float c = 1.f - (into ? -ddn : dot(transDir, N));\n"
"	const float c2 = c * c;\n"
"	const float Re = c2 * c2 * c;\n"
"	const float Tr = 1.f - Re;\n"
"	const float P = .25f + .5f * Re;\n"
"\n"
"	if (Tr == 0.f) {\n"
"		if (Re == 0.f)\n"
"			return BLACK;\n"
"		else {\n"
"			*event = SPECULAR | REFLECT;\n"
"			*sampledDir = reflDir;\n"
"			*cosSampledDir = fabs((*sampledDir).z);\n"
"			*pdfW = 1.f;\n"
"\n"
"			const float3 kr = Texture_GetColorValue(&texs[material->archglass.krTexIndex], uv\n"
"					IMAGEMAPS_PARAM);\n"
"			// The cosSampledDir is used to compensate the other one used inside the integrator\n"
"			return kr / (*cosSampledDir);\n"
"		}\n"
"	} else if (Re == 0.f) {\n"
"		*event = SPECULAR | TRANSMIT;\n"
"		*sampledDir = transDir;\n"
"		*cosSampledDir = fabs((*sampledDir).z);\n"
"		*pdfW = 1.f;\n"
"\n"
"		const float3 kt = Texture_GetColorValue(&texs[material->archglass.ktTexIndex], uv\n"
"				IMAGEMAPS_PARAM);\n"
"		// The cosSampledDir is used to compensate the other one used inside the integrator\n"
"		return kt / (*cosSampledDir);\n"
"	} else if (passThroughEvent < P) {\n"
"		*event = SPECULAR | REFLECT;\n"
"		*sampledDir = reflDir;\n"
"		*cosSampledDir = fabs((*sampledDir).z);\n"
"		*pdfW = P / Re;\n"
"\n"
"		const float3 kr = Texture_GetColorValue(&texs[material->archglass.krTexIndex], uv\n"
"				IMAGEMAPS_PARAM);\n"
"		// The cosSampledDir is used to compensate the other one used inside the integrator\n"
"		return kr / (*cosSampledDir);\n"
"	} else {\n"
"		*event = SPECULAR | TRANSMIT;\n"
"		*sampledDir = transDir;\n"
"		*cosSampledDir = fabs((*sampledDir).z);\n"
"		*pdfW = (1.f - P) / Tr;\n"
"\n"
"		const float3 kt = Texture_GetColorValue(&texs[material->archglass.ktTexIndex], uv\n"
"				IMAGEMAPS_PARAM);\n"
"		// The cosSampledDir is used to compensate the other one used inside the integrator\n"
"		return kt / (*cosSampledDir);\n"
"	}\n"
"}\n"
"\n"
"float3 ArchGlassMaterial_GetPassThroughTransparency(__global Material *material, __global Texture *texs,\n"
"		const float2 uv, const float3 fixedDir, const float passThroughEvent\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"	// Ray from outside going in ?\n"
"	const bool into = (fixedDir.z > 0.f);\n"
"\n"
"	// TODO: remove\n"
"	const float3 shadeN = (float3)(0.f, 0.f, into ? 1.f : -1.f);\n"
"	const float3 N = (float3)(0.f, 0.f, 1.f);\n"
"\n"
"	const float3 rayDir = -fixedDir;\n"
"\n"
"	const float ddn = dot(rayDir, shadeN);\n"
"	const float cos2t = ddn * ddn;\n"
"\n"
"	// Total internal reflection is not possible\n"
"	const float kk = (into ? 1.f : -1.f) * (ddn + sqrt(cos2t));\n"
"	const float3 nkk = kk * N;\n"
"	const float3 transDir = normalize(rayDir - nkk);\n"
"\n"
"	const float c = 1.f - (into ? -ddn : dot(transDir, N));\n"
"	const float c2 = c * c;\n"
"	const float Re = c2 * c2 * c;\n"
"	const float Tr = 1.f - Re;\n"
"	const float P = .25f + .5f * Re;\n"
"\n"
"	if (Tr == 0.f)\n"
"		return BLACK;\n"
"	else if (Re == 0.f) {\n"
"		const float3 kt = Texture_GetColorValue(&texs[material->archglass.ktTexIndex], uv\n"
"				IMAGEMAPS_PARAM);\n"
"		return kt;\n"
"	} else if (passThroughEvent < P)\n"
"		return BLACK;\n"
"	else {\n"
"		const float3 kt = Texture_GetColorValue(&texs[material->archglass.ktTexIndex], uv\n"
"				IMAGEMAPS_PARAM);\n"
"		return kt;\n"
"	}\n"
"}\n"
"\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// NULL material\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined (PARAM_ENABLE_MAT_NULL)\n"
"\n"
"float3 NullMaterial_Sample(__global Material *material, __global Texture *texs,\n"
"		const float2 uv, const float3 fixedDir, float3 *sampledDir,\n"
"		const float u0, const float u1,\n"
"		float *pdfW, float *cosSampledDir, BSDFEvent *event\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"	*sampledDir = -fixedDir;\n"
"	*cosSampledDir = 1.f;\n"
"\n"
"	*pdfW = 1.f;\n"
"	*event = SPECULAR | TRANSMIT;\n"
"	return WHITE;\n"
"}\n"
"\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// MatteTranslucent material\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined (PARAM_ENABLE_MAT_MATTETRANSLUCENT)\n"
"\n"
"float3 MatteTranslucentMaterial_Evaluate(__global Material *material, __global Texture *texs,\n"
"		const float2 uv, const float3 lightDir, const float3 eyeDir,\n"
"		BSDFEvent *event, float *directPdfW\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"	const float cosSampledDir = dot(lightDir, eyeDir);\n"
"	if (fabs(cosSampledDir) < DEFAULT_COS_EPSILON_STATIC)\n"
"		return BLACK;\n"
"\n"
"	const float3 r = Texture_GetColorValue(&texs[material->matteTranslucent.krTexIndex], uv\n"
"			IMAGEMAPS_PARAM);\n"
"	const float3 t = Texture_GetColorValue(&texs[material->matteTranslucent.ktTexIndex], uv\n"
"			IMAGEMAPS_PARAM) * \n"
"		// Energy conservation\n"
"		(1.f - r);\n"
"\n"
"	if (directPdfW)\n"
"		*directPdfW = .5f * fabs(lightDir.z * M_1_PI_F);\n"
"\n"
"	if (cosSampledDir > 0.f) {\n"
"		*event = DIFFUSE | REFLECT;\n"
"		return r * M_1_PI_F;\n"
"	} else {\n"
"		*event = DIFFUSE | TRANSMIT;\n"
"		return t * M_1_PI_F;\n"
"	}\n"
"}\n"
"\n"
"float3 MatteTranslucentMaterial_Sample(__global Material *material, __global Texture *texs,\n"
"		const float2 uv, const float3 fixedDir, float3 *sampledDir,\n"
"		const float u0, const float u1, \n"
"		const float passThroughEvent,\n"
"		float *pdfW, float *cosSampledDir, BSDFEvent *event\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"	if (fabs(fixedDir.z) < DEFAULT_COS_EPSILON_STATIC)\n"
"		return BLACK;\n"
"\n"
"	*sampledDir = CosineSampleHemisphereWithPdf(u0, u1, pdfW);\n"
"	*cosSampledDir = fabs((*sampledDir).z);\n"
"	if (*cosSampledDir < DEFAULT_COS_EPSILON_STATIC)\n"
"		return BLACK;\n"
"\n"
"	*pdfW *= .5f;\n"
"\n"
"	const float3 r = Texture_GetColorValue(&texs[material->matteTranslucent.krTexIndex], uv\n"
"			IMAGEMAPS_PARAM);\n"
"	const float3 t = Texture_GetColorValue(&texs[material->matteTranslucent.ktTexIndex], uv\n"
"			IMAGEMAPS_PARAM) * \n"
"		// Energy conservation\n"
"		(1.f - r);\n"
"\n"
"	if (passThroughEvent < .5f) {\n"
"		*sampledDir *= (signbit(fixedDir.z) ? -1.f : 1.f);\n"
"		*event = DIFFUSE | REFLECT;\n"
"		return r * M_1_PI_F;\n"
"	} else {\n"
"		*sampledDir *= -(signbit(fixedDir.z) ? -1.f : 1.f);\n"
"		*event = DIFFUSE | TRANSMIT;\n"
"		return t * M_1_PI_F;\n"
"	}\n"
"}\n"
"\n"
"#endif\n"
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
"		// Non Specular materials\n"
"#if defined (PARAM_ENABLE_MAT_MATTETRANSLUCENT)\n"
"		case MATTETRANSLUCENT:\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_MATTE)\n"
"		case MATTE:\n"
"			return false;\n"
"#endif\n"
"		// Specular materials\n"
"#if defined (PARAM_ENABLE_MAT_MIRROR)\n"
"		case MIRROR:\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_GLASS)\n"
"		case GLASS:\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_METAL)\n"
"		case METAL:\n"
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
"#if defined (PARAM_ENABLE_MAT_MIRROR)\n"
"		case MIRROR:\n"
"			return SPECULAR | REFLECT;\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_GLASS)\n"
"		case GLASS:\n"
"			return SPECULAR | REFLECT | TRANSMIT;\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_METAL)\n"
"		case METAL:\n"
"			return SPECULAR | REFLECT;\n"
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
"		default:\n"
"			return NONE;\n"
"	}\n"
"}\n"
"\n"
"float3 Material_SampleNoMix(__global Material *material, __global Texture *texs,\n"
"		const float2 uv, const float3 fixedDir, float3 *sampledDir,\n"
"		const float u0, const float u1,\n"
"#if defined(PARAM_HAS_PASSTHROUGHT)\n"
"		const float passThroughEvent,\n"
"#endif\n"
"		float *pdfW, float *cosSampledDir, BSDFEvent *event\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"	switch (material->type) {\n"
"#if defined (PARAM_ENABLE_MAT_MATTE)\n"
"		case MATTE:\n"
"			return MatteMaterial_Sample(material, texs, uv, fixedDir, sampledDir,\n"
"					u0, u1,	pdfW, cosSampledDir, event\n"
"					IMAGEMAPS_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_MIRROR)\n"
"		case MIRROR:\n"
"			return MirrorMaterial_Sample(material, texs, uv, fixedDir, sampledDir,\n"
"					u0, u1, pdfW, cosSampledDir, event\n"
"					IMAGEMAPS_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_GLASS)\n"
"		case GLASS:\n"
"			return GlassMaterial_Sample(material, texs, uv, fixedDir, sampledDir,\n"
"					u0, u1,	passThroughEvent, pdfW, cosSampledDir, event\n"
"					IMAGEMAPS_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_METAL)\n"
"		case METAL:\n"
"			return MetalMaterial_Sample(material, texs, uv, fixedDir, sampledDir,\n"
"					u0, u1,	pdfW, cosSampledDir, event\n"
"					IMAGEMAPS_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_ARCHGLASS)\n"
"		case ARCHGLASS:\n"
"			return ArchGlassMaterial_Sample(material, texs, uv, fixedDir, sampledDir,\n"
"					u0, u1,	passThroughEvent, pdfW, cosSampledDir, event\n"
"					IMAGEMAPS_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_NULL)\n"
"		case NULLMAT:\n"
"			return NullMaterial_Sample(material, texs, uv, fixedDir, sampledDir,\n"
"					u0, u1, pdfW, cosSampledDir, event\n"
"					IMAGEMAPS_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_MATTETRANSLUCENT)\n"
"		case MATTETRANSLUCENT:\n"
"			return MatteTranslucentMaterial_Sample(material, texs, uv, fixedDir, sampledDir,\n"
"					u0, u1,	passThroughEvent, pdfW, cosSampledDir, event\n"
"					IMAGEMAPS_PARAM);\n"
"#endif\n"
"		default:\n"
"			return BLACK;\n"
"	}\n"
"}\n"
"\n"
"float3 Material_EvaluateNoMix(__global Material *material, __global Texture *texs,\n"
"		const float2 uv, const float3 lightDir, const float3 eyeDir,\n"
"		BSDFEvent *event, float *directPdfW\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"	switch (material->type) {\n"
"#if defined (PARAM_ENABLE_MAT_MATTE)\n"
"		case MATTE:\n"
"			return MatteMaterial_Evaluate(material, texs, uv, lightDir, eyeDir, event, directPdfW\n"
"					IMAGEMAPS_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_MATTETRANSLUCENT)\n"
"		case MATTETRANSLUCENT:\n"
"			return MatteTranslucentMaterial_Evaluate(material, texs, uv, lightDir, eyeDir, event, directPdfW\n"
"					IMAGEMAPS_PARAM);\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_MIRROR)\n"
"		case MIRROR:\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_GLASS)\n"
"		case GLASS:\n"
"#endif\n"
"#if defined (PARAM_ENABLE_MAT_METAL)\n"
"		case METAL:\n"
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
"float3 Material_GetEmittedRadianceNoMix(__global Material *material, __global Texture *texs, const float2 uv\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"	const uint emitTexIndex = material->emitTexIndex;\n"
"	if (emitTexIndex == NULL_INDEX)\n"
"		return BLACK;\n"
"\n"
"	return Texture_GetColorValue(&texs[emitTexIndex], uv\n"
"			IMAGEMAPS_PARAM);\n"
"}\n"
"\n"
"float3 Material_GetPassThroughTransparencyNoMix(__global Material *material, __global Texture *texs,\n"
"		const float2 uv, const float3 fixedDir, const float passThroughEvent\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"	switch (material->type) {\n"
"#if defined (PARAM_ENABLE_MAT_ARCHGLASS)\n"
"		case ARCHGLASS:\n"
"			return ArchGlassMaterial_GetPassThroughTransparency(material, texs,\n"
"					uv, fixedDir, passThroughEvent\n"
"					IMAGEMAPS_PARAM);\n"
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
"		const float2 uv, const float3 lightDir, const float3 eyeDir,\n"
"		BSDFEvent *event, float *directPdfW\n"
"		MATERIALS_PARAM_DECL\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
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
"			const float factor = Texture_GetGreyValue(&texs[m->mix.mixFactorTexIndex], uv\n"
"					IMAGEMAPS_PARAM);\n"
"			const float weight2 = clamp(factor, 0.f, 1.f);\n"
"			const float weight1 = 1.f - weight2;\n"
"\n"
"			materialStack[++stackIndex] = &mats[m->mix.matAIndex];\n"
"			totalWeightStack[stackIndex] = totalWeight * weight1;\n"
"\n"
"			materialStack[++stackIndex] = &mats[m->mix.matBIndex];			\n"
"			totalWeightStack[stackIndex] = totalWeight * weight2;\n"
"		} else {\n"
"			// Normal GetEventTypes() evaluation\n"
"			if (totalWeight > 0.f) {\n"
"				BSDFEvent eventMat;\n"
"				float directPdfWMat;\n"
"				const float3 resultMat = Material_EvaluateNoMix(m, texs, uv, lightDir, eyeDir, &eventMat, &directPdfWMat\n"
"						IMAGEMAPS_PARAM);\n"
"				if (!Spectrum_IsBlack(BLACK)) {\n"
"					result += totalWeight * resultMat;\n"
"\n"
"					if (directPdfW)\n"
"						*directPdfW += totalWeight * directPdfWMat;\n"
"				}\n"
"				\n"
"				*event |= eventMat;\n"
"			}\n"
"		}\n"
"	}\n"
"\n"
"	return result;\n"
"}\n"
"\n"
"float3 MixMaterial_Sample(__global Material *material,\n"
"		const float2 uv, const float3 fixedDir, float3 *sampledDir,\n"
"		const float u0, const float u1, const float passEvent,\n"
"		float *pdfW, float *cosSampledDir, BSDFEvent *event\n"
"		MATERIALS_PARAM_DECL\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
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
"		const float factor = Texture_GetGreyValue(&texs[currentMixMat->mix.mixFactorTexIndex], uv\n"
"			IMAGEMAPS_PARAM);\n"
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
"			const float3 sampleResult = Material_SampleNoMix(matFirst, texs, uv,\n"
"					fixedDir, sampledDir,\n"
"					u0, u1, passThroughEventFirst,\n"
"					&pdfWMatFirst, cosSampledDir, event\n"
"					IMAGEMAPS_PARAM);\n"
"\n"
"			if (all(isequal(sampleResult, BLACK)))\n"
"				return BLACK;\n"
"\n"
"			const float weight = parentWeight * weightFirst;\n"
"			*pdfW += weight * pdfWMatFirst;\n"
"			result += weight * sampleResult;\n"
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
"			eval = MixMaterial_Evaluate(evalMat, uv, *sampledDir, fixedDir,\n"
"					&eventMat, &pdfWMat\n"
"					MATERIALS_PARAM\n"
"					IMAGEMAPS_PARAM);\n"
"		} else {\n"
"			eval = Material_EvaluateNoMix(evalMat, texs, uv, *sampledDir, fixedDir,\n"
"					&eventMat, &pdfWMat\n"
"					IMAGEMAPS_PARAM);\n"
"		}\n"
"		if (!Spectrum_IsBlack(eval)) {\n"
"			result += evalWeight * eval;\n"
"			*pdfW += evalWeight * pdfWMat;\n"
"		}\n"
"	}\n"
"\n"
"	return result;\n"
"}\n"
"\n"
"float3 MixMaterial_GetEmittedRadiance(__global Material *material, const float2 uv\n"
"		MATERIALS_PARAM_DECL\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
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
"			const float factor = Texture_GetGreyValue(&texs[m->mix.mixFactorTexIndex], uv\n"
"					IMAGEMAPS_PARAM);\n"
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
"			const float3 emitRad = Material_GetEmittedRadianceNoMix(m, texs, uv\n"
"				IMAGEMAPS_PARAM);\n"
"			if (!Spectrum_IsBlack(emitRad))\n"
"				result += totalWeight * emitRad;\n"
"		}\n"
"	}\n"
"	\n"
"	return result;\n"
"}\n"
"\n"
"float3 MixMaterial_GetPassThroughTransparency(__global Material *material,\n"
"		const float2 uv, const float3 fixedDir, const float passEvent\n"
"		MATERIALS_PARAM_DECL\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"	__global Material *currentMixMat = material;\n"
"	float passThroughEvent = passEvent;\n"
"//\n"
"//	for (;;) {\n"
"//		const float factor = Texture_GetGreyValue(&texs[currentMixMat->mix.mixFactorTexIndex], uv\n"
"//				IMAGEMAPS_PARAM);\n"
"//		const float weight2 = clamp(factor, 0.f, 1.f);\n"
"//		const float weight1 = 1.f - weight2;\n"
"//\n"
"//		const bool sampleMatA = (passThroughEvent < weight1);\n"
"//		passThroughEvent = sampleMatA ? (passThroughEvent / weight1) : (passThroughEvent - weight1) / weight2;\n"
"//		const uint matIndex = sampleMatA ? currentMixMat->mix.matAIndex : currentMixMat->mix.matBIndex;\n"
"//		__global Material *mat = &mats[matIndex];\n"
"//\n"
"//		if (mat->type == MIX) {\n"
"//			currentMixMat = mat;\n"
"//			continue;\n"
"//		} else\n"
"//			return Material_GetPassThroughTransparencyNoMix(mat, texs, uv, fixedDir, passThroughEvent\n"
"//					IMAGEMAPS_PARAM);\n"
"//	}\n"
"\n"
"	const float factor = Texture_GetGreyValue(&texs[material->mix.mixFactorTexIndex], uv\n"
"			IMAGEMAPS_PARAM);\n"
"	const float weight2 = clamp(factor, 0.f, 1.f);\n"
"	const float weight1 = 1.f - weight2;\n"
"\n"
"	if (passThroughEvent < weight1)\n"
"		return Material_GetPassThroughTransparencyNoMix(&mats[material->mix.matAIndex], texs, uv, fixedDir, passThroughEvent / weight1\n"
"					IMAGEMAPS_PARAM);\n"
"	else\n"
"		return Material_GetPassThroughTransparencyNoMix(&mats[material->mix.matBIndex], texs, uv, fixedDir, (passThroughEvent - weight2) / weight2\n"
"					IMAGEMAPS_PARAM);\n"
"}\n"
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
"		const float2 uv, const float3 lightDir, const float3 eyeDir,\n"
"		BSDFEvent *event, float *directPdfW\n"
"		MATERIALS_PARAM_DECL\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"#if defined (PARAM_ENABLE_MAT_MIX)\n"
"	if (material->type == MIX)\n"
"		return MixMaterial_Evaluate(material, uv, lightDir, eyeDir,\n"
"				event, directPdfW\n"
"				MATERIALS_PARAM\n"
"				IMAGEMAPS_PARAM);\n"
"	else\n"
"#endif\n"
"		return Material_EvaluateNoMix(material, texs, uv, lightDir, eyeDir,\n"
"				event, directPdfW\n"
"				IMAGEMAPS_PARAM);\n"
"}\n"
"\n"
"float3 Material_Sample(__global Material *material,	const float2 uv,\n"
"		const float3 fixedDir, float3 *sampledDir,\n"
"		const float u0, const float u1,\n"
"#if defined(PARAM_HAS_PASSTHROUGHT)\n"
"		const float passThroughEvent,\n"
"#endif\n"
"		float *pdfW, float *cosSampledDir, BSDFEvent *event\n"
"		MATERIALS_PARAM_DECL\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"#if defined (PARAM_ENABLE_MAT_MIX)\n"
"	if (material->type == MIX)\n"
"		return MixMaterial_Sample(material, uv,\n"
"				fixedDir, sampledDir,\n"
"				u0, u1,\n"
"				passThroughEvent,\n"
"				pdfW, cosSampledDir, event\n"
"				MATERIALS_PARAM\n"
"				IMAGEMAPS_PARAM);\n"
"	else\n"
"#endif\n"
"		return Material_SampleNoMix(material, texs, uv,\n"
"				fixedDir, sampledDir,\n"
"				u0, u1,\n"
"#if defined(PARAM_HAS_PASSTHROUGHT)\n"
"				passThroughEvent,\n"
"#endif\n"
"				pdfW, cosSampledDir, event\n"
"				IMAGEMAPS_PARAM);\n"
"}\n"
"\n"
"float3 Material_GetEmittedRadiance(__global Material *material, const float2 uv\n"
"		MATERIALS_PARAM_DECL\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"#if defined (PARAM_ENABLE_MAT_MIX)\n"
"	if (material->type == MIX)\n"
"		return MixMaterial_GetEmittedRadiance(material, uv\n"
"				MATERIALS_PARAM\n"
"				IMAGEMAPS_PARAM);\n"
"	else\n"
"#endif\n"
"		return Material_GetEmittedRadianceNoMix(material, texs, uv\n"
"				IMAGEMAPS_PARAM);\n"
"}\n"
"\n"
"#if defined(PARAM_HAS_PASSTHROUGHT)\n"
"float3 Material_GetPassThroughTransparency(__global Material *material,\n"
"		const float2 uv, const float3 fixedDir, const float passThroughEvent\n"
"		MATERIALS_PARAM_DECL\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"#if defined (PARAM_ENABLE_MAT_MIX)\n"
"	if (material->type == MIX)\n"
"		return MixMaterial_GetPassThroughTransparency(material,\n"
"				uv, fixedDir, passThroughEvent\n"
"				MATERIALS_PARAM\n"
"				IMAGEMAPS_PARAM);\n"
"	else\n"
"#endif\n"
"		return Material_GetPassThroughTransparencyNoMix(material, texs,\n"
"				uv, fixedDir, passThroughEvent\n"
"				IMAGEMAPS_PARAM);\n"
"}\n"
"#endif\n"
; } }
