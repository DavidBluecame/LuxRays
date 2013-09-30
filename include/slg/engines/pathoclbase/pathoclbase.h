/***************************************************************************
 *   Copyright (C) 1998-2013 by authors (see AUTHORS.txt)                  *
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

#ifndef _SLG_PATHOCLBASE_H
#define	_SLG_PATHOCLBASE_H

#if !defined(LUXRAYS_DISABLE_OPENCL)

#include "luxrays/core/intersectiondevice.h"
#include "luxrays/utils/ocl.h"

#include "slg/slg.h"
#include "slg/renderengine.h"
#include "slg/engines/pathoclbase/compiledscene.h"

#include <boost/thread/thread.hpp>

namespace slg {

class PathOCLBaseRenderEngine;

//------------------------------------------------------------------------------
// Path Tracing GPU-only render threads
// (base class for all types of OCL path tracers)
//------------------------------------------------------------------------------

class PathOCLBaseRenderThread {
public:
	PathOCLBaseRenderThread(const u_int index, luxrays::OpenCLIntersectionDevice *device,
			PathOCLBaseRenderEngine *re);
	virtual ~PathOCLBaseRenderThread();

	virtual void Start();
	virtual void Interrupt();
	virtual void Stop();

	virtual void BeginEdit();
	virtual void EndEdit(const EditActionList &editActions);

	friend class PathOCLBaseRenderEngine;

protected:
	virtual void RenderThreadImpl() = 0;
	virtual void GetThreadFilmSize(u_int *filmWidth, u_int *filmHeight) = 0;
	virtual void AdditionalInit() = 0;
	virtual std::string AdditionalKernelOptions() = 0;
	virtual std::string AdditionalKernelDefinitions() { return ""; }
	virtual std::string AdditionalKernelSources() = 0;
	virtual void SetAdditionalKernelArgs() = 0;
	virtual void CompileAdditionalKernels(cl::Program *program) = 0;

	void AllocOCLBufferRO(cl::Buffer **buff, void *src, const size_t size, const std::string &desc);
	void AllocOCLBufferRW(cl::Buffer **buff, const size_t size, const std::string &desc);
	void FreeOCLBuffer(cl::Buffer **buff);

	void StartRenderThread();
	void StopRenderThread();

	void InitRender();

	void InitFilm();
	void InitCamera();
	void InitGeometry();
	void InitImageMaps();
	void InitTextures();
	void InitMaterials();
	void InitTriangleAreaLights();
	void InitInfiniteLight();
	void InitSunLight();
	void InitSkyLight();
	void InitLightsDistribution();
	void InitKernels();

	void CompileKernel(cl::Program *program, cl::Kernel **kernel, size_t *workgroupSize, const std::string &name);
	void SetKernelArgs();

	void TransferFilm(cl::CommandQueue &oclQueue);

	luxrays::OpenCLIntersectionDevice *intersectionDevice;

	// OpenCL variables
	cl::Kernel *filmClearKernel;
	size_t filmClearWorkGroupSize;

	// Film buffers
	std::vector<cl::Buffer *> channel_RADIANCE_PER_PIXEL_NORMALIZEDs_Buff;
	cl::Buffer *channel_ALPHA_Buff;
	cl::Buffer *channel_DEPTH_Buff;
	cl::Buffer *channel_POSITION_Buff;
	cl::Buffer *channel_GEOMETRY_NORMAL_Buff;
	cl::Buffer *channel_SHADING_NORMAL_Buff;
	cl::Buffer *channel_MATERIAL_ID_Buff;
	cl::Buffer *channel_DIRECT_DIFFUSE_Buff;
	cl::Buffer *channel_DIRECT_GLOSSY_Buff;
	cl::Buffer *channel_EMISSION_Buff;
	cl::Buffer *channel_INDIRECT_DIFFUSE_Buff;
	cl::Buffer *channel_INDIRECT_GLOSSY_Buff;
	cl::Buffer *channel_INDIRECT_SPECULAR_Buff;
	cl::Buffer *channel_MATERIAL_ID_MASK_Buff;
	cl::Buffer *channel_DIRECT_SHADOW_MASK_Buff;
	cl::Buffer *channel_INDIRECT_SHADOW_MASK_Buff;
	cl::Buffer *channel_UV_Buff;
	// Scene buffers
	cl::Buffer *materialsBuff;
	cl::Buffer *texturesBuff;
	cl::Buffer *meshIDBuff;
	cl::Buffer *meshDescsBuff;
	cl::Buffer *meshMatsBuff;
	cl::Buffer *infiniteLightBuff;
	cl::Buffer *infiniteLightDistributionBuff;
	cl::Buffer *sunLightBuff;
	cl::Buffer *skyLightBuff;
	cl::Buffer *lightsDistributionBuff;
	cl::Buffer *vertsBuff;
	cl::Buffer *normalsBuff;
	cl::Buffer *uvsBuff;
	cl::Buffer *colsBuff;
	cl::Buffer *alphasBuff;
	cl::Buffer *trianglesBuff;
	cl::Buffer *cameraBuff;
	cl::Buffer *triLightDefsBuff;
	cl::Buffer *meshTriLightDefsOffsetBuff;
	cl::Buffer *imageMapDescsBuff;
	vector<cl::Buffer *> imageMapsBuff;

	std::string kernelsParameters;
	luxrays::oclKernelCache *kernelCache;

	boost::thread *renderThread;

	u_int threadIndex;
	PathOCLBaseRenderEngine *renderEngine;
	Film *threadFilm;

	bool started, editMode;
};

//------------------------------------------------------------------------------
// Path Tracing 100% OpenCL render engine
// (base class for all types of OCL path tracers)
//------------------------------------------------------------------------------

class PathOCLBaseRenderEngine : public OCLRenderEngine {
public:
	PathOCLBaseRenderEngine(RenderConfig *cfg, Film *flm, boost::mutex *flmMutex,
		const bool realTime = false);
	virtual ~PathOCLBaseRenderEngine();

	virtual bool IsHorizontalStereoSupported() const {
		return true;
	}

	virtual bool IsMaterialCompiled(const MaterialType type) const {
		return (compiledScene == NULL) ? false : compiledScene->IsMaterialCompiled(type);
	}

	friend class PathOCLBaseRenderThread;

	size_t maxMemPageSize;

protected:
	virtual PathOCLBaseRenderThread *CreateOCLThread(const u_int index, luxrays::OpenCLIntersectionDevice *device) = 0;

	virtual void StartLockLess();
	virtual void StopLockLess();

	virtual void BeginEditLockLess();
	virtual void EndEditLockLess(const EditActionList &editActions);

	boost::mutex setKernelArgsMutex;

	CompiledScene *compiledScene;

	vector<PathOCLBaseRenderThread *> renderThreads;
};

}

#endif

#endif	/* _SLG_PATHOCLBASE_H */
