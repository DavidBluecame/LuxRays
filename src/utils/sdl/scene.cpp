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

#include <cstdlib>
#include <istream>
#include <stdexcept>
#include <sstream>

#include <boost/detail/container_fwd.hpp>

#include "luxrays/core/dataset.h"
#include "luxrays/utils/properties.h"
#include "luxrays/utils/sdl/sdl.h"
#include "luxrays/utils/sdl/scene.h"
#include "luxrays/core/intersectiondevice.h"

using namespace luxrays;
using namespace luxrays::sdl;

Scene::Scene(const int accType) {
	camera = NULL;

	infiniteLight = NULL;
	sunLight = NULL;

	dataSet = NULL;

	extMeshCache = new ExtMeshCache();
	texMapCache = new TextureMapCache();

	accelType = accType;
}

Scene::Scene(const std::string &fileName, const int accType) {
	accelType = accType;

	extMeshCache = new ExtMeshCache();
	texMapCache = new TextureMapCache();

	SDL_LOG("Reading scene: " << fileName);

	Properties scnProp(fileName);

	//--------------------------------------------------------------------------
	// Read camera position and target
	//--------------------------------------------------------------------------

	CreateCamera(scnProp);

	//--------------------------------------------------------------------------
	// Read all materials
	//--------------------------------------------------------------------------

	AddMaterials(scnProp);

	//--------------------------------------------------------------------------
	// Read all objects .ply file
	//--------------------------------------------------------------------------

	std::vector<std::string> objKeys = scnProp.GetAllKeys("scene.objects.");
	if (objKeys.size() == 0)
		throw std::runtime_error("Unable to find object definitions");

	double lastPrint = WallClockTime();
	unsigned int objCount = 0;
	for (std::vector<std::string>::const_iterator objKey = objKeys.begin(); objKey != objKeys.end(); ++objKey) {
		const std::string &key = *objKey;

		// Check if it is the root of the definition of an object otherwise skip
		const size_t dot1 = key.find(".", std::string("scene.objects.").length());
		if (dot1 == std::string::npos)
			continue;
		const size_t dot2 = key.find(".", dot1 + 1);
		if (dot2 != std::string::npos)
			continue;

		// Extract the material name
		const std::string matName = Properties::ExtractField(key, 2);
		if (matName == "")
			throw std::runtime_error("Syntax error in material name: " + matName);

		// Extract the object name
		const std::string objName = Properties::ExtractField(key, 3);
		if (objName == "")
			throw std::runtime_error("Syntax error in " + key);

		AddObject(objName, matName, scnProp);
		++objCount;

		const double now = WallClockTime();
		if (now - lastPrint > 2.0) {
			SDL_LOG("PLY object count: " << objCount);
			lastPrint = now;
		}
	}
	SDL_LOG("PLY object count: " << objCount);

	//--------------------------------------------------------------------------
	// Check if there is an infinitelight source defined
	//--------------------------------------------------------------------------

	AddInfiniteLight(scnProp);

	//--------------------------------------------------------------------------
	// Check if there is a SkyLight defined
	//--------------------------------------------------------------------------

	AddSkyLight(scnProp);

	//--------------------------------------------------------------------------
	// Check if there is a SunLight defined
	//--------------------------------------------------------------------------

	AddSunLight(scnProp);

	//--------------------------------------------------------------------------

	dataSet = NULL;
}

Scene::~Scene() {
	delete camera;
	delete infiniteLight;
	delete sunLight;

	for (std::vector<LightSource *>::const_iterator l = lights.begin(); l != lights.end(); ++l)
		delete *l;
	for (std::vector<Material *>::const_iterator m = materials.begin(); m != materials.end(); ++m)
		delete *m;

	delete dataSet;

	delete extMeshCache;
	delete texMapCache;
}

void Scene::UpdateDataSet(Context *ctx) {
	delete dataSet;
	dataSet = new DataSet(ctx);

	// Check the type of accelerator to use
	switch (accelType) {
		case -1:
			// Use default settings
			break;
		case 0:
			dataSet->SetAcceleratorType(ACCEL_BVH);
			break;
		case 1:
		case 2:
			dataSet->SetAcceleratorType(ACCEL_QBVH);
			break;
		case 3:
			dataSet->SetAcceleratorType(ACCEL_MQBVH);
			break;
		default:
			throw std::runtime_error("Unknown accelerator.type");
			break;
	}

	// Add all objects
	for (std::vector<ExtMesh *>::const_iterator obj = objects.begin(); obj != objects.end(); ++obj)
		dataSet->Add(*obj);

	dataSet->Preprocess();
}

std::vector<float> Scene::GetParameters(const Properties &prop, const std::string &paramName,
		const unsigned int paramCount, const std::string &defaultValue) {
	const std::vector<float> vf = prop.GetFloatVector(paramName, defaultValue);
	if (vf.size() != paramCount) {
		std::stringstream ss;
		ss << "Syntax error in " << paramName << " (required " << paramCount << " parameters)";
		throw std::runtime_error(ss.str());
	}

	return vf;
}

//--------------------------------------------------------------------------
// Methods to build a scene from scratch
//--------------------------------------------------------------------------

void Scene::CreateCamera(const std::string &propsString) {
	Properties prop;
	prop.LoadFromString(propsString);

	CreateCamera(prop);
}

void Scene::CreateCamera(const Properties &props) {
	std::vector<float> vf = GetParameters(props, "scene.camera.lookat", 6, "10.0 0.0 0.0  0.0 0.0 0.0");
	Point orig(vf.at(0), vf.at(1), vf.at(2));
	Point target(vf.at(3), vf.at(4), vf.at(5));

	SDL_LOG("Camera postion: " << orig);
	SDL_LOG("Camera target: " << target);

	vf = GetParameters(props, "scene.camera.up", 3, "0.0 0.0 0.1");
	const Vector up(vf.at(0), vf.at(1), vf.at(2));

	if (props.IsDefined("scene.camera.screenwindow")) {
		vf = GetParameters(props, "scene.camera.screenwindow", 4, "0.0 1.0 0.0 1.0");

		camera = new PerspectiveCamera(orig, target, up, &vf[0]);
	} else
		camera = new PerspectiveCamera(orig, target, up);

	camera->clipHither = props.GetFloat("scene.camera.cliphither", 1e-3f);
	camera->clipYon = props.GetFloat("scene.camera.clipyon", 1e30f);
	camera->lensRadius = props.GetFloat("scene.camera.lensradius", 0.f);
	camera->focalDistance = props.GetFloat("scene.camera.focaldistance", 10.f);
	camera->fieldOfView = props.GetFloat("scene.camera.fieldofview", 45.f);
}

void Scene::AddMaterials(const std::string &propsString) {
	Properties prop;
	prop.LoadFromString(propsString);

	AddMaterials(prop);
}

void Scene::AddMaterials(const Properties &props) {
	std::vector<std::string> matKeys = props.GetAllKeys("scene.materials.");
	if (matKeys.size() == 0)
		throw std::runtime_error("No material definition found");

	for (std::vector<std::string>::const_iterator matKey = matKeys.begin(); matKey != matKeys.end(); ++matKey) {
		const std::string &key = *matKey;
		const std::string matType = Properties::ExtractField(key, 2);
		if (matType == "")
			throw std::runtime_error("Syntax error in " + key);
		const std::string matName = Properties::ExtractField(key, 3);
		if (matName == "")
			throw std::runtime_error("Syntax error in " + key);
		SDL_LOG("Material definition: " << matName << " [" << matType << "]");

		Material *mat = CreateMaterial(key, props);

		materialIndices[matName] = materials.size();
		materials.push_back(mat);
	}
}

void Scene::AddObject(const std::string &objName, const std::string &matName,
		const std::string &meshName, const std::string &propsString) {
	Properties prop;
	prop.LoadFromString("scene.objects." + matName + "." + objName + " = " + meshName + "\n");
	prop.LoadFromString(propsString);

	AddObject(objName, matName, prop);
}

void Scene::AddObject(const std::string &objName, const std::string &matName, const Properties &props) {
	const std::string key = "scene.objects." + matName + "." + objName;

	// Build the object
	const std::vector<std::string> args = props.GetStringVector(key, "");
	const std::string plyFileName = args.at(0);

	// Check if I have to calculate normal or not
	const bool usePlyNormals = (props.GetInt(key + ".useplynormals", 0) != 0);

	// Check if I have to use an instance mesh or not
	ExtMesh *meshObject;
	if (props.IsDefined(key + ".transformation")) {
		const std::vector<float> vf = GetParameters(props, key + ".transformation", 16, "1.0 0.0 0.0 0.0  0.0 1.0 0.0 0.0  0.0 0.0 1.0 0.0  0.0 0.0 0.0 1.0");
		const Matrix4x4 mat(
				vf.at(0), vf.at(4), vf.at(8), vf.at(12),
				vf.at(1), vf.at(5), vf.at(9), vf.at(13),
				vf.at(2), vf.at(6), vf.at(10), vf.at(14),
				vf.at(3), vf.at(7), vf.at(11), vf.at(15));
		const Transform trans(mat);

		meshObject = extMeshCache->GetExtMesh(plyFileName, usePlyNormals, trans);
	} else
		meshObject = extMeshCache->GetExtMesh(plyFileName, usePlyNormals);

	objectIndices[objName] = objects.size();
	objects.push_back(meshObject);

	// Get the material
	if (materialIndices.count(matName) < 1)
		throw std::runtime_error("Unknown material: " + matName);
	Material *mat = materials[materialIndices[matName]];

	// Check if it is a light sources
	if (mat->IsLightSource()) {
		SDL_LOG("The " << objName << " object is a light sources with " << meshObject->GetTotalTriangleCount() << " triangles");

		AreaLightMaterial *light = (AreaLightMaterial *)mat;
		objectMaterials.push_back(mat);
		for (unsigned int i = 0; i < meshObject->GetTotalTriangleCount(); ++i) {
			TriangleLight *tl = new TriangleLight(light, static_cast<unsigned int>(objects.size()) - 1, i, objects);
			lights.push_back(tl);
			triangleLightSource.push_back(tl);
		}
	} else {
		SurfaceMaterial *surfMat = (SurfaceMaterial *)mat;
		objectMaterials.push_back(surfMat);
		for (unsigned int i = 0; i < meshObject->GetTotalTriangleCount(); ++i)
			triangleLightSource.push_back(NULL);
	}

	// [old deprecated syntax] Check if there is a texture map associated to the object
	if (args.size() > 1) {
		// Check if the object has UV coords
		if (!meshObject->HasUVs())
			throw std::runtime_error("PLY object " + plyFileName + " is missing UV coordinates for texture mapping");

		TexMapInstance *tm = texMapCache->GetTexMapInstance(args.at(1), 2.2f);
		objectTexMaps.push_back(tm);
		objectBumpMaps.push_back(NULL);
		objectNormalMaps.push_back(NULL);
	} else {
		// Check for if there is a texture map associated to the object with the new syntax
		const std::string texMap = props.GetString(key + ".texmap", "");
		if (texMap != "") {
			// Check if the object has UV coords
			if (!meshObject->HasUVs())
				throw std::runtime_error("PLY object " + plyFileName + " is missing UV coordinates for texture mapping");

			const float gamma = props.GetFloat(key + ".texmap.gamma", 2.2f);
			const float uScale = props.GetFloat(key + ".texmap.uscale", 1.0f);
			const float vScale = props.GetFloat(key + ".texmap.vscale", 1.0f);
			const float uDelta = props.GetFloat(key + ".texmap.udelta", 0.0f);
			const float vDelta = props.GetFloat(key + ".texmap.vdelta", 0.0f);

			TexMapInstance *tm = texMapCache->GetTexMapInstance(texMap, gamma,
					uScale, vScale, uDelta, vDelta);
			objectTexMaps.push_back(tm);
		} else
			objectTexMaps.push_back(NULL);

		/**
		 * Check if there is an alpha map associated to the object
		 * If there is, the map is added to a previously added texturemap.
		 * If no texture map (diffuse map) is detected, a black texture
		 * is created and the alpha map is added to it. --PC
		 */
		const std::string alphaMap = props.GetString(key + ".alphamap", "");
		if (alphaMap != "") {
			// Got an alpha map, retrieve the textureMap and add the alpha channel to it.
			const std::string texMap = props.GetString(key + ".texmap", "");
			const float gamma = props.GetFloat(key + ".texmap.gamma", 2.2f);

			TextureMap *tm;
			if (!(tm = texMapCache->FindTextureMap(texMap, gamma))) {
				SDL_LOG("Alpha map " << alphaMap << " is for a materials without texture. A black texture has been created for support!");
				// We have an alpha map without a diffuse texture. In this case we need to create
				// a texture map filled with black
				tm = new TextureMap(alphaMap, gamma, 1.0, 1.0, 1.0);
				tm->AddAlpha(alphaMap);
				texMapCache->DefineTexMap(alphaMap, tm);

				const float uScale = props.GetFloat(key + ".texmap.uscale", 1.0f);
				const float vScale = props.GetFloat(key + ".texmap.vscale", 1.0f);
				const float uDelta = props.GetFloat(key + ".texmap.udelta", 0.0f);
				const float vDelta = props.GetFloat(key + ".texmap.vdelta", 0.0f);
				TexMapInstance *tmi = texMapCache->GetTexMapInstance(texMap, gamma,
					uScale, vScale, uDelta, vDelta);
				// Remove the NULL inserted above, when no texmap was found. Without doing this the whole thing will not work
				objectTexMaps.pop_back();
				// Add the new texture to the chain
				objectTexMaps.push_back(tmi);
			} else {
				// Add an alpha map to the pre-existing diffuse texture
				tm->AddAlpha(alphaMap);
			}
		}

		// Check for if there is a bump map associated to the object
		const std::string bumpMap = props.GetString(key + ".bumpmap", "");
		if (bumpMap != "") {
			// Check if the object has UV coords
			if (!meshObject->HasUVs())
				throw std::runtime_error("PLY object " + plyFileName + " is missing UV coordinates for bump mapping");

			const float scale = props.GetFloat(key + ".bumpmap.scale", 1.f);
			const float uScale = props.GetFloat(key + ".bumpmap.uscale", 1.0f);
			const float vScale = props.GetFloat(key + ".bumpmap.vscale", 1.0f);
			const float uDelta = props.GetFloat(key + ".bumpmap.udelta", 0.0f);
			const float vDelta = props.GetFloat(key + ".bumpmap.vdelta", 0.0f);

			BumpMapInstance *bm = texMapCache->GetBumpMapInstance(bumpMap, scale,
					uScale, vScale, uDelta, vDelta);
			objectBumpMaps.push_back(bm);
		} else
			objectBumpMaps.push_back(NULL);

		// Check for if there is a normal map associated to the object
		const std::string normalMap = props.GetString(key + ".normalmap", "");
		if (normalMap != "") {
			// Check if the object has UV coords
			if (!meshObject->HasUVs())
				throw std::runtime_error("PLY object " + plyFileName + " is missing UV coordinates for normal mapping");

			const float uScale = props.GetFloat(key + ".normalmap.uscale", 1.0f);
			const float vScale = props.GetFloat(key + ".normalmap.vscale", 1.0f);
			const float uDelta = props.GetFloat(key + ".normalmap.udelta", 0.0f);
			const float vDelta = props.GetFloat(key + ".normalmap.vdelta", 0.0f);

			NormalMapInstance *nm = texMapCache->GetNormalMapInstance(normalMap,
					uScale, vScale, uDelta, vDelta);
			objectNormalMaps.push_back(nm);
		} else
			objectNormalMaps.push_back(NULL);
	}
}

void Scene::AddInfiniteLight(const std::string &propsString) {
	Properties prop;
	prop.LoadFromString(propsString);

	AddInfiniteLight(prop);
}

void Scene::AddInfiniteLight(const Properties &props) {
	const std::vector<std::string> ilParams = props.GetStringVector("scene.infinitelight.file", "");
	if (ilParams.size() > 0) {
		const float gamma = props.GetFloat("scene.infinitelight.gamma", 2.2f);
		TexMapInstance *tex = texMapCache->GetTexMapInstance(ilParams.at(0), gamma);

		InfiniteLight *il = new InfiniteLight(tex);

		std::vector<float> vf = GetParameters(props, "scene.infinitelight.gain", 3, "1.0 1.0 1.0");
		il->SetGain(Spectrum(vf.at(0), vf.at(1), vf.at(2)));

		vf = GetParameters(props, "scene.infinitelight.shift", 2, "0.0 0.0");
		il->SetShift(vf.at(0), vf.at(1));
		il->Preprocess();

		infiniteLight = il;
	} else
		infiniteLight = NULL;
}

void Scene::AddSkyLight(const std::string &propsString) {
	Properties prop;
	prop.LoadFromString(propsString);

	AddSkyLight(prop);
}

void Scene::AddSkyLight(const Properties &props) {
	const std::vector<std::string> silParams = props.GetStringVector("scene.skylight.dir", "");
	if (silParams.size() > 0) {
		if (infiniteLight)
			throw std::runtime_error("Can not define a skylight when there is already an infinitelight defined");

		std::vector<float> sdir = GetParameters(props, "scene.skylight.dir", 3, "0.0 0.0 1.0");
		const float turb = props.GetFloat("scene.skylight.turbidity", 2.2f);
		std::vector<float> gain = GetParameters(props, "scene.skylight.gain", 3, "1.0 1.0 1.0");

		SkyLight *sl = new SkyLight(turb, Vector(sdir.at(0), sdir.at(1), sdir.at(2)));
		sl->SetGain(Spectrum(gain.at(0), gain.at(1), gain.at(2)));
		sl->Preprocess();

		infiniteLight = sl;
	}
}

void Scene::AddSunLight(const std::string &propsString) {
	Properties prop;
	prop.LoadFromString(propsString);

	AddSunLight(prop);
}

void Scene::AddSunLight(const Properties &props) {
	const std::vector<std::string> sulParams = props.GetStringVector("scene.sunlight.dir", "");
	if (sulParams.size() > 0) {
		std::vector<float> sdir = GetParameters(props, "scene.sunlight.dir", 3, "0.0 0.0 1.0");
		const float turb = props.GetFloat("scene.sunlight.turbidity", 2.2f);
		const float relSize = props.GetFloat("scene.sunlight.relsize", 1.0f);
		std::vector<float> gain = GetParameters(props, "scene.sunlight.gain", 3, "1.0 1.0 1.0");

		SunLight *sl = new SunLight(turb, relSize, Vector(sdir.at(0), sdir.at(1), sdir.at(2)));
		sl->SetGain(Spectrum(gain.at(0), gain.at(1), gain.at(2)));
		sl->Preprocess();

		sunLight = sl;
	} else
		sunLight = NULL;
}

Material *Scene::CreateMaterial(const std::string &propName, const Properties &prop) {
	const std::string matType = Properties::ExtractField(propName, 2);
	if (matType == "")
		throw std::runtime_error("Syntax error in " + propName);
	const std::string matName = Properties::ExtractField(propName, 3);
	if (matName == "")
		throw std::runtime_error("Syntax error in " + propName);

	if (matType == "matte") {
		const std::vector<float> vf = GetParameters(prop, propName, 3, "1.0 1.0 1.0");
		const Spectrum col(vf.at(0), vf.at(1), vf.at(2));

		return new MatteMaterial(col);
	} else if (matType == "light") {
		const std::vector<float> vf = GetParameters(prop, propName, 3, "1.0 1.0 1.0");
		const Spectrum gain(vf.at(0), vf.at(1), vf.at(2));

		return new AreaLightMaterial(gain);
	} else if (matType == "mirror") {
		const std::vector<float> vf = GetParameters(prop, propName, 4, "1.0 1.0 1.0 1.0");
		const Spectrum col(vf.at(0), vf.at(1), vf.at(2));

		return new MirrorMaterial(col, vf.at(3) != 0.f);
	} else if (matType == "mattemirror") {
		const std::vector<float> vf = GetParameters(prop, propName, 7, "1.0 1.0 1.0 1.0 1.0 1.0 1.0");
		const Spectrum Kd(vf.at(0), vf.at(1), vf.at(2));
		const Spectrum Kr(vf.at(3), vf.at(4), vf.at(5));

		return new MatteMirrorMaterial(Kd, Kr, vf.at(6) != 0.f);
	} else if (matType == "glass") {
		const std::vector<float> vf = GetParameters(prop, propName, 10, "1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.5 1.0 1.0");
		const Spectrum Krfl(vf.at(0), vf.at(1), vf.at(2));
		const Spectrum Ktrn(vf.at(3), vf.at(4), vf.at(5));

		return new GlassMaterial(Krfl, Ktrn, vf.at(6), vf.at(7), vf.at(8) != 0.f, vf.at(9) != 0.f);
	} else if (matType == "metal") {
		const std::vector<float> vf = GetParameters(prop, propName, 5, "1.0 1.0 1.0 10.0 1.0");
		const Spectrum col(vf.at(0), vf.at(1), vf.at(2));

		return new MetalMaterial(col, vf.at(3), vf.at(4) != 0.f);
	} else if (matType == "mattemetal") {
		const std::vector<float> vf = GetParameters(prop, propName, 8, "1.0 1.0 1.0 1.0 1.0 1.0 10.0 1.0");
		const Spectrum Kd(vf.at(0), vf.at(1), vf.at(2));
		const Spectrum Kr(vf.at(3), vf.at(4), vf.at(5));

		return new MatteMetalMaterial(Kd, Kr, vf.at(6), vf.at(7) != 0.f);
	} else if (matType == "archglass") {
		const std::vector<float> vf = GetParameters(prop, propName, 8, "1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0");
		const Spectrum Krfl(vf.at(0), vf.at(1), vf.at(2));
		const Spectrum Ktrn(vf.at(3), vf.at(4), vf.at(5));

		return new ArchGlassMaterial(Krfl, Ktrn, vf.at(6) != 0.f, vf.at(7) != 0.f);
	} else if (matType == "alloy") {
		const std::vector<float> vf = GetParameters(prop, propName, 9, "1.0 1.0 1.0 1.0 1.0 1.0 10.0 0.8 1.0");
		const Spectrum Kdiff(vf.at(0), vf.at(1), vf.at(2));
		const Spectrum Krfl(vf.at(3), vf.at(4), vf.at(5));

		return new AlloyMaterial(Kdiff, Krfl, vf.at(6), vf.at(7), vf.at(8) != 0.f);
	} else
		throw std::runtime_error("Unknown material type " + matType);
}

//------------------------------------------------------------------------------

LightSource *Scene::GetLightByType(const LightSourceType lightType) const {
	if (infiniteLight && (lightType == infiniteLight->GetType()))
			return infiniteLight;
	if (sunLight && (lightType == TYPE_SUN))
			return sunLight;

	for (unsigned int i = 0; i < static_cast<unsigned int>(lights.size()); ++i) {
		LightSource *ls = lights[i];
		if (ls->GetType() == lightType)
			return ls;
	}

	return NULL;
}

LightSource *Scene::SampleAllLights(const float u, float *pdf) const {
	unsigned int lightsSize = static_cast<unsigned int>(lights.size());
	if (infiniteLight)
		++lightsSize;
	if (sunLight)
		++lightsSize;

	// One Uniform light strategy
	const unsigned int lightIndex = Min(Floor2UInt(lightsSize * u), lightsSize - 1);
	*pdf = 1.f / lightsSize;

	if (infiniteLight) {
		if (sunLight) {
			if (lightIndex == lightsSize - 1)
				return sunLight;
			else if (lightIndex == lightsSize - 2)
				return infiniteLight;
			else
				return lights[lightIndex];
		} else {
			if (lightIndex == lightsSize - 1)
				return infiniteLight;
			else
				return lights[lightIndex];
		}
	} else {
		if (sunLight) {
			if (lightIndex == lightsSize - 1)
				return sunLight;
			else
				return lights[lightIndex];
		} else
			return lights[lightIndex];
	}
}

float Scene::PickLightPdf() const {
	unsigned int lightsSize = static_cast<unsigned int>(lights.size());
	if (infiniteLight)
		++lightsSize;
	if (sunLight)
		++lightsSize;

	return 1.f / lightsSize;
}

Spectrum Scene::GetEnvLightsRadiance(const Vector &dir,
			const Point &hitPoint,
			float *directPdfA,
			float *emissionPdfW) const {
	Spectrum radiance;
	if (infiniteLight)
		radiance += infiniteLight->GetRadiance(this, dir, hitPoint, directPdfA, emissionPdfW);
	if (sunLight)
		radiance += sunLight->GetRadiance(this, dir, hitPoint, directPdfA, emissionPdfW);

	if (directPdfA)
		*directPdfA *= PickLightPdf();
	if (emissionPdfW)
		*emissionPdfW *= PickLightPdf();

	return radiance;
}

bool Scene::Intersect(IntersectionDevice *device, const bool fromLight, const bool stopOnArchGlass,
		const float u0, Ray *ray, RayHit *rayHit, BSDF *bsdf, Spectrum *connectionThroughput) const {
	*connectionThroughput = Spectrum(1.f, 1.f, 1.f);
	for (;;) {
		if (!device->TraceRay(ray, rayHit)) {
			// Nothing was hit
			return false;
		} else {
			// Check if it is a pass through point
			bsdf->Init(fromLight, *this, *ray, *rayHit, u0);

			// Check if it is pass-through point
			if (bsdf->IsPassThrough()) {
				// It is a pass-through material, continue to trace the ray
				ray->mint = rayHit->t + MachineEpsilon::E(rayHit->t);

				continue;
			}

			// Check if it is a light source
			if (bsdf->IsLightSource())
				return true;

			// Check if it is architectural glass
			if (!stopOnArchGlass && bsdf->IsShadowTransparent()) {
				*connectionThroughput *= bsdf->GetSahdowTransparency();

				// It is a shadow transparent material, continue to trace the ray
				ray->mint = rayHit->t + MachineEpsilon::E(rayHit->t);

				continue;
			}

			return true;
		}
	}
}
