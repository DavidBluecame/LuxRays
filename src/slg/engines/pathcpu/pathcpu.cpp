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

#include "slg/engines/pathcpu/pathcpu.h"

using namespace luxrays;
using namespace slg;

//------------------------------------------------------------------------------
// PathCPURenderEngine
//------------------------------------------------------------------------------

PathCPURenderEngine::PathCPURenderEngine(const RenderConfig *rcfg, Film *flm, boost::mutex *flmMutex) :
		CPUNoTileRenderEngine(rcfg, flm, flmMutex) {
	film->AddChannel(Film::RADIANCE_PER_PIXEL_NORMALIZED);
	film->SetOverlappedScreenBufferUpdateFlag(true);
	film->SetRadianceGroupCount(rcfg->scene->lightDefs.GetLightGroupCount());
	film->Init();
}

void PathCPURenderEngine::StartLockLess() {
	const Properties &cfg = renderConfig->cfg;

	//--------------------------------------------------------------------------
	// Rendering parameters
	//--------------------------------------------------------------------------

	maxPathDepth = (u_int)Max(1, cfg.Get(Property("path.maxdepth")(5)).Get<int>());
	rrDepth = (u_int)Max(1, cfg.Get(Property("path.russianroulette.depth")(3)).Get<int>());
	rrImportanceCap = Clamp(cfg.Get(Property("path.russianroulette.cap")(.5f)).Get<float>(), 0.f, 1.f);

	// Clamping settings
	radianceClampMaxValue = Max(0.f, cfg.Get(Property("path.clamping.radiance.maxvalue")(0.f)).Get<float>());
	pdfClampValue = Max(0.f, cfg.Get(Property("path.clamping.pdf.value")(0.f)).Get<float>());

	CPUNoTileRenderEngine::StartLockLess();
}
