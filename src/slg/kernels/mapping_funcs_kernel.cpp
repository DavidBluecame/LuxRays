#include <string>
namespace slg { namespace ocl {
std::string KernelSource_mapping_funcs = 
"#line 2 \"mapping_funcs.cl\"\n"
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
"float2 UVMapping2D_Map(__global TextureMapping2D *mapping, __global HitPoint *hitPoint) {\n"
"	const float2 scale = VLOAD2F(&mapping->uvMapping2D.uScale);\n"
"	const float2 delta = VLOAD2F(&mapping->uvMapping2D.uDelta);\n"
"	const float2 uv = VLOAD2F(&hitPoint->uv.u);\n"
"	\n"
"	return uv * scale + delta;\n"
"}\n"
"\n"
"float3 UVMapping3D_Map(__global TextureMapping3D *mapping, __global HitPoint *hitPoint) {\n"
"	const float2 uv = VLOAD2F(&hitPoint->uv.u);\n"
"	return Transform_ApplyPoint(&mapping->worldToLocal, (float3)(uv.xy, 0.f));\n"
"}\n"
"\n"
"float3 GlobalMapping3D_Map(__global TextureMapping3D *mapping, __global HitPoint *hitPoint) {\n"
"	const float3 p = VLOAD3F(&hitPoint->p.x);\n"
"	return Transform_ApplyPoint(&mapping->worldToLocal, p);\n"
"}\n"
"\n"
"float2 TextureMapping2D_Map(__global TextureMapping2D *mapping, __global HitPoint *hitPoint) {\n"
"	switch (mapping->type) {\n"
"		case UVMAPPING2D:\n"
"			return UVMapping2D_Map(mapping, hitPoint);\n"
"		default:\n"
"			return 0.f;\n"
"	}\n"
"}\n"
"\n"
"float3 TextureMapping3D_Map(__global TextureMapping3D *mapping, __global HitPoint *hitPoint) {\n"
"	switch (mapping->type) {\n"
"		case UVMAPPING3D:\n"
"			return UVMapping3D_Map(mapping, hitPoint);\n"
"		case GLOBALMAPPING3D:\n"
"			return GlobalMapping3D_Map(mapping, hitPoint);\n"
"		default:\n"
"			return 0.f;\n"
"	}\n"
"}\n"
; } }
