#include <string>
namespace luxrays { namespace ocl {
std::string KernelSource_mapping_funcs = 
"#line 2 \"mapping_funcs.cl\"\n"
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
"float2 UVMapping_Map2D(__global TextureMapping *mapping, const float2 uv) {\n"
"	const float2 scale = VLOAD2F(&mapping->uvMapping.uScale);\n"
"	const float2 delta = VLOAD2F(&mapping->uvMapping.uDelta);\n"
"	\n"
"	return uv * scale + delta;\n"
"}\n"
"\n"
"float3 UVMapping_Map3D(__global TextureMapping *mapping, const float3 p) {\n"
"	const float2 scale = VLOAD2F(&mapping->uvMapping.uScale);\n"
"	const float2 delta = VLOAD2F(&mapping->uvMapping.uDelta);\n"
"	const float2 mp = (float2)(p.x, p.y) * scale + delta;\n"
"\n"
"	return (float3)(mp.xy, 0.f);\n"
"}\n"
"\n"
"float2 GlobalMapping3D_Map2D(__global TextureMapping *mapping, const float2 uv) {\n"
"	const float3 p = Transform_ApplyPoint(&mapping->globalMapping3D.worldToLocal, (float3)(uv.xy, 0.f));\n"
"	return p.xy;\n"
"}\n"
"\n"
"float3 GlobalMapping3D_Map3D(__global TextureMapping *mapping, const float3 p) {\n"
"	return Transform_ApplyPoint(&mapping->globalMapping3D.worldToLocal, p);\n"
"}\n"
"\n"
"float2 Mapping_Map2D(__global TextureMapping *mapping, const float2 uv) {\n"
"	switch (mapping->type) {\n"
"		case UVMAPPING:\n"
"			return UVMapping_Map2D(mapping, uv);\n"
"		case GLOBALMAPPING3D:\n"
"			return GlobalMapping3D_Map2D(mapping, uv);\n"
"		default:\n"
"			return 0.f;\n"
"	}\n"
"}\n"
"\n"
"float3 Mapping_Map3D(__global TextureMapping *mapping, const float3 p) {\n"
"	switch (mapping->type) {\n"
"		case UVMAPPING:\n"
"			return UVMapping_Map3D(mapping, p);\n"
"		case GLOBALMAPPING3D:\n"
"			return GlobalMapping3D_Map3D(mapping, p);\n"
"		default:\n"
"			return 0.f;\n"
"	}\n"
"}\n"
; } }
