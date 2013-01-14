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
"		//const bool fromL,\n"
"#if defined(PARAM_ACCEL_MQBVH)\n"
"		__global uint *meshFirstTriangleOffset,\n"
"		__global Mesh *meshDescs,\n"
"#endif\n"
"		__global uint *meshMats,\n"
"		__global uint *meshIDs,\n"
"#if (PARAM_DL_LIGHT_COUNT > 0)\n"
"		__global uint *meshLights,\n"
"#endif\n"
"		__global Point *vertices,\n"
"		__global Vector *vertNormals,\n"
"		__global UV *vertUVs,\n"
"		__global Triangle *triangles,\n"
"		__global Ray *ray,\n"
"		__global RayHit *rayHit\n"
"#if defined(PARAM_HAS_PASSTHROUGHT)\n"
"		, const float u0\n"
"#endif\n"
"#if defined(PARAM_HAS_BUMPMAPS) || defined(PARAM_HAS_NORMALMAPS)\n"
"		MATERIALS_PARAM_DECL\n"
"		IMAGEMAPS_PARAM_DECL\n"
"#endif\n"
"		) {\n"
"	//bsdf->fromLight = fromL;\n"
"#if defined(PARAM_HAS_PASSTHROUGHT)\n"
"	bsdf->passThroughEvent = u0;\n"
"#endif\n"
"\n"
"	const float3 rayOrig = vload3(0, &ray->o.x);\n"
"	const float3 rayDir = vload3(0, &ray->d.x);\n"
"	vstore3(rayOrig + rayHit->t * rayDir, 0, &bsdf->hitPoint.x);\n"
"	vstore3(-rayDir, 0, &bsdf->fixedDir.x);\n"
"\n"
"	const uint currentTriangleIndex = rayHit->index;\n"
"	const uint meshIndex = meshIDs[currentTriangleIndex];\n"
"\n"
"#if defined(PARAM_ACCEL_MQBVH)\n"
"	__global Mesh *meshDesc = &meshDescs[meshIndex];\n"
"	__global Point *iVertices = &vertices[meshDesc->vertsOffset];\n"
"	__global Vector *iVertNormals = &vertNormals[meshDesc->vertsOffset];\n"
"	__global UV *iVertUVs = &vertUVs[meshDesc->vertsOffset];\n"
"	__global Triangle *iTriangles = &triangles[meshDesc->trisOffset];\n"
"	const uint triangleID = currentTriangleIndex - meshFirstTriangleOffset[meshIndex];\n"
"#endif\n"
"\n"
"	// Get the material\n"
"	const uint matIndex = meshMats[meshIndex];\n"
"	bsdf->materialIndex = matIndex;\n"
"\n"
"	// Interpolate face normal and UV coordinates\n"
"	const float b1 = rayHit->b1;\n"
"	const float b2 = rayHit->b2;\n"
"#if defined(PARAM_ACCEL_MQBVH)\n"
"	vstore3(Mesh_GetGeometryNormal(iVertices, iTriangles, triangleID), 0, &bsdf->geometryN.x);\n"
"	float3 shadeN = Mesh_InterpolateNormal(iVertNormals, iTriangles, triangleID, b1, b2);\n"
"	shadeN = Transform_InvApplyVector(&meshDesc->trans, shadeN);\n"
"	const float2 hitPointUV = Mesh_InterpolateUV(iVertUVs, iTriangles, triangleID, b1, b2);\n"
"#else\n"
"	vstore3(Mesh_GetGeometryNormal(vertices, triangles, currentTriangleIndex), 0, &bsdf->geometryN.x);\n"
"	float3 shadeN = Mesh_InterpolateNormal(vertNormals, triangles, currentTriangleIndex, b1, b2);\n"
"	const float2 hitPointUV = Mesh_InterpolateUV(vertUVs, triangles, currentTriangleIndex, b1, b2);\n"
"#endif\n"
"	vstore2(hitPointUV, 0, &bsdf->hitPointUV.u);\n"
"\n"
"#if (PARAM_DL_LIGHT_COUNT > 0)\n"
"	// Check if it is a light source\n"
"	bsdf->triangleLightSourceIndex = meshLights[currentTriangleIndex];\n"
"#endif\n"
"\n"
"#if defined(PARAM_HAS_BUMPMAPS) || defined(PARAM_HAS_NORMALMAPS)\n"
"	__global Material *mat = &mats[matIndex];\n"
"\n"
"#if defined(PARAM_HAS_NORMALMAPS)\n"
"	// Check if I have to apply normal mapping\n"
"	const uint normalTexIndex = mat->normalTexIndex;\n"
"	if (normalTexIndex != NULL_INDEX) {\n"
"		// Apply normal mapping\n"
"		const float3 color = Texture_GetColorValue(&texs[normalTexIndex], hitPointUV\n"
"			IMAGEMAPS_PARAM);\n"
"		const float3 xyz = 2.f * color - 1.f;\n"
"\n"
"		float3 v1, v2;\n"
"		CoordinateSystem(shadeN, &v1, &v2);\n"
"		shadeN = normalize((float3)(\n"
"				v1.x * xyz.x + v2.x * xyz.y + shadeN.x * xyz.z,\n"
"				v1.y * xyz.x + v2.y * xyz.y + shadeN.y * xyz.z,\n"
"				v1.z * xyz.x + v2.z * xyz.y + shadeN.z * xyz.z));\n"
"	}\n"
"#endif\n"
"\n"
"#if defined(PARAM_HAS_BUMPMAPS)\n"
"	// Check if I have to apply bump mapping\n"
"	const uint bumpTexIndex = mat->bumpTexIndex;\n"
"	if (bumpTexIndex != NULL_INDEX) {\n"
"		// Apply bump mapping\n"
"		__global Texture *tex = &texs[bumpTexIndex];\n"
"		const float2 dudv = Texture_GetDuDv(tex);\n"
"\n"
"		const float b0 = Texture_GetGreyValue(tex, hitPointUV\n"
"			IMAGEMAPS_PARAM);\n"
"\n"
"		const float2 uvdu = (float2)(hitPointUV.s0 + dudv.s0, hitPointUV.s1);\n"
"		const float bu = Texture_GetGreyValue(tex, uvdu\n"
"			IMAGEMAPS_PARAM);\n"
"\n"
"		const float2 uvdv = (float2)(hitPointUV.s0, hitPointUV.s1 + dudv.s1);\n"
"		const float bv = Texture_GetGreyValue(tex, uvdv\n"
"			IMAGEMAPS_PARAM);\n"
"\n"
"		const float3 bump = (float3)(bu - b0, bv - b0, 1.f);\n"
"\n"
"		float3 v1, v2;\n"
"		CoordinateSystem(shadeN, &v1, &v2);\n"
"		shadeN = normalize((float3)(\n"
"				v1.x * bump.x + v2.x * bump.y + shadeN.x * bump.z,\n"
"				v1.y * bump.x + v2.y * bump.y + shadeN.y * bump.z,\n"
"				v1.z * bump.x + v2.z * bump.y + shadeN.z * bump.z));\n"
"	}\n"
"#endif\n"
"#endif\n"
"\n"
"	Frame_SetFromZ(&bsdf->frame, shadeN);\n"
"\n"
"	vstore3(shadeN, 0, &bsdf->shadeN.x);\n"
"}\n"
"\n"
"float3 BSDF_Evaluate(__global BSDF *bsdf,\n"
"		const float3 generatedDir, BSDFEvent *event, float *directPdfW\n"
"		MATERIALS_PARAM_DECL\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"	//const Vector &eyeDir = fromLight ? generatedDir : fixedDir;\n"
"	//const Vector &lightDir = fromLight ? fixedDir : generatedDir;\n"
"	//const float3 eyeDir = vload3(0, &bsdf->fixedDir.x);\n"
"	const float3 eyeDir = (float3)(bsdf->fixedDir.x, bsdf->fixedDir.y, bsdf->fixedDir.z);\n"
"	const float3 lightDir = generatedDir;\n"
"	const float3 geometryN = vload3(0, &bsdf->geometryN.x);\n"
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
"//	const float sideTest = dotEyeDirNG * dotLightDirNG;\n"
"//	const BSDFEvent matEvent = Material_GetEventTypes(mat\n"
"//			MATERIALS_PARAM);\n"
"//	if (((sideTest > 0.f) && !(matEvent & REFLECT)) ||\n"
"//			((sideTest < 0.f) && !(matEvent & TRANSMIT)))\n"
"\n"
"	if (dotEyeDirNG >= 0.f)\n"
"		return (float3)(dotEyeDirNG, 0.f ,0.f);\n"
"	else\n"
"		return (float3)(0.f, -dotEyeDirNG, 0.f);\n"
"\n"
"	__global Frame *frame = &bsdf->frame;\n"
"	const float3 localLightDir = Frame_ToLocal(frame, lightDir);\n"
"	const float3 localEyeDir = Frame_ToLocal(frame, eyeDir);\n"
"	const float3 result = Material_Evaluate(mat, vload2(0, &bsdf->hitPointUV.u),\n"
"			localLightDir, localEyeDir,	event, directPdfW\n"
"			MATERIALS_PARAM\n"
"			IMAGEMAPS_PARAM);\n"
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
"		MATERIALS_PARAM_DECL\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"	const float3 fixedDir = vload3(0, &bsdf->fixedDir.x);\n"
"	const float3 localFixedDir = Frame_ToLocal(&bsdf->frame, fixedDir);\n"
"	float3 localSampledDir;\n"
"\n"
"	const float3 result = Material_Sample(&mats[bsdf->materialIndex], vload2(0, &bsdf->hitPointUV.u),\n"
"			localFixedDir, &localSampledDir, u0, u1,\n"
"#if defined(PARAM_HAS_PASSTHROUGHT)\n"
"			bsdf->passThroughEvent,\n"
"#endif\n"
"			pdfW, cosSampledDir, event\n"
"			MATERIALS_PARAM\n"
"			IMAGEMAPS_PARAM);\n"
"	if (Spectrum_IsBlack(result))\n"
"		return 0.f;\n"
"\n"
"	*sampledDir = Frame_ToWorld(&bsdf->frame, localSampledDir);\n"
"\n"
"	// Adjoint BSDF\n"
"//	if (fromLight) {\n"
"//		const float absDotFixedDirNS = fabsf(localFixedDir.z);\n"
"//		const float absDotSampledDirNS = fabsf(localSampledDir.z);\n"
"//		const float absDotFixedDirNG = AbsDot(fixedDir, geometryN);\n"
"//		const float absDotSampledDirNG = AbsDot(*sampledDir, geometryN);\n"
"//		return result * ((absDotFixedDirNS * absDotSampledDirNG) / (absDotSampledDirNS * absDotFixedDirNG));\n"
"//	} else\n"
"		return result;\n"
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
"		__global Material *mats, __global Texture *texs,\n"
"		__global TriangleLight *triLightDefs, float *directPdfA\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"	const uint triangleLightSourceIndex = bsdf->triangleLightSourceIndex;\n"
"	if (triangleLightSourceIndex == NULL_INDEX)\n"
"		return BLACK;\n"
"	else\n"
"		return TriangleLight_GetRadiance(&triLightDefs[triangleLightSourceIndex], mats, texs,\n"
"				vload3(0, &bsdf->fixedDir.x), vload3(0, &bsdf->geometryN.x), vload2(0, &bsdf->hitPointUV.u), directPdfA\n"
"				IMAGEMAPS_PARAM);\n"
"}\n"
"#endif\n"
"\n"
"#if defined(PARAM_HAS_PASSTHROUGHT)\n"
"float3 BSDF_GetPassThroughTransparency(__global BSDF *bsdf\n"
"		MATERIALS_PARAM_DECL\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"	const float3 localFixedDir = Frame_ToLocal(&bsdf->frame, vload3(0, &bsdf->fixedDir.x));\n"
"\n"
"	return Material_GetPassThroughTransparency(&mats[bsdf->materialIndex],\n"
"			vload2(0, &bsdf->hitPointUV.u), localFixedDir, bsdf->passThroughEvent\n"
"			MATERIALS_PARAM\n"
"			IMAGEMAPS_PARAM);\n"
"}\n"
"#endif\n"
; } }
