#include <string>
namespace slg { namespace ocl {
std::string KernelSource_texture_blender_noise_funcs2 = 
"#line 2 \"texture_blender_noise_funcs2.cl\"\n"
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
"//------------------------------------------------------------------------------\n"
"// Blender Texture utility functions\n"
"//------------------------------------------------------------------------------\n"
"/******************/\n"
"/* VORONOI/WORLEY */\n"
"/******************/\n"
"\n"
"/* distance metrics for voronoi, e parameter only used in Minkovsky */\n"
"/* Camberra omitted, didn't seem useful */\n"
"\n"
"/* distance squared */\n"
"float dist_Squared(float x, float y, float z, float e) { return (x*x + y*y + z*z); }\n"
"/* real distance */\n"
"float dist_Real(float x, float y, float z, float e) { return sqrt(x*x + y*y + z*z); }\n"
"/* manhattan/taxicab/cityblock distance */\n"
"float dist_Manhattan(float x, float y, float z, float e) { return (fabs(x) + fabs(y) + fabs(z)); }\n"
"/* Chebychev */\n"
"float dist_Chebychev(float x, float y, float z, float e) {\n"
"	float t;\n"
"	x = fabs(x);\n"
"	y = fabs(y);\n"
"	z = fabs(z);\n"
"	t = (x>y)?x:y;\n"
"	return ((z>t)?z:t);\n"
"}\n"
"\n"
"/* minkovsky preset exponent 0.5 */\n"
"float dist_MinkovskyH(float x, float y, float z, float e) {\n"
"	float d = sqrt(fabs(x)) + sqrt(fabs(y)) + sqrt(fabs(z));\n"
"	return (d*d);\n"
"}\n"
"\n"
"/* minkovsky preset exponent 4 */\n"
"float dist_Minkovsky4(float x, float y, float z, float e) {\n"
"	x *= x;\n"
"	y *= y;\n"
"	z *= z;\n"
"	return sqrt(sqrt(x*x + y*y + z*z));\n"
"}\n"
"\n"
"/* Minkovsky, general case, slow, maybe too slow to be useful */\n"
"float dist_Minkovsky(float x, float y, float z, float e) {\n"
"	return pow(pow(fabs(x), e) + pow(fabs(y), e) + pow(fabs(z), e), 1.f/e);\n"
"}\n"
"\n"
"/* Not 'pure' Worley, but the results are virtually the same.\n"
"	 Returns distances in da and point coords in pa */\n"
"void voronoi(float x, float y, float z, float* da, float* pa, float me, DistanceMetric dtype) {\n"
"	int xx, yy, zz, xi, yi, zi;\n"
"	float xd, yd, zd, d, p0, p1, p2;\n"
"\n"
"	xi = (int)(floor(x));\n"
"	yi = (int)(floor(y));\n"
"	zi = (int)(floor(z));\n"
"	da[0] = da[1] = da[2] = da[3] = 1e10f;\n"
"	for (xx=xi-1;xx<=xi+1;xx++) {\n"
"		for (yy=yi-1;yy<=yi+1;yy++) {\n"
"			for (zz=zi-1;zz<=zi+1;zz++) {\n"
"			\n"
"				p0 = hashpntf[3*hash[ (hash[ (hash[(zz) & 255]+(yy)) & 255]+(xx)) & 255]];\n"
"				p1 = hashpntf[3*hash[ (hash[ (hash[(zz) & 255]+(yy)) & 255]+(xx)) & 255]+1];\n"
"				p2 = hashpntf[3*hash[ (hash[ (hash[(zz) & 255]+(yy)) & 255]+(xx)) & 255]+2];\n"
"				xd = x - (p0 + xx);\n"
"				yd = y - (p1 + yy);\n"
"				zd = z - (p2 + zz);\n"
"				switch (dtype) {\n"
"					case DISTANCE_SQUARED:						\n"
"						d = dist_Squared(xd, yd, zd, me);\n"
"						break;\n"
"					case MANHATTAN:						\n"
"						d = dist_Manhattan(xd, yd, zd, me);\n"
"						break;\n"
"					case CHEBYCHEV:						\n"
"						d = dist_Chebychev(xd, yd, zd, me);\n"
"						break;\n"
"					case MINKOWSKI_HALF:\n"
"						d = dist_MinkovskyH(xd, yd, zd, me);\n"
"						break;\n"
"					case MINKOWSKI_FOUR:						\n"
"						d = dist_Minkovsky4(xd, yd, zd, me);\n"
"						break;\n"
"					case MINKOWSKI:						\n"
"						d = dist_Minkovsky(xd, yd, zd, me);\n"
"						break;\n"
"					case ACTUAL_DISTANCE:\n"
"					default:						\n"
"						d = dist_Real(xd, yd, zd, me);\n"
"				}\n"
"				\n"
"				if (d<da[0]) {\n"
"					da[3]=da[2];  da[2]=da[1];  da[1]=da[0];  da[0]=d;\n"
"					pa[9]=pa[6];  pa[10]=pa[7];  pa[11]=pa[8];\n"
"					pa[6]=pa[3];  pa[7]=pa[4];  pa[8]=pa[5];\n"
"					pa[3]=pa[0];  pa[4]=pa[1];  pa[5]=pa[2];\n"
"					pa[0]=p0+xx;  pa[1]=p1+yy;  pa[2]=p2+zz;\n"
"				}\n"
"				else if (d<da[1]) {\n"
"					da[3]=da[2];  da[2]=da[1];  da[1]=d;\n"
"					pa[9]=pa[6];  pa[10]=pa[7];  pa[11]=pa[8];\n"
"					pa[6]=pa[3];  pa[7]=pa[4];  pa[8]=pa[5];\n"
"					pa[3]=p0+xx;  pa[4]=p1+yy;  pa[5]=p2+zz;\n"
"				}\n"
"				else if (d<da[2]) {\n"
"					da[3]=da[2];  da[2]=d;\n"
"					pa[9]=pa[6];  pa[10]=pa[7];  pa[11]=pa[8];\n"
"					pa[6]=p0+xx;  pa[7]=p1+yy;  pa[8]=p2+zz;\n"
"				}\n"
"				else if (d<da[3]) {\n"
"					da[3]=d;\n"
"					pa[9]=p0+xx;  pa[10]=p1+yy;  pa[11]=p2+zz;\n"
"				}\n"
"			}\n"
"		}\n"
"	}\n"
"}\n"
"\n"
"/* returns different feature points for use in BLI_gNoise() */\n"
"static float voronoi_F1(float x, float y, float z) {\n"
"	float da[4], pa[12];\n"
"	voronoi(x, y, z, da, pa, 1.f, ACTUAL_DISTANCE);\n"
"	return da[0];\n"
"}\n"
"\n"
"float voronoi_F2(float x, float y, float z) {\n"
"	float da[4], pa[12];\n"
"	voronoi(x, y, z, da, pa, 1.f, ACTUAL_DISTANCE);\n"
"	return da[1];\n"
"}\n"
"\n"
"float voronoi_F3(float x, float y, float z) {\n"
"	float da[4], pa[12];\n"
"	voronoi(x, y, z, da, pa, 1.f, ACTUAL_DISTANCE);\n"
"	return da[2];\n"
"}\n"
"\n"
"float voronoi_F4(float x, float y, float z) {\n"
"	float da[4], pa[12];\n"
"	voronoi(x, y, z, da, pa, 1.f, ACTUAL_DISTANCE);\n"
"	return da[3];\n"
"}\n"
"\n"
"float voronoi_F1F2(float x, float y, float z) {\n"
"	float da[4], pa[12];\n"
"	voronoi(x, y, z, da, pa, 1.f, ACTUAL_DISTANCE);\n"
"	return (da[1]-da[0]);\n"
"}\n"
"\n"
"/* Crackle type pattern, just a scale/clamp of F2-F1 */\n"
"static float voronoi_Cr(float x, float y, float z) {\n"
"	float t = 10.f*voronoi_F1F2(x, y, z);\n"
"	if (t>1.f) return 1.f;\n"
"	return t;\n"
"}\n"
"\n"
"/* Signed version of all 6 of the above, just 2x-1, not really correct though (range is potentially (0, sqrt(6)).\n"
"   Used in the musgrave functions */\n"
"static float voronoi_F1S(float x, float y, float z) {\n"
"	float da[4], pa[12];\n"
"	voronoi(x, y, z, da, pa, 1.f, ACTUAL_DISTANCE);\n"
"	return (2.f*da[0]-1.f);\n"
"}\n"
"\n"
"float voronoi_F2S(float x, float y, float z) {\n"
"	float da[4], pa[12];\n"
"	voronoi(x, y, z, da, pa, 1.f, ACTUAL_DISTANCE);\n"
"	return (2.f*da[1]-1.f);\n"
"}\n"
"\n"
"float voronoi_F3S(float x, float y, float z) {\n"
"	float da[4], pa[12];\n"
"	voronoi(x, y, z, da, pa, 1.f, ACTUAL_DISTANCE);\n"
"	return (2.f*da[2]-1.f);\n"
"}\n"
"\n"
"float voronoi_F4S(float x, float y, float z) {\n"
"	float da[4], pa[12];\n"
"	voronoi(x, y, z, da, pa, 1.f, ACTUAL_DISTANCE);\n"
"	return (2.f*da[3]-1.f);\n"
"}\n"
"\n"
"float voronoi_F1F2S(float x, float y, float z) {\n"
"	float da[4], pa[12];\n"
"	voronoi(x, y, z, da, pa, 1.f, ACTUAL_DISTANCE);\n"
"	return (2.f*(da[1]-da[0])-1.f);\n"
"}\n"
"\n"
"/* Crackle type pattern, just a scale/clamp of F2-F1 */\n"
"float voronoi_CrS(float x, float y, float z) {\n"
"	float t = 10.f*voronoi_F1F2(x, y, z);\n"
"	if (t>1.f) return 1.f;\n"
"	return (2.f*t-1.f);\n"
"}\n"
"\n"
"/***************/\n"
"/* voronoi end */\n"
"/***************/\n"
"\n"
"/*************/\n"
"/* CELLNOISE */\n"
"/*************/\n"
"\n"
"/* returns unsigned cellnoise */\n"
"static float cellNoiseU(float x, float y, float z) {\n"
"  int xi = (int)(floor(x));\n"
"  int yi = (int)(floor(y));\n"
"  int zi = (int)(floor(z));\n"
"  unsigned int n = xi + yi*1301 + zi*314159;\n"
"  n ^= (n<<13);\n"
"  return ((float)(n*(n*n*15731 + 789221) + 1376312589) / 4294967296.f);\n"
"}\n"
"\n"
"/* idem, signed */\n"
"float cellNoise(float x, float y, float z) {\n"
"  return (2.f*cellNoiseU(x, y, z)-1.f);\n"
"}\n"
"\n"
"/* returns a vector/point/color in ca, using point hasharray directly */\n"
"void cellNoiseV(float x, float y, float z, float *ca) {\n"
"	int xi = (int)(floor(x));\n"
"	int yi = (int)(floor(y));\n"
"	int zi = (int)(floor(z));\n"
"	\n"
"	ca[0] = hashpntf[3*hash[ (hash[ (hash[(zi) & 255]+(yi)) & 255]+(xi)) & 255]];\n"
"	ca[1] = hashpntf[3*hash[ (hash[ (hash[(zi) & 255]+(yi)) & 255]+(xi)) & 255]+1];\n"
"	ca[2] = hashpntf[3*hash[ (hash[ (hash[(zi) & 255]+(yi)) & 255]+(xi)) & 255]+2];\n"
"}\n"
"\n"
"/*****************/\n"
"/* end cellnoise */\n"
"/*****************/\n"
"\n"
"float noisefuncS(BlenderNoiseBasis noisebasis, float x, float y, float z) {\n"
"	float result;\n"
"	switch (noisebasis) {\n"
"		case ORIGINAL_PERLIN:\n"
"			result = orgPerlinNoise(x, y, z);\n"
"			break;\n"
"		case IMPROVED_PERLIN:\n"
"			result = newPerlin(x, y, z);\n"
"			break;\n"
"		case VORONOI_F1:\n"
"			result = voronoi_F1S(x, y, z);\n"
"			break;\n"
"		case VORONOI_F2:\n"
"			result = voronoi_F2S(x, y, z);\n"
"			break;\n"
"		case VORONOI_F3:\n"
"			result = voronoi_F3S(x, y, z);\n"
"			break;\n"
"		case VORONOI_F4:\n"
"			result = voronoi_F4S(x, y, z);\n"
"			break;\n"
"		case VORONOI_F2_F1:\n"
"			result = voronoi_F1F2S(x, y, z);\n"
"			break;\n"
"		case VORONOI_CRACKLE:\n"
"			result = voronoi_CrS(x, y, z);\n"
"			break;\n"
"		case CELL_NOISE:\n"
"			result = cellNoise(x, y, z);\n"
"			break;\n"
"		case BLENDER_ORIGINAL:\n"
"		default: {\n"
"			result = orgBlenderNoiseS(x, y, z);\n"
"		}\n"
"	}\n"
"	return result;\n"
"}\n"
"\n"
"/*\n"
" * The following code is based on Ken Musgrave's explanations and sample\n"
" * source code in the book \"Texturing and Modelling: A procedural approach\"\n"
" */\n"
"\n"
"/*\n"
" * Procedural fBm evaluated at \"point\"; returns value stored in \"value\".\n"
" *\n"
" * Parameters:\n"
" *    ``H''  is the fractal increment parameter\n"
" *    ``lacunarity''  is the gap between successive frequencies\n"
" *    ``octaves''  is the number of frequencies in the fBm\n"
" */\n"
"float mg_fBm(float x, float y, float z, float H, float lacunarity, float octaves, BlenderNoiseBasis noisebasis)\n"
"{\n"
"	float rmd, value=0.f, pwr=1.f, pwHL=pow(lacunarity, -H);\n"
"	int	i;\n"
"\n"
"	for (i=0; i<(int)octaves; i++) {\n"
"		value +=noisefuncS(noisebasis, x, y, z)*pwr;\n"
"		pwr *= pwHL;\n"
"		x *= lacunarity;\n"
"		y *= lacunarity;\n"
"		z *= lacunarity;\n"
"	}	\n"
"\n"
"	rmd = octaves - floor(octaves);\n"
"	if (rmd!=0.f) value += rmd * noisefuncS(noisebasis, x, y, z) * pwr;\n"
"\n"
"	return value;\n"
"\n"
"} /* fBm() */\n"
"\n"
"/*\n"
" * Procedural multifractal evaluated at \"point\";\n"
" * returns value stored in \"value\".\n"
" *\n"
" * Parameters:\n"
" *    ``H''  determines the highest fractal dimension\n"
" *    ``lacunarity''  is gap between successive frequencies\n"
" *    ``octaves''  is the number of frequencies in the fBm\n"
" *    ``offset''  is the zero offset, which determines multifractality (NOT USED??)\n"
" */\n"
" /* this one is in fact rather confusing,\n"
" 	* there seem to be errors in the original source code (in all three versions of proc.text&mod),\n"
"	* I modified it to something that made sense to me, so it might be wrong... */\n"
"\n"
"float mg_MultiFractal(float x, float y, float z, float H, float lacunarity, float octaves, BlenderNoiseBasis noisebasis) {\n"
"	float rmd, value=1.f, pwr=1.f, pwHL=pow(lacunarity, -H);\n"
"	int i;\n"
"	\n"
"	for (i=0; i<(int)octaves; i++) {\n"
"		value *= (pwr * noisefuncS(noisebasis, x, y, z) + 1.f);\n"
"		pwr *= pwHL;\n"
"		x *= lacunarity;\n"
"		y *= lacunarity;\n"
"		z *= lacunarity;\n"
"	}	\n"
"\n"
"	rmd = octaves - floor(octaves);\n"
"	if (rmd!=0.f) value *= (rmd * noisefuncS(noisebasis, x, y, z) * pwr + 1.f);\n"
"	\n"
"	return value;\n"
"\n"
"} /* multifractal() */\n"
"\n"
"/*\n"
" * Heterogeneous procedural terrain function: stats by altitude method.\n"
" * Evaluated at \"point\"; returns value stored in \"value\".\n"
" *\n"
" * Parameters:\n"
" *       ``H''  determines the fractal dimension of the roughest areas\n"
" *       ``lacunarity''  is the gap between successive frequencies\n"
" *       ``octaves''  is the number of frequencies in the fBm\n"
" *       ``offset''  raises the terrain from `sea level'\n"
" */\n"
"float mg_HeteroTerrain(float x, float y, float z, float H, float lacunarity, float octaves, float offset, BlenderNoiseBasis noisebasis)\n"
"{\n"
"	float	value, increment, rmd;\n"
"	int i;\n"
"	float pwHL = pow(lacunarity, -H);\n"
"	float pwr = pwHL;	/* starts with i=1 instead of 0 */\n"
"\n"
"	/* first unscaled octave of function; later octaves are scaled */\n"
"	value = offset + noisefuncS(noisebasis, x, y, z);\n"
"\n"
"	x *= lacunarity;\n"
"	y *= lacunarity;\n"
"	z *= lacunarity;	\n"
"\n"
"	for (i=1; i<(int)octaves; i++) {\n"
"		increment = (noisefuncS(noisebasis, x, y, z) + offset) * pwr * value;\n"
"		value += increment;\n"
"		pwr *= pwHL;\n"
"		x *= lacunarity;\n"
"		y *= lacunarity;\n"
"		z *= lacunarity;\n"
"	}	\n"
"\n"
"	rmd = octaves - floor(octaves);\n"
"\n"
"	if (rmd!=0.f) {\n"
"		increment = (noisefuncS(noisebasis, x, y, z) + offset) * pwr * value;\n"
"		value += rmd * increment;\n"
"	}\n"
"	return value;\n"
"}\n"
"\n"
"/* Hybrid additive/multiplicative multifractal terrain model.\n"
" *\n"
" * Some good parameter values to start with:\n"
" *\n"
" *      H:           0.25\n"
" *      offset:      0.7\n"
" */\n"
"float mg_HybridMultiFractal(float x, float y, float z, float H, float lacunarity, float octaves, float offset, float gain, BlenderNoiseBasis noisebasis)\n"
"{\n"
"	float result, signal, weight, rmd;\n"
"	int i;\n"
"	float pwHL = pow(lacunarity, -H);\n"
"	float pwr = pwHL;	/* starts with i=1 instead of 0 */\n"
"	\n"
"	result = noisefuncS(noisebasis, x, y, z) + offset;\n"
"	weight = gain * result;\n"
"	x *= lacunarity;\n"
"	y *= lacunarity;\n"
"	z *= lacunarity;\n"
"\n"
"	for (i=1; (weight>0.001f) && (i<(int)octaves); i++) {\n"
"		if (weight>1.f)  weight=1.f;\n"
"		signal = (noisefuncS(noisebasis, x, y, z) + offset) * pwr;\n"
"		pwr *= pwHL;\n"
"		result += weight * signal;\n"
"		weight *= gain * signal;\n"
"		x *= lacunarity;\n"
"		y *= lacunarity;\n"
"		z *= lacunarity;\n"
"	}\n"
"\n"
"	rmd = octaves - floor(octaves);\n"
"	if (rmd!=0.f) result += rmd * ((noisefuncS(noisebasis, x, y, z) + offset) * pwr);\n"
"\n"
"	return result;\n"
"\n"
"} /* HybridMultifractal() */\n"
"\n"
"\n"
"/* Ridged multifractal terrain model.\n"
" *\n"
" * Some good parameter values to start with:\n"
" *\n"
" *      H:           1.0\n"
" *      offset:      1.0\n"
" *      gain:        2.0\n"
" */\n"
"float mg_RidgedMultiFractal(float x, float y, float z, float H, float lacunarity, float octaves, float offset, float gain, BlenderNoiseBasis noisebasis)\n"
"{\n"
"	float result, signal, weight;\n"
"	int	i;\n"
"	float pwHL = pow(lacunarity, -H);\n"
"	float pwr = pwHL;	/* starts with i=1 instead of 0 */\n"
"\n"
"	signal = offset - fabs(noisefuncS(noisebasis, x, y, z));\n"
"	signal *= signal;\n"
"	result = signal;\n"
"	weight = 1.f;\n"
"\n"
"	for( i=1; i<(int)octaves; i++ ) {\n"
"		x *= lacunarity;\n"
"		y *= lacunarity;\n"
"		z *= lacunarity;\n"
"		weight = signal * gain;\n"
"		if (weight>1.f) weight=1.f; else if (weight<0.f) weight=0.f;\n"
"\n"
"		signal = offset - fabs(noisefuncS(noisebasis, x, y, z));\n"
"		signal *= signal;\n"
"		signal *= weight;\n"
"		result += signal * pwr;\n"
"		pwr *= pwHL;\n"
"	}\n"
"\n"
"	return result;\n"
"} /* RidgedMultifractal() */\n"
"\n"
"/* \"Variable Lacunarity Noise\"\n"
" * A distorted variety of Perlin noise.\n"
" */\n"
"float mg_VLNoise(float x, float y, float z, float distortion, BlenderNoiseBasis nbas1, BlenderNoiseBasis nbas2)\n"
"{\n"
"	float3 rv;\n"
"	float result;\n"
"\n"
"	/* get a random vector and scale the randomization */		\n"
"	rv.x = noisefuncS(nbas1, x+13.5f, y+13.5f, z+13.5f) * distortion;\n"
"	rv.y = noisefuncS(nbas1, x, y, z) * distortion;\n"
"	rv.z = noisefuncS(nbas1, x-13.5f, y-13.5f, z-13.5f) * distortion;\n"
"\n"
"	result = noisefuncS(nbas2, x+rv.x, y+rv.y, z+rv.z);\n"
"\n"
"	return result;\n"
"}\n"
"\n"
"/****************/\n"
"/* musgrave end */\n"
"/****************/\n"
"\n"
"/* newnoise: generic noise function for use with different noisebases */\n"
"float BLI_gNoise(float noisesize, float x, float y, float z, int hard, BlenderNoiseBasis noisebasis) {\n"
"	float result = 0.f;\n"
"\n"
"	if(noisebasis == BLENDER_ORIGINAL) {\n"
"		// add one to make return value same as BLI_hnoise \n"
"		x += 1.f;\n"
"		y += 1.f;\n"
"		z += 1.f;\n"
"	}\n"
"\n"
"	if (noisesize!=0.f) {\n"
"		noisesize = 1.f/noisesize;\n"
"		x *= noisesize;\n"
"		y *= noisesize;\n"
"		z *= noisesize;\n"
"	}\n"
"\n"
"	switch (noisebasis) {\n"
"		case ORIGINAL_PERLIN:\n"
"			result = orgPerlinNoiseU(x, y, z);\n"
"			break;\n"
"		case IMPROVED_PERLIN:\n"
"			result = newPerlinU(x, y, z);\n"
"			break;\n"
"		case VORONOI_F1:\n"
"			result = voronoi_F1(x, y, z);\n"
"			break;\n"
"		case VORONOI_F2:\n"
"			result = voronoi_F2(x, y, z);\n"
"			break;\n"
"		case VORONOI_F3:\n"
"			result = voronoi_F3(x, y, z);\n"
"			break;\n"
"		case VORONOI_F4:\n"
"			result = voronoi_F4(x, y, z);\n"
"			break;\n"
"		case VORONOI_F2_F1:\n"
"			result = voronoi_F1F2(x, y, z);\n"
"			break;\n"
"		case VORONOI_CRACKLE:\n"
"			result = voronoi_Cr(x, y, z);\n"
"			break;\n"
"		case CELL_NOISE:\n"
"			result = cellNoiseU(x, y, z);\n"
"			break;\n"
"		case BLENDER_ORIGINAL:\n"
"		default: 			\n"
"			result = orgBlenderNoise(x, y, z);		\n"
"	}\n"
"\n"
"	if (hard) return fabs(2.f*result-1.f);\n"
"	return result;\n"
"}\n"
"\n"
"/* newnoise: generic turbulence function for use with different noisebasis */\n"
"float BLI_gTurbulence(float noisesize, float x, float y, float z, int oct, int hard, BlenderNoiseBasis noisebasis) {\n"
"	float sum=0.f, t, amp=1.f, fscale=1.f;\n"
"	int i;\n"
"\n"
"	if(noisebasis == BLENDER_ORIGINAL) {\n"
"		// add one to make return value same as BLI_hnoise \n"
"		x += 1.f;\n"
"		y += 1.f;\n"
"		z += 1.f;\n"
"	}\n"
"\n"
"	if (noisesize!=0.f) {\n"
"		noisesize = 1.f/noisesize;\n"
"		x *= noisesize;\n"
"		y *= noisesize;\n"
"		z *= noisesize;\n"
"	}\n"
"\n"
"	for (i=0;i<=oct;i++, amp*=0.5f, fscale*=2.f) {\n"
"		switch (noisebasis) {\n"
"			case ORIGINAL_PERLIN:\n"
"				t = orgPerlinNoise(fscale*x, fscale*y, fscale*z);\n"
"				break;\n"
"			case IMPROVED_PERLIN:\n"
"				t = newPerlin(fscale*x, fscale*y, fscale*z);\n"
"				break;\n"
"			case VORONOI_F1:\n"
"				t = voronoi_F1(fscale*x, fscale*y, fscale*z);\n"
"				break;\n"
"			case VORONOI_F2:\n"
"				t = voronoi_F2(fscale*x, fscale*y, fscale*z);\n"
"				break;\n"
"			case VORONOI_F3:\n"
"				t = voronoi_F3(fscale*x, fscale*y, fscale*z);\n"
"				break;\n"
"			case VORONOI_F4:\n"
"				t = voronoi_F4(fscale*x, fscale*y, fscale*z);\n"
"				break;\n"
"			case VORONOI_F2_F1:\n"
"				t = voronoi_F1F2(fscale*x, fscale*y, fscale*z);\n"
"				break;\n"
"			case VORONOI_CRACKLE:\n"
"				t = voronoi_Cr(fscale*x, fscale*y, fscale*z);\n"
"				break;\n"
"			case CELL_NOISE:\n"
"				t = cellNoiseU(fscale*x, fscale*y, fscale*z);\n"
"				break;\n"
"			case BLENDER_ORIGINAL:\n"
"			default: 			\n"
"				t = orgBlenderNoise(fscale*x, fscale*y, fscale*z);		\n"
"		}\n"
"\n"
"		if (hard) t = fabs(2.f*t-1.f);\n"
"		sum += t * amp;\n"
"	}\n"
"	\n"
"	sum *= ((float)(1<<oct)/(float)((1<<(oct+1))-1));\n"
"\n"
"	return sum;\n"
"\n"
"}\n"
; } }
