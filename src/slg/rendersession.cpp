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
	// Create the filter
	//--------------------------------------------------------------------------

	const FilterType filterType = Filter::String2FilterType(cfg.GetString("film.filter.type", "GAUSSIAN"));
	const float filterWidth = cfg.GetFloat("film.filter.width", 1.5f);

	Filter *filter;
	switch (filterType) {
		case FILTER_NONE:
			filter = NULL;
			break;
		case FILTER_BOX:
			filter = new BoxFilter(filterWidth, filterWidth);
			break;
		case FILTER_GAUSSIAN: {
			const float alpha = cfg.GetFloat("film.filter.gaussian.alpha", 2.f);
			filter = new GaussianFilter(filterWidth, filterWidth, alpha);
			break;
		}
		case FILTER_MITCHELL: {
			const float b = cfg.GetFloat("film.filter.mitchell.b", 1.f / 3.f);
			const float c = cfg.GetFloat("film.filter.mitchell.c", 1.f / 3.f);
			filter = new MitchellFilter(filterWidth, filterWidth, b, c);
			break;
		}
		case FILTER_MITCHELL_SS: {
			const float b = cfg.GetFloat("film.filter.mitchellss.b", 1.f / 3.f);
			const float c = cfg.GetFloat("film.filter.mitchellss.c", 1.f / 3.f);
			filter = new MitchellFilterSS(filterWidth, filterWidth, b, c);
			break;
		}
		default:
			throw std::runtime_error("Unknown filter type: " + boost::lexical_cast<std::string>(filterType));
	}

	//--------------------------------------------------------------------------
	// Create the Film
	//--------------------------------------------------------------------------

	film = new Film(filmFullWidth, filmFullHeight);
	film->SetFilter(filter);

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
		film->SetGamma(gamma);

	// For compatibility with the past
	if (cfg.IsDefined("film.alphachannel.enable")) {
		if (cfg.GetInt("film.alphachannel.enable", 0) != 0)
			film->AddChannel(Film::ALPHA);
		else
			film->RemoveChannel(Film::ALPHA);
	}

	//--------------------------------------------------------------------------
	// Initialize the FilmOutputs
	//--------------------------------------------------------------------------

	set<string> outputNames;
	vector<string> outputKeys = cfg.GetAllNames("film.outputs.");
	for (vector<string>::const_iterator outputKey = outputKeys.begin(); outputKey != outputKeys.end(); ++outputKey) {
		const string &key = *outputKey;
		const size_t dot1 = key.find(".", string("film.outputs.").length());
		if (dot1 == string::npos)
			continue;

		// Extract the output type name
		const string outputName = Property::ExtractField(key, 2);
		if (outputName == "")
			throw runtime_error("Syntax error in film output definition: " + outputName);

		if (outputNames.count(outputName) > 0)
			continue;

		outputNames.insert(outputName);
		const string type = cfg.GetString("film.outputs." + outputName + ".type", "RGB_TONEMAPPED");
		const string fileName = cfg.GetString("film.outputs." + outputName + ".filename", "image.png");

		SDL_LOG("Film output definition: " << type << " [" << fileName << "]");

		// Check if it is a supported file format
		FREE_IMAGE_FORMAT fif = FREEIMAGE_GETFIFFROMFILENAME(FREEIMAGE_CONVFILENAME(fileName).c_str());
		if (fif == FIF_UNKNOWN)
			throw std::runtime_error("Unknown image format in film output: " + outputName);

		// HDR image or not
		const bool hdrImage = ((fif == FIF_HDR) || (fif == FIF_EXR));

		if (type == "RGB") {
			if (hdrImage)
				filmOutputs.Add(FilmOutputs::RGB, fileName);
			else
				throw std::runtime_error("Not tonemapped image can be saved only in HDR formats: " + outputName);
		} else if (type == "RGBA") {
			if (hdrImage) {
				film->AddChannel(Film::ALPHA);
				filmOutputs.Add(FilmOutputs::RGBA, fileName);
			} else
				throw std::runtime_error("Not tonemapped image can be saved only in HDR formats: " + outputName);
		} else if (type == "RGB_TONEMAPPED")
			filmOutputs.Add(FilmOutputs::RGB_TONEMAPPED, fileName);
		else if (type == "RGBA_TONEMAPPED") {
			film->AddChannel(Film::ALPHA);
			filmOutputs.Add(FilmOutputs::RGBA_TONEMAPPED, fileName);
		} else if (type == "ALPHA") {
			film->AddChannel(Film::ALPHA);
			filmOutputs.Add(FilmOutputs::ALPHA, fileName);
		} else if (type == "DEPTH") {
			if (hdrImage) {
				film->AddChannel(Film::DEPTH);
				filmOutputs.Add(FilmOutputs::DEPTH, fileName);
			} else
				throw std::runtime_error("Depth image can be saved only in HDR formats: " + outputName);
		} else if (type == "POSITION") {
			if (hdrImage) {
				film->AddChannel(Film::DEPTH);
				film->AddChannel(Film::POSITION);
				filmOutputs.Add(FilmOutputs::POSITION, fileName);
			} else
				throw std::runtime_error("Position image can be saved only in HDR formats: " + outputName);
		} else if (type == "GEOMETRY_NORMAL") {
			if (hdrImage) {
				film->AddChannel(Film::DEPTH);
				film->AddChannel(Film::GEOMETRY_NORMAL);
				filmOutputs.Add(FilmOutputs::GEOMETRY_NORMAL, fileName);
			} else
				throw std::runtime_error("Geometry normal image can be saved only in HDR formats: " + outputName);
		} else if (type == "SHADING_NORMAL") {
			if (hdrImage) {
				film->AddChannel(Film::DEPTH);
				film->AddChannel(Film::SHADING_NORMAL);
				filmOutputs.Add(FilmOutputs::SHADING_NORMAL, fileName);
			} else
				throw std::runtime_error("Shading normal image can be saved only in HDR formats: " + outputName);
		} else if (type == "MATERIAL_ID") {
			if (!hdrImage) {
				film->AddChannel(Film::DEPTH);
				film->AddChannel(Film::MATERIAL_ID);
				filmOutputs.Add(FilmOutputs::MATERIAL_ID, fileName);
			} else
				throw std::runtime_error("Material ID image can be saved only in no HDR formats: " + outputName);
		} else if (type == "DIRECT_DIFFUSE") {
			if (hdrImage) {
				film->AddChannel(Film::DIRECT_DIFFUSE);
				filmOutputs.Add(FilmOutputs::DIRECT_DIFFUSE, fileName);
			} else
				throw std::runtime_error("Direct diffuse image can be saved only in HDR formats: " + outputName);
		} else if (type == "DIRECT_GLOSSY") {
			if (hdrImage) {
				film->AddChannel(Film::DIRECT_GLOSSY);
				filmOutputs.Add(FilmOutputs::DIRECT_GLOSSY, fileName);
			} else
				throw std::runtime_error("Direct glossy image can be saved only in HDR formats: " + outputName);
		} else if (type == "EMISSION") {
			if (hdrImage) {
				film->AddChannel(Film::EMISSION);
				filmOutputs.Add(FilmOutputs::EMISSION, fileName);
			} else
				throw std::runtime_error("Emission image can be saved only in HDR formats: " + outputName);
		} else if (type == "INDIRECT_DIFFUSE") {
			if (hdrImage) {
				film->AddChannel(Film::INDIRECT_DIFFUSE);
				filmOutputs.Add(FilmOutputs::INDIRECT_DIFFUSE, fileName);
			} else
				throw std::runtime_error("Indirect diffuse image can be saved only in HDR formats: " + outputName);
		} else if (type == "INDIRECT_GLOSSY") {
			if (hdrImage) {
				film->AddChannel(Film::INDIRECT_GLOSSY);
				filmOutputs.Add(FilmOutputs::INDIRECT_GLOSSY, fileName);
			} else
				throw std::runtime_error("Indirect glossy image can be saved only in HDR formats: " + outputName);
		} else if (type == "INDIRECT_SPECULAR") {
			if (hdrImage) {
				film->AddChannel(Film::INDIRECT_SPECULAR);
				filmOutputs.Add(FilmOutputs::INDIRECT_SPECULAR, fileName);
			} else
				throw std::runtime_error("Indirect specular image can be saved only in HDR formats: " + outputName);
		} else if (type == "MATERIAL_ID_MASK") {
			const u_int materialID = cfg.GetInt("film.outputs." + outputName + ".id", 255);
			Properties prop;
			prop.SetString("id", ToString(materialID));

			film->AddChannel(Film::MATERIAL_ID);
			film->AddChannel(Film::MATERIAL_ID_MASK, &prop);
			filmOutputs.Add(FilmOutputs::MATERIAL_ID_MASK, fileName, &prop);
		} else if (type == "DIRECT_SHADOW_MASK") {
			film->AddChannel(Film::DIRECT_SHADOW_MASK);
			filmOutputs.Add(FilmOutputs::DIRECT_SHADOW_MASK, fileName);
		} else if (type == "INDIRECT_SHADOW_MASK") {
			film->AddChannel(Film::INDIRECT_SHADOW_MASK);
			filmOutputs.Add(FilmOutputs::INDIRECT_SHADOW_MASK, fileName);
		} else if (type == "RADIANCE_GROUP") {
			const u_int lightID = cfg.GetInt("film.outputs." + outputName + ".id", 0);
			Properties prop;
			prop.SetString("id", ToString(lightID));

			filmOutputs.Add(FilmOutputs::RADIANCE_GROUP, fileName, &prop);
		} else if (type == "UV") {
			film->AddChannel(Film::DEPTH);
			film->AddChannel(Film::UV);
			filmOutputs.Add(FilmOutputs::UV, fileName);
		} else if (type == "RAYCOUNT") {
			film->AddChannel(Film::RAYCOUNT);
			filmOutputs.Add(FilmOutputs::RAYCOUNT, fileName);
		} else
			throw std::runtime_error("Unknown type in film output: " + type);
	}

	// For compatibility with the past
	if (cfg.IsDefined("image.filename")) {
		filmOutputs.Add(film->HasChannel(Film::ALPHA) ? FilmOutputs::RGBA_TONEMAPPED : FilmOutputs::RGB_TONEMAPPED,
				cfg.GetString("image.filename", "image.png"));
	}

	// For compatibility with the past, old default
	if (filmOutputs.GetCount() == 0)
		filmOutputs.Add(FilmOutputs::RGB_TONEMAPPED, "image.png");

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

	if ((renderEngine->GetEngineType() != RTPATHOCL) &&
			(renderEngine->GetEngineType() != RTBIASPATHOCL)) {
		SLG_LOG("[RenderSession] Edit actions: " << editActions);

		// RTPATHOCL and RTBIASPATHOCL handle film Reset on their own
		if (editActions.HasAnyAction())
			film->Reset();
	}

	renderEngine->EndEdit(editActions);
	editMode = false;
}

void RenderSession::SetRenderingEngineType(const RenderEngineType engineType) {
	if (engineType != renderEngine->GetEngineType()) {
		Stop();

		// I need to allocate an uninitialized Film
		Film *newFilm = new Film(film->GetWidth(), film->GetHeight());
		newFilm->CopyDynamicSettings(*film);

		// Otherwise, switching from BIDIR to PATH will leave an unused channel
		newFilm->RemoveChannel(Film::RADIANCE_PER_PIXEL_NORMALIZED);
		newFilm->RemoveChannel(Film::RADIANCE_PER_SCREEN_NORMALIZED);
		delete film;
		film = newFilm;

		// Allocate the new rendering engine
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

void RenderSession::FilmSave() {
	// Ask to the RenderEngine to update the film
	renderEngine->UpdateFilm();

	// renderEngine->UpdateFilm() uses the film lock on its own
	boost::unique_lock<boost::mutex> lock(filmMutex);

	// Save the film
	film->UpdateScreenBuffer();
	film->Output(filmOutputs);
}
