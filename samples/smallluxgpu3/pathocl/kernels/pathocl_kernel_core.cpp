#include "pathocl/kernels/kernels.h"
std::string luxrays::KernelSource_PathOCL_kernel_core = 
"#line 2 \"patchocl_kernel_core.cl\"\n"
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
"//------------------------------------------------------------------------------\n"
"// Random number generator\n"
"// maximally equidistributed combined Tausworthe generator\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#define FLOATMASK 0x00ffffffu\n"
"\n"
"uint TAUSWORTHE(const uint s, const uint a,\n"
"	const uint b, const uint c,\n"
"	const uint d) {\n"
"	return ((s&c)<<d) ^ (((s << a) ^ s) >> b);\n"
"}\n"
"\n"
"uint LCG(const uint x) { return x * 69069; }\n"
"\n"
"uint ValidSeed(const uint x, const uint m) {\n"
"	return (x < m) ? (x + m) : x;\n"
"}\n"
"\n"
"void InitRandomGenerator(uint seed, Seed *s) {\n"
"	// Avoid 0 value\n"
"	seed = (seed == 0) ? (seed + 0xffffffu) : seed;\n"
"\n"
"	s->s1 = ValidSeed(LCG(seed), 1);\n"
"	s->s2 = ValidSeed(LCG(s->s1), 7);\n"
"	s->s3 = ValidSeed(LCG(s->s2), 15);\n"
"}\n"
"\n"
"unsigned long RndUintValue(Seed *s) {\n"
"	s->s1 = TAUSWORTHE(s->s1, 13, 19, 4294967294UL, 12);\n"
"	s->s2 = TAUSWORTHE(s->s2, 2, 25, 4294967288UL, 4);\n"
"	s->s3 = TAUSWORTHE(s->s3, 3, 11, 4294967280UL, 17);\n"
"\n"
"	return ((s->s1) ^ (s->s2) ^ (s->s3));\n"
"}\n"
"\n"
"float RndFloatValue(Seed *s) {\n"
"	return (RndUintValue(s) & FLOATMASK) * (1.f / (FLOATMASK + 1UL));\n"
"}\n"
"\n"
"//------------------------------------------------------------------------------\n"
"\n"
"float PowerHeuristic(const uint nf, const float fPdf, const uint ng, const float gPdf) {\n"
"	const float f = nf * fPdf;\n"
"	const float g = ng * gPdf;\n"
"\n"
"	return (f * f) / (f * f + g * g);\n"
"}\n"
"\n"
"float VanDerCorput(uint n, uint scramble) {\n"
"	// Reverse bits of n\n"
"	n = (n << 16) | (n >> 16);\n"
"	n = ((n & 0x00ff00ff) << 8) | ((n & 0xff00ff00) >> 8);\n"
"	n = ((n & 0x0f0f0f0f) << 4) | ((n & 0xf0f0f0f0) >> 4);\n"
"	n = ((n & 0x33333333) << 2) | ((n & 0xcccccccc) >> 2);\n"
"	n = ((n & 0x55555555) << 1) | ((n & 0xaaaaaaaa) >> 1);\n"
"	n ^= scramble;\n"
"\n"
"	// 0.9999999403953552f = 1 - epsilon\n"
"	return min(((n >> 8) & 0xffffff) / (float)(1 << 24), 0.9999999403953552f);\n"
"}\n"
"\n"
"#if defined(PARAM_USE_PIXEL_ATOMICS)\n"
"void AtomicAdd(__global float *val, const float delta) {\n"
"	union {\n"
"		float f;\n"
"		unsigned int i;\n"
"	} oldVal;\n"
"	union {\n"
"		float f;\n"
"		unsigned int i;\n"
"	} newVal;\n"
"\n"
"	do {\n"
"		oldVal.f = *val;\n"
"		newVal.f = oldVal.f + delta;\n"
"	} while (atom_cmpxchg((__global unsigned int *)val, oldVal.i, newVal.i) != oldVal.i);\n"
"}\n"
"#endif\n"
"\n"
"float Spectrum_Y(const Spectrum *s) {\n"
"	return 0.212671f * s->r + 0.715160f * s->g + 0.072169f * s->b;\n"
"}\n"
"\n"
"float Dot(const Vector *v0, const Vector *v1) {\n"
"	return v0->x * v1->x + v0->y * v1->y + v0->z * v1->z;\n"
"}\n"
"\n"
"void Normalize(Vector *v) {\n"
"	const float il = 1.f / sqrt(Dot(v, v));\n"
"\n"
"	v->x *= il;\n"
"	v->y *= il;\n"
"	v->z *= il;\n"
"}\n"
"\n"
"void TransformPoint(__global float (*m)[4], Point *v) {\n"
"	const float x = v->x;\n"
"	const float y = v->y;\n"
"	const float z = v->z;\n"
"\n"
"	v->x = m[0][0] * x + m[0][1] * y + m[0][2] * z;\n"
"	v->y = m[1][0] * x + m[1][1] * y + m[1][2] * z;\n"
"	v->z = m[2][0] * x + m[2][1] * y + m[2][2] * z;\n"
"\n"
"	const float wp = 1.f / (m[3][0] * x + m[3][1] * y + m[3][2] * z + m[3][3]);\n"
"	v->x *= wp;\n"
"	v->y *= wp;\n"
"	v->z *= wp;\n"
"}\n"
"\n"
"void TransformVector(__global float (*m)[4], Vector *v) {\n"
"	const float x = v->x;\n"
"	const float y = v->y;\n"
"	const float z = v->z;\n"
"\n"
"	v->x = m[0][0] * x + m[0][1] * y + m[0][2] * z;\n"
"	v->y = m[1][0] * x + m[1][1] * y + m[1][2] * z;\n"
"	v->z = m[2][0] * x + m[2][1] * y + m[2][2] * z;\n"
"}\n"
"\n"
"// Matrix m must be the inverse and transpose of normal transformation\n"
"void TransformNormal(__global float (*m)[4], Vector *v) {\n"
"	const float x = v->x;\n"
"	const float y = v->y;\n"
"	const float z = v->z;\n"
"\n"
"	v->x = m[0][0] * x + m[1][0] * y + m[2][0] * z;\n"
"	v->y = m[0][1] * x + m[1][1] * y + m[2][1] * z;\n"
"	v->z = m[0][2] * x + m[1][2] * y + m[2][2] * z;\n"
"\n"
"	Normalize(v);\n"
"}\n"
"\n"
"void Cross(Vector *v3, const Vector *v1, const Vector *v2) {\n"
"	v3->x = (v1->y * v2->z) - (v1->z * v2->y);\n"
"	v3->y = (v1->z * v2->x) - (v1->x * v2->z),\n"
"	v3->z = (v1->x * v2->y) - (v1->y * v2->x);\n"
"}\n"
"\n"
"int Mod(int a, int b) {\n"
"	if (b == 0)\n"
"		b = 1;\n"
"\n"
"	a %= b;\n"
"	if (a < 0)\n"
"		a += b;\n"
"\n"
"	return a;\n"
"}\n"
"\n"
"float Lerp(float t, float v1, float v2) {\n"
"	return (1.f - t) * v1 + t * v2;\n"
"}\n"
"\n"
"void ConcentricSampleDisk(const float u1, const float u2, float *dx, float *dy) {\n"
"	float r, theta;\n"
"	// Map uniform random numbers to $[-1,1]^2$\n"
"	float sx = 2.f * u1 - 1.f;\n"
"	float sy = 2.f * u2 - 1.f;\n"
"	// Map square to $(r,\theta)$\n"
"	// Handle degeneracy at the origin\n"
"	if (sx == 0.f && sy == 0.f) {\n"
"		*dx = 0.f;\n"
"		*dy = 0.f;\n"
"		return;\n"
"	}\n"
"	if (sx >= -sy) {\n"
"		if (sx > sy) {\n"
"			// Handle first region of disk\n"
"			r = sx;\n"
"			if (sy > 0.f)\n"
"				theta = sy / r;\n"
"			else\n"
"				theta = 8.f + sy / r;\n"
"		} else {\n"
"			// Handle second region of disk\n"
"			r = sy;\n"
"			theta = 2.f - sx / r;\n"
"		}\n"
"	} else {\n"
"		if (sx <= sy) {\n"
"			// Handle third region of disk\n"
"			r = -sx;\n"
"			theta = 4.f - sy / r;\n"
"		} else {\n"
"			// Handle fourth region of disk\n"
"			r = -sy;\n"
"			theta = 6.f + sx / r;\n"
"		}\n"
"	}\n"
"	theta *= M_PI / 4.f;\n"
"	*dx = r * cos(theta);\n"
"	*dy = r * sin(theta);\n"
"}\n"
"\n"
"void CosineSampleHemisphere(Vector *ret, const float u1, const float u2) {\n"
"	ConcentricSampleDisk(u1, u2, &ret->x, &ret->y);\n"
"	ret->z = sqrt(max(0.f, 1.f - ret->x * ret->x - ret->y * ret->y));\n"
"}\n"
"\n"
"void UniformSampleCone(Vector *ret, const float u1, const float u2, const float costhetamax,\n"
"	const Vector *x, const Vector *y, const Vector *z) {\n"
"	const float costheta = Lerp(u1, costhetamax, 1.f);\n"
"	const float sintheta = sqrt(1.f - costheta * costheta);\n"
"	const float phi = u2 * 2.f * M_PI;\n"
"\n"
"	const float kx = cos(phi) * sintheta;\n"
"	const float ky = sin(phi) * sintheta;\n"
"	const float kz = costheta;\n"
"\n"
"	ret->x = kx * x->x + ky * y->x + kz * z->x;\n"
"	ret->y = kx * x->y + ky * y->y + kz * z->y;\n"
"	ret->z = kx * x->z + ky * y->z + kz * z->z;\n"
"}\n"
"\n"
"float UniformConePdf(float costhetamax) {\n"
"	return 1.f / (2.f * M_PI * (1.f - costhetamax));\n"
"}\n"
"\n"
"void CoordinateSystem(const Vector *v1, Vector *v2, Vector *v3) {\n"
"	if (fabs(v1->x) > fabs(v1->y)) {\n"
"		float invLen = 1.f / sqrt(v1->x * v1->x + v1->z * v1->z);\n"
"		v2->x = -v1->z * invLen;\n"
"		v2->y = 0.f;\n"
"		v2->z = v1->x * invLen;\n"
"	} else {\n"
"		float invLen = 1.f / sqrt(v1->y * v1->y + v1->z * v1->z);\n"
"		v2->x = 0.f;\n"
"		v2->y = v1->z * invLen;\n"
"		v2->z = -v1->y * invLen;\n"
"	}\n"
"\n"
"	Cross(v3, v1, v2);\n"
"}\n"
"\n"
"float SphericalTheta(const Vector *v) {\n"
"	return acos(clamp(v->z, -1.f, 1.f));\n"
"}\n"
"\n"
"float SphericalPhi(const Vector *v) {\n"
"	float p = atan2(v->y, v->x);\n"
"	return (p < 0.f) ? p + 2.f * M_PI : p;\n"
"}\n"
;
