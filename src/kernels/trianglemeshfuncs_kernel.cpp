#include <string>
namespace luxrays { namespace ocl {
std::string KernelSource_TriangleMeshFuncs = 
"#line 2 \"trianglemesh_funcs.cl\"\n"
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
"float3 Mesh_GetGeometryNormal(__global Point *vertices,\n"
"		__global Triangle *triangles, const uint triIndex) {\n"
"	__global Triangle *tri = &triangles[triIndex];\n"
"	const float3 p0 = vload3(0, &vertices[tri->v[0]].x);\n"
"	const float3 p1 = vload3(0, &vertices[tri->v[1]].x);\n"
"	const float3 p2 = vload3(0, &vertices[tri->v[2]].x);\n"
"\n"
"	return normalize(cross(p1 - p0, p2 - p0));\n"
"}\n"
; } }
