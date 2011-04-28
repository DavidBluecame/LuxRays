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

#ifndef PATHOCL_H
#define	PATHOCL_H

#if !defined(LUXRAYS_DISABLE_OPENCL)

#include "smalllux.h"
#include "renderengine.h"
#include "ocldatatypes.h"

#include "luxrays/core/intersectiondevice.h"

#include <boost/thread/thread.hpp>

class PathOCLRenderEngine;

//------------------------------------------------------------------------------
// Path Tracing GPU-only render threads
//------------------------------------------------------------------------------

class PathOCLRenderThread {
public:
	PathOCLRenderThread(const unsigned int index, const unsigned int seedBase,
			const float samplingStart, OpenCLIntersectionDevice *device,
			PathOCLRenderEngine *re);
	~PathOCLRenderThread();

	void Start();
    void Interrupt();
	void Stop();

	void BeginEdit();
	void EndEdit(const EditActionList &editActions);

	friend class PathOCLRenderEngine;

private:
	static void RenderThreadImpl(PathOCLRenderThread *renderThread);

	void StartRenderThread();
	void StopRenderThread();

	void InitRender();
	void InitFrameBuffer();
	void InitCamera();
	void InitRenderGeometry();
	void InitKernels();

	void SetKernelArgs();

	OpenCLIntersectionDevice *intersectionDevice;

	// OpenCL variables
	string kernelsParameters;
	cl::Kernel *initKernel;
	size_t initWorkGroupSize;
	cl::Kernel *initFBKernel;
	size_t initFBWorkGroupSize;
	cl::Kernel *samplerKernel;
	size_t samplerWorkGroupSize;
	cl::Kernel *advancePathsKernel;
	size_t advancePathsWorkGroupSize;

	cl::Buffer *raysBuff;
	cl::Buffer *hitsBuff;
	cl::Buffer *tasksBuff;
	cl::Buffer *taskStatsBuff;
	cl::Buffer *frameBufferBuff;
	cl::Buffer *materialsBuff;
	cl::Buffer *meshIDBuff;
	cl::Buffer *triangleIDBuff;
	cl::Buffer *meshDescsBuff;
	cl::Buffer *meshMatsBuff;
	cl::Buffer *infiniteLightBuff;
	cl::Buffer *infiniteLightMapBuff;
	cl::Buffer *sunLightBuff;
	cl::Buffer *skyLightBuff;
	cl::Buffer *vertsBuff;
	cl::Buffer *normalsBuff;
	cl::Buffer *trianglesBuff;
	cl::Buffer *colorsBuff;
	cl::Buffer *cameraBuff;
	cl::Buffer *triLightsBuff;
	cl::Buffer *texMapRGBBuff;
	cl::Buffer *texMapAlphaBuff;
	cl::Buffer *texMapDescBuff;
	cl::Buffer *meshTexsBuff;
	cl::Buffer *meshBumpsBuff;
	cl::Buffer *meshBumpsScaleBuff;
	cl::Buffer *uvsBuff;

	float samplingStart;
	unsigned int seed;

	boost::thread *renderThread;

	unsigned int threadIndex;
	PathOCLRenderEngine *renderEngine;
	PathOCL::Pixel *frameBuffer;

	// TODO: cleanup
	unsigned int frameBufferPixelCount;
	size_t stratifiedDataSize;
	unsigned int areaLightCount;
	bool enable_MAT_MATTE, enable_MAT_AREALIGHT, enable_MAT_MIRROR, enable_MAT_GLASS;
	bool enable_MAT_MATTEMIRROR, enable_MAT_METAL, enable_MAT_MATTEMETAL, enable_MAT_ALLOY;
	bool enable_MAT_ARCHGLASS;

	bool started, editMode, reportedPermissionError;

	PathOCL::GPUTaskStats *gpuTaskStats;
};

//------------------------------------------------------------------------------
// Path Tracing 100% OpenCL render engine
//------------------------------------------------------------------------------

class PathOCLRenderEngine : public OCLRenderEngine {
public:
	PathOCLRenderEngine(RenderConfig *cfg, Film *flm, boost::mutex *flmMutex);
	virtual ~PathOCLRenderEngine();

	void Start();
	void Stop();

	void BeginEdit();
	void EndEdit(const EditActionList &editActions);

	void UpdateFilm();

	unsigned int GetPass() const;
	RenderEngineType GetEngineType() const { return PATHOCL; }
	double GetTotalSamplesSec() const {
		return (elapsedTime == 0.0) ? 0.0 : (samplesCount / elapsedTime);
	}

	friend class PathOCLRenderThread;

	// Signed because of the delta parameter
	int maxPathDepth;

	int rrDepth;
	float rrImportanceCap;

private:
	void UpdateFilmLockLess();

	mutable boost::mutex engineMutex;

	vector<PathOCLRenderThread *> renderThreads;
	SampleBuffer *sampleBuffer;

	double startTime;
	double elapsedTime;
	unsigned long long samplesCount;

	PathOCL::Sampler *sampler;
	PathOCL::Filter *filter;

	unsigned int taskCount;

	bool usePixelAtomics;
};

#endif

#endif	/* PATHOCL_H */