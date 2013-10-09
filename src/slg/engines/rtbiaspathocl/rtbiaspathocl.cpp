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

#include "slg/slg.h"
#include "slg/engines/rtbiaspathocl/rtbiaspathocl.h"

using namespace std;
using namespace luxrays;
using namespace slg;

//------------------------------------------------------------------------------
// RTBiasPathOCLRenderEngine
//------------------------------------------------------------------------------

RTBiasPathOCLRenderEngine::RTBiasPathOCLRenderEngine(RenderConfig *rcfg, Film *flm, boost::mutex *flmMutex) :
		BiasPathOCLRenderEngine(rcfg, flm, flmMutex, true) {
	frameBarrier = new boost::barrier(renderThreads.size() + 1);
	frameTime = 0.f;
}

RTBiasPathOCLRenderEngine::~RTBiasPathOCLRenderEngine() {
	delete frameBarrier;
}

BiasPathOCLRenderThread *RTBiasPathOCLRenderEngine::CreateOCLThread(const u_int index,
	OpenCLIntersectionDevice *device) {
	return new RTBiasPathOCLRenderThread(index, device, this);
}

void RTBiasPathOCLRenderEngine::StartLockLess() {
	const Properties &cfg = renderConfig->cfg;

	//--------------------------------------------------------------------------
	// Rendering parameters
	//--------------------------------------------------------------------------

	displayDeviceIndex = cfg.GetInt("rtpath.displaydevice.index", 0);
	if (displayDeviceIndex >= intersectionDevices.size())
		throw std::runtime_error("Not valid rtpath.displaydevice.index value: " + boost::lexical_cast<std::string>(displayDeviceIndex) +
				" >= " + boost::lexical_cast<std::string>(intersectionDevices.size()));

	BiasPathOCLRenderEngine::StartLockLess();
}

void RTBiasPathOCLRenderEngine::StopLockLess() {
	frameBarrier->wait();

	// All render threads are now suspended and I can set the interrupt signal
	for (size_t i = 0; i < renderThreads.size(); ++i)
		((RTBiasPathOCLRenderThread *)renderThreads[i])->renderThread->interrupt();

	frameBarrier->wait();

	// Render threads will now detect the interruption

	BiasPathOCLRenderEngine::StopLockLess();
}

void RTBiasPathOCLRenderEngine::UpdateFilmLockLess() {
	// Nothing to do: the display thread is in charge to update the film
}

bool RTBiasPathOCLRenderEngine::WaitNewFrame() {
	// Threads do the rendering
	const double t0 = WallClockTime();

	frameBarrier->wait();

	// Display thread does all frame post-processing steps 

	// Re-initialize the tile queue for the next frame
	tileRepository->InitTiles(film->GetWidth(), film->GetHeight());

	frameBarrier->wait();

	// Update the statistics
	UpdateCounters();

	frameTime = WallClockTime() - t0;

	return true;
}

#endif
