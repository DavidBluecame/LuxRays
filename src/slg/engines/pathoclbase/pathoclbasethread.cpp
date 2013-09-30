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

#if !defined(LUXRAYS_DISABLE_OPENCL)

#include <boost/lexical_cast.hpp>

#include "luxrays/core/geometry/transform.h"
#include "luxrays/utils/ocl.h"
#include "luxrays/core/oclintersectiondevice.h"
#include "luxrays/kernels/kernels.h"

#include "slg/slg.h"
#include "slg/kernels/kernels.h"
#include "slg/renderconfig.h"
#include "slg/engines/pathoclbase/pathoclbase.h"

using namespace std;
using namespace luxrays;
using namespace slg;

//------------------------------------------------------------------------------
// PathOCLBaseRenderThread
//------------------------------------------------------------------------------

PathOCLBaseRenderThread::PathOCLBaseRenderThread(const u_int index,
		OpenCLIntersectionDevice *device, PathOCLBaseRenderEngine *re) {
	intersectionDevice = device;

	renderThread = NULL;

	threadIndex = index;
	renderEngine = re;
	started = false;
	editMode = false;
	threadFilm = NULL;

	filmClearKernel = NULL;

	// Film buffers
	channel_ALPHA_Buff = NULL;
	channel_DEPTH_Buff = NULL;
	channel_POSITION_Buff = NULL;
	channel_GEOMETRY_NORMAL_Buff = NULL;
	channel_SHADING_NORMAL_Buff = NULL;
	channel_MATERIAL_ID_Buff = NULL;
	channel_DIRECT_DIFFUSE_Buff = NULL;
	channel_DIRECT_GLOSSY_Buff = NULL;
	channel_EMISSION_Buff = NULL;
	channel_INDIRECT_DIFFUSE_Buff = NULL;
	channel_INDIRECT_GLOSSY_Buff = NULL;
	channel_INDIRECT_SPECULAR_Buff = NULL;
	channel_MATERIAL_ID_MASK_Buff = NULL;
	channel_DIRECT_SHADOW_MASK_Buff = NULL;
	channel_INDIRECT_SHADOW_MASK_Buff = NULL;
	channel_UV_Buff = NULL;

	// Scene buffers
	materialsBuff = NULL;
	texturesBuff = NULL;
	meshDescsBuff = NULL;
	meshMatsBuff = NULL;
	infiniteLightBuff = NULL;
	infiniteLightDistributionBuff = NULL;
	sunLightBuff = NULL;
	skyLightBuff = NULL;
	lightsDistributionBuff = NULL;
	vertsBuff = NULL;
	normalsBuff = NULL;
	uvsBuff = NULL;
	colsBuff = NULL;
	alphasBuff = NULL;
	trianglesBuff = NULL;
	cameraBuff = NULL;
	triLightDefsBuff = NULL;
	meshTriLightDefsOffsetBuff = NULL;
	imageMapDescsBuff = NULL;

	// Check the kind of kernel cache to use
	string type = renderEngine->renderConfig->cfg.GetString("opencl.kernelcache", "PERSISTENT");
	if (type == "PERSISTENT")
		kernelCache = new oclKernelPersistentCache("SLG_" SLG_VERSION_MAJOR "." SLG_VERSION_MINOR);
	else if (type == "VOLATILE")
		kernelCache = new oclKernelVolatileCache();
	else if (type == "NONE")
		kernelCache = new oclKernelDummyCache();
	else
		throw runtime_error("Unknown opencl.kernelcache type: " + type);
}

PathOCLBaseRenderThread::~PathOCLBaseRenderThread() {
	if (editMode)
		EndEdit(EditActionList());
	if (started)
		Stop();

	delete filmClearKernel;
	delete threadFilm;
	delete kernelCache;
}

void PathOCLBaseRenderThread::AllocOCLBufferRO(cl::Buffer **buff, void *src, const size_t size, const string &desc) {
	// Check if the buffer is too big
	if (intersectionDevice->GetDeviceDesc()->GetMaxMemoryAllocSize() < size) {
		stringstream ss;
		ss << "The " << desc << " buffer is too big for " << intersectionDevice->GetName() << 
				" device (i.e. CL_DEVICE_MAX_MEM_ALLOC_SIZE=" <<
				intersectionDevice->GetDeviceDesc()->GetMaxMemoryAllocSize() <<
				"): try to reduce related parameters";
		throw runtime_error(ss.str());
	}

	if (*buff) {
		// Check the size of the already allocated buffer

		if (size == (*buff)->getInfo<CL_MEM_SIZE>()) {
			// I can reuse the buffer; just update the content

			//SLG_LOG("[PathOCLBaseRenderThread::" << threadIndex << "] " << desc << " buffer updated for size: " << (size / 1024) << "Kbytes");
			cl::CommandQueue &oclQueue = intersectionDevice->GetOpenCLQueue();
			oclQueue.enqueueWriteBuffer(**buff, CL_FALSE, 0, size, src);
			return;
		} else {
			// Free the buffer
			intersectionDevice->FreeMemory((*buff)->getInfo<CL_MEM_SIZE>());
			delete *buff;
		}
	}

	cl::Context &oclContext = intersectionDevice->GetOpenCLContext();

	SLG_LOG("[PathOCLBaseRenderThread::" << threadIndex << "] " << desc << " buffer size: " <<
			(size < 10000 ? size : (size / 1024)) << (size < 10000 ? "bytes" : "Kbytes"));
	*buff = new cl::Buffer(oclContext,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			size, src);
	intersectionDevice->AllocMemory((*buff)->getInfo<CL_MEM_SIZE>());
}

void PathOCLBaseRenderThread::AllocOCLBufferRW(cl::Buffer **buff, const size_t size, const string &desc) {
	// Check if the buffer is too big
	if (intersectionDevice->GetDeviceDesc()->GetMaxMemoryAllocSize() < size) {
		stringstream ss;
		ss << "The " << desc << " buffer is too big for " << intersectionDevice->GetName() << 
				" device (i.e. CL_DEVICE_MAX_MEM_ALLOC_SIZE=" <<
				intersectionDevice->GetDeviceDesc()->GetMaxMemoryAllocSize() <<
				"): try to reduce related parameters";
		throw runtime_error(ss.str());
	}

	if (*buff) {
		// Check the size of the already allocated buffer

		if (size == (*buff)->getInfo<CL_MEM_SIZE>()) {
			// I can reuse the buffer
			//SLG_LOG("[PathOCLBaseRenderThread::" << threadIndex << "] " << desc << " buffer reused for size: " << (size / 1024) << "Kbytes");
			return;
		} else {
			// Free the buffer
			intersectionDevice->FreeMemory((*buff)->getInfo<CL_MEM_SIZE>());
			delete *buff;
		}
	}

	cl::Context &oclContext = intersectionDevice->GetOpenCLContext();
 
	SLG_LOG("[PathOCLBaseRenderThread::" << threadIndex << "] " << desc << " buffer size: " <<
			(size < 10000 ? size : (size / 1024)) << (size < 10000 ? "bytes" : "Kbytes"));
	*buff = new cl::Buffer(oclContext,
			CL_MEM_READ_WRITE,
			size);
	intersectionDevice->AllocMemory((*buff)->getInfo<CL_MEM_SIZE>());
}

void PathOCLBaseRenderThread::FreeOCLBuffer(cl::Buffer **buff) {
	if (*buff) {

		intersectionDevice->FreeMemory((*buff)->getInfo<CL_MEM_SIZE>());
		delete *buff;
		*buff = NULL;
	}
}

void PathOCLBaseRenderThread::InitFilm() {
	const Film *engineFilm = renderEngine->film;
	u_int filmWidth, filmHeight;
	GetThreadFilmSize(&filmWidth, &filmHeight);
	const u_int filmPixelCount = filmWidth * filmHeight;

	// Delete previous allocated Film
	delete threadFilm;

	// Allocate the new Film
	threadFilm = new Film(filmWidth, filmHeight);
	threadFilm->CopyDynamicSettings(*engineFilm);
	threadFilm->Init();

	//--------------------------------------------------------------------------
	for (u_int i = 0; i < channel_RADIANCE_PER_PIXEL_NORMALIZEDs_Buff.size(); ++i)
		FreeOCLBuffer(&channel_RADIANCE_PER_PIXEL_NORMALIZEDs_Buff[i]);

	if (threadFilm->GetRadianceGroupCount() > 8)
		throw runtime_error("PathOCL supports only up to 8 Radiance Groups");

	channel_RADIANCE_PER_PIXEL_NORMALIZEDs_Buff.resize(threadFilm->GetRadianceGroupCount());
	for (u_int i = 0; i < channel_RADIANCE_PER_PIXEL_NORMALIZEDs_Buff.size(); ++i) {
		AllocOCLBufferRW(&channel_RADIANCE_PER_PIXEL_NORMALIZEDs_Buff[i],
				sizeof(float[4]) * filmPixelCount, "RADIANCE_PER_PIXEL_NORMALIZEDs[" + ToString(i) + "]");
	}
	//--------------------------------------------------------------------------
	if (threadFilm->HasChannel(Film::ALPHA))
		AllocOCLBufferRW(&channel_ALPHA_Buff, sizeof(float[2]) * filmPixelCount, "ALPHA");
	else
		FreeOCLBuffer(&channel_ALPHA_Buff);
	//--------------------------------------------------------------------------
	if (threadFilm->HasChannel(Film::DEPTH))
		AllocOCLBufferRW(&channel_DEPTH_Buff, sizeof(float) * filmPixelCount, "DEPTH");
	else
		FreeOCLBuffer(&channel_DEPTH_Buff);
	//--------------------------------------------------------------------------
	if (threadFilm->HasChannel(Film::POSITION))
		AllocOCLBufferRW(&channel_POSITION_Buff, sizeof(float[3]) * filmPixelCount, "POSITION");
	else
		FreeOCLBuffer(&channel_POSITION_Buff);
	//--------------------------------------------------------------------------
	if (threadFilm->HasChannel(Film::GEOMETRY_NORMAL))
		AllocOCLBufferRW(&channel_GEOMETRY_NORMAL_Buff, sizeof(float[3]) * filmPixelCount, "GEOMETRY_NORMAL");
	else
		FreeOCLBuffer(&channel_GEOMETRY_NORMAL_Buff);
	//--------------------------------------------------------------------------
	if (threadFilm->HasChannel(Film::SHADING_NORMAL))
		AllocOCLBufferRW(&channel_SHADING_NORMAL_Buff, sizeof(float[3]) * filmPixelCount, "SHADING_NORMAL");
	else
		FreeOCLBuffer(&channel_SHADING_NORMAL_Buff);
	//--------------------------------------------------------------------------
	if (threadFilm->HasChannel(Film::MATERIAL_ID))
		AllocOCLBufferRW(&channel_MATERIAL_ID_Buff, sizeof(u_int) * filmPixelCount, "MATERIAL_ID");
	else
		FreeOCLBuffer(&channel_MATERIAL_ID_Buff);
	//--------------------------------------------------------------------------
	if (threadFilm->HasChannel(Film::DIRECT_DIFFUSE))
		AllocOCLBufferRW(&channel_DIRECT_DIFFUSE_Buff, sizeof(float[4]) * filmPixelCount, "DIRECT_DIFFUSE");
	else
		FreeOCLBuffer(&channel_DIRECT_DIFFUSE_Buff);
	//--------------------------------------------------------------------------
	if (threadFilm->HasChannel(Film::DIRECT_GLOSSY))
		AllocOCLBufferRW(&channel_DIRECT_GLOSSY_Buff, sizeof(float[4]) * filmPixelCount, "DIRECT_GLOSSY");
	else
		FreeOCLBuffer(&channel_DIRECT_GLOSSY_Buff);
	//--------------------------------------------------------------------------
	if (threadFilm->HasChannel(Film::EMISSION))
		AllocOCLBufferRW(&channel_EMISSION_Buff, sizeof(float[4]) * filmPixelCount, "EMISSION");
	else
		FreeOCLBuffer(&channel_EMISSION_Buff);
	//--------------------------------------------------------------------------
	if (threadFilm->HasChannel(Film::INDIRECT_DIFFUSE))
		AllocOCLBufferRW(&channel_INDIRECT_DIFFUSE_Buff, sizeof(float[4]) * filmPixelCount, "INDIRECT_DIFFUSE");
	else
		FreeOCLBuffer(&channel_INDIRECT_DIFFUSE_Buff);
	//--------------------------------------------------------------------------
	if (threadFilm->HasChannel(Film::INDIRECT_GLOSSY))
		AllocOCLBufferRW(&channel_INDIRECT_GLOSSY_Buff, sizeof(float[4]) * filmPixelCount, "INDIRECT_GLOSSY");
	else
		FreeOCLBuffer(&channel_INDIRECT_GLOSSY_Buff);
	//--------------------------------------------------------------------------
	if (threadFilm->HasChannel(Film::INDIRECT_SPECULAR))
		AllocOCLBufferRW(&channel_INDIRECT_SPECULAR_Buff, sizeof(float[4]) * filmPixelCount, "INDIRECT_SPECULAR");
	else
		FreeOCLBuffer(&channel_INDIRECT_SPECULAR_Buff);
	//--------------------------------------------------------------------------
	if (threadFilm->HasChannel(Film::MATERIAL_ID_MASK)) {
		if (threadFilm->GetMaskMaterialIDCount() > 1)
			throw runtime_error("PathOCL supports only 1 MATERIAL_ID_MASK");
		else
			AllocOCLBufferRW(&channel_MATERIAL_ID_MASK_Buff,
					sizeof(float[2]) * filmPixelCount, "MATERIAL_ID_MASK");
	} else
		FreeOCLBuffer(&channel_MATERIAL_ID_MASK_Buff);
	//--------------------------------------------------------------------------
	if (threadFilm->HasChannel(Film::DIRECT_SHADOW_MASK))
		AllocOCLBufferRW(&channel_DIRECT_SHADOW_MASK_Buff, sizeof(float[2]) * filmPixelCount, "DIRECT_SHADOW_MASK");
	else
		FreeOCLBuffer(&channel_DIRECT_SHADOW_MASK_Buff);
	//--------------------------------------------------------------------------
	if (threadFilm->HasChannel(Film::INDIRECT_SHADOW_MASK))
		AllocOCLBufferRW(&channel_INDIRECT_SHADOW_MASK_Buff, sizeof(float[2]) * filmPixelCount, "INDIRECT_SHADOW_MASK");
	else
		FreeOCLBuffer(&channel_INDIRECT_SHADOW_MASK_Buff);
	//--------------------------------------------------------------------------
	if (threadFilm->HasChannel(Film::UV))
		AllocOCLBufferRW(&channel_UV_Buff, sizeof(float[2]) * filmPixelCount, "UV");
	else
		FreeOCLBuffer(&channel_UV_Buff);
}

void PathOCLBaseRenderThread::InitCamera() {
	AllocOCLBufferRO(&cameraBuff, &renderEngine->compiledScene->camera,
			sizeof(slg::ocl::Camera), "Camera");
}

void PathOCLBaseRenderThread::InitGeometry() {
	CompiledScene *cscene = renderEngine->compiledScene;

	if (cscene->normals.size() > 0)
		AllocOCLBufferRO(&normalsBuff, &cscene->normals[0],
				sizeof(Normal) * cscene->normals.size(), "Normals");
	else
		FreeOCLBuffer(&normalsBuff);

	if (cscene->uvs.size() > 0)
		AllocOCLBufferRO(&uvsBuff, &cscene->uvs[0],
			sizeof(UV) * cscene->uvs.size(), "UVs");
	else
		FreeOCLBuffer(&uvsBuff);

	if (cscene->cols.size() > 0)
		AllocOCLBufferRO(&colsBuff, &cscene->cols[0],
			sizeof(Spectrum) * cscene->cols.size(), "Colors");
	else
		FreeOCLBuffer(&colsBuff);

	if (cscene->alphas.size() > 0)
		AllocOCLBufferRO(&alphasBuff, &cscene->alphas[0],
			sizeof(float) * cscene->alphas.size(), "Alphas");
	else
		FreeOCLBuffer(&alphasBuff);

	AllocOCLBufferRO(&vertsBuff, &cscene->verts[0],
		sizeof(Point) * cscene->verts.size(), "Vertices");

	AllocOCLBufferRO(&trianglesBuff, &cscene->tris[0],
		sizeof(Triangle) * cscene->tris.size(), "Triangles");

	AllocOCLBufferRO(&meshDescsBuff, &cscene->meshDescs[0],
			sizeof(slg::ocl::Mesh) * cscene->meshDescs.size(), "Mesh description");
}

void PathOCLBaseRenderThread::InitMaterials() {
	const size_t materialsCount = renderEngine->compiledScene->mats.size();
	AllocOCLBufferRO(&materialsBuff, &renderEngine->compiledScene->mats[0],
			sizeof(slg::ocl::Material) * materialsCount, "Materials");

	const u_int meshCount = renderEngine->compiledScene->meshMats.size();
	AllocOCLBufferRO(&meshMatsBuff, &renderEngine->compiledScene->meshMats[0],
			sizeof(u_int) * meshCount, "Mesh material index");
}

void PathOCLBaseRenderThread::InitTextures() {
	const size_t texturesCount = renderEngine->compiledScene->texs.size();
	AllocOCLBufferRO(&texturesBuff, &renderEngine->compiledScene->texs[0],
			sizeof(slg::ocl::Texture) * texturesCount, "Textures");
}

void PathOCLBaseRenderThread::InitTriangleAreaLights() {
	CompiledScene *cscene = renderEngine->compiledScene;

	if (cscene->triLightDefs.size() > 0) {
		AllocOCLBufferRO(&triLightDefsBuff, &cscene->triLightDefs[0],
			sizeof(slg::ocl::TriangleLight) * cscene->triLightDefs.size(), "Triangle AreaLights");
		AllocOCLBufferRO(&meshTriLightDefsOffsetBuff, &cscene->meshTriLightDefsOffset[0],
			sizeof(u_int) * cscene->meshTriLightDefsOffset.size(), "Triangle AreaLights offsets");
	} else {
		FreeOCLBuffer(&triLightDefsBuff);
		FreeOCLBuffer(&meshTriLightDefsOffsetBuff);
	}
}

void PathOCLBaseRenderThread::InitInfiniteLight() {
	CompiledScene *cscene = renderEngine->compiledScene;

	if (cscene->infiniteLight) {
		AllocOCLBufferRO(&infiniteLightBuff, cscene->infiniteLight,
			sizeof(slg::ocl::InfiniteLight), "InfiniteLight");
		AllocOCLBufferRO(&infiniteLightDistributionBuff, cscene->infiniteLightDistribution,
			cscene->infiniteLightDistributionSize, "InfiniteLight Distribution");
	} else {
		FreeOCLBuffer(&infiniteLightBuff);
		FreeOCLBuffer(&infiniteLightDistributionBuff);
	}
}

void PathOCLBaseRenderThread::InitSunLight() {
	CompiledScene *cscene = renderEngine->compiledScene;

	if (cscene->sunLight)
		AllocOCLBufferRO(&sunLightBuff, cscene->sunLight,
			sizeof(slg::ocl::SunLight), "SunLight");
	else
		FreeOCLBuffer(&sunLightBuff);
}

void PathOCLBaseRenderThread::InitSkyLight() {
	CompiledScene *cscene = renderEngine->compiledScene;

	if (cscene->skyLight)
		AllocOCLBufferRO(&skyLightBuff, cscene->skyLight,
			sizeof(slg::ocl::SkyLight), "SkyLight");
	else
		FreeOCLBuffer(&skyLightBuff);
}

void PathOCLBaseRenderThread::InitLightsDistribution() {
	CompiledScene *cscene = renderEngine->compiledScene;

	AllocOCLBufferRO(&lightsDistributionBuff, cscene->lightsDistribution,
		cscene->lightsDistributionSize, "LightsDistribution");
}

void PathOCLBaseRenderThread::InitImageMaps() {
	CompiledScene *cscene = renderEngine->compiledScene;

	if (cscene->imageMapDescs.size() > 0) {
		AllocOCLBufferRO(&imageMapDescsBuff, &cscene->imageMapDescs[0],
				sizeof(slg::ocl::ImageMap) * cscene->imageMapDescs.size(), "ImageMaps description");

		// Free unused pages
		for (u_int i = cscene->imageMapMemBlocks.size(); i < imageMapsBuff.size(); ++i)
			FreeOCLBuffer(&imageMapsBuff[i]);
		imageMapsBuff.resize(cscene->imageMapMemBlocks.size(), NULL);

		for (u_int i = 0; i < imageMapsBuff.size(); ++i) {
			AllocOCLBufferRO(&(imageMapsBuff[i]), &(cscene->imageMapMemBlocks[i][0]),
					sizeof(float) * cscene->imageMapMemBlocks[i].size(), "ImageMaps");
		}
	} else {
		FreeOCLBuffer(&imageMapDescsBuff);
		for (u_int i = 0; i < imageMapsBuff.size(); ++i)
			FreeOCLBuffer(&imageMapsBuff[i]);
		imageMapsBuff.resize(0);
	}
}

void PathOCLBaseRenderThread::CompileKernel(cl::Program *program, cl::Kernel **kernel,
		size_t *workgroupSize, const string &name) {
	delete *kernel;
	SLG_LOG("[PathOCLBaseRenderThread::" << threadIndex << "] Compiling " << name << " Kernel");
	*kernel = new cl::Kernel(*program, name.c_str());
	
	if (intersectionDevice->GetDeviceDesc()->GetForceWorkGroupSize() > 0)
		*workgroupSize = intersectionDevice->GetDeviceDesc()->GetForceWorkGroupSize();
	else {
		cl::Device &oclDevice = intersectionDevice->GetOpenCLDevice();
		(*kernel)->getWorkGroupInfo<size_t>(oclDevice, CL_KERNEL_WORK_GROUP_SIZE, workgroupSize);
		SLG_LOG("[PathOCLBaseRenderThread::" << threadIndex << "] " << name << " workgroup size: " << *workgroupSize);
	}
}

void PathOCLBaseRenderThread::InitKernels() {
	//--------------------------------------------------------------------------
	// Compile kernels
	//--------------------------------------------------------------------------

	CompiledScene *cscene = renderEngine->compiledScene;
	cl::Context &oclContext = intersectionDevice->GetOpenCLContext();
	cl::Device &oclDevice = intersectionDevice->GetOpenCLDevice();

	// Set #define symbols
	stringstream ss;
	ss.precision(6);
	ss << scientific <<
			" -D LUXRAYS_OPENCL_KERNEL" <<
			" -D SLG_OPENCL_KERNEL" <<
			" -D PARAM_RAY_EPSILON_MIN=" << MachineEpsilon::GetMin() << "f"
			" -D PARAM_RAY_EPSILON_MAX=" << MachineEpsilon::GetMax() << "f"
			" -D PARAM_LIGHT_WORLD_RADIUS_SCALE=" << slg::LIGHT_WORLD_RADIUS_SCALE << "f"
			;

	switch (intersectionDevice->GetAccelerator()->GetType()) {
		case ACCEL_BVH:
			ss << " -D PARAM_ACCEL_BVH";
			break;
		case ACCEL_QBVH:
			ss << " -D PARAM_ACCEL_QBVH";
			break;
		case ACCEL_MQBVH:
			ss << " -D PARAM_ACCEL_MQBVH";
			break;
		case ACCEL_MBVH:
			ss << " -D PARAM_ACCEL_MBVH";
			break;
		default:
			throw new runtime_error("Unknown accelerator in PathOCLBaseRenderThread::InitKernels()");
	}

	// Film related parameters
	for (u_int i = 0; i < channel_RADIANCE_PER_PIXEL_NORMALIZEDs_Buff.size(); ++i)
		ss << " -D PARAM_FILM_RADIANCE_GROUP_" << i;
	ss << " -D PARAM_FILM_RADIANCE_GROUP_COUNT=" << channel_RADIANCE_PER_PIXEL_NORMALIZEDs_Buff.size();
	if (threadFilm->HasChannel(Film::ALPHA))
		ss << " -D PARAM_FILM_CHANNELS_HAS_ALPHA";
	if (threadFilm->HasChannel(Film::DEPTH))
		ss << " -D PARAM_FILM_CHANNELS_HAS_DEPTH";
	if (threadFilm->HasChannel(Film::POSITION))
		ss << " -D PARAM_FILM_CHANNELS_HAS_POSITION";
	if (threadFilm->HasChannel(Film::GEOMETRY_NORMAL))
		ss << " -D PARAM_FILM_CHANNELS_HAS_GEOMETRY_NORMAL";
	if (threadFilm->HasChannel(Film::SHADING_NORMAL))
		ss << " -D PARAM_FILM_CHANNELS_HAS_SHADING_NORMAL";
	if (threadFilm->HasChannel(Film::MATERIAL_ID))
		ss << " -D PARAM_FILM_CHANNELS_HAS_MATERIAL_ID";
	if (threadFilm->HasChannel(Film::DIRECT_DIFFUSE))
		ss << " -D PARAM_FILM_CHANNELS_HAS_DIRECT_DIFFUSE";
	if (threadFilm->HasChannel(Film::DIRECT_GLOSSY))
		ss << " -D PARAM_FILM_CHANNELS_HAS_DIRECT_GLOSSY";
	if (threadFilm->HasChannel(Film::EMISSION))
		ss << " -D PARAM_FILM_CHANNELS_HAS_EMISSION";
	if (threadFilm->HasChannel(Film::INDIRECT_DIFFUSE))
		ss << " -D PARAM_FILM_CHANNELS_HAS_INDIRECT_DIFFUSE";
	if (threadFilm->HasChannel(Film::INDIRECT_GLOSSY))
		ss << " -D PARAM_FILM_CHANNELS_HAS_INDIRECT_GLOSSY";
	if (threadFilm->HasChannel(Film::INDIRECT_SPECULAR))
		ss << " -D PARAM_FILM_CHANNELS_HAS_INDIRECT_SPECULAR";
	if (threadFilm->HasChannel(Film::MATERIAL_ID_MASK)) {
		ss << " -D PARAM_FILM_CHANNELS_HAS_MATERIAL_ID_MASK" <<
				" -D PARAM_FILM_MASK_MATERIAL_ID=" << threadFilm->GetMaskMaterialID(0);
	}
	if (threadFilm->HasChannel(Film::DIRECT_SHADOW_MASK))
		ss << " -D PARAM_FILM_CHANNELS_HAS_DIRECT_SHADOW_MASK";
	if (threadFilm->HasChannel(Film::INDIRECT_SHADOW_MASK))
		ss << " -D PARAM_FILM_CHANNELS_HAS_INDIRECT_SHADOW_MASK";
	if (threadFilm->HasChannel(Film::UV))
		ss << " -D PARAM_FILM_CHANNELS_HAS_UV";

	if (normalsBuff)
		ss << " -D PARAM_HAS_NORMALS_BUFFER";
	if (uvsBuff)
		ss << " -D PARAM_HAS_UVS_BUFFER";
	if (colsBuff)
		ss << " -D PARAM_HAS_COLS_BUFFER";
	if (alphasBuff)
		ss << " -D PARAM_HAS_ALPHAS_BUFFER";

	if (cscene->IsTextureCompiled(CONST_FLOAT))
		ss << " -D PARAM_ENABLE_TEX_CONST_FLOAT";
	if (cscene->IsTextureCompiled(CONST_FLOAT3))
		ss << " -D PARAM_ENABLE_TEX_CONST_FLOAT3";
	if (cscene->IsTextureCompiled(IMAGEMAP))
		ss << " -D PARAM_ENABLE_TEX_IMAGEMAP";
	if (cscene->IsTextureCompiled(SCALE_TEX))
		ss << " -D PARAM_ENABLE_TEX_SCALE";
	if (cscene->IsTextureCompiled(FRESNEL_APPROX_N))
		ss << " -D PARAM_ENABLE_FRESNEL_APPROX_N";
	if (cscene->IsTextureCompiled(FRESNEL_APPROX_K))
		ss << " -D PARAM_ENABLE_FRESNEL_APPROX_K";
	if (cscene->IsTextureCompiled(CHECKERBOARD2D))
		ss << " -D PARAM_ENABLE_CHECKERBOARD2D";
	if (cscene->IsTextureCompiled(CHECKERBOARD3D))
		ss << " -D PARAM_ENABLE_CHECKERBOARD3D";
	if (cscene->IsTextureCompiled(MIX_TEX))
		ss << " -D PARAM_ENABLE_TEX_MIX";
	if (cscene->IsTextureCompiled(FBM_TEX))
		ss << " -D PARAM_ENABLE_FBM_TEX";
	if (cscene->IsTextureCompiled(MARBLE))
		ss << " -D PARAM_ENABLE_MARBLE";
	if (cscene->IsTextureCompiled(DOTS))
		ss << " -D PARAM_ENABLE_DOTS";
	if (cscene->IsTextureCompiled(BRICK))
		ss << " -D PARAM_ENABLE_BRICK";
	if (cscene->IsTextureCompiled(ADD_TEX))
		ss << " -D PARAM_ENABLE_TEX_ADD";
	if (cscene->IsTextureCompiled(WINDY))
		ss << " -D PARAM_ENABLE_WINDY";
	if (cscene->IsTextureCompiled(WRINKLED))
		ss << " -D PARAM_ENABLE_WRINKLED";
	if (cscene->IsTextureCompiled(UV_TEX))
		ss << " -D PARAM_ENABLE_TEX_UV";
	if (cscene->IsTextureCompiled(BAND_TEX))
		ss << " -D PARAM_ENABLE_TEX_BAND";
	if (cscene->IsTextureCompiled(HITPOINTCOLOR))
		ss << " -D PARAM_ENABLE_TEX_HITPOINTCOLOR";
	if (cscene->IsTextureCompiled(HITPOINTALPHA))
		ss << " -D PARAM_ENABLE_TEX_HITPOINTALPHA";
	if (cscene->IsTextureCompiled(HITPOINTGREY))
		ss << " -D PARAM_ENABLE_TEX_HITPOINTGREY";

	if (cscene->IsMaterialCompiled(MATTE))
		ss << " -D PARAM_ENABLE_MAT_MATTE";
	if (cscene->IsMaterialCompiled(MIRROR))
		ss << " -D PARAM_ENABLE_MAT_MIRROR";
	if (cscene->IsMaterialCompiled(GLASS))
		ss << " -D PARAM_ENABLE_MAT_GLASS";
	if (cscene->IsMaterialCompiled(METAL))
		ss << " -D PARAM_ENABLE_MAT_METAL";
	if (cscene->IsMaterialCompiled(ARCHGLASS))
		ss << " -D PARAM_ENABLE_MAT_ARCHGLASS";
	if (cscene->IsMaterialCompiled(MIX))
		ss << " -D PARAM_ENABLE_MAT_MIX";
	if (cscene->IsMaterialCompiled(NULLMAT))
		ss << " -D PARAM_ENABLE_MAT_NULL";
	if (cscene->IsMaterialCompiled(MATTETRANSLUCENT))
		ss << " -D PARAM_ENABLE_MAT_MATTETRANSLUCENT";
	if (cscene->IsMaterialCompiled(GLOSSY2)) {
		ss << " -D PARAM_ENABLE_MAT_GLOSSY2";

		if (cscene->IsMaterialCompiled(GLOSSY2_ANISOTROPIC))
			ss << " -D PARAM_ENABLE_MAT_GLOSSY2_ANISOTROPIC";
		if (cscene->IsMaterialCompiled(GLOSSY2_ABSORPTION))
			ss << " -D PARAM_ENABLE_MAT_GLOSSY2_ABSORPTION";
		if (cscene->IsMaterialCompiled(GLOSSY2_INDEX))
			ss << " -D PARAM_ENABLE_MAT_GLOSSY2_INDEX";
		if (cscene->IsMaterialCompiled(GLOSSY2_MULTIBOUNCE))
			ss << " -D PARAM_ENABLE_MAT_GLOSSY2_MULTIBOUNCE";
	}
	if (cscene->IsMaterialCompiled(METAL2)) {
		ss << " -D PARAM_ENABLE_MAT_METAL2";
		if (cscene->IsMaterialCompiled(METAL2_ANISOTROPIC))
			ss << " -D PARAM_ENABLE_MAT_METAL2_ANISOTROPIC";
	}
	if (cscene->IsMaterialCompiled(ROUGHGLASS)) {
		ss << " -D PARAM_ENABLE_MAT_ROUGHGLASS";
		if (cscene->IsMaterialCompiled(ROUGHGLASS_ANISOTROPIC))
			ss << " -D PARAM_ENABLE_MAT_ROUGHGLASS_ANISOTROPIC";
	}

	if (cscene->RequiresPassThrough())
		ss << " -D PARAM_HAS_PASSTHROUGH";
	
	if (cscene->camera.lensRadius > 0.f)
		ss << " -D PARAM_CAMERA_HAS_DOF";
	if (cscene->enableHorizStereo) {
		ss << " -D PARAM_CAMERA_ENABLE_HORIZ_STEREO";

		if (cscene->enableOculusRiftBarrel)
			ss << " -D PARAM_CAMERA_ENABLE_OCULUSRIFT_BARREL";
	}

	if (infiniteLightBuff)
		ss << " -D PARAM_HAS_INFINITELIGHT";

	if (skyLightBuff)
		ss << " -D PARAM_HAS_SKYLIGHT";

	if (sunLightBuff)
		ss << " -D PARAM_HAS_SUNLIGHT";

	if (triLightDefsBuff)
		ss << " -D PARAM_DL_LIGHT_COUNT=" << renderEngine->compiledScene->triLightDefs.size();
	else
		ss << " -D PARAM_DL_LIGHT_COUNT=0";

	if (imageMapDescsBuff) {
		ss << " -D PARAM_HAS_IMAGEMAPS";
		if (imageMapsBuff.size() > 8)
			throw runtime_error("Too many memory pages required for image maps");
		for (u_int i = 0; i < imageMapsBuff.size(); ++i)
			ss << " -D PARAM_IMAGEMAPS_PAGE_" << i;
		ss << " -D PARAM_IMAGEMAPS_COUNT=" << imageMapsBuff.size();
	}

	if (renderEngine->compiledScene->useBumpMapping)
		ss << " -D PARAM_HAS_BUMPMAPS";
	if (renderEngine->compiledScene->useNormalMapping)
		ss << " -D PARAM_HAS_NORMALMAPS";

	// Some information about our place in the universe...
	ss << " -D PARAM_DEVICE_INDEX=" << threadIndex;
	ss << " -D PARAM_DEVICE_COUNT=" << renderEngine->intersectionDevices.size();

	ss << AdditionalKernelOptions();
	
	//--------------------------------------------------------------------------

	// Check the OpenCL vendor and use some specific compiler options
#if defined(__APPLE__)
	ss << " -D __APPLE_CL__";
#endif
	//--------------------------------------------------------------------------

	const double tStart = WallClockTime();

	// Check if I have to recompile the kernels
	string newKernelParameters = ss.str();
	if (kernelsParameters != newKernelParameters) {
		kernelsParameters = newKernelParameters;

		// Compile sources
		stringstream ssKernel;
		ssKernel <<
			// OpenCL LuxRays Types
			luxrays::ocl::KernelSource_luxrays_types <<
			luxrays::ocl::KernelSource_uv_types <<
			luxrays::ocl::KernelSource_point_types <<
			luxrays::ocl::KernelSource_vector_types <<
			luxrays::ocl::KernelSource_normal_types <<
			luxrays::ocl::KernelSource_triangle_types <<
			luxrays::ocl::KernelSource_ray_types <<
			luxrays::ocl::KernelSource_bbox_types <<
			luxrays::ocl::KernelSource_epsilon_types <<
			luxrays::ocl::KernelSource_spectrum_types <<
			luxrays::ocl::KernelSource_frame_types <<
			luxrays::ocl::KernelSource_matrix4x4_types <<
			luxrays::ocl::KernelSource_transform_types <<
			// OpenCL LuxRays Funcs
			luxrays::ocl::KernelSource_epsilon_funcs <<
			luxrays::ocl::KernelSource_utils_funcs <<
			luxrays::ocl::KernelSource_vector_funcs <<
			luxrays::ocl::KernelSource_ray_funcs <<
			luxrays::ocl::KernelSource_bbox_funcs <<
			luxrays::ocl::KernelSource_spectrum_funcs <<
			luxrays::ocl::KernelSource_frame_funcs <<
			luxrays::ocl::KernelSource_matrix4x4_funcs <<
			luxrays::ocl::KernelSource_transform_funcs <<
			// OpenCL SLG Types
			slg::ocl::KernelSource_randomgen_types <<
			slg::ocl::KernelSource_trianglemesh_types <<
			slg::ocl::KernelSource_hitpoint_types <<
			slg::ocl::KernelSource_mapping_types <<
			slg::ocl::KernelSource_texture_types <<
			slg::ocl::KernelSource_material_types <<
			slg::ocl::KernelSource_bsdf_types <<
			slg::ocl::KernelSource_film_types <<
			slg::ocl::KernelSource_filter_types <<
			slg::ocl::KernelSource_sampler_types <<
			slg::ocl::KernelSource_camera_types <<
			slg::ocl::KernelSource_light_types <<
			// OpenCL SLG Types
			slg::ocl::KernelSource_mc_funcs <<
			slg::ocl::KernelSource_randomgen_funcs <<
			slg::ocl::KernelSource_triangle_funcs <<
			slg::ocl::KernelSource_trianglemesh_funcs <<
			slg::ocl::KernelSource_mapping_funcs <<
			slg::ocl::KernelSource_texture_funcs <<
			slg::ocl::KernelSource_materialdefs_funcs <<
			slg::ocl::KernelSource_material_funcs <<
			slg::ocl::KernelSource_camera_funcs <<
			slg::ocl::KernelSource_light_funcs <<
			slg::ocl::KernelSource_filter_funcs <<
			slg::ocl::KernelSource_film_funcs <<
			slg::ocl::KernelSource_sampler_funcs <<
			slg::ocl::KernelSource_bsdf_funcs <<
			slg::ocl::KernelSource_scene_funcs;

		ssKernel << AdditionalKernelSources();

		string kernelSource = ssKernel.str();

		SLG_LOG("[PathOCLBaseRenderThread::" << threadIndex << "] Defined symbols: " << kernelsParameters);
		SLG_LOG("[PathOCLBaseRenderThread::" << threadIndex << "] Compiling kernels ");

		// Some debug code to write the OpenCL kernel source to a file
		/*const string kernelFileName = "kernel_source_device_" + ToString(threadIndex) + ".txt";
		ofstream kernelFile(kernelFileName.c_str());
		string kernelDefs = kernelsParameters;
		boost::replace_all(kernelDefs, "-D", "\n#define");
		boost::replace_all(kernelDefs, "=", " ");
		kernelFile << kernelDefs << endl << endl << kernelSource << endl;
		kernelFile.close();*/

		bool cached;
		cl::STRING_CLASS error;
		cl::Program *program = kernelCache->Compile(oclContext, oclDevice,
				kernelsParameters, kernelSource,
				&cached, &error);

		if (!program) {
			SLG_LOG("[PathOCLBaseRenderThread::" << threadIndex << "] PathOCL kernel compilation error" << endl << error);

			throw runtime_error("PathOCLBase kernel compilation error");
		}

		if (cached) {
			SLG_LOG("[PathOCLBaseRenderThread::" << threadIndex << "] Kernels cached");
		} else {
			SLG_LOG("[PathOCLBaseRenderThread::" << threadIndex << "] Kernels not cached");
		}

		// Film clear  kernel
		CompileKernel(program, &filmClearKernel, &filmClearWorkGroupSize, "Film_Clear");

		// Additional kernels
		CompileAdditionalKernels(program);

		const double tEnd = WallClockTime();
		SLG_LOG("[PathOCLBaseRenderThread::" << threadIndex << "] Kernels compilation time: " << int((tEnd - tStart) * 1000.0) << "ms");

		delete program;
	} else
		SLG_LOG("[PathOCLBaseRenderThread::" << threadIndex << "] Using cached kernels");
}

void PathOCLBaseRenderThread::InitRender() {
	//--------------------------------------------------------------------------
	// Film definition
	//--------------------------------------------------------------------------

	InitFilm();

	//--------------------------------------------------------------------------
	// Camera definition
	//--------------------------------------------------------------------------

	InitCamera();

	//--------------------------------------------------------------------------
	// Scene geometry
	//--------------------------------------------------------------------------

	InitGeometry();

	//--------------------------------------------------------------------------
	// Image maps
	//--------------------------------------------------------------------------

	InitImageMaps();

	//--------------------------------------------------------------------------
	// Texture definitions
	//--------------------------------------------------------------------------

	InitTextures();

	//--------------------------------------------------------------------------
	// Material definitions
	//--------------------------------------------------------------------------

	InitMaterials();

	//--------------------------------------------------------------------------
	// Translate triangle area lights
	//--------------------------------------------------------------------------

	InitTriangleAreaLights();

	//--------------------------------------------------------------------------
	// Check if there is an infinite light source
	//--------------------------------------------------------------------------

	InitInfiniteLight();

	//--------------------------------------------------------------------------
	// Check if there is an sun light source
	//--------------------------------------------------------------------------

	InitSunLight();

	//--------------------------------------------------------------------------
	// Check if there is an sky light source
	//--------------------------------------------------------------------------

	InitSkyLight();
	
	InitLightsDistribution();

	const u_int triAreaLightCount = renderEngine->compiledScene->triLightDefs.size();
	if (!skyLightBuff && !sunLightBuff && !infiniteLightBuff && (triAreaLightCount == 0))
		throw runtime_error("There are no light sources supported by PathOCL in the scene");

	AdditionalInit();

	//--------------------------------------------------------------------------
	// Compile kernels
	//--------------------------------------------------------------------------

	InitKernels();

	//--------------------------------------------------------------------------
	// Initialize
	//--------------------------------------------------------------------------

	// Set kernel arguments
	SetKernelArgs();

	cl::CommandQueue &oclQueue = intersectionDevice->GetOpenCLQueue();

	// Clear the film
	const u_int filmPixelCount = threadFilm->GetWidth() * threadFilm->GetHeight();
	oclQueue.enqueueNDRangeKernel(*filmClearKernel, cl::NullRange,
			cl::NDRange(RoundUp<u_int>(filmPixelCount, filmClearWorkGroupSize)),
			cl::NDRange(filmClearWorkGroupSize));

	oclQueue.finish();

	// Reset statistics in order to be more accurate
	intersectionDevice->ResetPerformaceStats();
}

void PathOCLBaseRenderThread::SetKernelArgs() {
	// Set OpenCL kernel arguments

	{
		// OpenCL kernel setArg() is the only no thread safe function in OpenCL 1.1 so
		// I need to use a mutex here
		boost::unique_lock<boost::mutex> lock(renderEngine->setKernelArgsMutex);

		//--------------------------------------------------------------------------
		// initFilmKernel
		//--------------------------------------------------------------------------

		u_int argIndex = 0;
		filmClearKernel->setArg(argIndex++, threadFilm->GetWidth());
		filmClearKernel->setArg(argIndex++, threadFilm->GetHeight());
		for (u_int i = 0; i < channel_RADIANCE_PER_PIXEL_NORMALIZEDs_Buff.size(); ++i)
			filmClearKernel->setArg(argIndex++, *(channel_RADIANCE_PER_PIXEL_NORMALIZEDs_Buff[i]));

		if (threadFilm->HasChannel(Film::ALPHA))
			filmClearKernel->setArg(argIndex++, *channel_ALPHA_Buff);
		if (threadFilm->HasChannel(Film::DEPTH))
			filmClearKernel->setArg(argIndex++, *channel_DEPTH_Buff);
		if (threadFilm->HasChannel(Film::POSITION))
			filmClearKernel->setArg(argIndex++, *channel_POSITION_Buff);
		if (threadFilm->HasChannel(Film::GEOMETRY_NORMAL))
			filmClearKernel->setArg(argIndex++, *channel_GEOMETRY_NORMAL_Buff);
		if (threadFilm->HasChannel(Film::SHADING_NORMAL))
			filmClearKernel->setArg(argIndex++, *channel_SHADING_NORMAL_Buff);
		if (threadFilm->HasChannel(Film::MATERIAL_ID))
			filmClearKernel->setArg(argIndex++, *channel_MATERIAL_ID_Buff);
		if (threadFilm->HasChannel(Film::DIRECT_DIFFUSE))
			filmClearKernel->setArg(argIndex++, *channel_DIRECT_DIFFUSE_Buff);
		if (threadFilm->HasChannel(Film::DIRECT_GLOSSY))
			filmClearKernel->setArg(argIndex++, *channel_DIRECT_GLOSSY_Buff);
		if (threadFilm->HasChannel(Film::EMISSION))
			filmClearKernel->setArg(argIndex++, *channel_EMISSION_Buff);
		if (threadFilm->HasChannel(Film::INDIRECT_DIFFUSE))
			filmClearKernel->setArg(argIndex++, *channel_INDIRECT_DIFFUSE_Buff);
		if (threadFilm->HasChannel(Film::INDIRECT_GLOSSY))
			filmClearKernel->setArg(argIndex++, *channel_INDIRECT_GLOSSY_Buff);
		if (threadFilm->HasChannel(Film::INDIRECT_SPECULAR))
			filmClearKernel->setArg(argIndex++, *channel_INDIRECT_SPECULAR_Buff);
		if (threadFilm->HasChannel(Film::MATERIAL_ID_MASK))
			filmClearKernel->setArg(argIndex++, *channel_MATERIAL_ID_MASK_Buff);
		if (threadFilm->HasChannel(Film::DIRECT_SHADOW_MASK))
			filmClearKernel->setArg(argIndex++, *channel_DIRECT_SHADOW_MASK_Buff);
		if (threadFilm->HasChannel(Film::INDIRECT_SHADOW_MASK))
			filmClearKernel->setArg(argIndex++, *channel_INDIRECT_SHADOW_MASK_Buff);
		if (threadFilm->HasChannel(Film::UV))
			filmClearKernel->setArg(argIndex++, *channel_UV_Buff);
	}

	SetAdditionalKernelArgs();
}

void PathOCLBaseRenderThread::Start() {
	started = true;

	InitRender();
	StartRenderThread();
}

void PathOCLBaseRenderThread::Interrupt() {
	if (renderThread)
		renderThread->interrupt();
}

void PathOCLBaseRenderThread::Stop() {
	StopRenderThread();

	TransferFilm(intersectionDevice->GetOpenCLQueue());
	
	// Film buffers
	for (u_int i = 0; i < channel_RADIANCE_PER_PIXEL_NORMALIZEDs_Buff.size(); ++i)
		FreeOCLBuffer(&channel_RADIANCE_PER_PIXEL_NORMALIZEDs_Buff[i]);
	channel_RADIANCE_PER_PIXEL_NORMALIZEDs_Buff.clear();
	FreeOCLBuffer(&channel_ALPHA_Buff);
	FreeOCLBuffer(&channel_DEPTH_Buff);
	FreeOCLBuffer(&channel_POSITION_Buff);
	FreeOCLBuffer(&channel_GEOMETRY_NORMAL_Buff);
	FreeOCLBuffer(&channel_SHADING_NORMAL_Buff);
	FreeOCLBuffer(&channel_MATERIAL_ID_Buff);
	FreeOCLBuffer(&channel_DIRECT_DIFFUSE_Buff);
	FreeOCLBuffer(&channel_DIRECT_GLOSSY_Buff);
	FreeOCLBuffer(&channel_EMISSION_Buff);
	FreeOCLBuffer(&channel_INDIRECT_DIFFUSE_Buff);
	FreeOCLBuffer(&channel_INDIRECT_GLOSSY_Buff);
	FreeOCLBuffer(&channel_INDIRECT_SPECULAR_Buff);
	FreeOCLBuffer(&channel_MATERIAL_ID_MASK_Buff);
	FreeOCLBuffer(&channel_DIRECT_SHADOW_MASK_Buff);
	FreeOCLBuffer(&channel_INDIRECT_SHADOW_MASK_Buff);
	FreeOCLBuffer(&channel_UV_Buff);

	// Scene buffers
	FreeOCLBuffer(&materialsBuff);
	FreeOCLBuffer(&texturesBuff);
	FreeOCLBuffer(&meshDescsBuff);
	FreeOCLBuffer(&meshMatsBuff);
	FreeOCLBuffer(&normalsBuff);
	FreeOCLBuffer(&uvsBuff);
	FreeOCLBuffer(&colsBuff);
	FreeOCLBuffer(&alphasBuff);
	FreeOCLBuffer(&trianglesBuff);
	FreeOCLBuffer(&vertsBuff);
	FreeOCLBuffer(&infiniteLightBuff);
	FreeOCLBuffer(&infiniteLightDistributionBuff);
	FreeOCLBuffer(&sunLightBuff);
	FreeOCLBuffer(&skyLightBuff);
	FreeOCLBuffer(&cameraBuff);
	FreeOCLBuffer(&triLightDefsBuff);
	FreeOCLBuffer(&meshTriLightDefsOffsetBuff);
	FreeOCLBuffer(&imageMapDescsBuff);
	for (u_int i = 0; i < imageMapsBuff.size(); ++i)
		FreeOCLBuffer(&imageMapsBuff[i]);
	imageMapsBuff.resize(0);

	started = false;

	// Film is delete on the destructor to allow image saving after
	// the rendering is finished
}

void PathOCLBaseRenderThread::StartRenderThread() {
	// Create the thread for the rendering
	renderThread = new boost::thread(&PathOCLBaseRenderThread::RenderThreadImpl, this);
}

void PathOCLBaseRenderThread::StopRenderThread() {
	if (renderThread) {
		renderThread->interrupt();
		renderThread->join();
		delete renderThread;
		renderThread = NULL;
	}
}

void PathOCLBaseRenderThread::BeginEdit() {
	StopRenderThread();
}

void PathOCLBaseRenderThread::EndEdit(const EditActionList &editActions) {
	//--------------------------------------------------------------------------
	// Update OpenCL buffers
	//--------------------------------------------------------------------------

	if (editActions.Has(FILM_EDIT)) {
		// Update Film
		InitFilm();
	}

	if (editActions.Has(CAMERA_EDIT)) {
		// Update Camera
		InitCamera();
	}

	if (editActions.Has(GEOMETRY_EDIT)) {
		// Update Scene Geometry
		InitGeometry();
	}

	if (editActions.Has(IMAGEMAPS_EDIT)) {
		// Update Image Maps
		InitImageMaps();
	}

	if (editActions.Has(MATERIALS_EDIT) || editActions.Has(MATERIAL_TYPES_EDIT)) {
		// Update Scene Textures and Materials
		InitTextures();
		InitMaterials();
	}

	if  (editActions.Has(AREALIGHTS_EDIT)) {
		// Update Scene Area Lights
		InitTriangleAreaLights();
	}

	if  (editActions.Has(INFINITELIGHT_EDIT)) {
		// Update Scene Infinite Light
		InitInfiniteLight();
	}

	if  (editActions.Has(SUNLIGHT_EDIT)) {
		// Update Scene Sun Light
		InitSunLight();
	}

	if  (editActions.Has(SKYLIGHT_EDIT)) {
		// Update Scene Sun Light
		InitSkyLight();
	}

	if (editActions.Has(GEOMETRY_EDIT) ||
			editActions.Has(AREALIGHTS_EDIT) ||
			editActions.Has(INFINITELIGHT_EDIT) ||
			editActions.Has(INFINITELIGHT_EDIT) ||
			editActions.Has(INFINITELIGHT_EDIT)) {
		// Update Scene light power distribution for direct light sampling
		InitLightsDistribution();
	}

	//--------------------------------------------------------------------------
	// Recompile Kernels if required
	//--------------------------------------------------------------------------

	if (editActions.Has(FILM_EDIT) || editActions.Has(MATERIAL_TYPES_EDIT))
		InitKernels();

	if (editActions.HasAnyAction()) {
		SetKernelArgs();

		//--------------------------------------------------------------------------
		// Execute initialization kernels
		//--------------------------------------------------------------------------

		cl::CommandQueue &oclQueue = intersectionDevice->GetOpenCLQueue();

		// Clear the frame buffer
		const u_int filmPixelCount = threadFilm->GetWidth() * threadFilm->GetHeight();
		oclQueue.enqueueNDRangeKernel(*filmClearKernel, cl::NullRange,
			cl::NDRange(RoundUp<u_int>(filmPixelCount, filmClearWorkGroupSize)),
			cl::NDRange(filmClearWorkGroupSize));
	}

	// Reset statistics in order to be more accurate
	intersectionDevice->ResetPerformaceStats();

	StartRenderThread();
}

void PathOCLBaseRenderThread::TransferFilm(cl::CommandQueue &oclQueue) {
	// Async. transfer of the Film buffers

	for (u_int i = 0; i < channel_RADIANCE_PER_PIXEL_NORMALIZEDs_Buff.size(); ++i) {
		if (channel_RADIANCE_PER_PIXEL_NORMALIZEDs_Buff[i]) {
			oclQueue.enqueueReadBuffer(
				*(channel_RADIANCE_PER_PIXEL_NORMALIZEDs_Buff[i]),
				CL_FALSE,
				0,
				channel_RADIANCE_PER_PIXEL_NORMALIZEDs_Buff[i]->getInfo<CL_MEM_SIZE>(),
				threadFilm->channel_RADIANCE_PER_PIXEL_NORMALIZEDs[i]->GetPixels());
		}
	}

	if (channel_ALPHA_Buff) {
		oclQueue.enqueueReadBuffer(
			*channel_ALPHA_Buff,
			CL_FALSE,
			0,
			channel_ALPHA_Buff->getInfo<CL_MEM_SIZE>(),
			threadFilm->channel_ALPHA->GetPixels());
	}
	if (channel_DEPTH_Buff) {
		oclQueue.enqueueReadBuffer(
			*channel_DEPTH_Buff,
			CL_FALSE,
			0,
			channel_DEPTH_Buff->getInfo<CL_MEM_SIZE>(),
			threadFilm->channel_DEPTH->GetPixels());
	}
	if (channel_POSITION_Buff) {
		oclQueue.enqueueReadBuffer(
			*channel_POSITION_Buff,
			CL_FALSE,
			0,
			channel_POSITION_Buff->getInfo<CL_MEM_SIZE>(),
			threadFilm->channel_POSITION->GetPixels());
	}
	if (channel_GEOMETRY_NORMAL_Buff) {
		oclQueue.enqueueReadBuffer(
			*channel_GEOMETRY_NORMAL_Buff,
			CL_FALSE,
			0,
			channel_GEOMETRY_NORMAL_Buff->getInfo<CL_MEM_SIZE>(),
			threadFilm->channel_GEOMETRY_NORMAL->GetPixels());
	}
	if (channel_SHADING_NORMAL_Buff) {
		oclQueue.enqueueReadBuffer(
			*channel_SHADING_NORMAL_Buff,
			CL_FALSE,
			0,
			channel_SHADING_NORMAL_Buff->getInfo<CL_MEM_SIZE>(),
			threadFilm->channel_SHADING_NORMAL->GetPixels());
	}
	if (channel_MATERIAL_ID_Buff) {
		oclQueue.enqueueReadBuffer(
			*channel_MATERIAL_ID_Buff,
			CL_FALSE,
			0,
			channel_MATERIAL_ID_Buff->getInfo<CL_MEM_SIZE>(),
			threadFilm->channel_MATERIAL_ID->GetPixels());
	}
	if (channel_DIRECT_DIFFUSE_Buff) {
		oclQueue.enqueueReadBuffer(
			*channel_DIRECT_DIFFUSE_Buff,
			CL_FALSE,
			0,
			channel_DIRECT_DIFFUSE_Buff->getInfo<CL_MEM_SIZE>(),
			threadFilm->channel_DIRECT_DIFFUSE->GetPixels());
	}
	if (channel_MATERIAL_ID_Buff) {
		oclQueue.enqueueReadBuffer(
			*channel_MATERIAL_ID_Buff,
			CL_FALSE,
			0,
			channel_MATERIAL_ID_Buff->getInfo<CL_MEM_SIZE>(),
			threadFilm->channel_MATERIAL_ID->GetPixels());
	}
	if (channel_DIRECT_GLOSSY_Buff) {
		oclQueue.enqueueReadBuffer(
			*channel_DIRECT_GLOSSY_Buff,
			CL_FALSE,
			0,
			channel_DIRECT_GLOSSY_Buff->getInfo<CL_MEM_SIZE>(),
			threadFilm->channel_DIRECT_GLOSSY->GetPixels());
	}
	if (channel_EMISSION_Buff) {
		oclQueue.enqueueReadBuffer(
			*channel_EMISSION_Buff,
			CL_FALSE,
			0,
			channel_EMISSION_Buff->getInfo<CL_MEM_SIZE>(),
			threadFilm->channel_EMISSION->GetPixels());
	}
	if (channel_INDIRECT_DIFFUSE_Buff) {
		oclQueue.enqueueReadBuffer(
			*channel_INDIRECT_DIFFUSE_Buff,
			CL_FALSE,
			0,
			channel_INDIRECT_DIFFUSE_Buff->getInfo<CL_MEM_SIZE>(),
			threadFilm->channel_INDIRECT_DIFFUSE->GetPixels());
	}
	if (channel_INDIRECT_GLOSSY_Buff) {
		oclQueue.enqueueReadBuffer(
			*channel_INDIRECT_GLOSSY_Buff,
			CL_FALSE,
			0,
			channel_INDIRECT_GLOSSY_Buff->getInfo<CL_MEM_SIZE>(),
			threadFilm->channel_INDIRECT_GLOSSY->GetPixels());
	}
	if (channel_INDIRECT_SPECULAR_Buff) {
		oclQueue.enqueueReadBuffer(
			*channel_INDIRECT_SPECULAR_Buff,
			CL_FALSE,
			0,
			channel_INDIRECT_SPECULAR_Buff->getInfo<CL_MEM_SIZE>(),
			threadFilm->channel_INDIRECT_SPECULAR->GetPixels());
	}
	if (channel_MATERIAL_ID_MASK_Buff) {
		oclQueue.enqueueReadBuffer(
			*channel_MATERIAL_ID_MASK_Buff,
			CL_FALSE,
			0,
			channel_MATERIAL_ID_MASK_Buff->getInfo<CL_MEM_SIZE>(),
			threadFilm->channel_MATERIAL_ID_MASKs[0]->GetPixels());
	}
	if (channel_DIRECT_SHADOW_MASK_Buff) {
		oclQueue.enqueueReadBuffer(
			*channel_DIRECT_SHADOW_MASK_Buff,
			CL_FALSE,
			0,
			channel_DIRECT_SHADOW_MASK_Buff->getInfo<CL_MEM_SIZE>(),
			threadFilm->channel_DIRECT_SHADOW_MASK->GetPixels());
	}
	if (channel_INDIRECT_SHADOW_MASK_Buff) {
		oclQueue.enqueueReadBuffer(
			*channel_INDIRECT_SHADOW_MASK_Buff,
			CL_FALSE,
			0,
			channel_INDIRECT_SHADOW_MASK_Buff->getInfo<CL_MEM_SIZE>(),
			threadFilm->channel_INDIRECT_SHADOW_MASK->GetPixels());
	}
	if (channel_UV_Buff) {
		oclQueue.enqueueReadBuffer(
			*channel_UV_Buff,
			CL_FALSE,
			0,
			channel_UV_Buff->getInfo<CL_MEM_SIZE>(),
			threadFilm->channel_UV->GetPixels());
	}
}

#endif
