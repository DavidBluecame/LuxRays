#include <string>
namespace slg { namespace ocl {
std::string KernelSource_samplers = 
"#line 2 \"samplers.cl\"\n"
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
"#if (PARAM_SAMPLER_TYPE == 0)\n"
"#define Sampler_GetSamplePath(index) (Rnd_FloatValue(seed))\n"
"#define Sampler_GetSamplePathVertex(index) (Rnd_FloatValue(seed))\n"
"#elif (PARAM_SAMPLER_TYPE == 1)\n"
"#define Sampler_GetSamplePath(index) (sampleDataPathBase[index])\n"
"#define Sampler_GetSamplePathVertex(index) (sampleDataPathVertexBase[index])\n"
"#endif\n"
"\n"
"void GenerateCameraRay(\n"
"		__global Camera *camera,\n"
"		__global Sample *sample,\n"
"		__global float *sampleDataPathBase,\n"
"		Seed *seed,\n"
"		__global Ray *ray) {\n"
"#if (PARAM_SAMPLER_TYPE == 0)\n"
"	// Don't use Sampler_GetSample() here\n"
"	const float scrSampleX = sampleDataPathBase[IDX_SCREEN_X];\n"
"	const float scrSampleY = sampleDataPathBase[IDX_SCREEN_Y];\n"
"#else\n"
"	const float scrSampleX = Sampler_GetSamplePath(IDX_SCREEN_X);\n"
"	const float scrSampleY = Sampler_GetSamplePath(IDX_SCREEN_Y);\n"
"#endif\n"
"\n"
"	const float screenX = min(scrSampleX * PARAM_IMAGE_WIDTH, (float)(PARAM_IMAGE_WIDTH - 1));\n"
"	const float screenY = min(scrSampleY * PARAM_IMAGE_HEIGHT, (float)(PARAM_IMAGE_HEIGHT - 1));\n"
"\n"
"	float3 Pras = (float3)(screenX, PARAM_IMAGE_HEIGHT - screenY - 1.f, 0.f);\n"
"	float3 rayOrig = Transform_ApplyPoint(&camera->rasterToCamera, Pras);\n"
"	float3 rayDir = rayOrig;\n"
"\n"
"	const float hither = camera->hither;\n"
"\n"
"#if defined(PARAM_CAMERA_HAS_DOF)\n"
"	const float dofSampleX = Sampler_GetSamplePath(IDX_DOF_X);\n"
"	const float dofSampleY = Sampler_GetSamplePath(IDX_DOF_Y);\n"
"\n"
"	// Sample point on lens\n"
"	float lensU, lensV;\n"
"	ConcentricSampleDisk(dofSampleX, dofSampleY, &lensU, &lensV);\n"
"	const float lensRadius = camera->lensRadius;\n"
"	lensU *= lensRadius;\n"
"	lensV *= lensRadius;\n"
"\n"
"	// Compute point on plane of focus\n"
"	const float focalDistance = camera->focalDistance;\n"
"	const float dist = focalDistance - hither;\n"
"	const float ft = dist / rayDir.z;\n"
"	float3 Pfocus;\n"
"	Pfocus = rayOrig + rayDir * ft;\n"
"\n"
"	// Update ray for effect of lens\n"
"	const float k = dist / focalDistance;\n"
"	rayOrig.x += lensU * k;\n"
"	rayOrig.y += lensV * k;\n"
"\n"
"	rayDir = Pfocus - rayOrig;\n"
"#endif\n"
"\n"
"	rayDir = normalize(rayDir);\n"
"	const float maxt = (camera->yon - hither) / rayDir.z;\n"
"\n"
"	// Transform ray in world coordinates\n"
"	rayOrig = Transform_ApplyPoint(&camera->cameraToWorld, rayOrig);\n"
"	rayDir = Transform_ApplyVector(&camera->cameraToWorld, rayDir);\n"
"\n"
"	Ray_Init3(ray, rayOrig, rayDir, maxt);\n"
"\n"
"	/*printf(\"(%f, %f, %f) (%f, %f, %f) [%f, %f]\\n\",\n"
"		ray->o.x, ray->o.y, ray->o.z, ray->d.x, ray->d.y, ray->d.z,\n"
"		ray->mint, ray->maxt);*/\n"
"}\n"
"\n"
"void GenerateCameraPath(\n"
"		__global GPUTask *task,\n"
"		__global float *sampleDataPathBase,\n"
"		Seed *seed,\n"
"		__global Camera *camera,\n"
"		__global Ray *ray) {\n"
"	__global Sample *sample = &task->sample;\n"
"\n"
"	GenerateCameraRay(camera, sample, sampleDataPathBase, seed, ray);\n"
"\n"
"	vstore3(BLACK, 0, &sample->radiance.r);\n"
"#if defined(PARAM_ENABLE_ALPHA_CHANNEL)\n"
"	sample->alpha = 1.f;\n"
"#endif\n"
"\n"
"	// Initialize the path state\n"
"	task->pathStateBase.state = RT_NEXT_VERTEX;\n"
"	task->pathStateBase.depth = 1;\n"
"	vstore3(WHITE, 0, &task->pathStateBase.throughput.r);\n"
"#if defined(PARAM_DIRECT_LIGHT_SAMPLING)\n"
"	task->directLightState.lastPdfW = 1.f;\n"
"	task->directLightState.lastSpecular = TRUE;\n"
"#endif\n"
"}\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// Random Sampler Kernel\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if (PARAM_SAMPLER_TYPE == 0)\n"
"\n"
"__global float *Sampler_GetSampleData(__global Sample *sample, __global float *samplesData) {\n"
"	const size_t gid = get_global_id(0);\n"
"	return &samplesData[gid * TOTAL_U_SIZE];\n"
"}\n"
"\n"
"__global float *Sampler_GetSampleDataPathBase(__global Sample *sample, __global float *sampleData) {\n"
"	return sampleData;\n"
"}\n"
"\n"
"__global float *Sampler_GetSampleDataPathVertex(__global Sample *sample,\n"
"		__global float *sampleDataPathBase, const uint depth) {\n"
"	return &sampleDataPathBase[IDX_BSDF_OFFSET + depth * SAMPLE_SIZE];\n"
"}\n"
"\n"
"void Sampler_Init(Seed *seed, __global Sample *sample, __global float *sampleData) {\n"
"	const size_t gid = get_global_id(0);\n"
"\n"
"	vstore3(BLACK, 0, &sample->radiance.r);\n"
"#if defined(PARAM_ENABLE_ALPHA_CHANNEL)\n"
"	sample->alpha = 1.f;\n"
"#endif\n"
"\n"
"	sampleData[IDX_SCREEN_X] = Rnd_FloatValue(seed);\n"
"	sampleData[IDX_SCREEN_Y] = Rnd_FloatValue(seed);\n"
"}\n"
"\n"
"void Sampler_NextSample(\n"
"		__global GPUTask *task,\n"
"		__global Sample *sample,\n"
"		__global float *sampleData,\n"
"		Seed *seed,\n"
"		__global Pixel *frameBuffer,\n"
"#if defined(PARAM_ENABLE_ALPHA_CHANNEL)\n"
"		__global AlphaPixel *alphaFrameBuffer,\n"
"#endif\n"
"		__global Camera *camera,\n"
"		__global Ray *ray\n"
"		) {\n"
"	const float scrX = sampleData[IDX_SCREEN_X];\n"
"	const float scrY = sampleData[IDX_SCREEN_Y];\n"
"\n"
"#if (PARAM_IMAGE_FILTER_TYPE == 0)\n"
"	const uint pixelIndex = PixelIndexFloat2D(scrX, scrY);\n"
"	SplatSample(frameBuffer,\n"
"#if defined(PARAM_ENABLE_ALPHA_CHANNEL)\n"
"			alphaFrameBuffer,\n"
"#endif\n"
"			pixelIndex, vload3(0, &sample->radiance.r),\n"
"#if defined(PARAM_ENABLE_ALPHA_CHANNEL)\n"
"			sample->alpha,\n"
"#endif\n"
"			1.f);\n"
"#else\n"
"	float sx, sy;\n"
"	const uint pixelIndex = PixelIndexFloat2DWithOffset(scrX, scrY, &sx, &sy);\n"
"	SplatSample(frameBuffer,\n"
"#if defined(PARAM_ENABLE_ALPHA_CHANNEL)\n"
"			alphaFrameBuffer,\n"
"#endif\n"
"			pixelIndex, sx, sy, vload3(0, &sample->radiance.r),\n"
"#if defined(PARAM_ENABLE_ALPHA_CHANNEL)\n"
"			sample->alpha,\n"
"#endif\n"
"			1.f);\n"
"#endif\n"
"\n"
"	// Move to the next assigned pixel\n"
"#if defined(PARAM_ENABLE_ALPHA_CHANNEL)\n"
"	sample->alpha = 1.f;\n"
"#endif\n"
"\n"
"	sampleData[IDX_SCREEN_X] = Rnd_FloatValue(seed);\n"
"	sampleData[IDX_SCREEN_Y] = Rnd_FloatValue(seed);\n"
"\n"
"	__global float *sampleDataPathBase = Sampler_GetSampleDataPathBase(sample, sampleData);\n"
"	GenerateCameraPath(task, sampleDataPathBase, seed, camera, ray);\n"
"}\n"
"\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// Metropolis Sampler Kernel\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if (PARAM_SAMPLER_TYPE == 1)\n"
"\n"
"__global float *Sampler_GetSampleData(__global Sample *sample, __global float *samplesData) {\n"
"	const size_t gid = get_global_id(0);\n"
"	return &samplesData[gid * (2 *TOTAL_U_SIZE)];\n"
"}\n"
"\n"
"__global float *Sampler_GetSampleDataPathBase(__global Sample *sample, __global float *sampleData) {\n"
"	const size_t gid = get_global_id(0);\n"
"	return &sampleData[sample->proposed * TOTAL_U_SIZE];\n"
"}\n"
"\n"
"__global float *Sampler_GetSampleDataPathVertex(__global Sample *sample,\n"
"		__global float *sampleDataPathBase, const uint depth) {\n"
"	return &sampleDataPathBase[IDX_BSDF_OFFSET + depth * SAMPLE_SIZE];\n"
"}\n"
"\n"
"void LargeStep(Seed *seed, const uint largeStepCount, __global float *proposedU) {\n"
"	for (int i = 0; i < TOTAL_U_SIZE; ++i)\n"
"		proposedU[i] = Rnd_FloatValue(seed);\n"
"}\n"
"\n"
"float Mutate(Seed *seed, const float x) {\n"
"	const float s1 = 1.f / 512.f;\n"
"	const float s2 = 1.f / 16.f;\n"
"\n"
"	const float randomValue = Rnd_FloatValue(seed);\n"
"\n"
"	const float dx = s1 / (s1 / s2 + fabs(2.f * randomValue - 1.f)) -\n"
"		s1 / (s1 / s2 + 1.f);\n"
"\n"
"	float mutatedX = x;\n"
"	if (randomValue < 0.5f) {\n"
"		mutatedX += dx;\n"
"		mutatedX = (mutatedX < 1.f) ? mutatedX : (mutatedX - 1.f);\n"
"	} else {\n"
"		mutatedX -= dx;\n"
"		mutatedX = (mutatedX < 0.f) ? (mutatedX + 1.f) : mutatedX;\n"
"	}\n"
"\n"
"	return mutatedX;\n"
"}\n"
"\n"
"float MutateScaled(Seed *seed, const float x, const float range) {\n"
"	const float s1 = 32.f;\n"
"\n"
"	const float randomValue = Rnd_FloatValue(seed);\n"
"\n"
"	const float dx = range / (s1 / (1.f + s1) + (s1 * s1) / (1.f + s1) *\n"
"		fabs(2.f * randomValue - 1.f)) - range / s1;\n"
"\n"
"	float mutatedX = x;\n"
"	if (randomValue < 0.5f) {\n"
"		mutatedX += dx;\n"
"		mutatedX = (mutatedX < 1.f) ? mutatedX : (mutatedX - 1.f);\n"
"	} else {\n"
"		mutatedX -= dx;\n"
"		mutatedX = (mutatedX < 0.f) ? (mutatedX + 1.f) : mutatedX;\n"
"	}\n"
"\n"
"	return mutatedX;\n"
"}\n"
"\n"
"void SmallStep(Seed *seed, __global float *currentU, __global float *proposedU) {\n"
"	proposedU[IDX_SCREEN_X] = MutateScaled(seed, currentU[IDX_SCREEN_X],\n"
"			PARAM_SAMPLER_METROPOLIS_IMAGE_MUTATION_RANGE);\n"
"	proposedU[IDX_SCREEN_Y] = MutateScaled(seed, currentU[IDX_SCREEN_Y],\n"
"			PARAM_SAMPLER_METROPOLIS_IMAGE_MUTATION_RANGE);\n"
"\n"
"	for (int i = IDX_SCREEN_Y + 1; i < TOTAL_U_SIZE; ++i)\n"
"		proposedU[i] = Mutate(seed, currentU[i]);\n"
"}\n"
"\n"
"void Sampler_Init(Seed *seed, __global Sample *sample, __global float *sampleData) {\n"
"	sample->totalI = 0.f;\n"
"	sample->largeMutationCount = 1.f;\n"
"\n"
"	sample->current = NULL_INDEX;\n"
"	sample->proposed = 1;\n"
"\n"
"	sample->smallMutationCount = 0;\n"
"	sample->consecutiveRejects = 0;\n"
"\n"
"	sample->weight = 0.f;\n"
"	vstore3(BLACK, 0, &sample->currentRadiance.r);\n"
"	vstore3(BLACK, 0, &sample->radiance.r);\n"
"#if defined(PARAM_ENABLE_ALPHA_CHANNEL)\n"
"	sample->currentAlpha = 1.f;\n"
"	sample->alpha = 1.f;\n"
"#endif\n"
"\n"
"	__global float *sampleDataPathBase = Sampler_GetSampleDataPathBase(sample, sampleData);\n"
"	LargeStep(seed, 0, sampleDataPathBase);\n"
"}\n"
"\n"
"void Sampler_NextSample(\n"
"		__global GPUTask *task,\n"
"		__global Sample *sample,\n"
"		__global float *sampleData,\n"
"		Seed *seed,\n"
"		__global Pixel *frameBuffer,\n"
"#if defined(PARAM_ENABLE_ALPHA_CHANNEL)\n"
"		__global AlphaPixel *alphaFrameBuffer,\n"
"#endif\n"
"		__global Camera *camera,\n"
"		__global Ray *ray\n"
"		) {\n"
"	//--------------------------------------------------------------------------\n"
"	// Accept/Reject the sample\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	uint current = sample->current;\n"
"	uint proposed = sample->proposed;\n"
"\n"
"	const float3 radiance = vload3(0, &sample->radiance.r);\n"
"\n"
"	if (current == NULL_INDEX) {\n"
"		// It is the very first sample, I have still to initialize the current\n"
"		// sample\n"
"\n"
"		vstore3(radiance, 0, &sample->currentRadiance.r);\n"
"#if defined(PARAM_ENABLE_ALPHA_CHANNEL)\n"
"		sample->currentAlpha = sample->alpha;\n"
"#endif\n"
"		sample->totalI = Spectrum_Y(radiance);\n"
"\n"
"		current = proposed;\n"
"		proposed ^= 1;\n"
"	} else {\n"
"		const float3 currentL = vload3(0, &sample->currentRadiance.r);\n"
"#if defined(PARAM_ENABLE_ALPHA_CHANNEL)\n"
"		const float currentAlpha = sample->currentAlpha;\n"
"#endif\n"
"		const float currentI = Spectrum_Y(currentL);\n"
"\n"
"		const float3 proposedL = radiance;\n"
"#if defined(PARAM_ENABLE_ALPHA_CHANNEL)\n"
"		const float proposedAlpha = sample->alpha;\n"
"#endif\n"
"		float proposedI = Spectrum_Y(proposedL);\n"
"		proposedI = isinf(proposedI) ? 0.f : proposedI;\n"
"\n"
"		float totalI = sample->totalI;\n"
"		uint largeMutationCount = sample->largeMutationCount;\n"
"		uint smallMutationCount = sample->smallMutationCount;\n"
"		if (smallMutationCount == 0) {\n"
"			// It is a large mutation\n"
"			totalI += Spectrum_Y(proposedL);\n"
"			largeMutationCount += 1;\n"
"\n"
"			sample->totalI = totalI;\n"
"			sample->largeMutationCount = largeMutationCount;\n"
"		}\n"
"\n"
"		const float meanI = (totalI > 0.f) ? (totalI / largeMutationCount) : 1.f;\n"
"\n"
"		// Calculate accept probability from old and new image sample\n"
"		uint consecutiveRejects = sample->consecutiveRejects;\n"
"\n"
"		float accProb;\n"
"		if ((currentI > 0.f) && (consecutiveRejects < PARAM_SAMPLER_METROPOLIS_MAX_CONSECUTIVE_REJECT))\n"
"			accProb = min(1.f, proposedI / currentI);\n"
"		else\n"
"			accProb = 1.f;\n"
"\n"
"		const float newWeight = accProb + ((smallMutationCount == 0) ? 1.f : 0.f);\n"
"		float weight = sample->weight;\n"
"		weight += 1.f - accProb;\n"
"\n"
"		const float rndVal = Rnd_FloatValue(seed);\n"
"\n"
"		/*if (get_global_id(0) == 0)\n"
"			printf(\\\"[%d] Current: (%f, %f, %f) [%f] Proposed: (%f, %f, %f) [%f] accProb: %f <%f>\\\\n\\\",\n"
"					consecutiveRejects,\n"
"					currentL.r, currentL.g, currentL.b, weight,\n"
"					proposedL.r, proposedL.g, proposedL.b, newWeight,\n"
"					accProb, rndVal);*/\n"
"\n"
"		float3 contrib;\n"
"#if defined(PARAM_ENABLE_ALPHA_CHANNEL)\n"
"		float contribAlpha;\n"
"#endif\n"
"		float norm;\n"
"		float scrX, scrY;\n"
"\n"
"		if ((accProb == 1.f) || (rndVal < accProb)) {\n"
"			/*if (get_global_id(0) == 0)\n"
"				printf(\\\"\\\\t\\\\tACCEPTED !\\\\n\\\");*/\n"
"\n"
"			// Add accumulated contribution of previous reference sample\n"
"			norm = weight / (currentI / meanI + PARAM_SAMPLER_METROPOLIS_LARGE_STEP_RATE);\n"
"			contrib = currentL;\n"
"#if defined(PARAM_ENABLE_ALPHA_CHANNEL)\n"
"			contribAlpha = currentAlpha;\n"
"#endif\n"
"			scrX = sampleData[current * TOTAL_U_SIZE + IDX_SCREEN_X];\n"
"			scrY = sampleData[current * TOTAL_U_SIZE + IDX_SCREEN_Y];\n"
"\n"
"			current ^= 1;\n"
"			proposed ^= 1;\n"
"			consecutiveRejects = 0;\n"
"\n"
"			weight = newWeight;\n"
"\n"
"			vstore3(proposedL, 0, &sample->currentRadiance.r);\n"
"#if defined(PARAM_ENABLE_ALPHA_CHANNEL)\n"
"			sample->currentAlpha = proposedAlpha;\n"
"#endif\n"
"		} else {\n"
"			/*if (get_global_id(0) == 0)\n"
"				printf(\\\"\\\\t\\\\tREJECTED !\\\\n\\\");*/\n"
"\n"
"			// Add contribution of new sample before rejecting it\n"
"			norm = newWeight / (proposedI / meanI + PARAM_SAMPLER_METROPOLIS_LARGE_STEP_RATE);\n"
"			contrib = proposedL;\n"
"#if defined(PARAM_ENABLE_ALPHA_CHANNEL)\n"
"			contribAlpha = proposedAlpha;\n"
"#endif\n"
"\n"
"			scrX = sampleData[proposed * TOTAL_U_SIZE + IDX_SCREEN_X];\n"
"			scrY = sampleData[proposed * TOTAL_U_SIZE + IDX_SCREEN_Y];\n"
"\n"
"			++consecutiveRejects;\n"
"		}\n"
"\n"
"		if (norm > 0.f) {\n"
"			/*if (get_global_id(0) == 0)\n"
"				printf(\\\"\\\\t\\\\tContrib: (%f, %f, %f) [%f] consecutiveRejects: %d\\\\n\\\",\n"
"						contrib.r, contrib.g, contrib.b, norm, consecutiveRejects);*/\n"
"\n"
"#if (PARAM_IMAGE_FILTER_TYPE == 0)\n"
"			const uint pixelIndex = PixelIndexFloat2D(scrX, scrY);\n"
"			SplatSample(frameBuffer,\n"
"#if defined(PARAM_ENABLE_ALPHA_CHANNEL)\n"
"				alphaFrameBuffer,\n"
"#endif\n"
"				pixelIndex, contrib,\n"
"#if defined(PARAM_ENABLE_ALPHA_CHANNEL)\n"
"				contribAlpha,\n"
"#endif\n"
"				norm);\n"
"#else\n"
"			float sx, sy;\n"
"			const uint pixelIndex = PixelIndexFloat2DWithOffset(scrX, scrY, &sx, &sy);\n"
"			SplatSample(frameBuffer,\n"
"#if defined(PARAM_ENABLE_ALPHA_CHANNEL)\n"
"				alphaFrameBuffer,\n"
"#endif\n"
"				pixelIndex, sx, sy, contrib,\n"
"#if defined(PARAM_ENABLE_ALPHA_CHANNEL)\n"
"				contribAlpha,\n"
"#endif\n"
"				norm);\n"
"#endif\n"
"		}\n"
"\n"
"		sample->weight = weight;\n"
"		sample->consecutiveRejects = consecutiveRejects;\n"
"	}\n"
"\n"
"	sample->current = current;\n"
"	sample->proposed = proposed;\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Mutate the sample\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	__global float *proposedU = &sampleData[proposed * TOTAL_U_SIZE];\n"
"	if (Rnd_FloatValue(seed) < PARAM_SAMPLER_METROPOLIS_LARGE_STEP_RATE) {\n"
"		LargeStep(seed, sample->largeMutationCount, proposedU);\n"
"		sample->smallMutationCount = 0;\n"
"	} else {\n"
"		__global float *currentU = &sampleData[current * TOTAL_U_SIZE];\n"
"\n"
"		SmallStep(seed, currentU, proposedU);\n"
"		sample->smallMutationCount += 1;\n"
"	}\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Generate a new camera path\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	__global float *sampleDataPathBase = Sampler_GetSampleDataPathBase(sample, sampleData);\n"
"	GenerateCameraPath(task, sampleDataPathBase, seed, camera, ray);\n"
"}\n"
"\n"
"#endif\n"
; } }
