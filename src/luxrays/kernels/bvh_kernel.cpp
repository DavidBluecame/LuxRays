#include <string>
namespace luxrays { namespace ocl {
std::string KernelSource_bvh = 
"#line 2 \"bvh_kernel.cl\"\n"
"\n"
"/***************************************************************************\n"
" *   Copyright (C) 1998-2013 by authors (see AUTHORS.txt)                  *\n"
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
"typedef struct {\n"
"	BBox bbox;\n"
"	unsigned int primitive;\n"
"	unsigned int skipIndex;\n"
"} BVHAccelArrayNode;\n"
"\n"
"void TriangleIntersect(\n"
"		const float4 rayOrig,\n"
"		const float4 rayDir,\n"
"		const float minT,\n"
"		float *maxT,\n"
"		unsigned int *hitIndex,\n"
"		float *hitB1,\n"
"		float *hitB2,\n"
"		const unsigned int currentIndex,\n"
"		__global Point *verts,\n"
"		__global Triangle *tris) {\n"
"\n"
"	// Load triangle vertices\n"
"	__global Point *p0 = &verts[tris[currentIndex].v[0]];\n"
"	__global Point *p1 = &verts[tris[currentIndex].v[1]];\n"
"	__global Point *p2 = &verts[tris[currentIndex].v[2]];\n"
"\n"
"	float4 v0 = (float4) (p0->x, p0->y, p0->z, 0.f);\n"
"	float4 v1 = (float4) (p1->x, p1->y, p1->z, 0.f);\n"
"	float4 v2 = (float4) (p2->x, p2->y, p2->z, 0.f);\n"
"\n"
"	// Calculate intersection\n"
"	float4 e1 = v1 - v0;\n"
"	float4 e2 = v2 - v0;\n"
"	float4 s1 = cross(rayDir, e2);\n"
"\n"
"	const float divisor = dot(s1, e1);\n"
"	if (divisor == 0.f)\n"
"		return;\n"
"\n"
"	const float invDivisor = 1.f / divisor;\n"
"\n"
"	// Compute first barycentric coordinate\n"
"	const float4 d = rayOrig - v0;\n"
"	const float b1 = dot(d, s1) * invDivisor;\n"
"	if (b1 < 0.f)\n"
"		return;\n"
"\n"
"	// Compute second barycentric coordinate\n"
"	const float4 s2 = cross(d, e1);\n"
"	const float b2 = dot(rayDir, s2) * invDivisor;\n"
"	if (b2 < 0.f)\n"
"		return;\n"
"\n"
"	const float b0 = 1.f - b1 - b2;\n"
"	if (b0 < 0.f)\n"
"		return;\n"
"\n"
"	// Compute _t_ to intersection point\n"
"	const float t = dot(e2, s2) * invDivisor;\n"
"	if (t < minT || t > *maxT)\n"
"		return;\n"
"\n"
"	*maxT = t;\n"
"	*hitB1 = b1;\n"
"	*hitB2 = b2;\n"
"	*hitIndex = currentIndex;\n"
"}\n"
"\n"
"int BBoxIntersectP(\n"
"		const float4 rayOrig, const float4 invRayDir,\n"
"		const float mint, const float maxt,\n"
"		const float4 pMin, const float4 pMax) {\n"
"	const float4 l1 = (pMin - rayOrig) * invRayDir;\n"
"	const float4 l2 = (pMax - rayOrig) * invRayDir;\n"
"	const float4 tNear = fmin(l1, l2);\n"
"	const float4 tFar = fmax(l1, l2);\n"
"\n"
"	float t0 = max(max(max(tNear.x, tNear.y), max(tNear.x, tNear.z)), mint);\n"
"    float t1 = min(min(min(tFar.x, tFar.y), min(tFar.x, tFar.z)), maxt);\n"
"\n"
"	return (t1 > t0);\n"
"}\n"
"\n"
"__kernel void Intersect(\n"
"		__global Ray *rays,\n"
"		__global RayHit *rayHits,\n"
"		__global Point *verts,\n"
"		__global Triangle *tris,\n"
"		const unsigned int triangleCount,\n"
"		const unsigned int nodeCount,\n"
"		__global BVHAccelArrayNode *bvhTree,\n"
"		const unsigned int rayCount) {\n"
"	// Select the ray to check\n"
"	const int gid = get_global_id(0);\n"
"	if (gid >= rayCount)\n"
"		return;\n"
"\n"
"	float4 rayOrig,rayDir;\n"
"	float minT, maxT;\n"
"	{\n"
"		__global float4 *basePtr =(__global float4 *)&rays[gid];\n"
"		float4 data0 = (*basePtr++);\n"
"		float4 data1 = (*basePtr);\n"
"\n"
"		rayOrig = (float4)(data0.x, data0.y, data0.z, 0.f);\n"
"		rayDir = (float4)(data0.w, data1.x, data1.y, 0.f);\n"
"\n"
"		minT = data1.z;\n"
"		maxT = data1.w;\n"
"	}\n"
"\n"
"	//float4 rayOrig = (float4) (rays[gid].o.x, rays[gid].o.y, rays[gid].o.z, 0.f);\n"
"	//float4 rayDir = (float4) (rays[gid].d.x, rays[gid].d.y, rays[gid].d.z, 0.f);\n"
"	//float minT = rays[gid].mint;\n"
"	//float maxT = rays[gid].maxt;\n"
"\n"
"	const float4 invRayDir = (float4) 1.f / rayDir;\n"
"\n"
"	unsigned int hitIndex = NULL_INDEX;\n"
"	unsigned int currentNode = 0; // Root Node\n"
"	float b1, b2;\n"
"	unsigned int stopNode = bvhTree[0].skipIndex; // Non-existent\n"
"\n"
"	float4 pMin, pMax, data0, data1;\n"
"	__global float4 *basePtr;\n"
"	while (currentNode < stopNode) {\n"
"		/*float4 pMin = (float4)(bvhTree[currentNode].bbox.pMin.x,\n"
"				bvhTree[currentNode].bbox.pMin.y,\n"
"				bvhTree[currentNode].bbox.pMin.z,\n"
"				0.f);\n"
"		float4 pMax = (float4)(bvhTree[currentNode].bbox.pMax.x,\n"
"				bvhTree[currentNode].bbox.pMax.y,\n"
"				bvhTree[currentNode].bbox.pMax.z,\n"
"				0.f);*/\n"
"\n"
"		basePtr =(__global float4 *)&bvhTree[currentNode];\n"
"		data0 = (*basePtr++);\n"
"		data1 = (*basePtr);\n"
"\n"
"		pMin = (float4)(data0.x, data0.y, data0.z, 0.f);\n"
"		pMax = (float4)(data0.w, data1.x, data1.y, 0.f);\n"
"\n"
"		if (BBoxIntersectP(rayOrig, invRayDir, minT, maxT, pMin, pMax)) {\n"
"			//const unsigned int triIndex = bvhTree[currentNode].primitive;\n"
"			const unsigned int triIndex = as_uint(data1.z);\n"
"\n"
"			if (triIndex != NULL_INDEX)\n"
"				TriangleIntersect(rayOrig, rayDir, minT, &maxT, &hitIndex, &b1, &b2, triIndex, verts, tris);\n"
"\n"
"			currentNode++;\n"
"		} else {\n"
"			//currentNode = bvhTree[currentNode].skipIndex;\n"
"			currentNode = as_uint(data1.w);\n"
"		}\n"
"	}\n"
"\n"
"	// Write result\n"
"	rayHits[gid].t = maxT;\n"
"	rayHits[gid].b1 = b1;\n"
"	rayHits[gid].b2 = b2;\n"
"	rayHits[gid].index = hitIndex;\n"
"}\n"
; } }
