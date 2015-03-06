#line 2 "texture_funcs.cl"

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

//------------------------------------------------------------------------------
// IrregularData texture
//------------------------------------------------------------------------------

#if defined(PARAM_ENABLE_TEX_IRREGULARDATA)

float IrregularDataTexture_ConstEvaluateFloat(__global HitPoint *hitPoint,
		const float3 rgb) {
	return Spectrum_Y(rgb);
}

float3 IrregularDataTexture_ConstEvaluateSpectrum(__global HitPoint *hitPoint,
		const float3 rgb) {
	return rgb;
}

#if defined(PARAM_DISABLE_TEX_DYNAMIC_EVALUATION)
void IrregularDataTexture_EvaluateFloat(__global Texture *texture, __global HitPoint *hitPoint,
		float texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	texValues[(*texValuesSize)++] = IrregularDataTexture_ConstEvaluateFloat(hitPoint,
			VLOAD3F(texture->irregulardata.rgb.c));
}

void IrregularDataTexture_EvaluateSpectrum(__global Texture *texture, __global HitPoint *hitPoint,
		float3 texValues[TEXTURE_STACK_SIZE], uint *texValuesSize) {
	texValues[(*texValuesSize)++] = IrregularDataTexture_ConstEvaluateSpectrum(hitPoint,
			VLOAD3F(texture->irregulardata.rgb.c));
}
#endif

#endif
