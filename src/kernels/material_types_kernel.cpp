#include <string>
namespace luxrays { namespace ocl {
std::string KernelSource_material_types = 
"#line 2 \"material_types.cl\"\n"
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
"typedef enum {\n"
"	MATTE, MIRROR, GLASS, METAL, ARCHGLASS, MIX, NULLMAT, MATTETRANSLUCENT, GLOSSY2\n"
"} MaterialType;\n"
"\n"
"typedef struct {\n"
"    unsigned int kdTexIndex;\n"
"} MatteParam;\n"
"\n"
"typedef struct {\n"
"    unsigned int krTexIndex;\n"
"} MirrorParam;\n"
"\n"
"typedef struct {\n"
"    unsigned int krTexIndex;\n"
"	unsigned int ktTexIndex;\n"
"	unsigned int ousideIorTexIndex, iorTexIndex;\n"
"} GlassParam;\n"
"\n"
"typedef struct {\n"
"    unsigned int krTexIndex;\n"
"	unsigned int expTexIndex;\n"
"} MetalParam;\n"
"\n"
"typedef struct {\n"
"    unsigned int krTexIndex;\n"
"	unsigned int ktTexIndex;\n"
"} ArchGlassParam;\n"
"\n"
"typedef struct {\n"
"	unsigned int matAIndex, matBIndex;\n"
"	unsigned int mixFactorTexIndex;\n"
"} MixParam;\n"
"\n"
"typedef struct {\n"
"    unsigned int krTexIndex;\n"
"	unsigned int ktTexIndex;\n"
"} MatteTranslucentParam;\n"
"\n"
"typedef struct {\n"
"    unsigned int kdTexIndex;\n"
"	unsigned int ksTexIndex;\n"
"	unsigned int nuTexIndex;\n"
"	unsigned int nvTexIndex;\n"
"	unsigned int kaTexIndex;\n"
"	unsigned int depthTexIndex;\n"
"	unsigned int indexTexIndex;\n"
"	int multibounce;\n"
"} Glossy2Param;\n"
"\n"
"typedef struct {\n"
"	MaterialType type;\n"
"	unsigned int emitTexIndex, bumpTexIndex, normalTexIndex;\n"
"	union {\n"
"		MatteParam matte;\n"
"		MirrorParam mirror;\n"
"		GlassParam glass;\n"
"		MetalParam metal;\n"
"		ArchGlassParam archglass;\n"
"		MixParam mix;\n"
"		// NULLMAT has no parameters\n"
"		MatteTranslucentParam matteTranslucent;\n"
"		Glossy2Param glossy2;\n"
"	};\n"
"} Material;\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// Some macro trick in order to have more readable code\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#define MATERIALS_PARAM_DECL , __global Material *mats TEXTURES_PARAM_DECL\n"
"#define MATERIALS_PARAM , mats TEXTURES_PARAM\n"
; } }
