/***************************************************************************
 * Copyright 1998-2013 by authors (see AUTHORS.txt)                        *
 *                                                                         *
 *   This file is part of LuxRender.                                       *
 *                                                                         *
 * Licensed under the Apache License, Version 2.0 (the "License");         *
 * you may not use this file except in compliance with the License.        *
 * You may obtain a copy of the License at                                 *
 *                                                                         *
 *     http://www.apache.org/licenses/LICENSE-2.0                          *
 *                                                                         *
 * Unless required by applicable law or agreed to in writing, software     *
 * distributed under the License is distributed on an "AS IS" BASIS,       *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*
 * See the License for the specific language governing permissions and     *
 * limitations under the License.                                          *
 ***************************************************************************/

#include "slg/lights/trianglelight.h"

using namespace std;
using namespace luxrays;
using namespace slg;

//------------------------------------------------------------------------------
// Triangle Area Light
//------------------------------------------------------------------------------

TriangleLight::TriangleLight() : mesh(NULL), triangleIndex(NULL_INDEX),
		triangleArea(0.f), invTriangleArea(0.f),
		meshArea(0.f), invMeshArea(0.f){
}

TriangleLight::~TriangleLight() {
}

float TriangleLight::GetPower(const Scene &scene) const {
	return triangleArea * M_PI * lightMaterial->GetEmittedRadianceY();
}

void TriangleLight::Preprocess() {
	triangleArea = mesh->GetTriangleArea(0.f, triangleIndex);
	invTriangleArea = 1.f / triangleArea;

	meshArea = mesh->GetMeshArea(0.f);
	invMeshArea = 1.f / meshArea;
}

Spectrum TriangleLight::Emit(const Scene &scene,
		const float u0, const float u1, const float u2, const float u3, const float passThroughEvent,
		Point *orig, Vector *dir,
		float *emissionPdfW, float *directPdfA, float *cosThetaAtLight) const {
	// Origin
	float b0, b1, b2;
	mesh->Sample(0.f, triangleIndex, u0, u1, orig, &b0, &b1, &b2);

	// Build the local frame
	const Normal N = mesh->GetGeometryNormal(0.f, triangleIndex); // Light sources are supposed to be flat
	Frame frame(N);

	Spectrum emissionColor(1.f);
	Vector localDirOut;
	const SampleableSphericalFunction *emissionFunc = lightMaterial->GetEmissionFunc();
	if (emissionFunc) {
		emissionFunc->Sample(u2, u3, &localDirOut, emissionPdfW);
		emissionColor = ((SphericalFunction *)emissionFunc)->Evaluate(localDirOut) / emissionFunc->Average();
	} else
		localDirOut = CosineSampleHemisphere(u2, u3, emissionPdfW);

	if (*emissionPdfW == 0.f)
			return Spectrum();
	*emissionPdfW *= invTriangleArea;

	// Cannot really not emit the particle, so just bias it to the correct angle
	localDirOut.z = Max(localDirOut.z, DEFAULT_COS_EPSILON_STATIC);

	// Direction
	*dir = frame.ToWorld(localDirOut);

	if (directPdfA)
		*directPdfA = invTriangleArea;

	if (cosThetaAtLight)
		*cosThetaAtLight = localDirOut.z;

	const UV triUV = mesh->InterpolateTriUV(triangleIndex, b1, b2);
	const Spectrum color = mesh->InterpolateTriColor(triangleIndex, b1, b2);
	const float alpha = mesh->InterpolateTriAlpha(triangleIndex, b1, b2);
	const HitPoint hitPoint = { Vector(-N), *orig, triUV, N, N,
		color, alpha, passThroughEvent, NULL, NULL, false, false };

	return lightMaterial->GetEmittedRadiance(hitPoint, invMeshArea) * localDirOut.z;
}

Spectrum TriangleLight::Illuminate(const Scene &scene, const Point &p,
		const float u0, const float u1, const float passThroughEvent,
        Vector *dir, float *distance, float *directPdfW,
		float *emissionPdfW, float *cosThetaAtLight) const {
	Point samplePoint;
	float b0, b1, b2;
	mesh->Sample(0.f, triangleIndex, u0, u1, &samplePoint, &b0, &b1, &b2);
	const Normal &sampleN = mesh->GetGeometryNormal(0.f, triangleIndex); // Light sources are supposed to be flat

	*dir = samplePoint - p;
	const float distanceSquared = dir->LengthSquared();
	*distance = sqrtf(distanceSquared);
	*dir /= (*distance);

	const float cosAtLight = Dot(sampleN, -(*dir));
	if (cosAtLight < DEFAULT_COS_EPSILON_STATIC)
		return Spectrum();

	if (cosThetaAtLight)
		*cosThetaAtLight = cosAtLight;

	Spectrum emissionColor(1.f);
	const SampleableSphericalFunction *emissionFunc = lightMaterial->GetEmissionFunc();
	if (emissionFunc) {
		// Build the local frame
		const Normal N = mesh->GetGeometryNormal(0.f, triangleIndex); // Light sources are supposed to be flat
		Frame frame(N);

		const Vector localFromLight = Normalize(frame.ToLocal(-(*dir)));
		
		if (emissionPdfW) {
			const float emissionFuncPdf = emissionFunc->Pdf(localFromLight);
			if (emissionFuncPdf == 0.f)
				return Spectrum();
			*emissionPdfW = emissionFuncPdf * invTriangleArea;
		}
		emissionColor = ((SphericalFunction *)emissionFunc)->Evaluate(localFromLight) / emissionFunc->Average();
		
		*directPdfW = invTriangleArea * distanceSquared;
	} else {
		if (emissionPdfW)
			*emissionPdfW = invTriangleArea * cosAtLight * INV_PI;

		*directPdfW = invTriangleArea * distanceSquared / cosAtLight;
	}

	const UV triUV = mesh->InterpolateTriUV(triangleIndex, b1, b2);
	const Spectrum color = mesh->InterpolateTriColor(triangleIndex, b1, b2);
	const float alpha = mesh->InterpolateTriAlpha(triangleIndex, b1, b2);
	const HitPoint hitPoint = { Vector(-sampleN), samplePoint, triUV, sampleN, sampleN,
		color, alpha, passThroughEvent, NULL, NULL, false, false };

	return lightMaterial->GetEmittedRadiance(hitPoint, invMeshArea) * emissionColor;
}

Spectrum TriangleLight::GetRadiance(const HitPoint &hitPoint,
		float *directPdfA,
		float *emissionPdfW) const {
	const float cosOutLight = Dot(hitPoint.geometryN, hitPoint.fixedDir);
	if (cosOutLight <= 0.f)
		return Spectrum();

	if (directPdfA)
		*directPdfA = invTriangleArea;

	Spectrum emissionColor(1.f);
	const SampleableSphericalFunction *emissionFunc = lightMaterial->GetEmissionFunc();
	if (emissionFunc) {
		// Build the local frame
		const Normal N = mesh->GetGeometryNormal(0.f, triangleIndex); // Light sources are supposed to be flat
		Frame frame(N);

		const Vector localFromLight = Normalize(frame.ToLocal(hitPoint.fixedDir));
		
		if (emissionPdfW) {
			const float emissionFuncPdf = emissionFunc->Pdf(localFromLight);
			if (emissionFuncPdf == 0.f)
				return Spectrum();
			*emissionPdfW = emissionFuncPdf * invTriangleArea;
		}
		emissionColor = ((SphericalFunction *)emissionFunc)->Evaluate(localFromLight) / emissionFunc->Average();
	} else {
		if (emissionPdfW)
			*emissionPdfW = invTriangleArea * cosOutLight * INV_PI;
	}

	return lightMaterial->GetEmittedRadiance(hitPoint, invMeshArea) * emissionColor;
}