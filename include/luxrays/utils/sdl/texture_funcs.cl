#line 2 "texture_funcs.cl"

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

#define TEXTURE_STACK_SIZE 16

//------------------------------------------------------------------------------
// Texture utility functions
//------------------------------------------------------------------------------

// Perlin Noise Data
#define NOISE_PERM_SIZE 256
__constant int NoisePerm[2 * NOISE_PERM_SIZE] = {
	151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96,
	53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142,
	// Rest of noise permutation table
	8, 99, 37, 240, 21, 10, 23,
	190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
	88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
	77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
	102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
	135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
	5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
	223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
	129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
	251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
	49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
	138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180,
	151, 160, 137, 91, 90, 15,
	131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
	190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
	88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
	77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
	102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
	135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
	5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
	223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
	129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
	251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
	49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
	138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
};

float Grad(int x, int y, int z, float dx, float dy, float dz) {
	const int h = NoisePerm[NoisePerm[NoisePerm[x] + y] + z] & 15;
	const float u = h < 8 || h == 12 || h == 13 ? dx : dy;
	const float v = h < 4 || h == 12 || h == 13 ? dy : dz;
	return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

float NoiseWeight(float t) {
	const float t3 = t * t * t;
	const float t4 = t3 * t;
	return 6.f * t4 * t - 15.f * t4 + 10.f * t3;
}

float Noise(float x, float y, float z) {
	// Compute noise cell coordinates and offsets
	int ix = Floor2Int(x);
	int iy = Floor2Int(y);
	int iz = Floor2Int(z);
	const float dx = x - ix, dy = y - iy, dz = z - iz;
	// Compute gradient weights
	ix &= (NOISE_PERM_SIZE - 1);
	iy &= (NOISE_PERM_SIZE - 1);
	iz &= (NOISE_PERM_SIZE - 1);
	const float w000 = Grad(ix, iy, iz, dx, dy, dz);
	const float w100 = Grad(ix + 1, iy, iz, dx - 1, dy, dz);
	const float w010 = Grad(ix, iy + 1, iz, dx, dy - 1, dz);
	const float w110 = Grad(ix + 1, iy + 1, iz, dx - 1, dy - 1, dz);
	const float w001 = Grad(ix, iy, iz + 1, dx, dy, dz - 1);
	const float w101 = Grad(ix + 1, iy, iz + 1, dx - 1, dy, dz - 1);
	const float w011 = Grad(ix, iy + 1, iz + 1, dx, dy - 1, dz - 1);
	const float w111 = Grad(ix + 1, iy + 1, iz + 1, dx - 1, dy - 1, dz - 1);
	// Compute trilinear interpolation of weights
	const float wx = NoiseWeight(dx);
	const float wy = NoiseWeight(dy);
	const float wz = NoiseWeight(dz);
	const float x00 = Lerp(wx, w000, w100);
	const float x10 = Lerp(wx, w010, w110);
	const float x01 = Lerp(wx, w001, w101);
	const float x11 = Lerp(wx, w011, w111);
	const float y0 = Lerp(wy, x00, x10);
	const float y1 = Lerp(wy, x01, x11);
	return Lerp(wz, y0, y1);
}

float Noise3(const float3 P) {
	return Noise(P.x, P.y, P.z);
}

float FBm(const float3 P, const float omega, const int maxOctaves) {
	// Compute number of octaves for anti-aliased FBm
	const float foctaves = fmin((float)maxOctaves, 1.f);
	const int octaves = Floor2Int(foctaves);
	// Compute sum of octaves of noise for FBm
	float sum = 0.f, lambda = 1.f, o = 1.f;
	for (int i = 0; i < octaves; ++i) {
		sum += o * Noise3(lambda * P);
		lambda *= 1.99f;
		o *= omega;
	}
	const float partialOctave = foctaves - (float)octaves;
	sum += o * SmoothStep(.3f, .7f, partialOctave) *
			Noise3(lambda * P);
	return sum;
}

//------------------------------------------------------------------------------
// ImageMaps support
//------------------------------------------------------------------------------

#if defined(PARAM_HAS_IMAGEMAPS)

__global float *ImageMap_GetPixelsAddress(
#if defined(PARAM_IMAGEMAPS_PAGE_0)
	__global float *imageMapBuff0,
#endif
#if defined(PARAM_IMAGEMAPS_PAGE_1)
	__global float *imageMapBuff1,
#endif
#if defined(PARAM_IMAGEMAPS_PAGE_2)
	__global float *imageMapBuff2,
#endif
#if defined(PARAM_IMAGEMAPS_PAGE_3)
	__global float *imageMapBuff3,
#endif
#if defined(PARAM_IMAGEMAPS_PAGE_4)
	__global float *imageMapBuff4,
#endif
	const uint page, const uint offset
    ) {
    switch (page) {
#if defined(PARAM_IMAGEMAPS_PAGE_1)
        case 1:
            return &imageMapBuff1[offset];
#endif
#if defined(PARAM_IMAGEMAPS_PAGE_2)
        case 2:
            return &imageMapBuff2[offset];
#endif
#if defined(PARAM_IMAGEMAPS_PAGE_3)
        case 3:
            return &imageMapBuff3[offset];
#endif
#if defined(PARAM_IMAGEMAPS_PAGE_4)
        case 4:
            return &imageMapBuff4[offset];
#endif
        default:
        case 0:
            return &imageMapBuff0[offset];
    }
}

float ImageMap_GetTexel_Float(__global float *pixels,
		const uint width, const uint height, const uint channelCount,
		const int s, const int t) {
	const uint u = Mod(s, width);
	const uint v = Mod(t, height);

	const uint index = channelCount * (v * width + u);

	return (channelCount == 1) ? pixels[index] : Spectrum_Y(VLOAD3F(&pixels[index]));
}

float3 ImageMap_GetTexel_Spectrum(__global float *pixels,
		const uint width, const uint height, const uint channelCount,
		const int s, const int t) {
	const uint u = Mod(s, width);
	const uint v = Mod(t, height);

	const uint index = channelCount * (v * width + u);

	return (channelCount == 1) ? pixels[index] : VLOAD3F(&pixels[index]);
}

float ImageMap_GetFloat(__global float *pixels,
		const uint width, const uint height, const uint channelCount,
		const float u, const float v) {
	const float s = u * width - 0.5f;
	const float t = v * height - 0.5f;

	const int s0 = Floor2Int(s);
	const int t0 = Floor2Int(t);

	const float ds = s - s0;
	const float dt = t - t0;

	const float ids = 1.f - ds;
	const float idt = 1.f - dt;

	const float c0 = ImageMap_GetTexel_Float(pixels, width, height, channelCount, s0, t0);
	const float c1 = ImageMap_GetTexel_Float(pixels, width, height, channelCount, s0, t0 + 1);
	const float c2 = ImageMap_GetTexel_Float(pixels, width, height, channelCount, s0 + 1, t0);
	const float c3 = ImageMap_GetTexel_Float(pixels, width, height, channelCount, s0 + 1, t0 + 1);

	const float k0 = ids * idt;
	const float k1 = ids * dt;
	const float k2 = ds * idt;
	const float k3 = ds * dt;

	return (k0 * c0 + k1 *c1 + k2 * c2 + k3 * c3);
}

float3 ImageMap_GetSpectrum(__global float *pixels,
		const uint width, const uint height, const uint channelCount,
		const float u, const float v) {
	const float s = u * width - 0.5f;
	const float t = v * height - 0.5f;

	const int s0 = Floor2Int(s);
	const int t0 = Floor2Int(t);

	const float ds = s - s0;
	const float dt = t - t0;

	const float ids = 1.f - ds;
	const float idt = 1.f - dt;

	const float3 c0 = ImageMap_GetTexel_Spectrum(pixels, width, height, channelCount, s0, t0);
	const float3 c1 = ImageMap_GetTexel_Spectrum(pixels, width, height, channelCount, s0, t0 + 1);
	const float3 c2 = ImageMap_GetTexel_Spectrum(pixels, width, height, channelCount, s0 + 1, t0);
	const float3 c3 = ImageMap_GetTexel_Spectrum(pixels, width, height, channelCount, s0 + 1, t0 + 1);

	const float k0 = ids * idt;
	const float k1 = ids * dt;
	const float k2 = ds * idt;
	const float k3 = ds * dt;

	return (k0 * c0 + k1 *c1 + k2 * c2 + k3 * c3);
}

#endif

//------------------------------------------------------------------------------
// ConstFloat texture
//------------------------------------------------------------------------------

#if defined (PARAM_ENABLE_TEX_CONST_FLOAT)

void ConstFloatTexture_EvaluateFloat(__global Texture *texture, __global HitPoint *hitPoint,
		float texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	texValues[(*texValuesSize)++] = texture->constFloat.value;
}

void ConstFloatTexture_EvaluateSpectrum(__global Texture *texture, __global HitPoint *hitPoint,
		float3 texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	texValues[(*texValuesSize)++] = texture->constFloat.value;
}

void ConstFloatTexture_EvaluateDuDv(__global Texture *texture, __global HitPoint *hitPoint,
		float2 texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	texValues[(*texValuesSize)++] = 0.f;
}

#endif

//------------------------------------------------------------------------------
// ConstFloat3 texture
//------------------------------------------------------------------------------

#if defined (PARAM_ENABLE_TEX_CONST_FLOAT3)

void ConstFloat3Texture_EvaluateFloat(__global Texture *texture, __global HitPoint *hitPoint,
		float texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	texValues[(*texValuesSize)++] = Spectrum_Y(VLOAD3F(&texture->constFloat3.color.r));
}

void ConstFloat3Texture_EvaluateSpectrum(__global Texture *texture, __global HitPoint *hitPoint,
		float3 texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	texValues[(*texValuesSize)++] = VLOAD3F(&texture->constFloat3.color.r);
}

void ConstFloat3Texture_EvaluateDuDv(__global Texture *texture, __global HitPoint *hitPoint,
		float2 texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	texValues[(*texValuesSize)++] = 0.f;
}

#endif

//------------------------------------------------------------------------------
// ImageMap texture
//------------------------------------------------------------------------------

#if defined (PARAM_ENABLE_TEX_IMAGEMAP)

void ImageMapTexture_EvaluateFloat(__global Texture *texture, __global HitPoint *hitPoint,
		float texValues[TEXTURE_STACK_SIZE], uint *texValuesSize
		IMAGEMAPS_PARAM_DECL) {
	__global ImageMap *imageMap = &imageMapDescs[texture->imageMapTex.imageMapIndex];
	__global float *pixels = ImageMap_GetPixelsAddress(
#if defined(PARAM_IMAGEMAPS_PAGE_0)
		imageMapBuff0,
#endif
#if defined(PARAM_IMAGEMAPS_PAGE_1)
		imageMapBuff1,
#endif
#if defined(PARAM_IMAGEMAPS_PAGE_2)
		imageMapBuff2,
#endif
#if defined(PARAM_IMAGEMAPS_PAGE_3)
		imageMapBuff3,
#endif
#if defined(PARAM_IMAGEMAPS_PAGE_4)
		imageMapBuff4,
#endif
		imageMap->pageIndex, imageMap->pixelsIndex);

	const float2 uv = VLOAD2F(&hitPoint->uv.u);
	const float2 mapUV = Mapping_Map2D(&texture->imageMapTex.mapping, uv);

	texValues[(*texValuesSize)++] = texture->imageMapTex.gain * ImageMap_GetFloat(
			pixels,
			imageMap->width, imageMap->height, imageMap->channelCount,
			mapUV.s0, mapUV.s1);
}

void ImageMapTexture_EvaluateSpectrum(__global Texture *texture, __global HitPoint *hitPoint,
		float3 texValues[TEXTURE_STACK_SIZE], uint *texValuesSize
		IMAGEMAPS_PARAM_DECL) {
	__global ImageMap *imageMap = &imageMapDescs[texture->imageMapTex.imageMapIndex];
	__global float *pixels = ImageMap_GetPixelsAddress(
#if defined(PARAM_IMAGEMAPS_PAGE_0)
		imageMapBuff0,
#endif
#if defined(PARAM_IMAGEMAPS_PAGE_1)
		imageMapBuff1,
#endif
#if defined(PARAM_IMAGEMAPS_PAGE_2)
		imageMapBuff2,
#endif
#if defined(PARAM_IMAGEMAPS_PAGE_3)
		imageMapBuff3,
#endif
#if defined(PARAM_IMAGEMAPS_PAGE_4)
		imageMapBuff4,
#endif
		imageMap->pageIndex, imageMap->pixelsIndex);

	const float2 uv = VLOAD2F(&hitPoint->uv.u);
	const float2 mapUV = Mapping_Map2D(&texture->imageMapTex.mapping, uv);

	texValues[(*texValuesSize)++] = texture->imageMapTex.gain * ImageMap_GetSpectrum(
			pixels,
			imageMap->width, imageMap->height, imageMap->channelCount,
			mapUV.s0, mapUV.s1);
}

void ImageMapTexture_EvaluateDuDv(__global Texture *texture, __global HitPoint *hitPoint,
		float2 texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	texValues[(*texValuesSize)++] = VLOAD2F(&texture->imageMapTex.Du);
}

#endif

//------------------------------------------------------------------------------
// Scale texture
//------------------------------------------------------------------------------

#if defined (PARAM_ENABLE_TEX_SCALE)

void ScaleTexture_EvaluateFloat(__global Texture *texture, __global HitPoint *hitPoint,
		float texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	const float value = texValues[--(*texValuesSize)] * texValues[--(*texValuesSize)];

	texValues[(*texValuesSize)++] = value;
}

void ScaleTexture_EvaluateSpectrum(__global Texture *texture, __global HitPoint *hitPoint,
		float3 texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	const float3 value = texValues[--(*texValuesSize)] * texValues[--(*texValuesSize)];

	texValues[(*texValuesSize)++] = value;
}

void ScaleTexture_EvaluateDuDv(__global Texture *texture, __global HitPoint *hitPoint,
		float2 texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	const float2 dudv1 = texValues[--(*texValuesSize)];
	const float2 dudv2 = texValues[--(*texValuesSize)];

	texValues[(*texValuesSize)++] = fmax(dudv1, dudv2);
}

#endif

//------------------------------------------------------------------------------
// FresnelApproxN & FresnelApproxK texture
//------------------------------------------------------------------------------

#if defined (PARAM_ENABLE_FRESNEL_APPROX_N)

float FresnelApproxN(const float Fr) {
	const float sqrtReflectance = sqrt(clamp(Fr, 0.f, .999f));

	return (1.f + sqrtReflectance) /
		(1.f - sqrtReflectance);
}

float3 FresnelApproxN3(const float3 Fr) {
	const float3 sqrtReflectance = Spectrum_Sqrt(clamp(Fr, 0.f, .999f));

	return (WHITE + sqrtReflectance) /
		(WHITE - sqrtReflectance);
}

void FresnelApproxNTexture_EvaluateFloat(__global Texture *texture, __global HitPoint *hitPoint,
		float texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	const float value = texValues[--(*texValuesSize)];

	texValues[(*texValuesSize)++] = FresnelApproxN(value);
}

void FresnelApproxN3Texture_EvaluateSpectrum(__global Texture *texture, __global HitPoint *hitPoint,
		float texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	const float3 value = texValues[--(*texValuesSize)];

	texValues[(*texValuesSize)++] = FresnelApproxN3(value);
}

void FresnelApproxNTexture_EvaluateDuDv(__global Texture *texture, __global HitPoint *hitPoint,
		float2 texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	texValues[(*texValuesSize)++] = texValues[--(*texValuesSize)];
}

#endif

#if defined (PARAM_ENABLE_FRESNEL_APPROX_K)

float FresnelApproxK(const float Fr) {
	const float reflectance = clamp(Fr, 0.f, .999f);

	return 2.f * sqrt(reflectance /
		(1.f - reflectance));
}

float3 FresnelApproxK3(const float3 Fr) {
	const float3 reflectance = clamp(Fr, 0.f, .999f);

	return 2.f * Spectrum_Sqrt(reflectance /
		(WHITE - reflectance));
}

void FresnelApproxKTexture_EvaluateFloat(__global Texture *texture, __global HitPoint *hitPoint,
		float texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	const float value = texValues[--(*texValuesSize)];

	texValues[(*texValuesSize)++] = FresnelApproxK(value);
}

void FresnelApproxKTexture_EvaluateSpectrum(__global Texture *texture, __global HitPoint *hitPoint,
		float texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	const float3 value = texValues[--(*texValuesSize)];

	texValues[(*texValuesSize)++] = FresnelApproxK3(value);
}

void FresnelApproxKTexture_EvaluateDuDv(__global Texture *texture, __global HitPoint *hitPoint,
		float2 texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	texValues[(*texValuesSize)++] = texValues[--(*texValuesSize)];
}

#endif

//------------------------------------------------------------------------------
// CheckerBoard 2D & 3D texture
//------------------------------------------------------------------------------

#if defined (PARAM_ENABLE_CHECKERBOARD2D)

void CheckerBoard2DTexture_EvaluateFloat(__global Texture *texture, __global HitPoint *hitPoint,
		float texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	const float value1 = texValues[--(*texValuesSize)];
	const float value2 = texValues[--(*texValuesSize)];

	const float2 uv = VLOAD2F(&hitPoint->uv.u);
	const float2 mapUV = Mapping_Map2D(&texture->checkerBoard2D.mapping, uv);

	texValues[(*texValuesSize)++] = ((Floor2Int(mapUV.s0) + Floor2Int(mapUV.s1)) % 2 == 0) ? value1 : value2;
}

void CheckerBoard2DTexture_EvaluateSpectrum(__global Texture *texture, __global HitPoint *hitPoint,
		float3 texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	const float3 value1 = texValues[--(*texValuesSize)];
	const float3 value2 = texValues[--(*texValuesSize)];

	const float2 uv = VLOAD2F(&hitPoint->uv.u);
	const float2 mapUV = Mapping_Map2D(&texture->checkerBoard2D.mapping, uv);

	texValues[(*texValuesSize)++] = ((Floor2Int(mapUV.s0) + Floor2Int(mapUV.s1)) % 2 == 0) ? value1 : value2;
}

void CheckerBoard2DTexture_EvaluateDuDv(__global Texture *texture, __global HitPoint *hitPoint,
		float2 texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	const float2 dudv1 = texValues[--(*texValuesSize)];
	const float2 dudv2 = texValues[--(*texValuesSize)];

	texValues[(*texValuesSize)++] = fmax(dudv1, dudv2);
}

#endif

#if defined (PARAM_ENABLE_CHECKERBOARD3D)

void CheckerBoard3DTexture_EvaluateFloat(__global Texture *texture, __global HitPoint *hitPoint,
		float texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	const float value1 = texValues[--(*texValuesSize)];
	const float value2 = texValues[--(*texValuesSize)];

	const float3 p = VLOAD3F(&hitPoint->p.x);
	const float3 mapP = Mapping_Map3D(&texture->checkerBoard3D.mapping, p);

	texValues[(*texValuesSize)++] = ((Floor2Int(mapP.x) + Floor2Int(mapP.y) + Floor2Int(mapP.z)) % 2 == 0) ? value1 : value2;
}

void CheckerBoard3DTexture_EvaluateSpectrum(__global Texture *texture, __global HitPoint *hitPoint,
		float3 texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	const float3 value1 = texValues[--(*texValuesSize)];
	const float3 value2 = texValues[--(*texValuesSize)];

	const float3 p = VLOAD3F(&hitPoint->p.x);
	const float3 mapP = Mapping_Map3D(&texture->checkerBoard3D.mapping, p);

	texValues[(*texValuesSize)++] = ((Floor2Int(mapP.x) + Floor2Int(mapP.y) + Floor2Int(mapP.z)) % 2 == 0) ? value1 : value2;
}

void CheckerBoard3DTexture_EvaluateDuDv(__global Texture *texture, __global HitPoint *hitPoint,
		float2 texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	const float2 dudv1 = texValues[--(*texValuesSize)];
	const float2 dudv2 = texValues[--(*texValuesSize)];

	texValues[(*texValuesSize)++] = fmax(dudv1, dudv2);
}

#endif

//------------------------------------------------------------------------------
// Mix texture
//------------------------------------------------------------------------------

#if defined (PARAM_ENABLE_MIX_TEX)

void MixTexture_EvaluateFloat(__global Texture *texture, __global HitPoint *hitPoint,
		float texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	const float amt = texValues[--(*texValuesSize)];
	const float value1 = texValues[--(*texValuesSize)];
	const float value2 = texValues[--(*texValuesSize)];

	texValues[(*texValuesSize)++] = Lerp(amt, value1, value2);
}

void MixTexture_EvaluateSpectrum(__global Texture *texture, __global HitPoint *hitPoint,
		float3 texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	const float3 amt = clamp(texValues[--(*texValuesSize)], 0.f, 1.f);
	const float3 value1 = texValues[--(*texValuesSize)];
	const float3 value2 = texValues[--(*texValuesSize)];

	texValues[(*texValuesSize)++] = mix(value1, value2, amt);
}

void MixTexture_EvaluateDuDv(__global Texture *texture, __global HitPoint *hitPoint,
		float2 texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	const float2 dudv1 = texValues[--(*texValuesSize)];
	const float2 dudv2 = texValues[--(*texValuesSize)];
	const float2 dudv3 = texValues[--(*texValuesSize)];

	texValues[(*texValuesSize)++] = fmax(fmax(dudv1, dudv2), dudv3);
}

#endif

//------------------------------------------------------------------------------
// FBM texture
//------------------------------------------------------------------------------

#if defined (PARAM_ENABLE_FBM_TEX)

void FBMTexture_EvaluateFloat(__global Texture *texture, __global HitPoint *hitPoint,
		float texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	const float3 p = VLOAD3F(&hitPoint->p.x);
	const float3 mapP = Mapping_Map3D(&texture->fbm.mapping, p);

	texValues[(*texValuesSize)++] = FBm(mapP, texture->fbm.omega, texture->fbm.octaves);
}

void FBMTexture_EvaluateSpectrum(__global Texture *texture, __global HitPoint *hitPoint,
		float3 texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	const float3 p = VLOAD3F(&hitPoint->p.x);
	const float3 mapP = Mapping_Map3D(&texture->fbm.mapping, p);

	texValues[(*texValuesSize)++] = FBm(mapP, texture->fbm.omega, texture->fbm.octaves);
}

void FBMTexture_EvaluateDuDv(__global Texture *texture, __global HitPoint *hitPoint,
		float2 texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	texValues[(*texValuesSize)++] = (float2)(DUDV_VALUE, DUDV_VALUE);
}

#endif

//------------------------------------------------------------------------------
// Marble texture
//------------------------------------------------------------------------------

#if defined (PARAM_ENABLE_MARBLE)

// Evaluate marble spline at _t_
__constant float MarbleTexture_c[9][3] = {
	{ .58f, .58f, .6f},
	{ .58f, .58f, .6f},
	{ .58f, .58f, .6f},
	{ .5f, .5f, .5f},
	{ .6f, .59f, .58f},
	{ .58f, .58f, .6f},
	{ .58f, .58f, .6f},
	{.2f, .2f, .33f},
	{ .58f, .58f, .6f}
};

float3 MarbleTexture_Evaluate(__global Texture *texture, __global HitPoint *hitPoint) {
	const float3 p = VLOAD3F(&hitPoint->p.x);
	const float3 P = texture->marble.scale * Mapping_Map3D(&texture->marble.mapping, p);

	float marble = P.y + texture->marble.variation * FBm(P, texture->marble.omega, texture->marble.octaves);
	float t = .5f + .5f * sin(marble);
#define NC  sizeof(MarbleTexture_c) / sizeof(MarbleTexture_c[0])
#define NSEG (NC-3)
	const int first = Floor2Int(t * NSEG);
	t = (t * NSEG - first);
#undef NC
#undef NSEG
#define ASSIGN_CF3(a) (float3)(a[0], a[1], a[2])
	const float3 c0 = ASSIGN_CF3(MarbleTexture_c[first]);
	const float3 c1 = ASSIGN_CF3(MarbleTexture_c[first + 1]);
	const float3 c2 = ASSIGN_CF3(MarbleTexture_c[first + 2]);
	const float3 c3 = ASSIGN_CF3(MarbleTexture_c[first + 3]);
#undef ASSIGN_CF3
	// Bezier spline evaluated with de Castilejau's algorithm
	
#if defined(__APPLE_CL__)
	float3 s0;
	s0.x = c0.x + (c1.x - c0.x) * t;
	s0.y = c0.y + (c1.y - c0.y) * t;
	s0.z = c0.z + (c1.z - c0.z) * t;
	
	float3 s1;
	s1.x = c1.x + (c2.x - c1.x) * t;
	s1.y = c1.y + (c2.y - c1.y) * t;
	s1.z = c1.z + (c2.z - c1.z) * t;
	
	float3 s2;
	s2.x = c2.x + (c3.x - c2.x) * t;
	s2.y = c2.y + (c3.y - c2.y) * t;
	s2.z = c2.z + (c3.z - c2.z) * t;

	s0.x = s0.x + (s1.x - s0.x) * t;
	s0.y = s0.y + (s1.y - s0.y) * t;
	s0.z = s0.z + (s1.z - s0.z) * t;

	s1.x = s1.x + (s2.x - s1.x) * t;
	s1.y = s1.y + (s2.y - s1.y) * t;
	s1.z = s1.z + (s2.z - s1.z) * t;

	float3 s4;
	s4.x = s0.x + (s1.x - s0.x) * t;
	s4.y = s0.y + (s1.y - s0.y) * t;
	s4.z = s0.z + (s1.z - s0.z) * t;
	// Extra scale of 1.5 to increase variation among colors
	return 1.5f * s4;
#else
	float3 s0 = mix(c0, c1, t);
	float3 s1 = mix(c1, c2, t);
	float3 s2 = mix(c2, c3, t);
	s0 = mix(s0, s1, t);
	s1 = mix(s1, s2, t);
	// Extra scale of 1.5 to increase variation among colors
	return 1.5f * mix(s0, s1, t);
#endif
}

void MarbleTexture_EvaluateFloat(__global Texture *texture, __global HitPoint *hitPoint,
		float texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	texValues[(*texValuesSize)++] = Spectrum_Y(MarbleTexture_Evaluate(texture, hitPoint));
}

void MarbleTexture_EvaluateSpectrum(__global Texture *texture, __global HitPoint *hitPoint,
		float3 texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	texValues[(*texValuesSize)++] = MarbleTexture_Evaluate(texture, hitPoint);
}

void MarbleTexture_EvaluateDuDv(__global Texture *texture, __global HitPoint *hitPoint,
		float2 texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	texValues[(*texValuesSize)++] = (float2)(DUDV_VALUE, DUDV_VALUE);
}

#endif

//------------------------------------------------------------------------------
// Generic texture functions with support for recursive textures
//------------------------------------------------------------------------------

uint Texture_AddSubTexture(__global Texture *texture,
		__global Texture *todoTex[TEXTURE_STACK_SIZE], uint *todoTexSize
		TEXTURES_PARAM_DECL) {
	switch (texture->type) {
#if defined(PARAM_ENABLE_TEX_SCALE)
		case SCALE_TEX:
			todoTex[(*todoTexSize)++] = &texs[texture->scaleTex.tex1Index];
			todoTex[(*todoTexSize)++] = &texs[texture->scaleTex.tex2Index];
			return 2;
#endif
#if defined (PARAM_ENABLE_FRESNEL_APPROX_N)
		case FRESNEL_APPROX_K:
			todoTex[(*todoTexSize)++] = &texs[texture->fresnelApproxN.texIndex];
			return 1;
#endif
#if defined (PARAM_ENABLE_FRESNEL_APPROX_K)
		case FRESNEL_APPROX_K:
			todoTex[(*todoTexSize)++] = &texs[texture->fresnelApproxK.texIndex];
			return 1;
#endif
#if defined(PARAM_ENABLE_CHECKERBOARD2D)
		case CHECKERBOARD2D:
			todoTex[(*todoTexSize)++] = &texs[texture->checkerBoard2D.tex1Index];
			todoTex[(*todoTexSize)++] = &texs[texture->checkerBoard2D.tex2Index];
			return 2;
#endif
#if defined(PARAM_ENABLE_CHECKERBOARD3D)
		case CHECKERBOARD3D:
			todoTex[(*todoTexSize)++] = &texs[texture->checkerBoard3D.tex1Index];
			todoTex[(*todoTexSize)++] = &texs[texture->checkerBoard3D.tex2Index];
			return 2;
#endif
#if defined (PARAM_ENABLE_MIX_TEX)
		case MIX_TEX:
			todoTex[(*todoTexSize)++] = &texs[texture->mixTex.amountTexIndex];
			todoTex[(*todoTexSize)++] = &texs[texture->mixTex.tex1Index];
			todoTex[(*todoTexSize)++] = &texs[texture->mixTex.tex2Index];
			return 3;
#endif
#if defined (PARAM_ENABLE_MARBLE)
		case MARBLE:
#endif
#if defined (PARAM_ENABLE_FBM_TEX)
		case FBM_TEX:
#endif
#if defined(PARAM_ENABLE_TEX_CONST_FLOAT)
		case CONST_FLOAT:
#endif
#if defined(PARAM_ENABLE_TEX_CONST_FLOAT3)
		case CONST_FLOAT3:
#endif
#if defined(PARAM_ENABLE_TEX_IMAGEMAP)
		case IMAGEMAP:
#endif
		default:
			return 0;
	}
}

//------------------------------------------------------------------------------
// Float texture channel
//------------------------------------------------------------------------------

void Texture_EvaluateFloat(__global Texture *texture, __global HitPoint *hitPoint,
		float texValues[TEXTURE_STACK_SIZE], uint *texValuesSize
		IMAGEMAPS_PARAM_DECL) {
	switch (texture->type) {
#if defined(PARAM_ENABLE_TEX_CONST_FLOAT)
		case CONST_FLOAT:
			ConstFloatTexture_EvaluateFloat(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
#if defined(PARAM_ENABLE_TEX_CONST_FLOAT3)
		case CONST_FLOAT3:
			ConstFloat3Texture_EvaluateFloat(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
#if defined(PARAM_ENABLE_TEX_IMAGEMAP)
		case IMAGEMAP:
			ImageMapTexture_EvaluateFloat(texture, hitPoint, texValues, texValuesSize
					IMAGEMAPS_PARAM);
			break;
#endif
#if defined(PARAM_ENABLE_TEX_SCALE)
		case SCALE_TEX:
			ScaleTexture_EvaluateFloat(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
#if defined (PARAM_ENABLE_FRESNEL_APPROX_N)
		case FRESNEL_APPROX_K:
			FresnelApproxNTexture_EvaluateFloat(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
#if defined (PARAM_ENABLE_FRESNEL_APPROX_K)
		case FRESNEL_APPROX_K:
			FresnelApproxKTexture_EvaluateFloat(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
#if defined (PARAM_ENABLE_CHECKERBOARD2D)
		case CHECKERBOARD2D:
			CheckerBoard2DTexture_EvaluateFloat(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
#if defined (PARAM_ENABLE_CHECKERBOARD3D)
		case CHECKERBOARD3D:
			CheckerBoard3DTexture_EvaluateFloat(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
#if defined (PARAM_ENABLE_MIX_TEX)
		case MIX_TEX:
			MixTexture_EvaluateFloat(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
#if defined (PARAM_ENABLE_FBM_TEX)
		case FBM_TEX:
			FBMTexture_EvaluateFloat(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
#if defined (PARAM_ENABLE_MARBLE)
		case MARBLE:
			MarbleTexture_EvaluateFloat(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
		default:
			// Do nothing
			break;
	}
}

float Texture_GetFloatValue(__global Texture *texture, __global HitPoint *hitPoint
		TEXTURES_PARAM_DECL) {
	__global Texture *todoTex[TEXTURE_STACK_SIZE];
	uint todoTexSize = 0;

	__global Texture *pendingTex[TEXTURE_STACK_SIZE];
	uint targetTexCount[TEXTURE_STACK_SIZE];
	uint pendingTexSize = 0;

	float texValues[TEXTURE_STACK_SIZE];
	uint texValuesSize = 0;

	const uint subTexCount = Texture_AddSubTexture(texture, todoTex, &todoTexSize
			TEXTURES_PARAM);
	if (subTexCount == 0) {
		// A fast path for evaluating non recursive textures
		Texture_EvaluateFloat(texture, hitPoint, texValues, &texValuesSize
			IMAGEMAPS_PARAM);
	} else {
		// Normal complex path for evaluating non recursive textures
		pendingTex[pendingTexSize] = texture;
		targetTexCount[pendingTexSize++] = subTexCount;
		do {
			if ((pendingTexSize > 0) && (texValuesSize == targetTexCount[pendingTexSize - 1])) {
				// Pop the a texture to do
				__global Texture *tex = pendingTex[--pendingTexSize];

				Texture_EvaluateFloat(tex, hitPoint, texValues, &texValuesSize
						IMAGEMAPS_PARAM);
				continue;
			}

			if (todoTexSize > 0) {
				// Pop the a texture to do
				__global Texture *tex = todoTex[--todoTexSize];

				// Add this texture to the list of pending one
				const uint subTexCount = Texture_AddSubTexture(tex, todoTex, &todoTexSize
						TEXTURES_PARAM);
				pendingTex[pendingTexSize] = tex;
				targetTexCount[pendingTexSize++] = subTexCount + texValuesSize;
			}
		} while ((todoTexSize > 0) || (pendingTexSize > 0));
	}

	return texValues[0];
}

//------------------------------------------------------------------------------
// Color texture channel
//------------------------------------------------------------------------------

void Texture_EvaluateSpectrum(__global Texture *texture, __global HitPoint *hitPoint,
		float3 texValues[TEXTURE_STACK_SIZE], uint *texValuesSize
		IMAGEMAPS_PARAM_DECL) {
	switch (texture->type) {
#if defined(PARAM_ENABLE_TEX_CONST_FLOAT)
		case CONST_FLOAT:
			ConstFloatTexture_EvaluateSpectrum(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
#if defined(PARAM_ENABLE_TEX_CONST_FLOAT3)
		case CONST_FLOAT3:
			ConstFloat3Texture_EvaluateSpectrum(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
#if defined(PARAM_ENABLE_TEX_IMAGEMAP)
		case IMAGEMAP:
			ImageMapTexture_EvaluateSpectrum(texture, hitPoint, texValues, texValuesSize
					IMAGEMAPS_PARAM);
			break;
#endif
#if defined(PARAM_ENABLE_TEX_SCALE)
		case SCALE_TEX:
			ScaleTexture_EvaluateSpectrum(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
#if defined (PARAM_ENABLE_FRESNEL_APPROX_N)
		case FRESNEL_APPROX_K:
			FresnelApproxNTexture_EvaluateSpectrum(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
#if defined (PARAM_ENABLE_FRESNEL_APPROX_K)
		case FRESNEL_APPROX_K:
			FresnelApproxKTexture_EvaluateSpectrum(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
#if defined (PARAM_ENABLE_CHECKERBOARD2D)
		case CHECKERBOARD2D:
			CheckerBoard2DTexture_EvaluateSpectrum(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
#if defined (PARAM_ENABLE_CHECKERBOARD3D)
		case CHECKERBOARD3D:
			CheckerBoard3DTexture_EvaluateSpectrum(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
#if defined (PARAM_ENABLE_MIX_TEX)
		case MIX_TEX:
			MixTexture_EvaluateSpectrum(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
#if defined (PARAM_ENABLE_FBM_TEX)
		case FBM_TEX:
			FBMTexture_EvaluateSpectrum(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
#if defined (PARAM_ENABLE_MARBLE)
		case MARBLE:
			MarbleTexture_EvaluateSpectrum(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
		default:
			// Do nothing
			break;
	}
}

float3 Texture_GetSpectrumValue(__global Texture *texture, __global HitPoint *hitPoint
		TEXTURES_PARAM_DECL) {
	__global Texture *todoTex[TEXTURE_STACK_SIZE];
	uint todoTexSize = 0;

	__global Texture *pendingTex[TEXTURE_STACK_SIZE];
	uint targetTexCount[TEXTURE_STACK_SIZE];
	uint pendingTexSize = 0;

	float3 texValues[TEXTURE_STACK_SIZE];
	uint texValuesSize = 0;

	const uint subTexCount = Texture_AddSubTexture(texture, todoTex, &todoTexSize
			TEXTURES_PARAM);
	if (subTexCount == 0) {
		// A fast path for evaluating non recursive textures
		Texture_EvaluateSpectrum(texture, hitPoint, texValues, &texValuesSize
			IMAGEMAPS_PARAM);
	} else {
		// Normal complex path for evaluating non recursive textures
		pendingTex[pendingTexSize] = texture;
		targetTexCount[pendingTexSize++] = subTexCount;
		do {
			if ((pendingTexSize > 0) && (texValuesSize == targetTexCount[pendingTexSize - 1])) {
				// Pop the a texture to do
				__global Texture *tex = pendingTex[--pendingTexSize];

				Texture_EvaluateSpectrum(tex, hitPoint, texValues, &texValuesSize
						IMAGEMAPS_PARAM);
				continue;
			}

			if (todoTexSize > 0) {
				// Pop the a texture to do
				__global Texture *tex = todoTex[--todoTexSize];

				// Add this texture to the list of pending one
				const uint subTexCount = Texture_AddSubTexture(tex, todoTex, &todoTexSize
						TEXTURES_PARAM);
				pendingTex[pendingTexSize] = tex;
				targetTexCount[pendingTexSize++] = subTexCount + texValuesSize;
			}
		} while ((todoTexSize > 0) || (pendingTexSize > 0));
	}

	return texValues[0];
}

//------------------------------------------------------------------------------
// DuDv texture information
//------------------------------------------------------------------------------

void Texture_EvaluateDuDv(__global Texture *texture, __global HitPoint *hitPoint,
		float2 texValues[TEXTURE_STACK_SIZE], uint *texValuesSize
		IMAGEMAPS_PARAM_DECL) {
	switch (texture->type) {
#if defined(PARAM_ENABLE_TEX_CONST_FLOAT)
		case CONST_FLOAT:
			ConstFloatTexture_EvaluateDuDv(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
#if defined(PARAM_ENABLE_TEX_CONST_FLOAT3)
		case CONST_FLOAT3:
			ConstFloat3Texture_EvaluateDuDv(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
#if defined(PARAM_ENABLE_TEX_IMAGEMAP)
		case IMAGEMAP:
			ImageMapTexture_EvaluateDuDv(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
#if defined(PARAM_ENABLE_TEX_SCALE)
		case SCALE_TEX:
			ScaleTexture_EvaluateDuDv(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
#if defined (PARAM_ENABLE_FRESNEL_APPROX_N)
		case FRESNEL_APPROX_K:
			FresnelApproxNTexture_EvaluateDuDv(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
#if defined (PARAM_ENABLE_FRESNEL_APPROX_K)
		case FRESNEL_APPROX_K:
			FresnelApproxKTexture_EvaluateDuDv(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
#if defined (PARAM_ENABLE_CHECKERBOARD2D)
		case CHECKERBOARD2D:
			CheckerBoard2DTexture_EvaluateDuDv(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
#if defined (PARAM_ENABLE_CHECKERBOARD3D)
		case CHECKERBOARD3D:
			CheckerBoard3DTexture_EvaluateDuDv(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
#if defined (PARAM_ENABLE_MIX_TEX)
		case MIX_TEX:
			MixTexture_EvaluateDuDv(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
#if defined (PARAM_ENABLE_FBM_TEX)
		case FBM_TEX:
			FBMTexture_EvaluateDuDv(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
#if defined (PARAM_ENABLE_MARBLE)
		case MARBLE:
			MarbleTexture_EvaluateDuDv(texture, hitPoint, texValues, texValuesSize);
			break;
#endif
		default:
			// Do nothing
			break;
	}
}

float2 Texture_GetDuDv(__global Texture *texture, __global HitPoint *hitPoint
		TEXTURES_PARAM_DECL) {
	__global Texture *todoTex[TEXTURE_STACK_SIZE];
	uint todoTexSize = 0;

	__global Texture *pendingTex[TEXTURE_STACK_SIZE];
	uint targetTexCount[TEXTURE_STACK_SIZE];
	uint pendingTexSize = 0;

	float2 texValues[TEXTURE_STACK_SIZE];
	uint texValuesSize = 0;

	const uint subTexCount = Texture_AddSubTexture(texture, todoTex, &todoTexSize
			TEXTURES_PARAM);
	if (subTexCount == 0) {
		// A fast path for evaluating non recursive textures
		Texture_EvaluateDuDv(texture, hitPoint, texValues, &texValuesSize
			IMAGEMAPS_PARAM);
	} else {
		// Normal complex path for evaluating non recursive textures
		pendingTex[pendingTexSize] = texture;
		targetTexCount[pendingTexSize++] = subTexCount;
		do {
			if ((pendingTexSize > 0) && (texValuesSize == targetTexCount[pendingTexSize - 1])) {
				// Pop the a texture to do
				__global Texture *tex = pendingTex[--pendingTexSize];

				Texture_EvaluateDuDv(tex, hitPoint, texValues, &texValuesSize
						IMAGEMAPS_PARAM);
				continue;
			}

			if (todoTexSize > 0) {
				// Pop the a texture to do
				__global Texture *tex = todoTex[--todoTexSize];

				// Add this texture to the list of pending one
				const uint subTexCount = Texture_AddSubTexture(tex, todoTex, &todoTexSize
						TEXTURES_PARAM);
				pendingTex[pendingTexSize] = tex;
				targetTexCount[pendingTexSize++] = subTexCount + texValuesSize;
			}
		} while ((todoTexSize > 0) || (pendingTexSize > 0));
	}

	return texValues[0];
}
