#include <string>
namespace slg { namespace ocl {
std::string KernelSource_texture_types = 
"#line 2 \"texture_types.cl\"\n"
"\n"
"/***************************************************************************\n"
" * Copyright 1998-2013 by authors (see AUTHORS.txt)                        *\n"
" *                                                                         *\n"
" *   This file is part of LuxRender.                                       *\n"
" *                                                                         *\n"
" * Licensed under the Apache License, Version 2.0 (the \"License\");         *\n"
" * you may not use this file except in compliance with the License.        *\n"
" * You may obtain a copy of the License at                                 *\n"
" *                                                                         *\n"
" *     http://www.apache.org/licenses/LICENSE-2.0                          *\n"
" *                                                                         *\n"
" * Unless required by applicable law or agreed to in writing, software     *\n"
" * distributed under the License is distributed on an \"AS IS\" BASIS,       *\n"
" * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*\n"
" * See the License for the specific language governing permissions and     *\n"
" * limitations under the License.                                          *\n"
" ***************************************************************************/\n"
"\n"
"#define DUDV_VALUE 0.001f\n"
"\n"
"typedef enum {\n"
"	CONST_FLOAT, CONST_FLOAT3, IMAGEMAP, SCALE_TEX, FRESNEL_APPROX_N,\n"
"	FRESNEL_APPROX_K, MIX_TEX, ADD_TEX, SUBTRACT_TEX, HITPOINTCOLOR, HITPOINTALPHA,\n"
"	HITPOINTGREY, NORMALMAP_TEX, BLACKBODY_TEX, IRREGULARDATA_TEX,\n"
"	// Procedural textures\n"
"	BLENDER_BLEND, BLENDER_CLOUDS, BLENDER_DISTORTED_NOISE, BLENDER_MAGIC,\n"
"	BLENDER_MARBLE, BLENDER_MUSGRAVE, BLENDER_STUCCI, BLENDER_WOOD, BLENDER_VORONOI,\n"
"	CHECKERBOARD2D, CHECKERBOARD3D, FBM_TEX, MARBLE, DOTS, BRICK, WINDY, WRINKLED, UV_TEX, BAND_TEX,\n"
"	// Fresnel textures\n"
"	FRESNELCOLOR_TEX\n"
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
"	TextureMapping2D mapping;\n"
"	float gain;\n"
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
"	unsigned int amountTexIndex, tex1Index, tex2Index;\n"
"} MixTexParam;\n"
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
"	unsigned int tex1Index, tex2Index;\n"
"} AddTexParam;\n"
"\n"
"typedef struct {\n"
"	TextureMapping3D mapping;\n"
"	int octaves;\n"
"	float omega;\n"
"} WindyTexParam;\n"
"\n"
"typedef enum { \n"
"	ACTUAL_DISTANCE, DISTANCE_SQUARED, MANHATTAN, CHEBYCHEV, MINKOWSKI_HALF, \n"
"	MINKOWSKI_FOUR, MINKOWSKI\n"
"} DistanceMetric;\n"
"\n"
"typedef enum {\n"
"	TEX_LIN, TEX_QUAD, TEX_EASE, TEX_DIAG, TEX_SPHERE, TEX_HALO, TEX_RAD\n"
"} ProgressionType;\n"
"\n"
"typedef enum {\n"
"    BLENDER_ORIGINAL, ORIGINAL_PERLIN, IMPROVED_PERLIN,\n"
"    VORONOI_F1, VORONOI_F2, VORONOI_F3, VORONOI_F4, VORONOI_F2_F1,\n"
"    VORONOI_CRACKLE, CELL_NOISE\n"
"} BlenderNoiseBasis;\n"
"\n"
"typedef enum {\n"
"	TEX_SIN, TEX_SAW, TEX_TRI\n"
"} BlenderNoiseBase;\n"
"\n"
"typedef struct {\n"
"	TextureMapping3D mapping;\n"
"	ProgressionType type;\n"
"	bool direction;\n"
"	float bright, contrast;\n"
"} BlenderBlendTexParam;\n"
"\n"
"typedef struct {\n"
"	TextureMapping3D mapping;\n"
"	BlenderNoiseBasis noisebasis;	\n"
"	float noisesize;\n"
"	int noisedepth;\n"
"	float bright, contrast;\n"
"	bool hard;\n"
"} BlenderCloudsTexParam;\n"
"\n"
"typedef struct {\n"
"	TextureMapping3D mapping;\n"
"	BlenderNoiseBasis noisedistortion;	\n"
"	BlenderNoiseBasis noisebasis;	\n"
"	float distortion;\n"
"	float noisesize;\n"
"	float bright, contrast;\n"
"} BlenderDistortedNoiseTexParam;\n"
"\n"
"typedef struct {\n"
"	TextureMapping3D mapping;\n"
"	int noisedepth;\n"
"	float turbulence;\n"
"	float bright, contrast;\n"
"} BlenderMagicTexParam;\n"
"\n"
"typedef enum {\n"
"	TEX_SOFT, TEX_SHARP, TEX_SHARPER\n"
"} BlenderMarbleType;\n"
"\n"
"typedef struct {\n"
"	TextureMapping3D mapping;\n"
"	BlenderMarbleType type;\n"
"	BlenderNoiseBasis noisebasis;	\n"
"	BlenderNoiseBase noisebasis2;	\n"
"	float noisesize, turbulence;\n"
"	int noisedepth;\n"
"	float bright, contrast;\n"
"	bool hard;\n"
"} BlenderMarbleTexParam;\n"
"\n"
"typedef enum {\n"
"	TEX_MULTIFRACTAL, TEX_RIDGED_MULTIFRACTAL, TEX_HYBRID_MULTIFRACTAL, TEX_FBM, TEX_HETERO_TERRAIN\n"
"} BlenderMusgraveType;\n"
"\n"
"typedef struct {\n"
"	TextureMapping3D mapping;\n"
"	BlenderMusgraveType type;\n"
"	BlenderNoiseBasis noisebasis;	\n"
"	float dimension;\n"
"	float intensity;\n"
"	float lacunarity;\n"
"	float offset;\n"
"	float gain;\n"
"	float octaves;\n"
"	float noisesize;\n"
"	float bright, contrast;\n"
"} BlenderMusgraveTexParam;\n"
"\n"
"typedef struct {\n"
"	int noisedepth;\n"
"	float bright, contrast;\n"
"} BlenderNoiseTexParam;\n"
"\n"
"typedef enum {\n"
"	TEX_PLASTIC, TEX_WALL_IN, TEX_WALL_OUT\n"
"} BlenderStucciType;\n"
"\n"
"typedef struct {\n"
"	TextureMapping3D mapping;\n"
"	BlenderStucciType type;\n"
"	BlenderNoiseBasis noisebasis;	\n"
"	float noisesize;\n"
"	float turbulence;\n"
"	float bright, contrast;\n"
"	bool hard;\n"
"} BlenderStucciTexParam;\n"
"\n"
"typedef enum {\n"
"	BANDS, RINGS, BANDNOISE, RINGNOISE\n"
"} BlenderWoodType;\n"
"\n"
"typedef struct {\n"
"	TextureMapping3D mapping;\n"
"	BlenderWoodType type;\n"
"	BlenderNoiseBase noisebasis2;\n"
"	BlenderNoiseBasis noisebasis;\n"
"	float noisesize, turbulence;\n"
"	float bright, contrast;\n"
"	int hard;\n"
"} BlenderWoodTexParam;\n"
"\n"
"typedef struct {\n"
" 	TextureMapping3D mapping;\n"
"	DistanceMetric distancemetric;\n"
"	float feature_weight1;\n"
"	float feature_weight2;\n"
"	float feature_weight3;\n"
"	float feature_weight4;\n"
"	float noisesize;\n"
"	float intensity;\n"
"	float exponent;\n"
" 	float bright, contrast;\n"
"} BlenderVoronoiTexParam;\n"
"\n"
"typedef struct {\n"
"	TextureMapping3D mapping;\n"
"	int octaves;\n"
"	float omega;\n"
"} WrinkledTexParam;\n"
"\n"
"typedef struct {\n"
"	TextureMapping2D mapping;\n"
"} UVTexParam;\n"
"\n"
"#define BAND_TEX_MAX_SIZE 16\n"
"\n"
"typedef struct {\n"
"	unsigned int amountTexIndex;\n"
"	unsigned int size;\n"
"	float offsets[BAND_TEX_MAX_SIZE];\n"
"	Spectrum values[BAND_TEX_MAX_SIZE];\n"
"} BandTexParam;\n"
"\n"
"typedef struct {\n"
"	unsigned int channel;\n"
"} HitPointGreyTexParam;\n"
"\n"
"typedef struct {\n"
"	unsigned int texIndex;\n"
"} NormalMapTexParam;\n"
"\n"
"typedef struct {\n"
"	Spectrum rgb;\n"
"} BlackBodyParam;\n"
"\n"
"typedef struct {\n"
"	Spectrum rgb;\n"
"} IrregularDataParam;\n"
"\n"
"typedef struct {\n"
"	unsigned int krIndex;\n"
"} FresnelColorParam;\n"
"\n"
"typedef struct {\n"
"	TextureType type;\n"
"	union {\n"
"		BlenderBlendTexParam blenderBlend;\n"
" 		BlenderCloudsTexParam blenderClouds;\n"
"		BlenderDistortedNoiseTexParam blenderDistortedNoise;\n"
"		BlenderMagicTexParam blenderMagic;\n"
"		BlenderMarbleTexParam blenderMarble;\n"
"		BlenderMusgraveTexParam blenderMusgrave;\n"
"		BlenderNoiseTexParam blenderNoise;\n"
"		BlenderStucciTexParam blenderStucci;\n"
" 		BlenderWoodTexParam blenderWood;\n"
"		BlenderVoronoiTexParam blenderVoronoi;\n"
"		ConstFloatParam constFloat;\n"
"		ConstFloat3Param constFloat3;\n"
"		ImageMapTexParam imageMapTex;\n"
"		ScaleTexParam scaleTex;\n"
"		FresnelApproxNTexParam fresnelApproxN;\n"
"		FresnelApproxKTexParam fresnelApproxK;\n"
"		MixTexParam mixTex;\n"
"		CheckerBoard2DTexParam checkerBoard2D;\n"
"		CheckerBoard3DTexParam checkerBoard3D;\n"
"		FBMTexParam fbm;\n"
"		MarbleTexParam marble;\n"
"		DotsTexParam dots;\n"
"		BrickTexParam brick;\n"
"		AddTexParam addTex;\n"
"		AddTexParam subtractTex;\n"
"		WindyTexParam windy;\n"
"		WrinkledTexParam wrinkled;\n"
"		UVTexParam uvTex;\n"
"		BandTexParam band;\n"
"		HitPointGreyTexParam hitPointGrey;\n"
"        NormalMapTexParam normalMap;\n"
"		BlackBodyParam blackBody;\n"
"		IrregularDataParam irregularData;\n"
"		FresnelColorParam fresnelColor;\n"
"	};\n"
"} Texture;\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// Some macro trick in order to have more readable code\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(SLG_OPENCL_KERNEL)\n"
"\n"
"#if defined(PARAM_HAS_IMAGEMAPS)\n"
"\n"
"#define IMAGEMAPS_PARAM_DECL , __global ImageMap *imageMapDescs, __global float **imageMapBuff\n"
"#define IMAGEMAPS_PARAM , imageMapDescs, imageMapBuff\n"
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
"\n"
"#endif\n"
; } }
