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

#ifndef _SLG_KERNELS_H
#define	_SLG_KERNELS_H

#include <string>

namespace slg { namespace ocl {

extern std::string KernelSource_datatypes;
extern std::string KernelSource_pathocl_kernels;
extern std::string KernelSource_rtpathocl_kernels;
extern std::string KernelSource_sampler_types;
extern std::string KernelSource_sampler_funcs;
extern std::string KernelSource_film_types;
extern std::string KernelSource_film_funcs;
extern std::string KernelSource_filter_types;
extern std::string KernelSource_filter_funcs;
extern std::string KernelSource_camera_types;
extern std::string KernelSource_camera_funcs;
extern std::string KernelSource_mc_funcs;
extern std::string KernelSource_bsdf_types;
extern std::string KernelSource_bsdf_funcs;
extern std::string KernelSource_material_types;
extern std::string KernelSource_materialdefs_funcs;
extern std::string KernelSource_material_funcs;
extern std::string KernelSource_texture_types;
extern std::string KernelSource_texture_funcs;
extern std::string KernelSource_light_types;
extern std::string KernelSource_light_funcs;
extern std::string KernelSource_scene_funcs;
extern std::string KernelSource_mapping_types;
extern std::string KernelSource_mapping_funcs;
extern std::string KernelSource_hitpoint_types;

} }

#endif	/* _SLG_KERNELS_H */
