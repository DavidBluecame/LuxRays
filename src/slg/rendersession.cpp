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

#include "slg/rendersession.h"

using namespace std;
using namespace luxrays;
using namespace slg;

string slg::SLG_LABEL = "SmallLuxGPU v" SLG_VERSION_MAJOR "." SLG_VERSION_MINOR " (LuxRays demo: http://www.luxrender.net)";
string slg::LUXVR_LABEL = "LuxVR v" SLG_VERSION_MAJOR "." SLG_VERSION_MINOR " (http://www.luxrender.net)";

void (*slg::LuxRays_DebugHandler)(const char *msg) = NULL;
void (*slg::SLG_DebugHandler)(const char *msg) = NULL;

// Empty debug handler
void slg::NullDebugHandler(const char *msg) {
}

RenderSession::RenderSession(RenderConfig *rcfg) {
	renderConfig = rcfg;
	started = false;
	editMode = false;

	const Properties &cfg = renderConfig->cfg;

	periodiceSaveTime = cfg.GetFloat("batch.periodicsave", 0.f);
	lastPeriodicSave = WallClockTime();
	periodicSaveEnabled = (periodiceSaveTime > 0.f);

	//--------------------------------------------------------------------------
	// Update the Camera
	//--------------------------------------------------------------------------

	u_int filmFullWidth, filmFullHeight, filmSubRegion[4];
	u_int *subRegion = (renderConfig->GetFilmSize(&filmFullWidth, &filmFullHeight, filmSubRegion)) ?
		filmSubRegion : NULL;
	renderConfig->scene->camera->Update(filmFullWidth, filmFullHeight, subRegion);

	//--------------------------------------------------------------------------
	// Create the Film
	//--------------------------------------------------------------------------

	film = new Film(filmFullWidth, filmFullHeight);

	const FilterType filterType = Filter::String2FilterType(cfg.GetString("film.filter.type", "GAUSSIAN"));
	film->SetFilterType(filterType);

	const int toneMapType = cfg.GetInt("film.tonemap.type", 0);
	if (toneMapType == 0) {
		LinearToneMapParams params;
		params.scale = cfg.GetFloat("film.tonemap.linear.scale", params.scale);
		film->SetToneMapParams(params);
	} else {
		Reinhard02ToneMapParams params;
		params.preScale = cfg.GetFloat("film.tonemap.reinhard02.prescale", params.preScale);
		params.postScale = cfg.GetFloat("film.tonemap.reinhard02.postscale", params.postScale);
		params.burn = cfg.GetFloat("film.tonemap.reinhard02.burn", params.burn);
		film->SetToneMapParams(params);
	}

	const float gamma = cfg.GetFloat("film.gamma", 2.2f);
	if (gamma != 2.2f)
		film->InitGammaTable(gamma);

	// Check if I have to enable the alpha channel
	film->SetAlphaChannelFlag(cfg.GetInt("film.alphachannel.enable", 0) != 0);

	//--------------------------------------------------------------------------
	// Create the RenderEngine
	//--------------------------------------------------------------------------

	// Check the kind of render engine to start
	const RenderEngineType renderEngineType = RenderEngine::String2RenderEngineType(cfg.GetString("renderengine.type", "PATHOCL"));
	renderEngine = RenderEngine::AllocRenderEngine(renderEngineType, renderConfig, film, &filmMutex);
}

RenderSession::~RenderSession() {
	if (editMode)
		EndEdit();
	if (started)
		Stop();

	delete renderEngine;
	delete film;
	delete renderConfig;
}

void RenderSession::Start() {
	assert (!started);
	started = true;

	renderEngine->Start();
}

void RenderSession::Stop() {
	assert (started);
	started = false;

	renderEngine->Stop();
}

void RenderSession::BeginEdit() {
	assert (started);
	assert (!editMode);

	renderEngine->BeginEdit();
	editActions.Reset();

	editMode = true;
}

void RenderSession::EndEdit() {
	assert (started);
	assert (editMode);

	if (editActions.HasAnyAction())
		film->Reset();

	if (renderEngine->GetEngineType() != RTPATHOCL)
		SLG_LOG("[RenderSession] Edit actions: " << editActions);
	renderEngine->EndEdit(editActions);
	editMode = false;
}

void RenderSession::SetRenderingEngineType(const RenderEngineType engineType) {
	if (engineType != renderEngine->GetEngineType()) {
		Stop();

		delete renderEngine;
		renderEngine = RenderEngine::AllocRenderEngine(engineType,
				renderConfig, film, &filmMutex);

		Start();
	}
}

bool RenderSession::NeedPeriodicSave() {
	if (periodicSaveEnabled) {
		const double now = WallClockTime();
		if (now - lastPeriodicSave > periodiceSaveTime) {
			lastPeriodicSave = now;
			return true;
		} else
			return false;
	} else
		return false;
}

void RenderSession::SaveFilmImage() {
	// Ask to the RenderEngine to update the film
	renderEngine->UpdateFilm();

	// renderEngine->UpdateFilm() uses the film lock on its own
	boost::unique_lock<boost::mutex> lock(filmMutex);

	// Save the film
	const string fileName = renderConfig->cfg.GetString("image.filename", "image.png");
	film->UpdateScreenBuffer();
	film->SaveScreenBuffer(fileName);
}

