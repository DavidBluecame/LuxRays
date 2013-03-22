#include <string>
namespace slg { namespace ocl {
std::string KernelSource_hitpoint_types = 
"#line 2 \"hitpoint_types.cl\"\n"
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
"// This is defined only under OpenCL because of variable size structures\n"
"#if defined(SLG_OPENCL_KERNEL)\n"
"\n"
"typedef struct {\n"
"	// The incoming direction. It is the eyeDir when fromLight = false and\n"
"	// lightDir when fromLight = true\n"
"	Vector fixedDir;\n"
"	Point p;\n"
"	UV uv;\n"
"	Normal geometryN;\n"
"	Normal shadeN;\n"
"\n"
"#if defined(PARAM_ENABLE_TEX_HITPOINTCOLOR)\n"
"	Spectrum color;\n"
"#endif\n"
"#if defined(PARAM_ENABLE_TEX_HITPOINTALPHA)\n"
"	float alpha;\n"
"#endif\n"
"\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"	// passThroughEvent can be stored here in a path state even before of\n"
"	// BSDF initialization (while tracing the next path vertex ray)\n"
"	float passThroughEvent;\n"
"#endif\n"
"} HitPoint;\n"
"\n"
"#endif\n"
; } }
