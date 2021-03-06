#include <string>
namespace slg { namespace ocl {
std::string KernelSource_bsdf_funcs = 
"#line 2 \"bsdf_funcs.cl\"\n"
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
"// TODO: move in a separate extmesh_funcs.h file\n"
"\n"
"void ExtMesh_GetDifferentials(\n"
"		__global const Mesh *meshDescs,\n"
"		__global const Point *vertices,\n"
"#if defined(PARAM_HAS_NORMALS_BUFFER)\n"
"		__global const Vector *vertNormals,\n"
"#endif\n"
"#if defined(PARAM_HAS_UVS_BUFFER)\n"
"		__global const UV *vertUVs,\n"
"#endif\n"
"		__global const Triangle *triangles,\n"
"		const uint meshIndex,\n"
"		const uint triangleIndex,\n"
"		float3 *dpdu, float3 *dpdv,\n"
"        float3 *dndu, float3 *dndv) {\n"
"	__global const Mesh *meshDesc = &meshDescs[meshIndex];\n"
"	__global const Point *iVertices = &vertices[meshDesc->vertsOffset];\n"
"	__global const Triangle *iTriangles = &triangles[meshDesc->trisOffset];\n"
"\n"
"	// Compute triangle partial derivatives\n"
"	__global const Triangle *tri = &iTriangles[triangleIndex];\n"
"	const uint vi0 = tri->v[0];\n"
"	const uint vi1 = tri->v[1];\n"
"	const uint vi2 = tri->v[2];\n"
"\n"
"	float2 uv0, uv1, uv2;\n"
"#if defined(PARAM_HAS_UVS_BUFFER)\n"
"	if (meshDesc->uvsOffset != NULL_INDEX) {\n"
"		// Ok, UV coordinates are available, use them to build the reference\n"
"		// system around the shading normal.\n"
"\n"
"		__global const UV *iVertUVs = &vertUVs[meshDesc->uvsOffset];\n"
"		uv0 = VLOAD2F(&iVertUVs[vi0].u);\n"
"		uv1 = VLOAD2F(&iVertUVs[vi1].u);\n"
"		uv2 = VLOAD2F(&iVertUVs[vi2].u);\n"
"	} else {\n"
"#endif\n"
"		uv0 = (float2)(.5f, .5f);\n"
"		uv1 = (float2)(.5f, .5f);\n"
"		uv2 = (float2)(.5f, .5f);\n"
"#if defined(PARAM_HAS_UVS_BUFFER)\n"
"	}\n"
"#endif\n"
"\n"
"	// Compute deltas for triangle partial derivatives\n"
"	const float du1 = uv0.s0 - uv2.s0;\n"
"	const float du2 = uv1.s0 - uv2.s0;\n"
"	const float dv1 = uv0.s1 - uv2.s1;\n"
"	const float dv2 = uv1.s1 - uv2.s1;\n"
"	const float determinant = du1 * dv2 - dv1 * du2;\n"
"\n"
"	const float3 p0 = VLOAD3F(&iVertices[vi0].x);\n"
"	const float3 p1 = VLOAD3F(&iVertices[vi1].x);\n"
"	const float3 p2 = VLOAD3F(&iVertices[vi2].x);\n"
"	const float3 dp1 = p0 - p2;\n"
"	const float3 dp2 = p1 - p2;\n"
"\n"
"	if (determinant == 0.f) {\n"
"		// Handle 0 determinant for triangle partial derivative matrix\n"
"		CoordinateSystem(normalize(cross(dp1, dp2)), dpdu, dpdv);\n"
"		*dndu = ZERO;\n"
"		*dndv = ZERO;\n"
"	} else {\n"
"		const float invdet = 1.f / determinant;\n"
"\n"
"		//------------------------------------------------------------------\n"
"		// Compute dpdu and dpdv\n"
"		//------------------------------------------------------------------\n"
"\n"
"		*dpdu = ( dv2 * dp1 - dv1 * dp2) * invdet;\n"
"		*dpdv = (-du2 * dp1 + du1 * dp2) * invdet;\n"
"		// Transform to global coordinates\n"
"		*dpdu = normalize(Transform_InvApplyNormal(&meshDesc->trans, *dpdu));\n"
"		*dpdv = normalize(Transform_InvApplyNormal(&meshDesc->trans, *dpdv));\n"
"\n"
"		//------------------------------------------------------------------\n"
"		// Compute dndu and dndv\n"
"		//------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_HAS_NORMALS_BUFFER)\n"
"		if (meshDesc->normalsOffset != NULL_INDEX) {\n"
"			__global const Vector *iVertNormals = &vertNormals[meshDesc->normalsOffset];\n"
"			// Shading normals expressed in local coordinates\n"
"			const float3 n0 = VLOAD3F(&iVertNormals[tri->v[0]].x);\n"
"			const float3 n1 = VLOAD3F(&iVertNormals[tri->v[1]].x);\n"
"			const float3 n2 = VLOAD3F(&iVertNormals[tri->v[2]].x);\n"
"			const float3 dn1 = n0 - n2;\n"
"			const float3 dn2 = n1 - n2;\n"
"\n"
"			*dndu = ( dv2 * dn1 - dv1 * dn2) * invdet;\n"
"			*dndv = (-du2 * dn1 + du1 * dn2) * invdet;\n"
"			// Transform to global coordinates\n"
"			*dndu = normalize(Transform_InvApplyNormal(&meshDesc->trans, *dndu));\n"
"			*dndv = normalize(Transform_InvApplyNormal(&meshDesc->trans, *dndv));\n"
"		} else {\n"
"#endif\n"
"			*dndu = ZERO;\n"
"			*dndv = ZERO;\n"
"#if defined(PARAM_HAS_NORMALS_BUFFER)\n"
"		}\n"
"#endif\n"
"	}\n"
"}\n"
"\n"
"// Used when hitting a surface\n"
"void BSDF_Init(\n"
"		__global BSDF *bsdf,\n"
"		//const bool fromL,\n"
"		__global const Mesh *meshDescs,\n"
"		__global const uint *meshMats,\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"		__global const uint *meshTriLightDefsOffset,\n"
"#endif\n"
"		__global const Point *vertices,\n"
"#if defined(PARAM_HAS_NORMALS_BUFFER)\n"
"		__global const Vector *vertNormals,\n"
"#endif\n"
"#if defined(PARAM_HAS_UVS_BUFFER)\n"
"		__global const UV *vertUVs,\n"
"#endif\n"
"#if defined(PARAM_HAS_COLS_BUFFER)\n"
"		__global const Spectrum *vertCols,\n"
"#endif\n"
"#if defined(PARAM_HAS_ALPHAS_BUFFER)\n"
"		__global const float *vertAlphas,\n"
"#endif\n"
"		__global const Triangle *triangles,\n"
"#if !defined(RENDER_ENGINE_BIASPATHOCL) && !defined(RENDER_ENGINE_RTBIASPATHOCL)\n"
"		__global\n"
"#endif\n"
"		Ray *ray,\n"
"#if !defined(RENDER_ENGINE_BIASPATHOCL) && !defined(RENDER_ENGINE_RTBIASPATHOCL)\n"
"		__global\n"
"#endif\n"
"		const RayHit *rayHit\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"		, const float u0\n"
"#endif\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"		, __global PathVolumeInfo *volInfo\n"
"#endif\n"
"		MATERIALS_PARAM_DECL\n"
"		) {\n"
"	//bsdf->fromLight = fromL;\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"	bsdf->hitPoint.passThroughEvent = u0;\n"
"#endif\n"
"\n"
"#if !defined(RENDER_ENGINE_BIASPATHOCL) && !defined(RENDER_ENGINE_RTBIASPATHOCL)\n"
"	const float3 rayOrig = VLOAD3F(&ray->o.x);\n"
"	const float3 rayDir = VLOAD3F(&ray->d.x);\n"
"#else\n"
"	const float3 rayOrig = (float3)(ray->o.x, ray->o.y, ray->o.z);\n"
"	const float3 rayDir = (float3)(ray->d.x, ray->d.y, ray->d.z);\n"
"#endif\n"
"	const float3 hitPointP = rayOrig + rayHit->t * rayDir;\n"
"	VSTORE3F(hitPointP, &bsdf->hitPoint.p.x);\n"
"	VSTORE3F(-rayDir, &bsdf->hitPoint.fixedDir.x);\n"
"\n"
"	const uint meshIndex = rayHit->meshIndex;\n"
"	const uint triangleIndex = rayHit->triangleIndex;\n"
"\n"
"	__global const Mesh *meshDesc = &meshDescs[meshIndex];\n"
"	__global const Point *iVertices = &vertices[meshDesc->vertsOffset];\n"
"	__global const Triangle *iTriangles = &triangles[meshDesc->trisOffset];\n"
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
"		__global const Vector *iVertNormals = &vertNormals[meshDesc->normalsOffset];\n"
"		// Shading normal expressed in local coordinates\n"
"		shadeN = Mesh_InterpolateNormal(iVertNormals, iTriangles, triangleIndex, b1, b2);\n"
"		// Transform to global coordinates\n"
"		shadeN = normalize(Transform_InvApplyNormal(&meshDesc->trans, shadeN));\n"
"	} else\n"
"#endif\n"
"		shadeN = geometryN;\n"
"    VSTORE3F(shadeN, &bsdf->hitPoint.shadeN.x);\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Set interior and exterior volumes\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"	bsdf->hitPoint.intoObject = (dot(rayDir, geometryN) < 0.f);\n"
"\n"
"	PathVolumeInfo_SetHitPointVolumes(\n"
"			volInfo,\n"
"			&bsdf->hitPoint,\n"
"			Material_GetInteriorVolume(matIndex, &bsdf->hitPoint\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"				, u0\n"
"#endif\n"
"			MATERIALS_PARAM),\n"
"			Material_GetExteriorVolume(matIndex, &bsdf->hitPoint\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"				, u0\n"
"#endif\n"
"			MATERIALS_PARAM)\n"
"			MATERIALS_PARAM);\n"
"#endif\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Get UV coordinate\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	float2 hitPointUV;\n"
"#if defined(PARAM_HAS_UVS_BUFFER)\n"
"	if (meshDesc->uvsOffset != NULL_INDEX) {\n"
"		__global const UV *iVertUVs = &vertUVs[meshDesc->uvsOffset];\n"
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
"#if defined(PARAM_ENABLE_TEX_HITPOINTCOLOR) || defined(PARAM_ENABLE_TEX_HITPOINTGREY) || defined(PARAM_TRIANGLE_LIGHT_HAS_VERTEX_COLOR)\n"
"	float3 hitPointColor;\n"
"#if defined(PARAM_HAS_COLS_BUFFER)\n"
"	if (meshDesc->colsOffset != NULL_INDEX) {\n"
"		__global const Spectrum *iVertCols = &vertCols[meshDesc->colsOffset];\n"
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
"	if (meshDesc->alphasOffset != NULL_INDEX) {\n"
"		__global const float *iVertAlphas = &vertAlphas[meshDesc->alphasOffset];\n"
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
"    //--------------------------------------------------------------------------\n"
"	// Build the local reference system\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	float3 geometryDndu, geometryDndv, geometryDpdu, geometryDpdv;\n"
"	ExtMesh_GetDifferentials(\n"
"			meshDescs,\n"
"			vertices,\n"
"#if defined(PARAM_HAS_NORMALS_BUFFER)\n"
"			vertNormals,\n"
"#endif\n"
"#if defined(PARAM_HAS_UVS_BUFFER)\n"
"			vertUVs,\n"
"#endif\n"
"			triangles,\n"
"			meshIndex,\n"
"			triangleIndex,\n"
"			&geometryDpdu, &geometryDpdv,\n"
"			&geometryDndu, &geometryDndv);\n"
"	\n"
"	// Initialize shading differentials\n"
"	float3 shadeDpdv = normalize(cross(shadeN, geometryDpdu));\n"
"	float3 shadeDpdu = cross(shadeDpdv, shadeN);\n"
"	shadeDpdv *= (dot(geometryDpdv, shadeDpdv) > 0.f) ? 1.f : -1.f;\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Apply bump or normal mapping\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_HAS_BUMPMAPS)\n"
"	VSTORE3F(shadeDpdu, &bsdf->hitPoint.dpdu.x);\n"
"	VSTORE3F(shadeDpdv, &bsdf->hitPoint.dpdv.x);\n"
"	VSTORE3F(geometryDndu, &bsdf->hitPoint.dndu.x);\n"
"	VSTORE3F(geometryDndv, &bsdf->hitPoint.dndv.x);\n"
"	Material_Bump(matIndex,\n"
"			&bsdf->hitPoint, 1.f\n"
"			MATERIALS_PARAM);\n"
"	// Re-read the shadeN modified by Material_Bump()\n"
"	shadeN = VLOAD3F(&bsdf->hitPoint.shadeN.x);\n"
"	shadeDpdu = VLOAD3F(&bsdf->hitPoint.dpdu.x);\n"
"	shadeDpdv = VLOAD3F(&bsdf->hitPoint.dpdv.x);\n"
"#endif\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Build the local reference system\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	ExtMesh_GetFrame(shadeN, shadeDpdu, shadeDpdv, &bsdf->frame);\n"
"\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"	bsdf->isVolume = false;\n"
"#endif\n"
"}\n"
"\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"// Used when hitting a volume scatter point\n"
"void BSDF_InitVolume(\n"
"		__global BSDF *bsdf,\n"
"		__global const Material *mats,\n"
"#if !defined(RENDER_ENGINE_BIASPATHOCL) && !defined(RENDER_ENGINE_RTBIASPATHOCL)\n"
"		__global\n"
"#endif\n"
"		Ray *ray,\n"
"		const uint volumeIndex, const float t, const float passThroughEvent) {\n"
"#if !defined(RENDER_ENGINE_BIASPATHOCL) && !defined(RENDER_ENGINE_RTBIASPATHOCL)\n"
"	const float3 rayOrig = VLOAD3F(&ray->o.x);\n"
"	const float3 rayDir = VLOAD3F(&ray->d.x);\n"
"#else\n"
"	const float3 rayOrig = (float3)(ray->o.x, ray->o.y, ray->o.z);\n"
"	const float3 rayDir = (float3)(ray->d.x, ray->d.y, ray->d.z);\n"
"#endif\n"
"	const float3 hitPointP = rayOrig + t * rayDir;\n"
"	VSTORE3F(hitPointP, &bsdf->hitPoint.p.x);\n"
"	const float3 shadeN = -rayDir;\n"
"	VSTORE3F(shadeN, &bsdf->hitPoint.fixedDir.x);\n"
"\n"
"	bsdf->hitPoint.passThroughEvent = passThroughEvent;\n"
"\n"
"	bsdf->materialIndex = volumeIndex;\n"
"\n"
"	VSTORE3F(shadeN, &bsdf->hitPoint.geometryN.x);\n"
"	VSTORE3F(shadeN, &bsdf->hitPoint.shadeN.x);\n"
"#if defined(PARAM_HAS_BUMPMAPS)\n"
"	float3 dpdu, dpdv;\n"
"	CoordinateSystem(shadeN, &dpdu, &dpdv);\n"
"	VSTORE3F(dpdu, &bsdf->hitPoint.dpdu.x);\n"
"	VSTORE3F(dpdv, &bsdf->hitPoint.dpdv.x);\n"
"	VSTORE3F((float3)(0.f, 0.f, 0.f), &bsdf->hitPoint.dndu.x);\n"
"	VSTORE3F((float3)(0.f, 0.f, 0.f), &bsdf->hitPoint.dndv.x);\n"
"#endif\n"
"\n"
"	bsdf->hitPoint.intoObject = true;\n"
"	bsdf->hitPoint.interiorVolumeIndex = volumeIndex;\n"
"	bsdf->hitPoint.exteriorVolumeIndex = volumeIndex;\n"
"\n"
"	const uint iorTexIndex = (volumeIndex != NULL_INDEX) ? mats[volumeIndex].volume.iorTexIndex : NULL_INDEX;\n"
"	bsdf->hitPoint.interiorIorTexIndex = iorTexIndex;\n"
"	bsdf->hitPoint.exteriorIorTexIndex = iorTexIndex;\n"
"\n"
"#if defined(PARAM_ENABLE_TEX_HITPOINTCOLOR) || defined(PARAM_ENABLE_TEX_HITPOINTGREY) || defined(PARAM_TRIANGLE_LIGHT_HAS_VERTEX_COLOR)\n"
"	VSTORE3F(WHITE, bsdf->hitPoint.color.c);\n"
"#endif\n"
"#if defined(PARAM_ENABLE_TEX_HITPOINTALPHA)\n"
"	bsdf->hitPoint.alpha = 1.f;\n"
"#endif\n"
"\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"	bsdf->triangleLightSourceIndex = NULL_INDEX;\n"
"#endif\n"
"\n"
"	VSTORE2F((float2)(0.f, 0.f), &bsdf->hitPoint.uv.u);\n"
"\n"
"	bsdf->isVolume = true;\n"
"\n"
"	// Build the local reference system\n"
"	Frame_SetFromZ(&bsdf->frame, shadeN);\n"
"}\n"
"#endif\n"
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
"#if defined(PARAM_HAS_VOLUMES)\n"
"	if (!bsdf->isVolume) {\n"
"		// These kind of tests make sense only for materials\n"
"#endif\n"
"		if ((absDotLightDirNG < DEFAULT_COS_EPSILON_STATIC) ||\n"
"				(absDotEyeDirNG < DEFAULT_COS_EPSILON_STATIC))\n"
"			return BLACK;\n"
"\n"
"		const float sideTest = dotEyeDirNG * dotLightDirNG;\n"
"		const BSDFEvent matEvent = Material_GetEventTypes(bsdf->materialIndex\n"
"				MATERIALS_PARAM);\n"
"		if (((sideTest > 0.f) && !(matEvent & REFLECT)) ||\n"
"				((sideTest < 0.f) && !(matEvent & TRANSMIT)))\n"
"			return BLACK;\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"	}\n"
"#endif\n"
"\n"
"	__global Frame *frame = &bsdf->frame;\n"
"	const float3 localLightDir = Frame_ToLocal(frame, lightDir);\n"
"	const float3 localEyeDir = Frame_ToLocal(frame, eyeDir);\n"
"	const float3 result = Material_Evaluate(bsdf->materialIndex, &bsdf->hitPoint,\n"
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
"	const float3 result = Material_Sample(bsdf->materialIndex, &bsdf->hitPoint,\n"
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
"		return IntersectableLight_GetRadiance(&lights[triangleLightSourceIndex],\n"
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
"	return Material_GetPassThroughTransparency(bsdf->materialIndex,\n"
"			&bsdf->hitPoint, localFixedDir, bsdf->hitPoint.passThroughEvent\n"
"			MATERIALS_PARAM);\n"
"}\n"
"#endif\n"
; } }
