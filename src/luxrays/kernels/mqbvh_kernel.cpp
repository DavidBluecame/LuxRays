#include <string>
namespace luxrays { namespace ocl {
std::string KernelSource_mqbvh = 
"#line 2 \"mqbvh_kernel.cl\"\n"
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
"// Using a large stack size to avoid the allocation of the array on\n"
"// GPU registers (otherwise the GPU can easily run out of registers)\n"
"#define STACK_SIZE 64\n"
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
"	uint4 primitives;\n"
"} QuadTiangle;\n"
"\n"
"typedef struct {\n"
"	float4 bboxes[2][3];\n"
"	int4 children;\n"
"} QBVHNode;\n"
"\n"
"void TransformP(Point *ptrans, const Point *p, __global Matrix4x4 *m) {\n"
"    const float x = p->x;\n"
"    const float y = p->y;\n"
"    const float z = p->z;\n"
"\n"
"	ptrans->x = m->m[0][0] * x + m->m[0][1] * y + m->m[0][2] * z + m->m[0][3];\n"
"	ptrans->y = m->m[1][0] * x + m->m[1][1] * y + m->m[1][2] * z + m->m[1][3];\n"
"	ptrans->z = m->m[2][0] * x + m->m[2][1] * y + m->m[2][2] * z + m->m[2][3];\n"
"	const float w = m->m[3][0] * x + m->m[3][1] * y + m->m[3][2] * z + m->m[3][3];\n"
"\n"
"    ptrans->x /= w;\n"
"    ptrans->y /= w;\n"
"    ptrans->z /= w;\n"
"}\n"
"\n"
"void TransformV(Vector *ptrans, const Vector *p, __global Matrix4x4 *m) {\n"
"    const float x = p->x;\n"
"    const float y = p->y;\n"
"    const float z = p->z;\n"
"\n"
"	ptrans->x = m->m[0][0] * x + m->m[0][1] * y + m->m[0][2] * z;\n"
"	ptrans->y = m->m[1][0] * x + m->m[1][1] * y + m->m[1][2] * z;\n"
"	ptrans->z = m->m[2][0] * x + m->m[2][1] * y + m->m[2][2] * z;\n"
"}\n"
"\n"
"#define emptyLeafNode 0xffffffff\n"
"\n"
"#define QBVHNode_IsLeaf(index) (index < 0)\n"
"#define QBVHNode_IsEmpty(index) (index == emptyLeafNode)\n"
"#define QBVHNode_NbQuadPrimitives(index) ((uint)(((index >> 27) & 0xf) + 1))\n"
"#define QBVHNode_FirstQuadIndex(index) (index & 0x07ffffff)\n"
"\n"
"// Using invDir0/invDir1/invDir2 instead of an\n"
"// array because I dont' trust OpenCL compiler =)\n"
"int4 QBVHNode_BBoxIntersect(\n"
"        const float4 bboxes_minX, const float4 bboxes_maxX,\n"
"        const float4 bboxes_minY, const float4 bboxes_maxY,\n"
"        const float4 bboxes_minZ, const float4 bboxes_maxZ,\n"
"        const QuadRay *ray4,\n"
"		const float4 invDir0, const float4 invDir1, const float4 invDir2) {\n"
"	float4 tMin = ray4->mint;\n"
"	float4 tMax = ray4->maxt;\n"
"\n"
"	// X coordinate\n"
"	tMin = fmax(tMin, (bboxes_minX - ray4->ox) * invDir0);\n"
"	tMax = fmin(tMax, (bboxes_maxX - ray4->ox) * invDir0);\n"
"\n"
"	// Y coordinate\n"
"	tMin = fmax(tMin, (bboxes_minY - ray4->oy) * invDir1);\n"
"	tMax = fmin(tMax, (bboxes_maxY - ray4->oy) * invDir1);\n"
"\n"
"	// Z coordinate\n"
"	tMin = fmax(tMin, (bboxes_minZ - ray4->oz) * invDir2);\n"
"	tMax = fmin(tMax, (bboxes_maxZ - ray4->oz) * invDir2);\n"
"\n"
"	// Return the visit flags\n"
"	return  (tMax >= tMin);\n"
"}\n"
"\n"
"void QuadTriangle_Intersect(\n"
"    const float4 origx, const float4 origy, const float4 origz,\n"
"    const float4 edge1x, const float4 edge1y, const float4 edge1z,\n"
"    const float4 edge2x, const float4 edge2y, const float4 edge2z,\n"
"    const uint4 primitives,\n"
"    QuadRay *ray4, RayHit *rayHit) {\n"
"	//--------------------------------------------------------------------------\n"
"	// Calc. b1 coordinate\n"
"\n"
"	const float4 s1x = (ray4->dy * edge2z) - (ray4->dz * edge2y);\n"
"	const float4 s1y = (ray4->dz * edge2x) - (ray4->dx * edge2z);\n"
"	const float4 s1z = (ray4->dx * edge2y) - (ray4->dy * edge2x);\n"
"\n"
"	const float4 divisor = (s1x * edge1x) + (s1y * edge1y) + (s1z * edge1z);\n"
"\n"
"	const float4 dx = ray4->ox - origx;\n"
"	const float4 dy = ray4->oy - origy;\n"
"	const float4 dz = ray4->oz - origz;\n"
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
"    float _b1, _b2;\n"
"	float maxt = ray4->maxt.s0;\n"
"    uint index;\n"
"\n"
"    int4 cond = isnotequal(divisor, (float4)0.f) & isgreaterequal(b0, (float4)0.f) &\n"
"			isgreaterequal(b1, (float4)0.f) & isgreaterequal(b2, (float4)0.f) &\n"
"			isgreater(t, ray4->mint);\n"
"\n"
"    const int cond0 = cond.s0 && (t.s0 < maxt);\n"
"    maxt = select(maxt, t.s0, cond0);\n"
"    _b1 = select(0.f, b1.s0, cond0);\n"
"    _b2 = select(0.f, b2.s0, cond0);\n"
"    index = select(NULL_INDEX, primitives.s0, cond0);\n"
"\n"
"    const int cond1 = cond.s1 && (t.s1 < maxt);\n"
"    maxt = select(maxt, t.s1, cond1);\n"
"    _b1 = select(_b1, b1.s1, cond1);\n"
"    _b2 = select(_b2, b2.s1, cond1);\n"
"    index = select(index, primitives.s1, cond1);\n"
"\n"
"    const int cond2 = cond.s2 && (t.s2 < maxt);\n"
"    maxt = select(maxt, t.s2, cond2);\n"
"    _b1 = select(_b1, b1.s2, cond2);\n"
"    _b2 = select(_b2, b2.s2, cond2);\n"
"    index = select(index, primitives.s2, cond2);\n"
"\n"
"    const int cond3 = cond.s3 && (t.s3 < maxt);\n"
"    maxt = select(maxt, t.s3, cond3);\n"
"    _b1 = select(_b1, b1.s3, cond3);\n"
"    _b2 = select(_b2, b2.s3, cond3);\n"
"    index = select(index, primitives.s3, cond3);\n"
"\n"
"	if (index == NULL_INDEX)\n"
"		return;\n"
"\n"
"	ray4->maxt = (float4)maxt;\n"
"\n"
"	rayHit->t = maxt;\n"
"	rayHit->b1 = _b1;\n"
"	rayHit->b2 = _b2;\n"
"	rayHit->index = index;\n"
"}\n"
"\n"
"void LeafIntersect(\n"
"		const Ray *ray,\n"
"		RayHit *rayHit,\n"
"		__global QBVHNode *nodes,\n"
"		__global QuadTiangle *quadTris) {\n"
"	// Prepare the ray for intersection\n"
"	QuadRay ray4;\n"
"    ray4.ox = (float4)ray->o.x;\n"
"    ray4.oy = (float4)ray->o.y;\n"
"    ray4.oz = (float4)ray->o.z;\n"
"\n"
"    ray4.dx = (float4)ray->d.x;\n"
"    ray4.dy = (float4)ray->d.y;\n"
"    ray4.dz = (float4)ray->d.z;\n"
"\n"
"    ray4.mint = (float4)ray->mint;\n"
"    ray4.maxt = (float4)ray->maxt;\n"
"\n"
"	const float4 invDir0 = (float4)(1.f / ray4.dx.s0);\n"
"	const float4 invDir1 = (float4)(1.f / ray4.dy.s0);\n"
"	const float4 invDir2 = (float4)(1.f / ray4.dz.s0);\n"
"\n"
"	const int signs0 = signbit(ray4.dx.s0);\n"
"	const int signs1 = signbit(ray4.dy.s0);\n"
"	const int signs2 = signbit(ray4.dz.s0);\n"
"\n"
"	rayHit->index = NULL_INDEX;\n"
"\n"
"	//------------------------------\n"
"	// Main loop\n"
"	int todoNode = 0; // the index in the stack\n"
"	int nodeStack[STACK_SIZE];\n"
"	nodeStack[0] = 0; // first node to handle: root node\n"
"\n"
"	while (todoNode >= 0) {\n"
"		const int nodeData = nodeStack[todoNode];\n"
"		--todoNode;\n"
"\n"
"		// Leaves are identified by a negative index\n"
"		if (!QBVHNode_IsLeaf(nodeData)) {\n"
"			__global QBVHNode *node = &nodes[nodeData];\n"
"            const int4 visit = QBVHNode_BBoxIntersect(\n"
"                node->bboxes[signs0][0], node->bboxes[1 - signs0][0],\n"
"                node->bboxes[signs1][1], node->bboxes[1 - signs1][1],\n"
"                node->bboxes[signs2][2], node->bboxes[1 - signs2][2],\n"
"                &ray4,\n"
"				invDir0, invDir1, invDir2);\n"
"\n"
"			const int4 children = node->children;\n"
"\n"
"			// For some reason doing logic operations with int4 is very slow\n"
"			nodeStack[todoNode + 1] = children.s3;\n"
"			todoNode += (visit.s3 && !QBVHNode_IsEmpty(children.s3)) ? 1 : 0;\n"
"			nodeStack[todoNode + 1] = children.s2;\n"
"			todoNode += (visit.s2 && !QBVHNode_IsEmpty(children.s2)) ? 1 : 0;\n"
"			nodeStack[todoNode + 1] = children.s1;\n"
"			todoNode += (visit.s1 && !QBVHNode_IsEmpty(children.s1)) ? 1 : 0;\n"
"			nodeStack[todoNode + 1] = children.s0;\n"
"			todoNode += (visit.s0 && !QBVHNode_IsEmpty(children.s0)) ? 1 : 0;\n"
"		} else {\n"
"			// Perform intersection\n"
"			const uint nbQuadPrimitives = QBVHNode_NbQuadPrimitives(nodeData);\n"
"			const uint offset = QBVHNode_FirstQuadIndex(nodeData);\n"
"\n"
"			for (uint primNumber = offset; primNumber < (offset + nbQuadPrimitives); ++primNumber) {\n"
"                __global QuadTiangle *quadTri = &quadTris[primNumber];\n"
"                const float4 origx = quadTri->origx;\n"
"                const float4 origy = quadTri->origy;\n"
"                const float4 origz = quadTri->origz;\n"
"                const float4 edge1x = quadTri->edge1x;\n"
"                const float4 edge1y = quadTri->edge1y;\n"
"                const float4 edge1z = quadTri->edge1z;\n"
"                const float4 edge2x = quadTri->edge2x;\n"
"                const float4 edge2y = quadTri->edge2y;\n"
"                const float4 edge2z = quadTri->edge2z;\n"
"                const uint4 primitives = quadTri->primitives;\n"
"\n"
"				QuadTriangle_Intersect(\n"
"                    origx, origy, origz,\n"
"                    edge1x, edge1y, edge1z,\n"
"                    edge2x, edge2y, edge2z,\n"
"                    primitives,\n"
"                    &ray4, rayHit);\n"
"            }\n"
"		}\n"
"	}\n"
"}\n"
"\n"
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void Intersect(\n"
"		__global Ray *rays,\n"
"		__global RayHit *rayHits,\n"
"		__global QBVHNode *nodes,\n"
"		const uint rayCount,\n"
"        __global unsigned int *qbvhMemMap,\n"
"		__global QBVHNode *leafNodes,\n"
"		__global QuadTiangle *leafQuadTris,\n"
"        __global Matrix4x4 *leafTransformations,\n"
"        __global unsigned int *leafsOffset) {\n"
"	// Select the ray to check\n"
"	const int gid = get_global_id(0);\n"
"	if (gid >= rayCount)\n"
"		return;\n"
"\n"
"	// Prepare the ray for intersection\n"
"	QuadRay ray4;\n"
"    Point rayOrig;\n"
"    Vector rayDir;\n"
"	{\n"
"        __global float4 *basePtr =(__global float4 *)&rays[gid];\n"
"        float4 data0 = (*basePtr++);\n"
"        float4 data1 = (*basePtr);\n"
"\n"
"        rayOrig.x = data0.x;\n"
"        rayOrig.y = data0.y;\n"
"        rayOrig.z = data0.z;\n"
"\n"
"        rayDir.x = data0.w;\n"
"        rayDir.y = data1.x;\n"
"        rayDir.z = data1.y;\n"
"\n"
"        ray4.ox = (float4)data0.x;\n"
"        ray4.oy = (float4)data0.y;\n"
"        ray4.oz = (float4)data0.z;\n"
"\n"
"        ray4.dx = (float4)data0.w;\n"
"        ray4.dy = (float4)data1.x;\n"
"        ray4.dz = (float4)data1.y;\n"
"\n"
"        ray4.mint = (float4)data1.z;\n"
"        ray4.maxt = (float4)data1.w;\n"
"	}\n"
"\n"
"	const float4 invDir0 = (float4)(1.f / ray4.dx.s0);\n"
"	const float4 invDir1 = (float4)(1.f / ray4.dy.s0);\n"
"	const float4 invDir2 = (float4)(1.f / ray4.dz.s0);\n"
"\n"
"	const int signs0 = signbit(ray4.dx.s0);\n"
"	const int signs1 = signbit(ray4.dy.s0);\n"
"	const int signs2 = signbit(ray4.dz.s0);\n"
"\n"
"	RayHit rayHit;\n"
"	rayHit.index = NULL_INDEX;\n"
"\n"
"	//------------------------------\n"
"	// Main loop\n"
"	int todoNode = 0; // the index in the stack\n"
"	int nodeStack[STACK_SIZE];\n"
"	nodeStack[0] = 0; // first node to handle: root node\n"
"\n"
"	while (todoNode >= 0) {\n"
"		const int nodeData = nodeStack[todoNode];\n"
"		--todoNode;\n"
"\n"
"		// Leaves are identified by a negative index\n"
"		if (!QBVHNode_IsLeaf(nodeData)) {\n"
"			__global QBVHNode *node = &nodes[nodeData];\n"
"            const int4 visit = QBVHNode_BBoxIntersect(\n"
"                node->bboxes[signs0][0], node->bboxes[1 - signs0][0],\n"
"                node->bboxes[signs1][1], node->bboxes[1 - signs1][1],\n"
"                node->bboxes[signs2][2], node->bboxes[1 - signs2][2],\n"
"                &ray4,\n"
"				invDir0, invDir1, invDir2);\n"
"\n"
"			const int4 children = node->children;\n"
"\n"
"			// For some reason doing logic operations with int4 are very slow\n"
"			nodeStack[todoNode + 1] = children.s3;\n"
"			todoNode += (visit.s3 && !QBVHNode_IsEmpty(children.s3)) ? 1 : 0;\n"
"			nodeStack[todoNode + 1] = children.s2;\n"
"			todoNode += (visit.s2 && !QBVHNode_IsEmpty(children.s2)) ? 1 : 0;\n"
"			nodeStack[todoNode + 1] = children.s1;\n"
"			todoNode += (visit.s1 && !QBVHNode_IsEmpty(children.s1)) ? 1 : 0;\n"
"			nodeStack[todoNode + 1] = children.s0;\n"
"			todoNode += (visit.s0 && !QBVHNode_IsEmpty(children.s0)) ? 1 : 0;\n"
"		} else {\n"
"			// Perform intersection with QBVH leaf\n"
"			const uint leafIndex = QBVHNode_FirstQuadIndex(nodeData);\n"
"\n"
"            Ray tray;\n"
"            TransformP(&tray.o, &rayOrig, &leafTransformations[leafIndex]);\n"
"            TransformV(&tray.d, &rayDir, &leafTransformations[leafIndex]);\n"
"            tray.mint = ray4.mint.s0;\n"
"            tray.maxt = ray4.maxt.s0;\n"
"\n"
"            const unsigned int memIndex = leafIndex * 2;\n"
"            const unsigned int leafNodeOffset = qbvhMemMap[memIndex];\n"
"            __global QBVHNode *n = &leafNodes[leafNodeOffset];\n"
"            const unsigned int leafQuadTriOffset = qbvhMemMap[memIndex + 1];\n"
"            __global QuadTiangle *qt = &leafQuadTris[leafQuadTriOffset];\n"
"\n"
"            RayHit tmpRayHit;\n"
"            LeafIntersect(&tray, &tmpRayHit, n, qt);\n"
"\n"
"            if (tmpRayHit.index != NULL_INDEX) {\n"
"                rayHit.t = tmpRayHit.t;\n"
"                rayHit.b1 = tmpRayHit.b1;\n"
"                rayHit.b2 = tmpRayHit.b2;\n"
"                rayHit.index = tmpRayHit.index + leafsOffset[leafIndex];\n"
"\n"
"                ray4.maxt = (float4)tmpRayHit.t;\n"
"            }\n"
"		}\n"
"	}\n"
"\n"
"	// Write result\n"
"	RayHit_WriteAligned4(&rayHits[gid], rayHit.t, rayHit.b1, rayHit.b2, rayHit.index);\n"
"}\n"
; } }
