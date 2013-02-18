#include <string>
namespace luxrays { namespace ocl {
std::string KernelSource_frame_funcs = 
"#line 2 \"frame_funcs.cl\"\n"
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
"void Frame_SetFromZ(__global Frame *frame, const float3 z) {\n"
"	const float3 Z = normalize(z);\n"
"	const float3 tmpZ = Z;\n"
"	const float3 tmpX = (fabs(tmpZ.x) > 0.99f) ? (float3)(0.f, 1.f, 0.f) : (float3)(1.f, 0.f, 0.f);\n"
"	const float3 Y = normalize(cross(tmpZ, tmpX));\n"
"	const float3 X = cross(Y, tmpZ);\n"
"\n"
"	VSTORE3F(X, &frame->X.x);\n"
"	VSTORE3F(Y, &frame->Y.x);\n"
"	VSTORE3F(Z, &frame->Z.x);\n"
"}\n"
"\n"
"float3 ToWorld(const float3 X, const float3 Y, const float3 Z, const float3 v) {\n"
"	return X * v.x + Y * v.y + Z * v.z;\n"
"}\n"
"\n"
"float3 Frame_ToWorld(__global Frame *frame, const float3 v) {\n"
"	return ToWorld(VLOAD3F(&frame->X.x), VLOAD3F(&frame->Y.x), VLOAD3F(&frame->Z.x), v);\n"
"}\n"
"\n"
"float3 ToLocal(const float3 X, const float3 Y, const float3 Z, const float3 a) {\n"
"	return (float3)(dot(a, X), dot(a, Y), dot(a, Z));\n"
"}\n"
"\n"
"float3 Frame_ToLocal(__global Frame *frame, const float3 v) {\n"
"	return ToLocal(VLOAD3F(&frame->X.x), VLOAD3F(&frame->Y.x), VLOAD3F(&frame->Z.x), v);\n"
"}\n"
; } }
