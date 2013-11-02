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

#ifndef _SLG_BIASPATHCPU_H
#define	_SLG_BIASPATHCPU_H

#include "luxrays/core/randomgen.h"
#include "slg/slg.h"
#include "slg/renderengine.h"
#include "slg/sampler/sampler.h"
#include "slg/film/film.h"
#include "slg/sdl/bsdf.h"

namespace slg {

//------------------------------------------------------------------------------
// Biased path tracing CPU render engine
//------------------------------------------------------------------------------

class PathDepthInfo {
public:
	PathDepthInfo();
	~PathDepthInfo() { }

	void IncDepths(const BSDFEvent event);

	bool CheckDepths(const PathDepthInfo &maxPathDepth) const;

	u_int depth, diffuseDepth, glossyDepth, specularDepth;
};

class BiasPathCPURenderEngine;

class BiasPathCPURenderThread : public CPUTileRenderThread {
public:
	BiasPathCPURenderThread(BiasPathCPURenderEngine *engine, const u_int index,
			luxrays::IntersectionDevice *device);

	friend class BiasPathCPURenderEngine;

private:
	virtual boost::thread *AllocRenderThread() { return new boost::thread(&BiasPathCPURenderThread::RenderFunc, this); }

	void RenderFunc();

	void SampleGrid(luxrays::RandomGenerator *rndGen, const u_int size,
		const u_int ix, const u_int iy, float *u0, float *u1) const;

	bool DirectLightSampling(const bool firstPathVertex,
		const BSDFEvent pathBSDFEvent,
		const LightSource *light, const float lightPickPdf,
		const float u0, const float u1,
		const float u2, const float u3,
		const luxrays::Spectrum &pathThrouput, const BSDF &bsdf,
		SampleResult *sampleResult);
	bool DirectLightSamplingONE(luxrays::RandomGenerator *rndGen,
		const bool firstPathVertex,	const BSDFEvent pathBSDFEvent,
		const luxrays::Spectrum &pathThrouput, const BSDF &bsdf,
		SampleResult *sampleResult);
	void DirectLightSamplingALL(luxrays::RandomGenerator *rndGen,
		const luxrays::Spectrum &pathThrouput, const BSDF &bsdf,
		SampleResult *sampleResult);

	void AddEmission(const bool firstPathVertex, const BSDFEvent pathBSDFEvent, const u_int lightID,
		SampleResult *sampleResult, const luxrays::Spectrum &emission) const;
	bool DirectHitFiniteLight(const bool firstPathVertex, const BSDFEvent lastBSDFEvent,
		const BSDFEvent pathBSDFEvent, const luxrays::Spectrum &pathThrouput,
		const float distance, const BSDF &bsdf, const float lastPdfW,
		SampleResult *sampleResult);
	bool DirectHitInfiniteLight(const bool firstPathVertex, const BSDFEvent lastBSDFEvent,
		const BSDFEvent pathBSDFEvent, const luxrays::Spectrum &pathThrouput,
		const luxrays::Vector &eyeDir, const float lastPdfW,
		SampleResult *sampleResult);

	bool ContinueTracePath(luxrays::RandomGenerator *rndGen, PathDepthInfo depthInfo, luxrays::Ray ray,
		luxrays::Spectrum pathThrouput, BSDFEvent lastBSDFEvent, float lastPdfW,
		SampleResult *sampleResult);
	// NOTE: bsdf.hitPoint.passThroughEvent is modified by this method
	void SampleComponent(luxrays::RandomGenerator *rndGen, const BSDFEvent requestedEventTypes,
		const u_int size, const luxrays::Spectrum &pathThrouput, BSDF &bsdf,
		SampleResult *sampleResult);
	void TraceEyePath(luxrays::RandomGenerator *rndGen, const luxrays::Ray &ray,
		SampleResult *sampleResult);
	void RenderPixelSample(luxrays::RandomGenerator *rndGen,
		const u_int x, const u_int y,
		const u_int xOffset, const u_int yOffset,
		const u_int sampleX, const u_int sampleY);
};

class BiasPathCPURenderEngine : public CPUTileRenderEngine {
public:
	BiasPathCPURenderEngine(const RenderConfig *cfg, Film *flm, boost::mutex *flmMutex);
	~BiasPathCPURenderEngine();

	RenderEngineType GetEngineType() const { return BIASPATHCPU; }

	// Path depth settings
	PathDepthInfo maxPathDepth;

	// Samples settings
	u_int aaSamples, diffuseSamples, glossySamples, specularSamples, directLightSamples;

	// Clamping settings
	float radianceClampMaxValue;
	float pdfClampValue;

	// Light settings
	float lowLightThreashold, nearStartLight;
	bool lightSamplingStrategyONE;

	friend class BiasPathCPURenderThread;

protected:
	virtual void StartLockLess();

	FilterDistribution *pixelFilterDistribution;

private:
	CPURenderThread *NewRenderThread(const u_int index,
			luxrays::IntersectionDevice *device) {
		return new BiasPathCPURenderThread(this, index, device);
	}

	void InitPixelFilterDistribution();
};

}

#endif	/* _SLG_BIASPATHCPU_H */