/***************************************************************************
 * Copyright 1998-2015 by authors (see AUTHORS.txt)                        *
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

#include <cstdlib>
#include <cassert>
#include <deque>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#include "luxrays/core/dataset.h"
#include "luxrays/core/context.h"
#include "luxrays/core/trianglemesh.h"
#include "luxrays/accelerators/bvhaccel.h"
#include "luxrays/accelerators/qbvhaccel.h"
#include "luxrays/accelerators/mqbvhaccel.h"
#include "luxrays/accelerators/mbvhaccel.h"
#include "luxrays/accelerators/embreeaccel.h"
#include "luxrays/core/geometry/bsphere.h"

using namespace luxrays;

static u_int DataSetID = 0;
static boost::mutex DataSetIDMutex;

DataSet::DataSet(const Context *luxRaysContext) {
	{
		boost::unique_lock<boost::mutex> lock(DataSetIDMutex);
		dataSetID = DataSetID++;
	}
	context = luxRaysContext;

	totalVertexCount = 0;
	totalTriangleCount = 0;

	accelType = ACCEL_AUTO;
	preprocessed = false;
	hasInstances = false;
	enableInstanceSupport = true;
	hasMotionBlur = false;
	enableMotionBlurSupport = true;
}

DataSet::~DataSet() {
	for (boost::unordered_map<AcceleratorType, Accelerator *>::const_iterator it = accels.begin(); it != accels.end(); ++it)
		delete it->second;
}

TriangleMeshID DataSet::Add(const Mesh *mesh) {
	assert (!preprocessed);

	const TriangleMeshID id = meshes.size();
	meshes.push_back(mesh);

	totalVertexCount += mesh->GetTotalVertexCount();
	totalTriangleCount += mesh->GetTotalTriangleCount();

	if ((mesh->GetType() == TYPE_TRIANGLE_INSTANCE) || (mesh->GetType() == TYPE_EXT_TRIANGLE_INSTANCE))
		hasInstances = true;
	else if ((mesh->GetType() == TYPE_TRIANGLE_MOTION) || (mesh->GetType() == TYPE_EXT_TRIANGLE_MOTION))
		hasMotionBlur = true;

	return id;
}
void DataSet::Preprocess() {
	assert (!preprocessed);

	LR_LOG(context, "Preprocessing DataSet");
	LR_LOG(context, "Total vertex count: " << totalVertexCount);
	LR_LOG(context, "Total triangle count: " << totalTriangleCount);

	if (totalTriangleCount == 0)
		throw std::runtime_error("An empty DataSet can not be preprocessed");

	BOOST_FOREACH(const Mesh *m, meshes)
		bbox = Union(bbox, m->GetBBox());
	bsphere = bbox.BoundingSphere();

	preprocessed = true;
	LR_LOG(context, "Preprocessing DataSet done");
}

const Accelerator *DataSet::GetAccelerator() {
	boost::unordered_map<AcceleratorType, Accelerator *>::const_iterator it = accels.begin();
	return it->second;
}

const Accelerator *DataSet::GetAccelerator(const AcceleratorType accelType) {
	boost::unordered_map<AcceleratorType, Accelerator *>::const_iterator it = accels.find(accelType);
	if (it == accels.end()) {
		LR_LOG(context, "Adding DataSet accelerator: " << Accelerator::AcceleratorType2String(accelType));
		LR_LOG(context, "Total vertex count: " << totalVertexCount);
		LR_LOG(context, "Total triangle count: " << totalTriangleCount);

		if (totalTriangleCount == 0)
			throw std::runtime_error("An empty DataSet can not be preprocessed");

		// Build the Accelerator
		Accelerator *accel;
		switch (accelType) {
			case ACCEL_BVH: {
				const int treeType = 4; // Tree type to generate (2 = binary, 4 = quad, 8 = octree)
				const int costSamples = 0; // Samples to get for cost minimization
				const int isectCost = 80;
				const int travCost = 10;
				const float emptyBonus = 0.5f;

				accel = new BVHAccel(context, treeType, costSamples, isectCost, travCost, emptyBonus);
				break;
			}
			case ACCEL_QBVH: {
				const int maxPrimsPerLeaf = 4;
				const int fullSweepThreshold = 4 * maxPrimsPerLeaf;
				const int skipFactor = 1;

				accel = new QBVHAccel(context,
						maxPrimsPerLeaf, fullSweepThreshold, skipFactor);
				break;
			}
			case ACCEL_MQBVH: {
				const int fullSweepThreshold = 4;
				const int skipFactor = 1;

				accel = new MQBVHAccel(context, fullSweepThreshold, skipFactor);
				break;
			}
			case ACCEL_MBVH: {
				const int treeType = 4; // Tree type to generate (2 = binary, 4 = quad, 8 = octree)
				const int costSamples = 0; // Samples to get for cost minimization
				const int isectCost = 80;
				const int travCost = 10;
				const float emptyBonus = 0.5f;

				accel = new MBVHAccel(context, treeType, costSamples, isectCost, travCost, emptyBonus);
				break;
			}
			case ACCEL_EMBREE: {
				accel = new EmbreeAccel(context);
				break;
			}
			default:
				throw std::runtime_error("Unknown AcceleratorType in DataSet::AddAccelerator()");
		}

		accel->Init(meshes, totalVertexCount, totalTriangleCount);

		accels[accelType] = accel;

		return accel;
	} else
		return it->second;
}

bool DataSet::DoesAllAcceleratorsSupportUpdate() const {
	for (boost::unordered_map<AcceleratorType, Accelerator *>::const_iterator it = accels.begin(); it != accels.end(); ++it) {
		if (!it->second->DoesSupportUpdate())
			return false;
	}

	return true;
}

const void DataSet::Update() {
	for (boost::unordered_map<AcceleratorType, Accelerator *>::const_iterator it = accels.begin(); it != accels.end(); ++it) {
		assert(it->second->DoesSupportUpdate());
		it->second->Update();
	}
}

bool DataSet::IsEqual(const DataSet *dataSet) const {
	return (dataSet != NULL) && (dataSetID == dataSet->dataSetID);
}
