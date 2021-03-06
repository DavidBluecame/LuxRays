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

#ifndef _SLG_METROPOLIS_SAMPLER_H
#define	_SLG_METROPOLIS_SAMPLER_H

#include <string>
#include <vector>

#include "luxrays/core/randomgen.h"
#include "slg/slg.h"
#include "slg/film/film.h"
#include "slg/samplers/sampler.h"

namespace slg {

//------------------------------------------------------------------------------
// Metropolis sampler
//------------------------------------------------------------------------------

class MetropolisSampler : public Sampler {
public:
	MetropolisSampler(luxrays::RandomGenerator *rnd, Film *film, const u_int maxRej,
			const float pLarge, const float imgRange,
			double *sharedTotalLuminance, double *sharedSampleCount);
	virtual ~MetropolisSampler();

	virtual SamplerType GetType() const { return METROPOLIS; }
	virtual void RequestSamples(const u_int size);

	virtual float GetSample(const u_int index);
	virtual void NextSample(const std::vector<SampleResult> &sampleResults);

private:
	u_int maxRejects;
	float largeMutationProbability, imageMutationRange;

	// I'm storing totalLuminance and sampleCount on external (shared) variables
	// in order to have far more accurate estimation in the image mean intensity
	// computation
	double *sharedTotalLuminance, *sharedSampleCount;

	u_int sampleSize;
	float *samples;
	u_int *sampleStamps;

	float weight;
	u_int consecRejects;
	u_int stamp;

	// Data saved for the current sample
	u_int currentStamp;
	double currentLuminance;
	float *currentSamples;
	u_int *currentSampleStamps;
	std::vector<SampleResult> currentSampleResult;

	bool isLargeMutation, cooldown;
};

}

#endif	/* _SLG_METROPOLIS_SAMPLER_H */
