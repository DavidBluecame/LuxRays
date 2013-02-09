#include <string>
namespace luxrays { namespace ocl {
std::string KernelSource_bsdf_funcs = 
"#line 2 \"bsdf_funcs.cl\"\n"
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
"void BSDF_Init(\n"
"		__global BSDF *bsdf,\n"
"		__global uint *meshMats,\n"
"		__global uint *meshIDs,\n"
"		__global Point *vertices,\n"
"		__global Vector *vertNormals,\n"
"		__global UV *vertUVs,\n"
"		__global Triangle *triangles,\n"
"		__global Ray *ray,\n"
"		__global RayHit *rayHit\n"
"		) {\n"
"	const float3 rayOrig = VLOAD3F(&ray->o.x);\n"
"	const float3 rayDir = VLOAD3F(&ray->d.x);\n"
"	VSTORE3F(rayOrig + rayHit->t * rayDir, &bsdf->hitPoint.x);\n"
"	VSTORE3F(-rayDir, &bsdf->fixedDir.x);\n"
"\n"
"	const uint currentTriangleIndex = rayHit->index;\n"
"	const uint meshIndex = meshIDs[currentTriangleIndex];\n"
"\n"
"	// Get the material\n"
"	const uint matIndex = meshMats[meshIndex];\n"
"	bsdf->materialIndex = matIndex;\n"
"\n"
"	// Interpolate face normal and UV coordinates\n"
"//	const float b1 = rayHit->b1;\n"
"//	const float b2 = rayHit->b2;\n"
"\n"
"	const float3 geometryN = Mesh_GetGeometryNormal(vertices, triangles, currentTriangleIndex);\n"
"	VSTORE3F(geometryN, &bsdf->geometryN.x);\n"
"//	float3 shadeN = Mesh_InterpolateNormal(vertNormals, triangles, currentTriangleIndex, b1, b2);\n"
"//	const float2 hitPointUV = Mesh_InterpolateUV(vertUVs, triangles, currentTriangleIndex, b1, b2);\n"
"//\n"
"//	VSTORE2F(hitPointUV, &bsdf->hitPointUV.u);\n"
"//\n"
"//	Frame_SetFromZ(&bsdf->frame, shadeN);\n"
"//\n"
"//	VSTORE3F(shadeN, &bsdf->shadeN.x);\n"
"}\n"
"\n"
"float3 BSDF_Evaluate(__global BSDF *bsdf,\n"
"		const float3 generatedDir, BSDFEvent *event, float *directPdfW\n"
"		MATERIALS_PARAM_DECL) {\n"
"	//const Vector &eyeDir = fromLight ? generatedDir : fixedDir;\n"
"	//const Vector &lightDir = fromLight ? fixedDir : generatedDir;\n"
"	const float3 eyeDir = VLOAD3F(&bsdf->fixedDir.x);\n"
"	const float3 lightDir = generatedDir;\n"
"	const float3 geometryN = VLOAD3F(&bsdf->geometryN.x);\n"
"\n"
"	const float dotLightDirNG = dot(lightDir, geometryN);\n"
"	const float absDotLightDirNG = fabs(dotLightDirNG);\n"
"	const float dotEyeDirNG = dot(eyeDir, geometryN);\n"
"	const float absDotEyeDirNG = fabs(dotEyeDirNG);\n"
"\n"
"	if ((absDotLightDirNG < DEFAULT_COS_EPSILON_STATIC) ||\n"
"			(absDotEyeDirNG < DEFAULT_COS_EPSILON_STATIC))\n"
"		return BLACK;\n"
"\n"
"	__global Material *mat = &mats[bsdf->materialIndex];\n"
"	const float sideTest = dotEyeDirNG * dotLightDirNG;\n"
"	const BSDFEvent matEvent = Material_GetEventTypes(mat\n"
"			MATERIALS_PARAM);\n"
"	if (((sideTest > 0.f) && !(matEvent & REFLECT)) ||\n"
"			((sideTest < 0.f) && !(matEvent & TRANSMIT)))\n"
"		return BLACK;\n"
"\n"
"	__global Frame *frame = &bsdf->frame;\n"
"	const float3 localLightDir = Frame_ToLocal(frame, lightDir);\n"
"	const float3 localEyeDir = Frame_ToLocal(frame, eyeDir);\n"
"	const float3 result = Material_Evaluate(mat, VLOAD2F(&bsdf->hitPointUV.u),\n"
"			localLightDir, localEyeDir,	event, directPdfW\n"
"			MATERIALS_PARAM);\n"
"\n"
"	// Adjoint BSDF\n"
"//	if (fromLight) {\n"
"//		const float absDotLightDirNS = AbsDot(lightDir, shadeN);\n"
"//		const float absDotEyeDirNS = AbsDot(eyeDir, shadeN);\n"
"//		return result * ((absDotLightDirNS * absDotEyeDirNG) / (absDotEyeDirNS * absDotLightDirNG));\n"
"//	} else\n"
"		return result;\n"
"}\n"
"\n"
"float3 BSDF_Sample(__global BSDF *bsdf, const float u0, const float u1,\n"
"		float3 *sampledDir, float *pdfW, float *cosSampledDir, BSDFEvent *event\n"
"		MATERIALS_PARAM_DECL) {\n"
"	const float3 fixedDir = VLOAD3F(&bsdf->fixedDir.x);\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Original code:\n"
"\n"
"	//const float3 localFixedDir = Frame_ToLocal(&bsdf->frame, fixedDir);\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Looking for a work around: \n"
"\n"
"//	const float Zx = bsdf->shadeN.x;\n"
"//	const float Zy = bsdf->shadeN.y;\n"
"//	const float Zz = bsdf->shadeN.z;\n"
"//\n"
"//	float Xx, Xy, Xz;\n"
"//	//if (fabs(Zx) > fabs(Zy)) {\n"
"//	if (Zx * Zx > Zy * Zy) {\n"
"//		//const float invLen = 1.f / sqrt(Z.x * Z.x + Z.z * Z.z);\n"
"//		const float len = sqrt(Zx * Zx + Zz * Zz);\n"
"//		Xx = -Zz / len;\n"
"//		Xy = 0.f;\n"
"//		Xz = Zx / len;\n"
"//	} else {\n"
"//		//const float invLen = 1.f / sqrt(Z.y * Z.y + Z.z * Z.z);\n"
"//		const float len = sqrt(Zy * Zy + Zz * Zz);\n"
"//		Xx = 0.f;\n"
"//		Xy = Zz / len;\n"
"//		Xz = -Zy / len;\n"
"//	}\n"
"//\n"
"//	float Yx, Yy, Yz;\n"
"//	Yx = (Zy * Xz) - (Zz * Xy);\n"
"//	Yy = (Zz * Xx) - (Zx * Xz);\n"
"//	Yz = (Zx * Xy) - (Zy * Xx);\n"
"//\n"
"//	float dotx = fixedDir.x * Xx + fixedDir.y * Xy + fixedDir.z * Xz;\n"
"//	float doty = fixedDir.x * Yx + fixedDir.y * Yy + fixedDir.z * Yz;\n"
"//	float dotz = fixedDir.x * Zx + fixedDir.y * Zy + fixedDir.z * Zz;\n"
"//	const float3 localFixedDir = (float3)(dotx, doty, dotz);\n"
"\n"
"	const float3 localFixedDir = fixedDir;\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	return (float3)(fabs(localFixedDir.x), fabs(localFixedDir.y), fabs(localFixedDir.z));\n"
"\n"
"//	float3 localSampledDir;\n"
"//\n"
"//	const float3 result = Material_Sample(&mats[bsdf->materialIndex], VLOAD2F(&bsdf->hitPointUV.u),\n"
"//			localFixedDir, &localSampledDir, u0, u1,\n"
"//#if defined(PARAM_HAS_PASSTHROUGH)\n"
"//			bsdf->passThroughEvent,\n"
"//#endif\n"
"//			pdfW, cosSampledDir, event\n"
"//			MATERIALS_PARAM);\n"
"//	if (Spectrum_IsBlack(result))\n"
"//		return 0.f;\n"
"//\n"
"//	//*sampledDir = Frame_ToWorld(&bsdf->frame, localSampledDir);\n"
"//\n"
"//	(*sampledDir).x = Xx * localSampledDir.x + Yx * localSampledDir.y + Zx * localSampledDir.z;\n"
"//	(*sampledDir).y = Xy * localSampledDir.x + Yy * localSampledDir.y + Zy * localSampledDir.z;\n"
"//	(*sampledDir).z = Xz * localSampledDir.x + Yz * localSampledDir.y + Zz * localSampledDir.z;\n"
"//\n"
"//	// Adjoint BSDF\n"
"////	if (fromLight) {\n"
"////		const float absDotFixedDirNS = fabsf(localFixedDir.z);\n"
"////		const float absDotSampledDirNS = fabsf(localSampledDir.z);\n"
"////		const float absDotFixedDirNG = AbsDot(fixedDir, geometryN);\n"
"////		const float absDotSampledDirNG = AbsDot(*sampledDir, geometryN);\n"
"////		return result * ((absDotFixedDirNS * absDotSampledDirNG) / (absDotSampledDirNS * absDotFixedDirNG));\n"
"////	} else\n"
"//		return result;\n"
"}\n"
"\n"
"bool BSDF_IsDelta(__global BSDF *bsdf\n"
"		MATERIALS_PARAM_DECL) {\n"
"	return Material_IsDelta(&mats[bsdf->materialIndex]\n"
"			MATERIALS_PARAM);\n"
"}\n"
"\n"
"#if (PARAM_DL_LIGHT_COUNT > 0)\n"
"float3 BSDF_GetEmittedRadiance(__global BSDF *bsdf,\n"
"		__global TriangleLight *triLightDefs, float *directPdfA\n"
"		MATERIALS_PARAM_DECL) {\n"
"	const uint triangleLightSourceIndex = bsdf->triangleLightSourceIndex;\n"
"	if (triangleLightSourceIndex == NULL_INDEX)\n"
"		return BLACK;\n"
"	else\n"
"		return TriangleLight_GetRadiance(&triLightDefs[triangleLightSourceIndex],\n"
"				VLOAD3F(&bsdf->fixedDir.x), VLOAD3F(&bsdf->geometryN.x), VLOAD2F(&bsdf->hitPointUV.u), directPdfA\n"
"				MATERIALS_PARAM);\n"
"}\n"
"#endif\n"
"\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"float3 BSDF_GetPassThroughTransparency(__global BSDF *bsdf\n"
"		MATERIALS_PARAM_DECL) {\n"
"	const float3 localFixedDir = Frame_ToLocal(&bsdf->frame, VLOAD3F(&bsdf->fixedDir.x));\n"
"\n"
"	return Material_GetPassThroughTransparency(&mats[bsdf->materialIndex],\n"
"			VLOAD2F(&bsdf->hitPointUV.u), localFixedDir, bsdf->passThroughEvent\n"
"			MATERIALS_PARAM);\n"
"}\n"
"#endif\n"
; } }
