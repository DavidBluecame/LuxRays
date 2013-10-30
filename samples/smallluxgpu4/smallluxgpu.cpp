/***************************************************************************
 * Copyright 1998-2013 by authors (see AUTHORS.txt)                        *
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

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>

// Required when using XInitThread()
//#include <X11/Xlib.h>

#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include "luxrays/core/geometry/bbox.h"
#include "luxrays/core/device.h"
#include "luxrays/utils/ocl.h"

#include "smallluxgpu.h"
#include "displayfunc.h"
#include "slg/rendersession.h"
#include "slg/engines/filesaver/filesaver.h"
#include "slg/telnet/telnet.h"

using namespace std;
using namespace luxrays;
using namespace slg;

RenderConfig *config = NULL;
RenderSession *session = NULL;

//------------------------------------------------------------------------------
// Global options
//------------------------------------------------------------------------------

// Mouse "grab" mode. This is the natural way cameras are usually manipulated
// The flag is off by default but can be turned on by using the -m switch
bool optMouseGrabMode = false;
bool optUseLuxVRName = false;
bool optOSDPrintHelp = false;
bool optRealTimeMode = false;
bool optUseGameMode = false;
float optMoveScale = 1.f;
float optMoveStep = .5f;
float optRotateStep = 4.f;

//------------------------------------------------------------------------------

void LuxRaysDebugHandler(const char *msg) {
	cerr << "[LuxRays] " << msg << endl;
}

void SDLDebugHandler(const char *msg) {
	cerr << "[SDL] " << msg << endl;
}

void SLGDebugHandler(const char *msg) {
	if (optUseLuxVRName)
		cerr << "[LuxVR] " << msg << endl;
	else
		cerr << "[SLG] " << msg << endl;
}

#if defined(__GNUC__) && !defined(__CYGWIN__)
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <typeinfo>
#include <cxxabi.h>

static string Demangle(const char *symbol) {
	size_t size;
	int status;
	char temp[128];
	char* result;

	if (1 == sscanf(symbol, "%*[^'(']%*[^'_']%[^')''+']", temp)) {
		if (NULL != (result = abi::__cxa_demangle(temp, NULL, &size, &status))) {
			string r = result;
			return r + " [" + symbol + "]";
		}
	}

	if (1 == sscanf(symbol, "%127s", temp))
		return temp;

	return symbol;
}

void SLGTerminate(void) {
	SLG_LOG("=========================================================");
	SLG_LOG("Unhandled exception");

	void *array[32];
	size_t size = backtrace(array, 32);
	char **strings = backtrace_symbols(array, size);

	SLG_LOG("Obtained " << size << " stack frames.");

	for (size_t i = 0; i < size; i++)
		SLG_LOG("  " << Demangle(strings[i]));

	free(strings);
}
#endif

static void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message) {
	printf("\n*** ");
	if(fif != FIF_UNKNOWN)
		printf("%s Format\n", FreeImage_GetFormatFromFIF(fif));

	printf("%s", message);
	printf(" ***\n");
}

static int BatchSimpleMode(const double haltTime, const unsigned int haltSpp, const float haltThreshold) {
	RenderConfig *config = session->renderConfig;
	RenderEngine *engine = session->renderEngine;

	// Force the film update at 2.5secs (mostly used by PathOCL)
	config->SetScreenRefreshInterval(2500);

	// Start the rendering
	session->Start();

	const double startTime = WallClockTime();

	double lastFilmUpdate = WallClockTime();
	char buf[512];
	for (;;) {
		boost::this_thread::sleep(boost::posix_time::millisec(1000));

		// Check if periodic save is enabled
		if (session->NeedPeriodicFilmSave()) {
			// Time to save the image and film
			session->SaveFilm();
			lastFilmUpdate =  WallClockTime();
		} else {
			// Film update may be required by some render engine to
			// update statistics, convergence test and more
			if (WallClockTime() - lastFilmUpdate > 5.0) {
				session->renderEngine->UpdateFilm();
				lastFilmUpdate =  WallClockTime();
			}
		}

		const double now = WallClockTime();
		const double elapsedTime = now - startTime;
		if ((haltTime > 0) && (elapsedTime >= haltTime))
			break;

		const unsigned int pass = engine->GetPass();
		if ((haltSpp > 0) && (pass >= haltSpp))
			break;

		// Convergence test is update inside UpdateFilm()
		const float convergence = engine->GetConvergence();
		if ((haltThreshold >= 0.f) && (1.f - convergence <= haltThreshold))
			break;

		// Print some information about the rendering progress
		sprintf(buf, "[Elapsed time: %3d/%dsec][Samples %4d/%d][Convergence %f%%][Avg. samples/sec % 3.2fM on %.1fK tris]",
				int(elapsedTime), int(haltTime), pass, haltSpp, 100.f * convergence, engine->GetTotalSamplesSec() / 1000000.0,
				config->scene->dataSet->GetTotalTriangleCount() / 1000.0);

		SLG_LOG(buf);
	}

	// Stop the rendering
	session->Stop();

	// Save the rendered image
	session->SaveFilm();

	delete session;
	SLG_LOG("Done.");

	return EXIT_SUCCESS;
}

void UpdateMoveStep() {
	const BBox &worldBBox = session->renderConfig->scene->dataSet->GetBBox();
	int maxExtent = worldBBox.MaximumExtent();

	const float worldSize = Max(worldBBox.pMax[maxExtent] - worldBBox.pMin[maxExtent], .001f);
	optMoveStep = optMoveScale * worldSize / 50.f;
}

int main(int argc, char *argv[]) {
#if defined(__GNUC__) && !defined(__CYGWIN__)
	set_terminate(SLGTerminate);
#endif

	// This is required to run AMD GPU profiler
	//XInitThreads();

	LuxRays_DebugHandler = ::LuxRaysDebugHandler;
	SLG_DebugHandler = ::SLGDebugHandler;
	SLG_SDLDebugHandler = ::SDLDebugHandler;

	try {
		// Initialize FreeImage Library
		FreeImage_Initialise(TRUE);
		FreeImage_SetOutputMessage(FreeImageErrorHandler);

		bool batchMode = false;
		bool telnetServerEnabled = false;
		Properties cmdLineProp;
		string configFileName;
		for (int i = 1; i < argc; i++) {
			if (argv[i][0] == '-') {
				// I should check for out of range array index...

				if (argv[i][1] == 'h') {
					SLG_LOG("Usage: " << argv[0] << " [options] [configuration file]" << endl <<
							" -o [configuration file]" << endl <<
							" -f [scene file]" << endl <<
							" -w [window width]" << endl <<
							" -e [window height]" << endl <<
							" -t [halt time in secs]" << endl <<
							" -T <enable the telnet server>" << endl <<
							" -D [property name] [property value]" << endl <<
							" -d [current directory path]" << endl <<
							" -m Makes the mouse operations work in \"grab mode\"" << endl << 
							" -R <use LuxVR name>" << endl <<
							" -g <enable full screen mode>" << endl <<
							" -h <display this help and exit>");
					exit(EXIT_SUCCESS);
				}
				else if (argv[i][1] == 'o') {
					if (configFileName.compare("") != 0)
						throw runtime_error("Used multiple configuration files");

					configFileName = string(argv[++i]);
				}

				else if (argv[i][1] == 'e') cmdLineProp.SetString("film.height", argv[++i]);

				else if (argv[i][1] == 'w') cmdLineProp.SetString("film.width", argv[++i]);

				else if (argv[i][1] == 'f') cmdLineProp.SetString("scene.file", argv[++i]);

				else if (argv[i][1] == 't') cmdLineProp.SetString("batch.halttime", argv[++i]);

				else if (argv[i][1] == 'T') telnetServerEnabled = true;

				else if (argv[i][1] == 'm') optMouseGrabMode = true;

				else if (argv[i][1] == 'D') {
					cmdLineProp.SetString(argv[i + 1], argv[i + 2]);
					i += 2;
				}

				else if (argv[i][1] == 'd') boost::filesystem::current_path(boost::filesystem::path(argv[++i]));

				else if (argv[i][1] == 'R') optUseLuxVRName = true;

				else if (argv[i][1] == 'g') optUseGameMode = true;

				else {
					SLG_LOG("Invalid option: " << argv[i]);
					exit(EXIT_FAILURE);
				}
			} else {
				string s = argv[i];
				if ((s.length() >= 4) && (s.substr(s.length() - 4) == ".cfg")) {
					if (configFileName.compare("") != 0)
						throw runtime_error("Used multiple configuration files");
					configFileName = s;
				} else
					throw runtime_error("Unknown file extension: " + s);
			}
		}

		// Load the Scene
		
		if (configFileName.compare("") == 0)
			configFileName = "scenes/luxball/luxball.cfg";

		config = new RenderConfig(Properties(configFileName).Set(cmdLineProp));

		const unsigned int halttime = config->cfg.GetInt("batch.halttime", 0);
		const unsigned int haltspp = config->cfg.GetInt("batch.haltspp", 0);
		const float haltthreshold = config->cfg.GetFloat("batch.haltthreshold", -1.f);
		if ((halttime > 0) || (haltspp > 0) || (haltthreshold >= 0.f))
			batchMode = true;
		else
			batchMode = false;

		const bool fileSaverRenderEngine = (RenderEngine::String2RenderEngineType(
			config->cfg.GetString("renderengine.type", "PATHOCL")) == FILESAVER);

		if (fileSaverRenderEngine) {
			session = new RenderSession(config);

			// Save the scene and exit
			session->Start();
			session->Stop();

			delete session;
			SLG_LOG("Done.");

			return EXIT_SUCCESS;
		} else if (batchMode) {
			session = new RenderSession(config);

			return BatchSimpleMode(halttime, haltspp, haltthreshold);
		} else {
			// It is important to initialize OpenGL before OpenCL
			// (for OpenGL/OpenCL inter-operability)
			u_int width, height;
			config->GetScreenSize(&width, &height);
			InitGlut(argc, argv, width, height);

			session = new RenderSession(config);

			// Start the rendering
			session->Start();
			UpdateMoveStep();

			if (telnetServerEnabled) {
				TelnetServer telnetServer(18081, session);
				RunGlut();
			} else
				RunGlut();
		}
#if !defined(LUXRAYS_DISABLE_OPENCL)
	} catch (cl::Error err) {
		SLG_LOG("OpenCL ERROR: " << err.what() << "(" << oclErrorString(err.err()) << ")");
		return EXIT_FAILURE;
#endif
	} catch (runtime_error err) {
		SLG_LOG("RUNTIME ERROR: " << err.what());
		return EXIT_FAILURE;
	} catch (exception err) {
		SLG_LOG("ERROR: " << err.what());
		return EXIT_FAILURE;
	}

	delete session;
	delete config;

	return EXIT_SUCCESS;
}
