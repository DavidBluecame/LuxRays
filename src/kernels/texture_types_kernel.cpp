#include <string>
namespace luxrays { namespace ocl {
std::string KernelSource_texture_types = 
"#line 2 \"texture_types.cl\"\n"
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
"#define DUDV_VALUE 0.001f\n"
"\n"
"typedef enum {\n"
"	CONST_FLOAT, CONST_FLOAT3, IMAGEMAP, SCALE_TEX, FRESNEL_APPROX_N,\n"
"	FRESNEL_APPROX_K, MIX_TEX,\n"
"	// Procedural textures\n"
"	CHECKERBOARD2D, CHECKERBOARD3D, FBM_TEX, MARBLE, DOTS, BRICK\n"
"} TextureType;\n"
"\n"
"typedef struct {\n"
"	float value;\n"
"} ConstFloatParam;\n"
"\n"
"typedef struct {\n"
"	Spectrum color;\n"
"} ConstFloat3Param;\n"
"\n"
"typedef struct {\n"
"	unsigned int channelCount, width, height;\n"
"	unsigned int pageIndex, pixelsIndex;\n"
"} ImageMap;\n"
"\n"
"typedef struct {\n"
"	TextureMapping2D mapping;\n"
"	float gain, Du, Dv;\n"
"\n"
"	unsigned int imageMapIndex;\n"
"} ImageMapTexParam;\n"
"\n"
"typedef struct {\n"
"	unsigned int tex1Index, tex2Index;\n"
"} ScaleTexParam;\n"
"\n"
"typedef struct {\n"
"	unsigned int texIndex;\n"
"} FresnelApproxNTexParam;\n"
"\n"
"typedef struct {\n"
"	unsigned int texIndex;\n"
"} FresnelApproxKTexParam;\n"
"\n"
"typedef struct {\n"
"	TextureMapping2D mapping;\n"
"	unsigned int tex1Index, tex2Index;\n"
"} CheckerBoard2DTexParam;\n"
"\n"
"typedef struct {\n"
"	TextureMapping3D mapping;\n"
"	unsigned int tex1Index, tex2Index;\n"
"} CheckerBoard3DTexParam;\n"
"\n"
"typedef struct {\n"
"	unsigned int amountTexIndex, tex1Index, tex2Index;\n"
"} MixTexParam;\n"
"\n"
"typedef struct {\n"
"	TextureMapping3D mapping;\n"
"	int octaves;\n"
"	float omega;\n"
"} FBMTexParam;\n"
"\n"
"typedef struct {\n"
"	TextureMapping3D mapping;\n"
"	int octaves;\n"
"	float omega, scale, variation;\n"
"} MarbleTexParam;\n"
"\n"
"typedef struct {\n"
"	TextureMapping2D mapping;\n"
"	unsigned int insideIndex, outsideIndex;\n"
"} DotsTexParam;\n"
"\n"
"typedef enum {\n"
"	FLEMISH, RUNNING, ENGLISH, HERRINGBONE, BASKET, KETTING\n"
"} MasonryBond;\n"
"\n"
"typedef struct {\n"
"	TextureMapping3D mapping;\n"
"	unsigned int tex1Index, tex2Index, tex3Index;\n"
"	MasonryBond bond;\n"
"	float offsetx, offsety, offsetz;\n"
"	float brickwidth, brickheight, brickdepth, mortarsize;\n"
"	float proportion, invproportion, run;\n"
"	float mortarwidth, mortarheight, mortardepth;\n"
"	float bevelwidth, bevelheight, beveldepth;\n"
"	int usebevel;\n"
"} BrickTexParam;\n"
"\n"
"typedef struct {\n"
"	TextureType type;\n"
"	union {\n"
"		ConstFloatParam constFloat;\n"
"		ConstFloat3Param constFloat3;\n"
"		ImageMapTexParam imageMapTex;\n"
"		ScaleTexParam scaleTex;\n"
"		FresnelApproxNTexParam fresnelApproxN;\n"
"		FresnelApproxKTexParam fresnelApproxK;\n"
"		CheckerBoard2DTexParam checkerBoard2D;\n"
"		CheckerBoard3DTexParam checkerBoard3D;\n"
"		MixTexParam mixTex;\n"
"		FBMTexParam fbm;\n"
"		MarbleTexParam marble;\n"
"		DotsTexParam dots;\n"
"		BrickTexParam brick;\n"
"	};\n"
"} Texture;\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// Some macro trick in order to have more readable code\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_HAS_IMAGEMAPS)\n"
"\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_4)\n"
"#define IMAGEMAPS_PARAM_DECL , __global ImageMap *imageMapDescs, __global float *imageMapBuff0, __global float *imageMapBuff1, __global float *imageMapBuff2, __global float *imageMapBuff3, __global float *imageMapBuff4\n"
"#elif  defined(PARAM_IMAGEMAPS_PAGE_3)\n"
"#define IMAGEMAPS_PARAM_DECL , __global ImageMap *imageMapDescs, __global float *imageMapBuff0, __global float *imageMapBuff1, __global float *imageMapBuff2, __global float *imageMapBuff3\n"
"#elif  defined(PARAM_IMAGEMAPS_PAGE_2)\n"
"#define IMAGEMAPS_PARAM_DECL , __global ImageMap *imageMapDescs, __global float *imageMapBuff0, __global float *imageMapBuff1, __global float *imageMapBuff2\n"
"#elif  defined(PARAM_IMAGEMAPS_PAGE_1)\n"
"#define IMAGEMAPS_PARAM_DECL , __global ImageMap *imageMapDescs, __global float *imageMapBuff0, __global float *imageMapBuff1\n"
"#elif  defined(PARAM_IMAGEMAPS_PAGE_0)\n"
"#define IMAGEMAPS_PARAM_DECL , __global ImageMap *imageMapDescs, __global float *imageMapBuff0\n"
"#endif\n"
"\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_4)\n"
"#define IMAGEMAPS_PARAM , imageMapDescs, imageMapBuff0, imageMapBuff1, imageMapBuff2, imageMapBuff3, imageMapBuff4\n"
"#elif  defined(PARAM_IMAGEMAPS_PAGE_3)\n"
"#define IMAGEMAPS_PARAM , imageMapDescs, imageMapBuff0, imageMapBuff1, imageMapBuff2, imageMapBuff3\n"
"#elif  defined(PARAM_IMAGEMAPS_PAGE_2)\n"
"#define IMAGEMAPS_PARAM , imageMapDescs, imageMapBuff0, imageMapBuff1, imageMapBuff2\n"
"#elif  defined(PARAM_IMAGEMAPS_PAGE_1)\n"
"#define IMAGEMAPS_PARAM , imageMapDescs, imageMapBuff0, imageMapBuff1\n"
"#elif  defined(PARAM_IMAGEMAPS_PAGE_0)\n"
"#define IMAGEMAPS_PARAM , imageMapDescs, imageMapBuff0\n"
"#endif\n"
"\n"
"#else\n"
"\n"
"#define IMAGEMAPS_PARAM_DECL\n"
"#define IMAGEMAPS_PARAM\n"
"\n"
"#endif\n"
"\n"
"#define TEXTURES_PARAM_DECL , __global Texture *texs IMAGEMAPS_PARAM_DECL\n"
"#define TEXTURES_PARAM , texs IMAGEMAPS_PARAM\n"
; } }
