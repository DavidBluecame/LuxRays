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

#include "slg/sdl/sdl.h"
#include "slg/sdl/bsdf.h"
#include "slg/sdl/blender_texture.h"

using namespace luxrays;
using namespace slg;

//------------------------------------------------------------------------------
// Blender blend texture
//------------------------------------------------------------------------------

BlenderBlendTexture::BlenderBlendTexture(const TextureMapping3D *mp, const ProgressionType type, 
										 const bool direction, float bright, float contrast) : 
		mapping(mp), type(type), direction(direction), bright(bright), contrast(contrast) {

}

float BlenderBlendTexture::GetFloatValue(const HitPoint &hitPoint) const {
	float result = 0.f;
	Point P(mapping->Map(hitPoint));

	float x, y, t;
    
	if(direction) {
		//horizontal
		x = P.x;
		y = P.y;
	} else {
		//vertical
		x = P.y;
		y = P.x;
	};


    if (type == TEX_LIN) { /* lin */
        result = (1.f + x) / 2.f;
    } else if (type == TEX_QUAD) { /* quad */
        result = (1.f + x) / 2.f;
        if (result < 0.f) result = 0.f;
        else result *= result;
    } else if (type == TEX_EASE) { /* ease */
        result = (1.f + x) / 2.f;
        if (result <= 0.f) result = 0.f;
        else if (result >= 1.f) result = 1.f;
        else {
            t = result * result;
            result = (3.f * t - 2.f * t * result);
        }
    } else if (type == TEX_DIAG) { /* diag */
        result = (2.f + x + y) / 4.f;
    } else if (type == TEX_RAD) { /* radial */
        result = (atan2f(y, x) / (2.f * M_PI) + 0.5f);
    } else { /* sphere TEX_SPHERE */
        result = 1.f - sqrt(x * x + y * y + P.z * P.z);
        if (result < 0.f) result = 0.f;
        if (type == TEX_HALO) result *= result; /* halo */
    }

	result = (result - 0.5f) * contrast + bright - 0.5f;
    if(result < 0.f) result = 0.f; 
	else if(result > 1.f) result = 1.f;

	return result;
}

Spectrum BlenderBlendTexture::GetSpectrumValue(const HitPoint &hitPoint) const {
	return Spectrum(GetFloatValue(hitPoint));
}

Properties BlenderBlendTexture::ToProperties(const ImageMapCache &imgMapCache) const {
	Properties props;

	std::string progressiontype;
	switch(type) {
		default:
		case TEX_LIN:
			progressiontype = "linear";
			break;
		case TEX_QUAD:
			progressiontype = "quadratic";
			break;
		case TEX_EASE:
			progressiontype = "easing";
			break;
		case TEX_DIAG:
			progressiontype = "diagonal";
			break;
		case TEX_SPHERE:
			progressiontype = "spherical";
			break;
		case TEX_HALO:
			progressiontype = "quadratic_spherical";
			break;
		case TEX_RAD:
			progressiontype = "radial";
			break;
	}
	std::string directiontype = "horizontal";
	if(direction) directiontype = "vertical";

	const std::string name = GetName();

	props.Set(Property("scene.textures." + name + ".type")("blender_blend"));
	props.Set(Property("scene.textures." + name + ".progressiontype")(progressiontype));
	props.Set(Property("scene.textures." + name + ".direction")(directiontype));
	props.Set(Property("scene.textures." + name + ".bright")(bright));
	props.Set(Property("scene.textures." + name + ".contrast")(contrast));
	props.Set(mapping->ToProperties("scene.textures." + name + ".mapping"));

	return props;
}

//------------------------------------------------------------------------------
// Blender magic texture
//------------------------------------------------------------------------------

BlenderMagicTexture::BlenderMagicTexture(const TextureMapping3D *mp, const int noisedepth, 
										 const float turbulence, float bright, float contrast) : 
		mapping(mp), noisedepth(noisedepth), turbulence(turbulence), bright(bright), contrast(contrast) {

}

float BlenderMagicTexture::GetFloatValue(const HitPoint &hitPoint) const {	
	return GetSpectrumValue(hitPoint).Y();
}

Spectrum BlenderMagicTexture::GetSpectrumValue(const HitPoint &hitPoint) const {
	Point P(mapping->Map(hitPoint));
	Spectrum s;

    float x, y, z, turb = 1.f;
    float r, g, b;
    int n;

    n = noisedepth;
    turb = turbulence / 5.f;

    x = sin((P.x + P.y + P.z)*5.f);
    y = cos((-P.x + P.y - P.z)*5.f);
    z = -cos((-P.x - P.y + P.z)*5.f);
    if (n > 0) {
        x *= turb;
        y *= turb;
        z *= turb;
        y = -cos(x - y + z);
        y *= turb;
        if (n > 1) {
            x = cos(x - y - z);
            x *= turb;
            if (n > 2) {
                z = sin(-x - y - z);
                z *= turb;
                if (n > 3) {
                    x = -cos(-x + y - z);
                    x *= turb;
                    if (n > 4) {
                        y = -sin(-x + y + z);
                        y *= turb;
                        if (n > 5) {
                            y = -cos(-x + y + z);
                            y *= turb;
                            if (n > 6) {
                                x = cos(x + y + z);
                                x *= turb;
                                if (n > 7) {
                                    z = sin(x + y - z);
                                    z *= turb;
                                    if (n > 8) {
                                        x = -cos(-x - y + z);
                                        x *= turb;
                                        if (n > 9) {
                                            y = -sin(x - y + z);
                                            y *= turb;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (turb != 0.f) {
        turb *= 2.f;
        x /= turb;
        y /= turb;
        z /= turb;
    }
    r = 0.5f - x;
    g = 0.5f - y;
    b = 0.5f - z;

	r = (r - 0.5f) * contrast + bright - 0.5f;
    if(r < 0.f) r = 0.f; 
	else if(r > 1.f) r = 1.f;

	g = (g - 0.5f) * contrast + bright - 0.5f;
    if(g < 0.f) g = 0.f; 
	else if(g > 1.f) g = 1.f;

	b = (b - 0.5f) * contrast + bright - 0.5f;
    if(b < 0.f) b = 0.f; 
	else if(b > 1.f) b = 1.f;

	return Spectrum(RGBColor(r,g,b));
}

float BlenderMagicTexture::Y() const {
	static float c[][3] = { { .58f, .58f, .6f }, { .58f, .58f, .6f }, { .58f, .58f, .6f },
		{ .5f, .5f, .5f }, { .6f, .59f, .58f }, { .58f, .58f, .6f },
		{ .58f, .58f, .6f }, {.2f, .2f, .33f }, { .58f, .58f, .6f }, };
	luxrays::Spectrum cs;
#define NC  sizeof(c) / sizeof(c[0])
	for (u_int i = 0; i < NC; ++i)
		cs += luxrays::Spectrum(c[i]);
	return cs.Y() / NC;
#undef NC
}

float BlenderMagicTexture::Filter() const {
	static float c[][3] = { { .58f, .58f, .6f }, { .58f, .58f, .6f }, { .58f, .58f, .6f },
		{ .5f, .5f, .5f }, { .6f, .59f, .58f }, { .58f, .58f, .6f },
		{ .58f, .58f, .6f }, {.2f, .2f, .33f }, { .58f, .58f, .6f }, };
	luxrays::Spectrum cs;
#define NC  sizeof(c) / sizeof(c[0])
	for (u_int i = 0; i < NC; ++i)
		cs += luxrays::Spectrum(c[i]);
	return cs.Filter() / NC;
#undef NC
}

Properties BlenderMagicTexture::ToProperties(const ImageMapCache &imgMapCache) const {
	Properties props;
	const std::string name = GetName();

	props.Set(Property("scene.textures." + name + ".type")("blender_magic"));
	props.Set(Property("scene.textures." + name + ".noisedepth")(noisedepth));
	props.Set(Property("scene.textures." + name + ".turbulence")(turbulence));
	props.Set(Property("scene.textures." + name + ".bright")(bright));
	props.Set(Property("scene.textures." + name + ".contrast")(contrast));
	props.Set(mapping->ToProperties("scene.textures." + name + ".mapping"));

	return props;
}

//------------------------------------------------------------------------------
// Blender wood texture
//------------------------------------------------------------------------------

BlenderWoodTexture::BlenderWoodTexture(const TextureMapping3D *mp, const std::string &ptype, const std::string &pnoise,
		const float noisesize, float turb, bool hard, float bright, float contrast) : 
		mapping(mp), type(BANDS), noisebasis2(TEX_SIN),  noisesize(noisesize),
		turbulence(turb), hard(hard), bright(bright), contrast(contrast) {
	if(ptype == "bands") {
		type = BANDS;
	} else if(ptype == "rings") {
		type = RINGS;
	} else if(ptype == "bandnoise") {
		type = BANDNOISE;
	} else if(ptype == "ringnoise") {
		type = RINGNOISE;
	};

	if(pnoise == "sin") {
		noisebasis2 = TEX_SIN;
	} else if(pnoise == "saw") {
		noisebasis2 = TEX_SAW;
	} else if(pnoise == "tri") {
		noisebasis2 = TEX_TRI;
	};
}

float BlenderWoodTexture::GetFloatValue(const HitPoint &hitPoint) const {
	Point P(mapping->Map(hitPoint));
	float scale = 1.f;
	if(fabs(noisesize) > 0.00001f) scale = (1.f/noisesize);

    float (*waveform[3])(float); /* create array of pointers to waveform functions */
    waveform[0] = tex_sin; /* assign address of tex_sin() function to pointer array */
    waveform[1] = tex_saw;
    waveform[2] = tex_tri;

	u_int wf = 0;
	if(noisebasis2 == TEX_SAW) { 
		wf = 1;
	} else if(noisebasis2 == TEX_TRI) 
		wf = 2;

	float wood = 0.f;
	switch(type) {
		case BANDS:
			wood = waveform[wf]((P.x + P.y + P.z) * 10.f);
			break;
		case RINGS:
			wood = waveform[wf](sqrtf(P.x*P.x + P.y*P.y + P.z*P.z) * 20.f);
			break;
		case BANDNOISE:
			if(hard) wood = turbulence * fabs(2.f * Noise(scale*P) - 1.f);			
			else wood = turbulence * Noise(scale*P);
			wood = waveform[wf]((P.x + P.y + P.z) * 10.f + wood);
			break;
		case RINGNOISE:
			if(hard) wood = turbulence * fabs(2.f * Noise(scale*P) - 1.f);			
			else wood = turbulence * Noise(scale*P);
			wood = waveform[wf](sqrtf(P.x*P.x + P.y*P.y + P.z*P.z) * 20.f + wood);
			break;
	}
    
	wood = (wood - 0.5f) * contrast + bright - 0.5f;
    if(wood < 0.f) wood = 0.f; 
	else if(wood > 1.f) wood = 1.f;
	
    return wood;
}

Spectrum BlenderWoodTexture::GetSpectrumValue(const HitPoint &hitPoint) const {
	return Spectrum(GetFloatValue(hitPoint));
}

Properties BlenderWoodTexture::ToProperties(const ImageMapCache &imgMapCache) const {
	Properties props;

	std::string noise;
	switch(noisebasis2) {
		default:
		case TEX_SIN:
			noise = "sin";
			break;
		case TEX_SAW:
			noise = "saw";
			break;
		case TEX_TRI:
			noise = "tri";
			break;
	}
	std::string woodtype;
	switch(type) {
		default:
		case BANDS:
			woodtype = "bands";
			break;
		case RINGS:
			woodtype = "rings";
			break;
		case BANDNOISE:
			woodtype = "bandnoise";
			break;
		case RINGNOISE:
			woodtype = "ringnoise";
			break;
	}
	std::string noisetype = "soft_noise";
	if(hard) noisetype = "hard_noise";

	const std::string name = GetName();

	props.Set(Property("scene.textures." + name + ".type")("blender_wood"));
	props.Set(Property("scene.textures." + name + ".woodtype")(woodtype));
	props.Set(Property("scene.textures." + name + ".noisebasis2")(noise));
	props.Set(Property("scene.textures." + name + ".noisesize")(noisesize));
	props.Set(Property("scene.textures." + name + ".noisetype")(noisetype));
	props.Set(Property("scene.textures." + name + ".turbulence")(turbulence));
	props.Set(Property("scene.textures." + name + ".bright")(bright));
	props.Set(Property("scene.textures." + name + ".contrast")(contrast));
	props.Set(mapping->ToProperties("scene.textures." + name + ".mapping"));

	return props;
}

//------------------------------------------------------------------------------
// Blender clouds texture
//------------------------------------------------------------------------------

BlenderCloudsTexture::BlenderCloudsTexture(const TextureMapping3D *mp, const float noisesize, const int noisedepth, 
		bool hard, float bright, float contrast) : 
		mapping(mp), noisedepth(noisedepth), noisesize(noisesize), hard(hard), bright(bright), contrast(contrast) {
}

float BlenderCloudsTexture::GetFloatValue(const HitPoint &hitPoint) const {
	Point P(mapping->Map(hitPoint));
	float scale = 1.f;
	if(fabs(noisesize) > 0.00001f) scale = (1.f/noisesize);

	float clouds = Turbulence(scale*P, noisesize, noisedepth);

	clouds = (clouds - 0.5f) * contrast + bright - 0.5f;
    if(clouds < 0.f) clouds = 0.f; 
	else if(clouds > 1.f) clouds = 1.f;
	
    return clouds;
}

Spectrum BlenderCloudsTexture::GetSpectrumValue(const HitPoint &hitPoint) const {
	return Spectrum(GetFloatValue(hitPoint));
}

Properties BlenderCloudsTexture::ToProperties(const ImageMapCache &imgMapCache) const {
	Properties props;

	std::string noisetype = "soft_noise";
	if(hard) noisetype = "hard_noise";

	const std::string name = GetName();

	props.Set(Property("scene.textures." + name + ".type")("blender_clouds"));
	props.Set(Property("scene.textures." + name + ".noisesize")(noisesize));
	props.Set(Property("scene.textures." + name + ".noisedepth")(noisedepth));
	props.Set(Property("scene.textures." + name + ".bright")(bright));
	props.Set(Property("scene.textures." + name + ".contrast")(contrast));
	props.Set(mapping->ToProperties("scene.textures." + name + ".mapping"));

	return props;
}

//------------------------------------------------------------------------------
// Blender voronoi texture
//------------------------------------------------------------------------------

BlenderVoronoiTexture::BlenderVoronoiTexture(const TextureMapping3D *mp, const float intensity, const float exponent,
        const float fw1, const float fw2, const float fw3, const float fw4, DistanceMetric distmetric, float noisesize,  float bright, float contrast) :
		mapping(mp), intensity(intensity), feature_weight1(fw1), feature_weight2(fw2), feature_weight3(fw3), feature_weight4(fw4),
		distancemetric(distmetric), exponent(exponent),
		noisesize(noisesize), bright(bright), contrast(contrast) {
}

float BlenderVoronoiTexture::GetFloatValue(const HitPoint &hitPoint) const {
    float da[4], pa[12]; /* distance and point coordinate arrays of 4 nearest neighbours */
	luxrays::Point P(mapping->Map(hitPoint));

    const float aw1 = fabs(feature_weight1);
    const float aw2 = fabs(feature_weight2);
    const float aw3 = fabs(feature_weight3);
    const float aw4 = fabs(feature_weight4);

    float sc = (aw1 + aw2 + aw3 + aw4);

    if (sc > 1e-3f) sc = noisesize / sc;

    float result = 1.f;

	voronoi(P, da, pa, exponent, distancemetric);
    result = sc * fabs(feature_weight1 * da[0] + feature_weight2 * da[1] + feature_weight3 * da[2] + feature_weight4 * da[3]);

	result = (result - 0.5f) * contrast + bright - 0.5f;
    if(result < 0.f) result = 0.f; 
	else if(result > 1.f) result = 1.f;

    return result;
}

Spectrum BlenderVoronoiTexture::GetSpectrumValue(const HitPoint &hitPoint) const {
	return Spectrum(GetFloatValue(hitPoint));
}

Properties BlenderVoronoiTexture::ToProperties(const ImageMapCache &imgMapCache) const {
	Properties props;

	const std::string name = GetName();

	props.Set(Property("scene.textures." + name + ".type")("blender_voronoi"));
	props.Set(Property("scene.textures." + name + ".intentity")(intensity));
	props.Set(Property("scene.textures." + name + ".exponent")(exponent));
	props.Set(Property("scene.textures." + name + ".w1")(feature_weight1));
	props.Set(Property("scene.textures." + name + ".w2")(feature_weight2));
	props.Set(Property("scene.textures." + name + ".w3")(feature_weight3));
	props.Set(Property("scene.textures." + name + ".w4")(feature_weight4));
	props.Set(Property("scene.textures." + name + ".noisesize")(noisesize));
	props.Set(Property("scene.textures." + name + ".bright")(bright));
	props.Set(Property("scene.textures." + name + ".contrast")(contrast));
	props.Set(mapping->ToProperties("scene.textures." + name + ".mapping"));

	return props;
}