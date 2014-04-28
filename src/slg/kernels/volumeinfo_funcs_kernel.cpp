#include <string>
namespace slg { namespace ocl {
std::string KernelSource_volumeinfo_funcs = 
"#line 2 \"volumeinfo_funcs.cl\"\n"
"\n"
"/***************************************************************************\n"
" * Copyright 1998-2013 by authors (see AUTHORS.txt)                        *\n"
" *                                                                         *\n"
" *   This file is part of LuxRender.                                       *\n"
" *                                                                         *\n"
" * Licensed under the Apache License, Version 2.0 (the \"License\");         *\n"
" * you may not use this file except in compliance with the License.        *\n"
" * You may obtain a copy of the License at                                 *\n"
" *                                                                         *\n"
" *     http://www.apache.org/licenses/LICENSE-2.0                          *\n"
" *                                                                         *\n"
" * Unless required by applicable law or agreed to in writing, software     *\n"
" * distributed under the License is distributed on an \"AS IS\" BASIS,       *\n"
" * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*\n"
" * See the License for the specific language governing permissions and     *\n"
" * limitations under the License.                                          *\n"
" ***************************************************************************/\n"
"\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// PathVolumeInfo\n"
"//------------------------------------------------------------------------------\n"
"\n"
"void PathVolumeInfo_Init(__global PathVolumeInfo *pvi) {\n"
"	pvi->currentVolumeIndex = NULL_INDEX;\n"
"	pvi->volumeIndexListSize = 0;\n"
"\n"
"	pvi->scatteredStart = false;\n"
"}\n"
"\n"
"void PathVolumeInfo_AddVolume(__global PathVolumeInfo *pvi, const uint volIndex\n"
"		MATERIALS_PARAM_DECL) {\n"
"	if ((volIndex == NULL_INDEX) || (pvi->volumeIndexListSize == OPENCL_PATHVOLUMEINFO_SIZE)) {\n"
"		// NULL volume or out of space, I just ignore the volume\n"
"		return;\n"
"	}\n"
"\n"
"	// Update the current volume. \">=\" because I want to catch the last added volume.\n"
"	if ((pvi->currentVolumeIndex == NULL_INDEX) ||\n"
"			(mats[volIndex].volume.priority >= mats[pvi->currentVolumeIndex].volume.priority))\n"
"		pvi->currentVolumeIndex = volIndex;\n"
"\n"
"	// Add the volume to the list\n"
"	pvi->volumeIndexList[(pvi->volumeIndexListSize)++] = volIndex;\n"
"}\n"
"\n"
"void PathVolumeInfo_RemoveVolume(__global PathVolumeInfo *pvi, const uint volIndex\n"
"		MATERIALS_PARAM_DECL) {\n"
"	if ((volIndex == NULL_INDEX) || (pvi->volumeIndexListSize == 0)) {\n"
"		// NULL volume or empty volume list\n"
"		return;\n"
"	}\n"
"\n"
"	// Update the current volume and the list\n"
"	bool found = false;\n"
"	uint newCurrentVolumeIndex = NULL_INDEX;\n"
"	for (uint i = 0; i < pvi->volumeIndexListSize; ++i) {\n"
"		if (found) {\n"
"			// Re-compact the list\n"
"			pvi->volumeIndexList[i - 1] = pvi->volumeIndexList[i];\n"
"		} else if (pvi->volumeIndexList[i] == volIndex) {\n"
"			// Found the volume to remove\n"
"			found = true;\n"
"			continue;\n"
"		}\n"
"\n"
"		// Update currentVolume. \">=\" because I want to catch the last added volume.\n"
"		if ((newCurrentVolumeIndex == NULL_INDEX) ||\n"
"				(mats[pvi->volumeIndexList[i]].volume.priority >= mats[newCurrentVolumeIndex].volume.priority))\n"
"			newCurrentVolumeIndex = pvi->volumeIndexList[i];\n"
"	}\n"
"	pvi->currentVolumeIndex = newCurrentVolumeIndex;\n"
"\n"
"	// Update the list size\n"
"	--(pvi->volumeIndexListSize);\n"
"}\n"
"\n"
"const uint PathVolumeInfo_SimulateAddVolume(__global PathVolumeInfo *pvi, const uint volIndex\n"
"		MATERIALS_PARAM_DECL) {\n"
"	// A volume wins over current if and only if it is the same volume or has an\n"
"	// higher priority\n"
"\n"
"	const uint currentVolumeIndex = pvi->currentVolumeIndex;\n"
"	if (currentVolumeIndex != NULL_INDEX) {\n"
"		if (volIndex != NULL_INDEX) {\n"
"			return (mats[currentVolumeIndex].volume.priority > mats[volIndex].volume.priority) ? currentVolumeIndex : volIndex;\n"
"		} else\n"
"			return pvi->currentVolumeIndex;\n"
"	} else\n"
"		return volIndex;\n"
"}\n"
"\n"
"uint PathVolumeInfo_SimulateRemoveVolume(__global PathVolumeInfo *pvi, const uint volIndex\n"
"		MATERIALS_PARAM_DECL) {\n"
"	if ((volIndex == NULL_INDEX) || (pvi->volumeIndexListSize == 0)) {\n"
"		// NULL volume or empty volume list\n"
"		return pvi->currentVolumeIndex;\n"
"	}\n"
"\n"
"	// Update the current volume\n"
"	bool found = false;\n"
"	uint newCurrentVolumeIndex = NULL_INDEX;\n"
"	for (uint i = 0; i < pvi->volumeIndexListSize; ++i) {\n"
"		if ((!found) && (pvi->volumeIndexList[i] == volIndex)) {\n"
"			// Found the volume to remove\n"
"			found = true;\n"
"			continue;\n"
"		}\n"
"\n"
"		// Update currentVolume. \">=\" because I want to catch the last added volume.\n"
"		if ((newCurrentVolumeIndex == NULL_INDEX) ||\n"
"				(mats[pvi->volumeIndexList[i]].volume.priority >= mats[newCurrentVolumeIndex].volume.priority))\n"
"			newCurrentVolumeIndex = pvi->volumeIndexList[i];\n"
"	}\n"
"\n"
"	return newCurrentVolumeIndex;\n"
"}\n"
"\n"
"void PathVolumeInfo_Update(__global PathVolumeInfo *pvi, const BSDFEvent eventType,\n"
"		__global BSDF *bsdf\n"
"		MATERIALS_PARAM_DECL) {\n"
"	// Update only if it isn't a volume scattering and the material can TRANSMIT\n"
"	if (bsdf->isVolume)\n"
"		pvi->scatteredStart = true;\n"
"	else {\n"
"		pvi->scatteredStart = false;\n"
"\n"
"		if(eventType & TRANSMIT) {\n"
"			const uint volIndex = BSDF_GetMaterialInteriorVolume(bsdf\n"
"					MATERIALS_PARAM);\n"
"\n"
"			if (bsdf->hitPoint.intoObject)\n"
"				PathVolumeInfo_AddVolume(pvi, volIndex\n"
"						MATERIALS_PARAM);\n"
"			else\n"
"				PathVolumeInfo_RemoveVolume(pvi, volIndex\n"
"						MATERIALS_PARAM);\n"
"		}\n"
"	}\n"
"}\n"
"\n"
"bool PathVolumeInfo_CompareVolumePriorities(const uint vol1Index, const uint vol2Index\n"
"	MATERIALS_PARAM_DECL) {\n"
"	// A volume wins over another if and only if it is the same volume or has an\n"
"	// higher priority\n"
"\n"
"	if (vol1Index != NULL_INDEX) {\n"
"		if (vol2Index != NULL_INDEX) {\n"
"			if (vol1Index == vol2Index)\n"
"				return true;\n"
"			else\n"
"				return (mats[vol1Index].volume.priority > mats[vol2Index].volume.priority);\n"
"		} else\n"
"			return false;\n"
"	} else\n"
"		return false;\n"
"}\n"
"\n"
"bool PathVolumeInfo_ContinueToTrace(__global PathVolumeInfo *pvi, __global BSDF *bsdf\n"
"		MATERIALS_PARAM_DECL) {\n"
"	// Check if the volume priority system has to be applied\n"
"	if (BSDF_GetEventTypes(bsdf\n"
"			MATERIALS_PARAM) & TRANSMIT) {\n"
"		// Ok, the surface can transmit so check if volume priority\n"
"		// system is telling me to continue to trace the ray\n"
"\n"
"		// I have to continue to trace the ray if:\n"
"		//\n"
"		// 1) I'm entering an object and the interior volume has a\n"
"		// lower priority than the current one (or is the same volume).\n"
"		//\n"
"		// 2) I'm exiting an object and I'm not leaving the current volume.\n"
"\n"
"		const bool intoObject = bsdf->hitPoint.intoObject;\n"
"		const uint bsdfInteriorVolIndex = BSDF_GetMaterialInteriorVolume(bsdf\n"
"			MATERIALS_PARAM);\n"
"\n"
"		// Condition #1\n"
"		if (intoObject && PathVolumeInfo_CompareVolumePriorities(pvi->currentVolumeIndex, bsdfInteriorVolIndex\n"
"				MATERIALS_PARAM))\n"
"			return true;\n"
"\n"
"		// Condition #2\n"
"		//\n"
"		// I have to calculate the potentially new currentVolume in order\n"
"		// to check if I'm leaving the current one\n"
"		if ((!intoObject) && (PathVolumeInfo_SimulateRemoveVolume(pvi, bsdfInteriorVolIndex\n"
"				MATERIALS_PARAM) == pvi->currentVolumeIndex))\n"
"			return true;\n"
"	}\n"
"\n"
"	return false;\n"
"}\n"
"\n"
"void PathVolumeInfo_SetHitPointVolumes(__global PathVolumeInfo *pvi, \n"
"		__global HitPoint *hitPoint,\n"
"		const uint matInteriorVolumeIndex,\n"
"		const uint matExteriorVolumeIndex\n"
"		MATERIALS_PARAM_DECL) {\n"
"	// Set interior and exterior volumes\n"
"\n"
"	const uint currentVolumeIndex = pvi->currentVolumeIndex;\n"
"	if (hitPoint->intoObject) {\n"
"		// From outside to inside the object\n"
"\n"
"		hitPoint->interiorVolumeIndex = PathVolumeInfo_SimulateAddVolume(pvi, matInteriorVolumeIndex\n"
"				MATERIALS_PARAM);\n"
"\n"
"		if (currentVolumeIndex == NULL_INDEX)\n"
"			hitPoint->exteriorVolumeIndex = matExteriorVolumeIndex;\n"
"		else {\n"
"			// if (!material->GetExteriorVolume()) there may be conflict here\n"
"			// between the material definition and the currentVolume value.\n"
"			// The currentVolume value wins.\n"
"			hitPoint->exteriorVolumeIndex = currentVolumeIndex;\n"
"		}\n"
"\n"
"		if (hitPoint->exteriorVolumeIndex == NULL_INDEX) {\n"
"			// No volume information, I use the default volume\n"
"			hitPoint->exteriorVolumeIndex = SCENE_DEFAULT_VOLUME_INDEX;\n"
"		}\n"
"	} else {\n"
"		// From inside to outside the object\n"
"\n"
"		if (currentVolumeIndex == NULL_INDEX)\n"
"			hitPoint->interiorVolumeIndex = matInteriorVolumeIndex;\n"
"		else {\n"
"			// if (!material->GetInteriorVolume()) there may be conflict here\n"
"			// between the material definition and the currentVolume value.\n"
"			// The currentVolume value wins.\n"
"			hitPoint->interiorVolumeIndex = currentVolumeIndex;\n"
"		}\n"
"		\n"
"		if (hitPoint->interiorVolumeIndex == NULL_INDEX) {\n"
"			// No volume information, I use the default volume\n"
"			hitPoint->interiorVolumeIndex = SCENE_DEFAULT_VOLUME_INDEX;\n"
"		}\n"
"\n"
"		hitPoint->exteriorVolumeIndex = PathVolumeInfo_SimulateRemoveVolume(pvi, matExteriorVolumeIndex\n"
"				MATERIALS_PARAM);\n"
"	}\n"
"}\n"
"\n"
"#endif\n"
; } }
