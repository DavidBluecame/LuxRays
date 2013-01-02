#line 2 "trianglemesh_funcs.cl"

/***************************************************************************
 *   Copyright (C) 1998-2010 by authors (see AUTHORS.txt )                 *
 *                                                                         *
 *   This file is part of LuxRays.                                         *
 *                                                                         *
 *   LuxRays is free software; you can redistribute it and/or modify       *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   LuxRays is distributed in the hope that it will be useful,            *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 *   LuxRays website: http://www.luxrender.net                             *
 ***************************************************************************/

float3 Mesh_GetGeometryNormal(__global Point *vertices,
		__global Triangle *triangles, const uint triIndex) {
	__global Triangle *tri = &triangles[triIndex];
	const float3 p0 = vload3(0, &vertices[tri->v[0]].x);
	const float3 p1 = vload3(0, &vertices[tri->v[1]].x);
	const float3 p2 = vload3(0, &vertices[tri->v[2]].x);

	return normalize(cross(p1 - p0, p2 - p0));
}

float3 Mesh_InterpolateNormal(__global Vector *normals, __global Triangle *triangles,
		const uint triIndex, const float b1, const float b2) {
	__global Triangle *tri = &triangles[triIndex];
	const float3 n0 = vload3(0, &normals[tri->v[0]].x);
	const float3 n1 = vload3(0, &normals[tri->v[1]].x);
	const float3 n2 = vload3(0, &normals[tri->v[2]].x);

	const float b0 = 1.f - b1 - b2;
	return normalize(b0 * n0 + b1 * n1 + b2 * n2);
}