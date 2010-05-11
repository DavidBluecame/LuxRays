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

#include "luxrays/core/intersectiondevice.h"
#include "luxrays/core/context.h"
#include "luxrays/kernels/kernels.h"
#include "luxrays/accelerators/bvhaccel.h"
#include "luxrays/accelerators/qbvhaccel.h"

using namespace luxrays;

//------------------------------------------------------------------------------
// OpenCL IntersectionDevice
//------------------------------------------------------------------------------

#if !defined(LUXRAYS_DISABLE_OPENCL)

size_t OpenCLIntersectionDevice::RayBufferSize = OPENCL_RAYBUFFER_SIZE;

OpenCLIntersectionDevice::OpenCLIntersectionDevice(const Context *context,
		OpenCLDeviceDescription *desc,
		const unsigned int index, const unsigned int forceWorkGroupSize) :
		HardwareIntersectionDevice(context, DEVICE_TYPE_OPENCL, index) {
	deviceDesc = desc;
	deviceName = (desc->GetName() +"Intersect").c_str();
	reportedPermissionError = false;
	qbvhUseImage = true;
	intersectionThread = NULL;

	bvhKernel = NULL;
	qbvhKernel = NULL;
	qbvhImageKernel = NULL;
	oclQueue = NULL;

	bvhBuff = NULL;
	vertsBuff = NULL;
	trisBuff = NULL;
	bvhBuff = NULL;

	qbvhBuff = NULL;
	qbvhTrisBuff = NULL;

	qbvhImageBuffPartFloat = NULL;
	qbvhImageBuffPartInt = NULL;
	qbvhTrisImageBuffPartFloat = NULL;
	qbvhTrisImageBuffPartInt = NULL;

	externalRayBufferQueue = NULL;

	cl::Context &oclContext = deviceDesc->GetOCLContext();
	cl::Device &oclDevice = deviceDesc->GetOCLDevice();

	// Allocate the queue for this device
	oclQueue = new cl::CommandQueue(oclContext, oclDevice);

	//--------------------------------------------------------------------------
	// BVH kernel
	//--------------------------------------------------------------------------

	{
		// Compile sources
		cl::Program::Sources source(1, std::make_pair(KernelSource_BVH.c_str(), KernelSource_BVH.length()));
		cl::Program program = cl::Program(oclContext, source);
		try {
			VECTOR_CLASS<cl::Device> buildDevice;
			buildDevice.push_back(oclDevice);
			program.build(buildDevice, "-I.");
		} catch (cl::Error err) {
			cl::STRING_CLASS strError = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(oclDevice);
			LR_LOG(deviceContext, "[OpenCL device::" << deviceName << "] BVH compilation error:\n" << strError.c_str());

			throw err;
		}

		bvhKernel = new cl::Kernel(program, "Intersect");
		bvhKernel->getWorkGroupInfo<size_t>(oclDevice, CL_KERNEL_WORK_GROUP_SIZE, &bvhWorkGroupSize);
		LR_LOG(deviceContext, "[OpenCL device::" << deviceName << "] BVH kernel work group size: " << bvhWorkGroupSize);
		cl_ulong memSize;
		bvhKernel->getWorkGroupInfo<cl_ulong>(oclDevice, CL_KERNEL_LOCAL_MEM_SIZE, &memSize);
		LR_LOG(deviceContext, "[OpenCL device::" << deviceName << "] BVH kernel memory footprint: " << memSize);

		bvhKernel->getWorkGroupInfo<size_t>(oclDevice, CL_KERNEL_WORK_GROUP_SIZE, &bvhWorkGroupSize);
		LR_LOG(deviceContext, "[OpenCL device::" << deviceName << "]" << " Suggested work group size: " << bvhWorkGroupSize);

		if (forceWorkGroupSize > 0) {
			bvhWorkGroupSize = forceWorkGroupSize;
			LR_LOG(deviceContext, "[OpenCL device::" << deviceName << "]" << " Forced work group size: " << bvhWorkGroupSize);
		}
	}

	//--------------------------------------------------------------------------
	// QBVH kernel
	//--------------------------------------------------------------------------

	{
		// Compile sources
		{
			cl::Program::Sources source(1, std::make_pair(KernelSource_QBVH.c_str(), KernelSource_QBVH.length()));
			cl::Program program = cl::Program(oclContext, source);
			try {
				VECTOR_CLASS<cl::Device> buildDevice;
				buildDevice.push_back(oclDevice);
				program.build(buildDevice, "-I.");
			} catch (cl::Error err) {
				cl::STRING_CLASS strError = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(oclDevice);
				LR_LOG(deviceContext, "[OpenCL device::" << deviceName << "] QBVH compilation error:\n" << strError.c_str());

				throw err;
			}

			qbvhKernel = new cl::Kernel(program, "Intersect");
			qbvhKernel->getWorkGroupInfo<size_t>(oclDevice, CL_KERNEL_WORK_GROUP_SIZE, &qbvhWorkGroupSize);
			LR_LOG(deviceContext, "[OpenCL device::" << deviceName << "] QBVH kernel work group size: " << qbvhWorkGroupSize);
			cl_ulong memSize;
			qbvhKernel->getWorkGroupInfo<cl_ulong>(oclDevice, CL_KERNEL_LOCAL_MEM_SIZE, &memSize);
			LR_LOG(deviceContext, "[OpenCL device::" << deviceName << "] QBVH kernel memory footprint: " << memSize);

			qbvhKernel->getWorkGroupInfo<size_t>(oclDevice, CL_KERNEL_WORK_GROUP_SIZE, &qbvhWorkGroupSize);
			LR_LOG(deviceContext, "[OpenCL device::" << deviceName << "]" << " Suggested work group size: " << qbvhWorkGroupSize);

			if (forceWorkGroupSize > 0) {
				qbvhWorkGroupSize = forceWorkGroupSize;
				LR_LOG(deviceContext, "[OpenCL device::" << deviceName << "]" << " Forced work group size: " << qbvhWorkGroupSize);
			}
		}

		{
			cl::Program::Sources source(1, std::make_pair(KernelSource_QBVH.c_str(), KernelSource_QBVH.length()));
			cl::Program program = cl::Program(oclContext, source);
			try {
				VECTOR_CLASS<cl::Device> buildDevice;
				buildDevice.push_back(oclDevice);
				program.build(buildDevice, "-I. -DUSE_IMAGE_STORAGE");
			} catch (cl::Error err) {
				cl::STRING_CLASS strError = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(oclDevice);
				LR_LOG(deviceContext, "[OpenCL device::" << deviceName << "] QBVH Image Storage compilation error:\n" << strError.c_str());

				throw err;
			}

			qbvhImageKernel = new cl::Kernel(program, "Intersect");
			qbvhImageKernel->getWorkGroupInfo<size_t>(oclDevice, CL_KERNEL_WORK_GROUP_SIZE, &qbvhImageWorkGroupSize);
			LR_LOG(deviceContext, "[OpenCL device::" << deviceName << "] QBVH Image Storage kernel work group size: " << qbvhImageWorkGroupSize);
			cl_ulong memSize;
			qbvhImageKernel->getWorkGroupInfo<cl_ulong>(oclDevice, CL_KERNEL_LOCAL_MEM_SIZE, &memSize);
			LR_LOG(deviceContext, "[OpenCL device::" << deviceName << "] QBVH Image Storage kernel memory footprint: " << memSize);

			qbvhImageKernel->getWorkGroupInfo<size_t>(oclDevice, CL_KERNEL_WORK_GROUP_SIZE, &qbvhImageWorkGroupSize);
			LR_LOG(deviceContext, "[OpenCL device::" << deviceName << "]" << " Suggested work group size: " << qbvhImageWorkGroupSize);

			if (forceWorkGroupSize > 0) {
				qbvhImageWorkGroupSize = forceWorkGroupSize;
				LR_LOG(deviceContext, "[OpenCL device::" << deviceName << "]" << " Forced work group size: " << qbvhImageWorkGroupSize);
			}
		}
	}
}

OpenCLIntersectionDevice::~OpenCLIntersectionDevice() {
	if (started)
		Stop();

	if (dataSet) {
		deviceDesc->usedMemory -= raysBuff->getInfo<CL_MEM_SIZE>();
		delete raysBuff;
		deviceDesc->usedMemory -= hitsBuff->getInfo<CL_MEM_SIZE>();
		delete hitsBuff;

		if (bvhBuff) {
			deviceDesc->usedMemory -= vertsBuff->getInfo<CL_MEM_SIZE>();
			delete vertsBuff;
			deviceDesc->usedMemory -= trisBuff->getInfo<CL_MEM_SIZE>();
			delete trisBuff;
			deviceDesc->usedMemory -= bvhBuff->getInfo<CL_MEM_SIZE>();
			delete bvhBuff;
		}

		if (qbvhBuff) {
			if (qbvhUseImage) {
				deviceDesc->usedMemory -= qbvhImageBuffPartFloat->getInfo<CL_MEM_SIZE>();
				delete qbvhImageBuffPartFloat;
				deviceDesc->usedMemory -= qbvhImageBuffPartInt->getInfo<CL_MEM_SIZE>();
				delete qbvhImageBuffPartInt;
				deviceDesc->usedMemory -= qbvhTrisImageBuffPartFloat->getInfo<CL_MEM_SIZE>();
				delete qbvhTrisImageBuffPartFloat;
				deviceDesc->usedMemory -= qbvhTrisImageBuffPartInt->getInfo<CL_MEM_SIZE>();
				delete qbvhTrisImageBuffPartInt;
			} else {
				deviceDesc->usedMemory -= qbvhBuff->getInfo<CL_MEM_SIZE>();
				delete qbvhBuff;
				deviceDesc->usedMemory -= qbvhTrisBuff->getInfo<CL_MEM_SIZE>();
				delete qbvhTrisBuff;
			}
		}
	}

	delete bvhKernel;
	delete qbvhKernel;
	delete qbvhImageKernel;
	delete oclQueue;
}

void OpenCLIntersectionDevice::SetExternalRayBufferQueue(RayBufferQueue *queue) {
	assert (!started);

	externalRayBufferQueue = queue;
}

RayBuffer *OpenCLIntersectionDevice::NewRayBuffer() {
	return new RayBuffer(RayBufferSize);
}

void OpenCLIntersectionDevice::PushRayBuffer(RayBuffer *rayBuffer) {
	assert (started);
	assert (!externalRayBufferQueue);

	rayBufferQueue.PushToDo(rayBuffer, 0);
}

RayBuffer *OpenCLIntersectionDevice::PopRayBuffer() {
	assert (started);
	assert (!externalRayBufferQueue);

	return rayBufferQueue.PopDone(0);
}

void OpenCLIntersectionDevice::SetDataSet(const DataSet *newDataSet) {
	// Check if I have to free something from previous DataSet
	if (dataSet) {
		deviceDesc->usedMemory -= raysBuff->getInfo<CL_MEM_SIZE>();
		delete raysBuff;
		deviceDesc->usedMemory -= hitsBuff->getInfo<CL_MEM_SIZE>();
		delete hitsBuff;

		if (bvhBuff) {
			deviceDesc->usedMemory -= vertsBuff->getInfo<CL_MEM_SIZE>();
			delete vertsBuff;
			deviceDesc->usedMemory -= trisBuff->getInfo<CL_MEM_SIZE>();
			delete trisBuff;
			deviceDesc->usedMemory -= bvhBuff->getInfo<CL_MEM_SIZE>();
			delete bvhBuff;
		}

		if (qbvhBuff) {
			if (qbvhUseImage) {
				deviceDesc->usedMemory -= qbvhImageBuffPartFloat->getInfo<CL_MEM_SIZE>();
				delete qbvhImageBuffPartFloat;
				deviceDesc->usedMemory -= qbvhImageBuffPartInt->getInfo<CL_MEM_SIZE>();
				delete qbvhImageBuffPartInt;
				deviceDesc->usedMemory -= qbvhTrisImageBuffPartFloat->getInfo<CL_MEM_SIZE>();
				delete qbvhTrisImageBuffPartFloat;
				deviceDesc->usedMemory -= qbvhTrisImageBuffPartInt->getInfo<CL_MEM_SIZE>();
				delete qbvhTrisImageBuffPartInt;
			} else {
				deviceDesc->usedMemory -= qbvhBuff->getInfo<CL_MEM_SIZE>();
				delete qbvhBuff;
				deviceDesc->usedMemory -= qbvhTrisBuff->getInfo<CL_MEM_SIZE>();
				delete qbvhTrisBuff;
			}
		}
	}

	IntersectionDevice::SetDataSet(newDataSet);

	cl::Context &oclContext = deviceDesc->GetOCLContext();

	// Allocate OpenCL buffers
	LR_LOG(deviceContext, "[OpenCL device::" << deviceName << "]" << " Ray buffer size: " << (sizeof(Ray) * RayBufferSize / 1024) << "Kbytes");
	raysBuff = new cl::Buffer(oclContext,
			CL_MEM_READ_ONLY,
			sizeof(Ray) * RayBufferSize);
	deviceDesc->usedMemory += raysBuff->getInfo<CL_MEM_SIZE>();

	LR_LOG(deviceContext, "[OpenCL device::" << deviceName << "]" << " Ray hits buffer size: " << (sizeof(RayHit) * RayBufferSize / 1024) << "Kbytes");
	hitsBuff = new cl::Buffer(oclContext,
			CL_MEM_WRITE_ONLY,
			sizeof(RayHit) * RayBufferSize);
	deviceDesc->usedMemory += hitsBuff->getInfo<CL_MEM_SIZE>();

	switch (dataSet->GetAcceleratorType()) {
		case ACCEL_BVH: {
			LR_LOG(deviceContext, "[OpenCL device::" << deviceName << "]" << " Vertices buffer size: " << (sizeof(Point) * dataSet->GetTotalVertexCount() / 1024) << "Kbytes");
			vertsBuff = new cl::Buffer(oclContext,
					CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
					sizeof(Point) * dataSet->GetTotalVertexCount(),
					dataSet->GetTriangleMesh()->GetVertices());
			deviceDesc->usedMemory += vertsBuff->getInfo<CL_MEM_SIZE>();

			LR_LOG(deviceContext, "[OpenCL device::" << deviceName << "]" << " Triangle indices buffer size: " << (sizeof(Triangle) * dataSet->GetTotalTriangleCount() / 1024) << "Kbytes");
			trisBuff = new cl::Buffer(oclContext,
					CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
					sizeof(Triangle) * dataSet->GetTotalTriangleCount(),
					dataSet->GetTriangleMesh()->GetTriangles());
			deviceDesc->usedMemory += trisBuff->getInfo<CL_MEM_SIZE>();

			const BVHAccel *bvh = (BVHAccel *)dataSet->GetAccelerator();
			LR_LOG(deviceContext, "[OpenCL device::" << deviceName << "]" << " BVH buffer size: " << (sizeof(BVHAccelArrayNode) * bvh->nNodes / 1024) << "Kbytes");
			bvhBuff = new cl::Buffer(oclContext,
					CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
					sizeof(BVHAccelArrayNode) * bvh->nNodes,
					bvh->bvhTree);
			deviceDesc->usedMemory += bvhBuff->getInfo<CL_MEM_SIZE>();

			// Set arguments
			bvhKernel->setArg(0, *raysBuff);
			bvhKernel->setArg(1, *hitsBuff);
			bvhKernel->setArg(2, *vertsBuff);
			bvhKernel->setArg(3, *trisBuff);
			bvhKernel->setArg(4, dataSet->GetTotalTriangleCount());
			bvhKernel->setArg(5, bvh->nNodes);
			bvhKernel->setArg(6, *bvhBuff);
			break;
		}
		case ACCEL_QBVH: {
			const QBVHAccel *qbvh = (QBVHAccel *)dataSet->GetAccelerator();
			if (qbvhUseImage) {
				// Calculate the required image size for the storage

				{
					// 6 pixels required for the storage of a QBVH node BBoxes
					const size_t pixelRequired = qbvh->nNodes * 6;
					const size_t width = RoundUp<size_t>(sqrt(pixelRequired), 6);
					const size_t height = pixelRequired / width + (((pixelRequired % width) == 0) ? 0 : 1);

					__m128 *bboxes = new __m128[width * height * 6];
					for (size_t i = 0; i < qbvh->nNodes; ++i) {
						bboxes[i * 6 + 0] = qbvh->nodes[i].bboxes[0][0];
						bboxes[i * 6 + 1] = qbvh->nodes[i].bboxes[0][1];
						bboxes[i * 6 + 2] = qbvh->nodes[i].bboxes[0][2];
						bboxes[i * 6 + 3] = qbvh->nodes[i].bboxes[1][0];
						bboxes[i * 6 + 4] = qbvh->nodes[i].bboxes[1][1];
						bboxes[i * 6 + 5] = qbvh->nodes[i].bboxes[1][2];
					}

					LR_LOG(deviceContext, "[OpenCL device::" << deviceName << "] QBVH node image buffer size Part1: " << width << "x" << height);
					qbvhImageBuffPartFloat = new cl::Image2D(oclContext,
							CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
							cl::ImageFormat(CL_RGBA, CL_FLOAT), width, height, 0, bboxes);
					deviceDesc->usedMemory += qbvhImageBuffPartFloat->getInfo<CL_MEM_SIZE>();

					delete bboxes;
				}

				{
					// 1 pixels required for the storage of a QBVH node childrens
					const size_t pixelRequired = qbvh->nNodes * 1;
					const size_t width = sqrt(pixelRequired);
					const size_t height = pixelRequired / width + (((pixelRequired % width) == 0) ? 0 : 1);

					int *childrens = new int[width * height * 4];
					for (size_t i = 0; i < qbvh->nNodes; ++i) {
						childrens[i * 4 + 0] = qbvh->nodes[i].children[0];
						childrens[i * 4 + 1] = qbvh->nodes[i].children[1];
						childrens[i * 4 + 2] = qbvh->nodes[i].children[2];
						childrens[i * 4 + 3] = qbvh->nodes[i].children[3];
					}

					LR_LOG(deviceContext, "[OpenCL device::" << deviceName << "] QBVH node image buffer size Part2: " << width << "x" << height);
					qbvhImageBuffPartInt = new cl::Image2D(oclContext,
							CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
							cl::ImageFormat(CL_RGBA, CL_SIGNED_INT32), width, height, 0, childrens);
					deviceDesc->usedMemory += qbvhImageBuffPartInt->getInfo<CL_MEM_SIZE>();
					delete childrens;
				}

				{
					// 9 pixels required for the storage of QBVH Triangles
					const size_t pixelRequired = qbvh->nQuads * 9;
					const size_t width = RoundUp<size_t>(sqrt(pixelRequired), 9);
					const size_t height = pixelRequired / width + (((pixelRequired % width) == 0) ? 0 : 1);

					__m128 *tris = new __m128[width * height * 9];
					for (size_t i = 0; i < qbvh->nQuads; ++i) {
						tris[i * 9 + 0] = qbvh->prims[i].origx;
						tris[i * 9 + 1] = qbvh->prims[i].origy;
						tris[i * 9 + 2] = qbvh->prims[i].origz;
						tris[i * 9 + 3] = qbvh->prims[i].edge1x;
						tris[i * 9 + 4] = qbvh->prims[i].edge1y;
						tris[i * 9 + 5] = qbvh->prims[i].edge1z;
						tris[i * 9 + 6] = qbvh->prims[i].edge2x;
						tris[i * 9 + 7] = qbvh->prims[i].edge2y;
						tris[i * 9 + 8] = qbvh->prims[i].edge2z;
					}

					LR_LOG(deviceContext, "[OpenCL device::" << deviceName << "] QBVH triangle image buffer size Part1: " << width << "x" << height);
					qbvhTrisImageBuffPartFloat = new cl::Image2D(oclContext,
							CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
							cl::ImageFormat(CL_RGBA, CL_FLOAT), width, height, 0, tris);
					deviceDesc->usedMemory += qbvhTrisImageBuffPartFloat->getInfo<CL_MEM_SIZE>();

					delete tris;
				}

				{
					// 1 pixels required for the storage of QBVH Triangle indices
					const size_t pixelRequired = qbvh->nQuads * 1;
					const size_t width = sqrt(pixelRequired);
					const size_t height = pixelRequired / width + (((pixelRequired % width) == 0) ? 0 : 1);

					unsigned int *trisIdx = new unsigned int[width * height * 4];
					for (size_t i = 0; i < qbvh->nQuads; ++i) {
						trisIdx[i * 4 + 0] = qbvh->prims[i].primitives[0];
						trisIdx[i * 4 + 1] = qbvh->prims[i].primitives[1];
						trisIdx[i * 4 + 2] = qbvh->prims[i].primitives[2];
						trisIdx[i * 4 + 3] = qbvh->prims[i].primitives[3];
					}

					LR_LOG(deviceContext, "[OpenCL device::" << deviceName << "] QBVH triangle image buffer size Part2: " << width << "x" << height);
					qbvhTrisImageBuffPartInt = new cl::Image2D(oclContext,
							CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
							cl::ImageFormat(CL_RGBA, CL_UNSIGNED_INT32), width, height, 0, trisIdx);
					deviceDesc->usedMemory += qbvhTrisImageBuffPartInt->getInfo<CL_MEM_SIZE>();

					delete trisIdx;
				}

				// Set arguments
				qbvhImageKernel->setArg(0, *raysBuff);
				qbvhImageKernel->setArg(1, *hitsBuff);
				qbvhImageKernel->setArg(2, *qbvhImageBuffPartFloat);
				qbvhImageKernel->setArg(3, *qbvhImageBuffPartInt);
				qbvhImageKernel->setArg(4, *qbvhTrisImageBuffPartFloat);
				qbvhImageKernel->setArg(5, *qbvhTrisImageBuffPartInt);
				qbvhImageKernel->setArg(7, 24 * qbvhImageWorkGroupSize * sizeof(cl_int), NULL);
			} else {
				LR_LOG(deviceContext, "[OpenCL device::" << deviceName << "] QBVH buffer size: " << (sizeof(QBVHNode) * qbvh->nNodes / 1024) << "Kbytes");
				qbvhBuff = new cl::Buffer(oclContext,
						CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
						sizeof(QBVHNode) * qbvh->nNodes,
						qbvh->nodes);
				deviceDesc->usedMemory += qbvhBuff->getInfo<CL_MEM_SIZE>();

				LR_LOG(deviceContext, "[OpenCL device::" << deviceName << "] QuadTriangle buffer size: " << (sizeof(QuadTriangle) * qbvh->nQuads / 1024) << "Kbytes");
				qbvhTrisBuff = new cl::Buffer(oclContext,
						CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
						sizeof(QuadTriangle) * qbvh->nQuads,
						qbvh->prims);
				deviceDesc->usedMemory += qbvhTrisBuff->getInfo<CL_MEM_SIZE>();

				// Set arguments
				qbvhKernel->setArg(0, *raysBuff);
				qbvhKernel->setArg(1, *hitsBuff);
				qbvhKernel->setArg(2, *qbvhBuff);
				qbvhKernel->setArg(3, *qbvhTrisBuff);
				qbvhKernel->setArg(5, 24 * qbvhWorkGroupSize * sizeof(cl_int), NULL);
			}
			break;
		}
		default:
			assert (false);
	}
}

void OpenCLIntersectionDevice::Start() {
	IntersectionDevice::Start();

	// Create the thread for the rendering
	intersectionThread = new boost::thread(boost::bind(OpenCLIntersectionDevice::IntersectionThread, this));

	// Set intersectionThread priority
	bool res = SetThreadRRPriority(intersectionThread);
	if (res && !reportedPermissionError) {
		LR_LOG(deviceContext, "[OpenCL device::" << deviceName << "] Failed to set ray intersection thread priority (you probably need root/administrator permission to set thread realtime priority)");
		reportedPermissionError = true;
	}
}

void OpenCLIntersectionDevice::Interrupt() {
	assert (started);
	intersectionThread->interrupt();
}

void OpenCLIntersectionDevice::Stop() {
	IntersectionDevice::Stop();

	intersectionThread->interrupt();
	intersectionThread->join();
	delete intersectionThread;
	intersectionThread = NULL;

	if (!externalRayBufferQueue)
		rayBufferQueue.Clear();
}

void OpenCLIntersectionDevice::TraceRayBuffer(RayBuffer *rayBuffer, cl::Event *event) {
	// Download the rays to the GPU
	oclQueue->enqueueWriteBuffer(
			*raysBuff,
			CL_FALSE,
			0,
			sizeof(Ray) * rayBuffer->GetRayCount(),
			rayBuffer->GetRayBuffer());

	switch (dataSet->GetAcceleratorType()) {
		case ACCEL_BVH: {
			bvhKernel->setArg(7, (unsigned int)rayBuffer->GetRayCount());
			oclQueue->enqueueNDRangeKernel(*bvhKernel, cl::NullRange,
				cl::NDRange(rayBuffer->GetSize()), cl::NDRange(bvhWorkGroupSize));
			break;
		}
		case ACCEL_QBVH: {
			if (qbvhUseImage) {
				qbvhImageKernel->setArg(6, (unsigned int)rayBuffer->GetRayCount());
				oclQueue->enqueueNDRangeKernel(*qbvhImageKernel, cl::NullRange,
					cl::NDRange(rayBuffer->GetSize()), cl::NDRange(qbvhImageWorkGroupSize));
			} else {
				qbvhKernel->setArg(4, (unsigned int)rayBuffer->GetRayCount());
				oclQueue->enqueueNDRangeKernel(*qbvhKernel, cl::NullRange,
					cl::NDRange(rayBuffer->GetSize()), cl::NDRange(qbvhWorkGroupSize));
			}
			break;
		}
		default:
			assert (false);
	}

	// Upload the results
	oclQueue->enqueueReadBuffer(
			*hitsBuff,
			CL_FALSE,
			0,
			sizeof(RayHit) * rayBuffer->GetRayCount(),
			rayBuffer->GetHitBuffer(), NULL, event);
}

void OpenCLIntersectionDevice::IntersectionThread(OpenCLIntersectionDevice *renderDevice) {
	LR_LOG(renderDevice->deviceContext, "[OpenCL device::" << renderDevice->deviceName << "] Rendering thread started");

	try {
		RayBufferQueue *queue = renderDevice->externalRayBufferQueue ?
			renderDevice->externalRayBufferQueue : &(renderDevice->rayBufferQueue);

		RayBuffer *rayBuffer0, *rayBuffer1, *rayBuffer2;
		const double startTime = WallClockTime();
		while (!boost::this_thread::interruption_requested()) {
			const double t1 = WallClockTime();
			queue->Pop3xToDo(&rayBuffer0, &rayBuffer1, &rayBuffer2);
			renderDevice->statsDeviceIdleTime += WallClockTime() - t1;
			const unsigned int count = (rayBuffer0 ? 1 : 0) + (rayBuffer1 ? 1 : 0) + (rayBuffer2 ? 1 : 0);

			switch(count) {
				case 1: {
					// Only one ray buffer to trace available
					cl::Event event;
					renderDevice->TraceRayBuffer(rayBuffer0, &event);

					event.wait();
					renderDevice->statsTotalRayCount += rayBuffer0->GetRayCount();
					queue->PushDone(rayBuffer0);

					renderDevice->statsDeviceTotalTime = WallClockTime() - startTime;
					break;
				}
				case 2: {
					// At least 2 ray buffers to trace

					// Trace 0 ray buffer
					cl::Event event0;
					renderDevice->TraceRayBuffer(rayBuffer0, &event0);

					// Trace 1 ray buffer
					cl::Event event1;
					renderDevice->TraceRayBuffer(rayBuffer1, &event1);

					// Pop 0 ray buffer
					event0.wait();
					renderDevice->statsTotalRayCount += rayBuffer0->GetRayCount();
					queue->PushDone(rayBuffer0);

					// Pop 1 ray buffer
					event1.wait();
					renderDevice->statsTotalRayCount += rayBuffer1->GetRayCount();
					queue->PushDone(rayBuffer1);

					renderDevice->statsDeviceTotalTime = WallClockTime() - startTime;
					break;
				}
				case 3: {
					// At least 3 ray buffers to trace

					// Trace 0 ray buffer
					cl::Event event0;
					renderDevice->TraceRayBuffer(rayBuffer0, &event0);

					// Trace 1 ray buffer
					cl::Event event1;
					renderDevice->TraceRayBuffer(rayBuffer1, &event1);

					// Trace 2 ray buffer
					cl::Event event2;
					renderDevice->TraceRayBuffer(rayBuffer2, &event2);

					// Pop 0 ray buffer
					event0.wait();
					renderDevice->statsTotalRayCount += rayBuffer0->GetRayCount();
					queue->PushDone(rayBuffer0);

					// Pop 1 ray buffer
					event1.wait();
					renderDevice->statsTotalRayCount += rayBuffer1->GetRayCount();
					queue->PushDone(rayBuffer1);

					// Pop 2 ray buffer
					event2.wait();
					renderDevice->statsTotalRayCount += rayBuffer2->GetRayCount();
					queue->PushDone(rayBuffer2);

					renderDevice->statsDeviceTotalTime = WallClockTime() - startTime;
					break;
				}
				default:
					assert (false);
			}
		}

		LR_LOG(renderDevice->deviceContext, "[OpenCL device::" << renderDevice->deviceName << "] Rendering thread halted");
	} catch (boost::thread_interrupted) {
		LR_LOG(renderDevice->deviceContext, "[OpenCL device::" << renderDevice->deviceName << "] Rendering thread halted");
	} catch (cl::Error err) {
		LR_LOG(renderDevice->deviceContext, "[OpenCL device::" << renderDevice->deviceName << "] Rendering thread ERROR: " << err.what() << "(" << err.err() << ")");
	}
}

#endif
