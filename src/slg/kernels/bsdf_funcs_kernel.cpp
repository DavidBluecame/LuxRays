#include <string>
namespace slg { namespace ocl {
std::string KernelSource_bsdf_funcs = 
"#line 2 \"bsdf_funcs.cl\"\n"
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
"#if !defined(BSDF_INIT_PARAM_RAY_MEM_SPACE)\n"
"#define BSDF_INIT_PARAM_RAY_MEM_SPACE __global\n"
"#endif\n"
"#if !defined(BSDF_INIT_PARAM_RAYHIT_MEM_SPACE)\n"
"#define BSDF_INIT_PARAM_RAYHIT_MEM_SPACE __global\n"
"#endif\n"
"void BSDF_Init(\n"
"		__global BSDF *bsdf,\n"
"		//const bool fromL,\n"
"		__global Mesh *meshDescs,\n"
"		__global uint *meshMats,\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"		__global uint *meshTriLightDefsOffset,\n"
"#endif\n"
"		__global Point *vertices,\n"
"#if defined(PARAM_HAS_NORMALS_BUFFER)\n"
"		__global Vector *vertNormals,\n"
"#endif\n"
"#if defined(PARAM_HAS_UVS_BUFFER)\n"
"		__global UV *vertUVs,\n"
"#endif\n"
"#if defined(PARAM_HAS_COLS_BUFFER)\n"
"		__global Spectrum *vertCols,\n"
"#endif\n"
"#if defined(PARAM_HAS_ALPHAS_BUFFER)\n"
"		__global float *vertAlphas,\n"
"#endif\n"
"		__global Triangle *triangles,\n"
"#if !defined(BSDF_INIT_PARAM_MEM_SPACE_PRIVATE)\n"
"		__global\n"
"#endif\n"
"		Ray *ray,\n"
"#if !defined(BSDF_INIT_PARAM_MEM_SPACE_PRIVATE)\n"
"		__global\n"
"#endif\n"
"		RayHit *rayHit\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"		, const float u0\n"
"#endif\n"
"#if defined(PARAM_HAS_BUMPMAPS) || defined(PARAM_HAS_NORMALMAPS)\n"
"		MATERIALS_PARAM_DECL\n"
"#endif\n"
"		) {\n"
"	//bsdf->fromLight = fromL;\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"	bsdf->hitPoint.passThroughEvent = u0;\n"
"#endif\n"
"\n"
"#if defined(BSDF_INIT_PARAM_MEM_SPACE_PRIVATE)\n"
"	const float3 rayOrig = (float3)(ray->o.x, ray->o.y, ray->o.z);\n"
"	const float3 rayDir = (float3)(ray->d.x, ray->d.y, ray->d.z);\n"
"#else\n"
"	const float3 rayOrig = VLOAD3F(&ray->o.x);\n"
"	const float3 rayDir = VLOAD3F(&ray->d.x);\n"
"#endif\n"
"	const float3 hitPointP = rayOrig + rayHit->t * rayDir;\n"
"	VSTORE3F(hitPointP, &bsdf->hitPoint.p.x);\n"
"	VSTORE3F(-rayDir, &bsdf->hitPoint.fixedDir.x);\n"
"\n"
"	const uint meshIndex = rayHit->meshIndex;\n"
"	const uint triangleIndex = rayHit->triangleIndex;\n"
"\n"
"	__global Mesh *meshDesc = &meshDescs[meshIndex];\n"
"	__global Point *iVertices = &vertices[meshDesc->vertsOffset];\n"
"	__global Triangle *iTriangles = &triangles[meshDesc->trisOffset];\n"
"\n"
"	// Get the material\n"
"	const uint matIndex = meshMats[meshIndex];\n"
"	bsdf->materialIndex = matIndex;\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Get face normal\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	const float b1 = rayHit->b1;\n"
"	const float b2 = rayHit->b2;\n"
"\n"
"	// Geometry normal expressed in local coordinates\n"
"	float3 geometryN = Mesh_GetGeometryNormal(iVertices, iTriangles, triangleIndex);\n"
"	// Transform to global coordinates\n"
"	geometryN = normalize(Transform_InvApplyNormal(&meshDesc->trans, geometryN));\n"
"	// Store the geometry normal\n"
"	VSTORE3F(geometryN, &bsdf->hitPoint.geometryN.x);\n"
"\n"
"	// The shading normal\n"
"	float3 shadeN;\n"
"#if defined(PARAM_HAS_NORMALS_BUFFER)\n"
"	if (meshDesc->normalsOffset != NULL_INDEX) {\n"
"		__global Vector *iVertNormals = &vertNormals[meshDesc->normalsOffset];\n"
"		// Shading normal expressed in local coordinates\n"
"		shadeN = Mesh_InterpolateNormal(iVertNormals, iTriangles, triangleIndex, b1, b2);\n"
"		// Transform to global coordinates\n"
"		shadeN = normalize(Transform_InvApplyNormal(&meshDesc->trans, shadeN));\n"
"	} else\n"
"#endif\n"
"		shadeN = geometryN;\n"
"	// Shade normal can not yet be stored because of normal and bump mapping\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Get UV coordinate\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	float2 hitPointUV;\n"
"#if defined(PARAM_HAS_UVS_BUFFER)\n"
"	if (meshDesc->uvsOffset != NULL_INDEX) {\n"
"		__global UV *iVertUVs = &vertUVs[meshDesc->uvsOffset];\n"
"		hitPointUV = Mesh_InterpolateUV(iVertUVs, iTriangles, triangleIndex, b1, b2);\n"
"	} else\n"
"#endif\n"
"		hitPointUV = 0.f;\n"
"	VSTORE2F(hitPointUV, &bsdf->hitPoint.uv.u);\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Get color value\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_ENABLE_TEX_HITPOINTCOLOR) || defined(PARAM_ENABLE_TEX_HITPOINTGREY)\n"
"	float3 hitPointColor;\n"
"#if defined(PARAM_HAS_COLS_BUFFER)\n"
"	if (meshDesc->colsOffset != NULL_INDEX) {\n"
"		__global Spectrum *iVertCols = &vertCols[meshDesc->colsOffset];\n"
"		hitPointColor = Mesh_InterpolateColor(iVertCols, iTriangles, triangleIndex, b1, b2);\n"
"	} else\n"
"#endif\n"
"		hitPointColor = WHITE;\n"
"	VSTORE3F(hitPointColor, bsdf->hitPoint.color.c);\n"
"#endif\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Get alpha value\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_ENABLE_TEX_HITPOINTALPHA)\n"
"	float hitPointAlpha;\n"
"#if defined(PARAM_HAS_ALPHAS_BUFFER)\n"
"	if (meshDesc->colsOffset != NULL_INDEX) {\n"
"		__global float *iVertAlphas = &vertAlphas[meshDesc->alphasOffset];\n"
"		hitPointAlpha = Mesh_InterpolateAlpha(iVertAlphas, iTriangles, triangleIndex, b1, b2);\n"
"	} else\n"
"#endif\n"
"		hitPointAlpha = 1.f;\n"
"	bsdf->hitPoint.alpha = hitPointAlpha;\n"
"#endif\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"	// Check if it is a light source\n"
"	bsdf->triangleLightSourceIndex = meshTriLightDefsOffset[meshIndex];\n"
"#endif\n"
"\n"
"#if defined(PARAM_HAS_BUMPMAPS) || defined(PARAM_HAS_NORMALMAPS)\n"
"	__global Material *mat = &mats[matIndex];\n"
"\n"
"#if defined(PARAM_HAS_NORMALMAPS)\n"
"	//--------------------------------------------------------------------------\n"
"	// Check if I have to apply normal mapping\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	const float3 normalColor = Material_GetNormalTexValue(mat, &bsdf->hitPoint\n"
"		MATERIALS_PARAM);\n"
"\n"
"	if (!Spectrum_IsBlack(normalColor)) {\n"
"		const float3 xyz = 2.f * normalColor - 1.f;\n"
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
"	//--------------------------------------------------------------------------\n"
"	// Check if I have to apply bump mapping\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	const float2 bumpUV = Material_GetBumpTexValue(mat, &bsdf->hitPoint\n"
"		MATERIALS_PARAM);\n"
"\n"
"	if ((bumpUV.s0 != 0.f) || (bumpUV.s1 != 0.f)) {\n"
"		float3 v1, v2;\n"
"		CoordinateSystem(shadeN, &v1, &v2);\n"
"		shadeN = normalize((float3)(\n"
"				v1.x * bumpUV.s0 + v2.x * bumpUV.s1 + shadeN.x,\n"
"				v1.y * bumpUV.s0 + v2.y * bumpUV.s1 + shadeN.y,\n"
"				v1.z * bumpUV.s0 + v2.z * bumpUV.s1 + shadeN.z));\n"
"	}\n"
"#endif\n"
"#endif\n"
"\n"
"	Frame_SetFromZ(&bsdf->frame, shadeN);\n"
"\n"
"	VSTORE3F(shadeN, &bsdf->hitPoint.shadeN.x);\n"
"}\n"
"\n"
"float3 BSDF_Evaluate(__global BSDF *bsdf,\n"
"		const float3 generatedDir, BSDFEvent *event, float *directPdfW\n"
"		MATERIALS_PARAM_DECL) {\n"
"	//const Vector &eyeDir = fromLight ? generatedDir : hitPoint.fixedDir;\n"
"	//const Vector &lightDir = fromLight ? hitPoint.fixedDir : generatedDir;\n"
"	const float3 eyeDir = VLOAD3F(&bsdf->hitPoint.fixedDir.x);\n"
"	const float3 lightDir = generatedDir;\n"
"	const float3 geometryN = VLOAD3F(&bsdf->hitPoint.geometryN.x);\n"
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
"	const float3 result = Material_Evaluate(mat, &bsdf->hitPoint,\n"
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
"		float3 *sampledDir, float *pdfW, float *cosSampledDir, BSDFEvent *event,\n"
"		const BSDFEvent requestedEvent\n"
"		MATERIALS_PARAM_DECL) {\n"
"	const float3 fixedDir = VLOAD3F(&bsdf->hitPoint.fixedDir.x);\n"
"	const float3 localFixedDir = Frame_ToLocal(&bsdf->frame, fixedDir);\n"
"	float3 localSampledDir;\n"
"\n"
"	const float3 result = Material_Sample(&mats[bsdf->materialIndex], &bsdf->hitPoint,\n"
"			localFixedDir, &localSampledDir, u0, u1,\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"			bsdf->hitPoint.passThroughEvent,\n"
"#endif\n"
"			pdfW, cosSampledDir, event,\n"
"			requestedEvent\n"
"			MATERIALS_PARAM);\n"
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
"BSDFEvent BSDF_GetEventTypes(__global BSDF *bsdf\n"
"		MATERIALS_PARAM_DECL) {\n"
"	return Material_GetEventTypes(&mats[bsdf->materialIndex]\n"
"			MATERIALS_PARAM);\n"
"}\n"
"\n"
"bool BSDF_IsDelta(__global BSDF *bsdf\n"
"		MATERIALS_PARAM_DECL) {\n"
"	return Material_IsDelta(&mats[bsdf->materialIndex]\n"
"			MATERIALS_PARAM);\n"
"}\n"
"\n"
"uint BSDF_GetMaterialID(__global BSDF *bsdf\n"
"		MATERIALS_PARAM_DECL) {\n"
"	return mats[bsdf->materialIndex].matID;\n"
"}\n"
"\n"
"uint BSDF_GetLightID(__global BSDF *bsdf\n"
"		MATERIALS_PARAM_DECL) {\n"
"	return mats[bsdf->materialIndex].lightID;\n"
"}\n"
"\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"bool BSDF_IsLightSource(__global BSDF *bsdf) {\n"
"	return (bsdf->triangleLightSourceIndex != NULL_INDEX);\n"
"}\n"
"\n"
"float3 BSDF_GetEmittedRadiance(__global BSDF *bsdf, float *directPdfA\n"
"		LIGHTS_PARAM_DECL) {\n"
"	const uint triangleLightSourceIndex = bsdf->triangleLightSourceIndex;\n"
"	if (triangleLightSourceIndex == NULL_INDEX)\n"
"		return BLACK;\n"
"	else\n"
"		return IntersecableLight_GetRadiance(&lights[triangleLightSourceIndex],\n"
"				&bsdf->hitPoint, directPdfA\n"
"				LIGHTS_PARAM);\n"
"}\n"
"#endif\n"
"\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"float3 BSDF_GetPassThroughTransparency(__global BSDF *bsdf\n"
"		MATERIALS_PARAM_DECL) {\n"
"	const float3 localFixedDir = Frame_ToLocal(&bsdf->frame, VLOAD3F(&bsdf->hitPoint.fixedDir.x));\n"
"\n"
"	return Material_GetPassThroughTransparency(&mats[bsdf->materialIndex],\n"
"			&bsdf->hitPoint, localFixedDir, bsdf->hitPoint.passThroughEvent\n"
"			MATERIALS_PARAM);\n"
"}\n"
"#endif\n"
; } }
