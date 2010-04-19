#include "luxrays/kernels/kernels.h"
std::string luxrays::KernelSource_QBVH = 
"/***************************************************************************\n"
" *   Copyright (C) 1998-2009 by David Bucciarelli (davibu@interfree.it)    *\n"
" *                                                                         *\n"
" *   This file is part of SmallLuxGPU.                                     *\n"
" *                                                                         *\n"
" *   SmallLuxGPU is free software; you can redistribute it and/or modify   *\n"
" *   it under the terms of the GNU General Public License as published by  *\n"
" *   the Free Software Foundation; either version 3 of the License, or     *\n"
" *   (at your option) any later version.                                   *\n"
" *                                                                         *\n"
" *   SmallLuxGPU is distributed in the hope that it will be useful,        *\n"
" *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *\n"
" *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *\n"
" *   GNU General Public License for more details.                          *\n"
" *                                                                         *\n"
" *   You should have received a copy of the GNU General Public License     *\n"
" *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *\n"
" *                                                                         *\n"
" *   This project is based on PBRT ; see http://www.pbrt.org               *\n"
" *   and Lux Renderer website : http://www.luxrender.net                   *\n"
" ***************************************************************************/\n"
"\n"
"typedef struct {\n"
"	float x, y, z;\n"
"} Point;\n"
"\n"
"typedef struct {\n"
"	float x, y, z;\n"
"} Vector;\n"
"\n"
"typedef struct {\n"
"	Point o;\n"
"	Vector d;\n"
"	float mint, maxt;\n"
"} Ray;\n"
"\n"
"typedef struct {\n"
"	float t;\n"
"	float b1, b2; // Barycentric coordinates of the hit point\n"
"	unsigned int index;\n"
"} RayHit;\n"
"\n"
"typedef struct {\n"
"	Point pMin, pMax;\n"
"} BBox;\n"
"\n"
"typedef struct QuadRay {\n"
"	float4 ox, oy, oz;\n"
"	float4 dx, dy, dz;\n"
"	float4 mint, maxt;\n"
"} QuadRay;\n"
"\n"
"typedef struct {\n"
"	float4 origx, origy, origz;\n"
"	float4 edge1x, edge1y, edge1z;\n"
"	float4 edge2x, edge2y, edge2z;\n"
"	unsigned int primitives[4];\n"
"} QuadTiangle;\n"
"\n"
"typedef struct {\n"
"	float4 bboxes[2][3];\n"
"	int4 children;\n"
"} QBVHNode;\n"
"\n"
"#define emptyLeafNode 0xffffffff\n"
"\n"
"#define QBVHNode_IsLeaf(index) (index < 0)\n"
"#define QBVHNode_IsEmpty(index) (index == emptyLeafNode)\n"
"#define QBVHNode_NbQuadPrimitives(index) ((unsigned int)(((index >> 27) & 0xf) + 1))\n"
"#define QBVHNode_FirstQuadIndex(index) (index & 0x07ffffff)\n"
"\n"
"// Using invDir0/invDir1/invDir2 and sign0/sign1/sign2 instead of an\n"
"// array because I dont' trust OpenCL compiler =)\n"
"static int4 QBVHNode_BBoxIntersect(__global QBVHNode *node, const QuadRay *ray4,\n"
"		const float4 invDir0, const float4 invDir1, const float4 invDir2,\n"
"		const int sign0, const int sign1, const int sign2) {\n"
"	float4 tMin = ray4->mint;\n"
"	float4 tMax = ray4->maxt;\n"
"\n"
"	// X coordinate\n"
"	tMin = max(tMin, (node->bboxes[sign0][0] - ray4->ox) * invDir0);\n"
"	tMax = min(tMax, (node->bboxes[1 - sign0][0] - ray4->ox) * invDir0);\n"
"\n"
"	// Y coordinate\n"
"	tMin = max(tMin, (node->bboxes[sign1][1] - ray4->oy) * invDir1);\n"
"	tMax = min(tMax, (node->bboxes[1 - sign1][1] - ray4->oy) * invDir1);\n"
"\n"
"	// Z coordinate\n"
"	tMin = max(tMin, (node->bboxes[sign2][2] - ray4->oz) * invDir2);\n"
"	tMax = min(tMax, (node->bboxes[1 - sign2][2] - ray4->oz) * invDir2);\n"
"\n"
"	//return the visit flags\n"
"	return  (tMax >= tMin);\n"
"}\n"
"\n"
"static void QuadTriangle_Intersect(const __global QuadTiangle *qt, QuadRay *ray4, RayHit *rayHit) {\n"
"	const float4 zero = (float4)0.f;\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Calc. b1 coordinate\n"
"\n"
"	// Read data from memory\n"
"	const float4 edge1x = qt->edge1x;\n"
"	const float4 edge1y = qt->edge1y;\n"
"	const float4 edge1z = qt->edge1z;\n"
"\n"
"	const float4 edge2x = qt->edge2x;\n"
"	const float4 edge2y = qt->edge2y;\n"
"	const float4 edge2z = qt->edge2z;\n"
"\n"
"	const float4 s1x = (ray4->dy * edge2z) - (ray4->dz * edge2y);\n"
"	const float4 s1y = (ray4->dz * edge2x) - (ray4->dx * edge2z);\n"
"	const float4 s1z = (ray4->dx * edge2y) - (ray4->dy * edge2x);\n"
"\n"
"	const float4 divisor = (s1x * edge1x) + (s1y * edge1y) + (s1z * edge1z);\n"
"\n"
"	const float4 dx = ray4->ox - qt->origx;\n"
"	const float4 dy = ray4->oy - qt->origy;\n"
"	const float4 dz = ray4->oz - qt->origz;\n"
"\n"
"	const float4 b1 = ((dx * s1x) + (dy * s1y) + (dz * s1z)) / divisor;\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Calc. b2 coordinate\n"
"\n"
"	const float4 s2x = (dy * edge1z) - (dz * edge1y);\n"
"	const float4 s2y = (dz * edge1x) - (dx * edge1z);\n"
"	const float4 s2z = (dx * edge1y) - (dy * edge1x);\n"
"\n"
"	const float4 b2 = ((ray4->dx * s2x) + (ray4->dy * s2y) + (ray4->dz * s2z)) / divisor;\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Calc. b0 coordinate\n"
"\n"
"	const float4 b0 = ((float4)1.f) - b1 - b2;\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	const float4 t = ((edge2x * s2x) + (edge2y * s2y) + (edge2z * s2z)) / divisor;\n"
"\n"
"	// The '&&' operator is still bugged in the ATI compiler\n"
"	const int4 test = (divisor != zero) &\n"
"		(b0 >= zero) & (b1 >= zero) & (b2 >= zero) &\n"
"		(t > ray4->mint) & (t < ray4->maxt);\n"
"\n"
"	unsigned int hit = 4;\n"
"	float _b1, _b2;\n"
"	float maxt = ray4->maxt.s0;\n"
"	if (test.s0 && (t.s0 < maxt)) {\n"
"		hit = 0;\n"
"		maxt = t.s0;\n"
"		_b1 = b1.s0;\n"
"		_b2 = b2.s0;\n"
"	}\n"
"	if (test.s1 && (t.s1 < maxt)) {\n"
"		hit = 1;\n"
"		maxt = t.s1;\n"
"		_b1 = b1.s1;\n"
"		_b2 = b2.s1;\n"
"	}\n"
"	if (test.s2 && (t.s2 < maxt)) {\n"
"		hit = 2;\n"
"		maxt = t.s2;\n"
"		_b1 = b1.s2;\n"
"		_b2 = b2.s2;\n"
"	}\n"
"	if (test.s3 && (t.s3 < maxt)) {\n"
"		hit = 3;\n"
"		maxt = t.s3;\n"
"		_b1 = b1.s3;\n"
"		_b2 = b2.s3;\n"
"	}\n"
"\n"
"	if (hit == 4)\n"
"		return;\n"
"\n"
"	ray4->maxt = (float4)maxt;\n"
"\n"
"	rayHit->t = maxt;\n"
"	rayHit->b1 = _b1;\n"
"	rayHit->b2 = _b2;\n"
"	rayHit->index = qt->primitives[hit];\n"
"}\n"
"\n"
"__kernel void Intersect(\n"
"		__global Ray *rays,\n"
"		__global RayHit *rayHits,\n"
"		__global QBVHNode *nodes,\n"
"		__global QuadTiangle *quadTris,\n"
"		const unsigned int rayCount,\n"
"		__local int *nodeStacks) {\n"
"	// Select the ray to check\n"
"	const int gid = get_global_id(0);\n"
"	if (gid >= rayCount)\n"
"		return;\n"
"\n"
"	// Prepare the ray for intersection\n"
"	QuadRay ray4;\n"
"	{\n"
"			__global float4 *basePtr =(__global float4 *)&rays[gid];\n"
"			float4 data0 = (*basePtr++);\n"
"			float4 data1 = (*basePtr);\n"
"\n"
"			ray4.ox = (float4)data0.x;\n"
"			ray4.oy = (float4)data0.y;\n"
"			ray4.oz = (float4)data0.z;\n"
"\n"
"			ray4.dx = (float4)data0.w;\n"
"			ray4.dy = (float4)data1.x;\n"
"			ray4.dz = (float4)data1.y;\n"
"\n"
"			ray4.mint = (float4)data1.z;\n"
"			ray4.maxt = (float4)data1.w;\n"
"	}\n"
"\n"
"	const float4 invDir0 = (float4)(1.f / ray4.dx.s0);\n"
"	const float4 invDir1 = (float4)(1.f / ray4.dy.s0);\n"
"	const float4 invDir2 = (float4)(1.f / ray4.dz.s0);\n"
"\n"
"	const int signs0 = (ray4.dx.s0 < 0.f);\n"
"	const int signs1 = (ray4.dy.s0 < 0.f);\n"
"	const int signs2 = (ray4.dz.s0 < 0.f);\n"
"\n"
"	RayHit rayHit;\n"
"	rayHit.index = 0xffffffffu;\n"
"\n"
"	//------------------------------\n"
"	// Main loop\n"
"	int todoNode = 0; // the index in the stack\n"
"	__local int *nodeStack = &nodeStacks[24 * get_local_id(0)];\n"
"	nodeStack[0] = 0; // first node to handle: root node\n"
"\n"
"	while (todoNode >= 0) {\n"
"		const int nodeData = nodeStack[todoNode];\n"
"		--todoNode;\n"
"\n"
"		// Leaves are identified by a negative index\n"
"		if (!QBVHNode_IsLeaf(nodeData)) {\n"
"			__global QBVHNode *node = &nodes[nodeData];\n"
"			const int4 visit = QBVHNode_BBoxIntersect(node, &ray4,\n"
"				invDir0, invDir1, invDir2,\n"
"				signs0, signs1, signs2);\n"
"\n"
"			const int4 children = node->children;\n"
"			if (visit.s3)\n"
"				nodeStack[++todoNode] = children.s3;\n"
"			if (visit.s2)\n"
"				nodeStack[++todoNode] = children.s2;\n"
"			if (visit.s1)\n"
"				nodeStack[++todoNode] = children.s1;\n"
"			if (visit.s0)\n"
"				nodeStack[++todoNode] = children.s0;\n"
"		} else {\n"
"			//----------------------\n"
"			// It is a leaf,\n"
"			// all the informations are encoded in the index\n"
"\n"
"			if (QBVHNode_IsEmpty(nodeData))\n"
"				continue;\n"
"\n"
"			// Perform intersection\n"
"			const unsigned int nbQuadPrimitives = QBVHNode_NbQuadPrimitives(nodeData);\n"
"			const unsigned int offset = QBVHNode_FirstQuadIndex(nodeData);\n"
"\n"
"			for (unsigned int primNumber = offset; primNumber < (offset + nbQuadPrimitives); ++primNumber)\n"
"				QuadTriangle_Intersect(&quadTris[primNumber], &ray4, &rayHit);\n"
"		}\n"
"	}\n"
"\n"
"	// Write result\n"
"	rayHits[gid].t = rayHit.t;\n"
"	rayHits[gid].b1 = rayHit.b1;\n"
"	rayHits[gid].b2 = rayHit.b2;\n"
"	rayHits[gid].index = rayHit.index;\n"
"}\n"
;
