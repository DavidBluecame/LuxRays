################################################################################
# Copyright 1998-2015 by authors (see AUTHORS.txt)
#
#   This file is part of LuxRender.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################

#############################################################################
#############################################################################
#############################################################################
###
### LuxRays Library
###
#############################################################################
#############################################################################
#############################################################################

set(LUXRAYS_VERSION_MAJOR 1)
set(LUXRAYS_VERSION_MINOR 5dev)

# Configure a header file to pass some of the CMake settings
# to the source code
configure_file(
  "${LuxRays_SOURCE_DIR}/include/luxrays/cfg.h.in"
  "${LuxRays_SOURCE_DIR}/include/luxrays/cfg.h"
  )

#############################################################################
#
# LuxRays Docs
#
#############################################################################

find_package(Doxygen)

if(DOXYGEN_FOUND)
	message(STATUS "Found Doxygen and generating LuxRays documentation")

	# Generate doxygen.template
	set(DOXYGEN_LUXRAYS_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/../../doxygen/luxrays.template)
	configure_file(
	  "${DOXYGEN_LUXRAYS_TEMPLATE}.in"
	  "${DOXYGEN_LUXRAYS_TEMPLATE}"
	  )

	set(DOXYGEN_LUXRAYS_INPUT ${CMAKE_CURRENT_BINARY_DIR}/../../doxygen-luxrays.conf)
	set(DOXYGEN_LUXRAYS_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/../../doc/luxrays)
	set(DOXYGEN_LUXRAYS_OUTPUT ${DOXYGEN_OUTPUT_DIR}/luxrays/html/index.html)

	message(STATUS "Doxygen LuxRays output: " ${DOXYGEN_LUXRAYS_OUTPUT})

	if(DOXYGEN_DOT_FOUND)
			message(STATUS "Found dot")
			set(DOXYGEN_DOT_CONF "HAVE_DOT = YES")
	endif(DOXYGEN_DOT_FOUND)

	add_custom_command(
		OUTPUT ${DOXYGEN_LUXRAYS_OUTPUT}
		# Creating custom doxygen-luxrays.conf
		COMMAND mkdir -p ${DOXYGEN_LUXRAYS_OUTPUT_DIR}
		COMMAND cp ${DOXYGEN_LUXRAYS_TEMPLATE} ${DOXYGEN_LUXRAYS_INPUT}
		COMMAND echo "INPUT = " ${CMAKE_CURRENT_SOURCE_DIR}/../../include/luxrays  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/luxrays >> ${DOXYGEN_LUXRAYS_INPUT}
		COMMAND echo "OUTPUT_DIRECTORY = " ${DOXYGEN_LUXRAYS_OUTPUT_DIR} >> ${DOXYGEN_LUXRAYS_INPUT}
		COMMAND echo ${DOXYGEN_DOT_CONF} >> ${DOXYGEN_LUXRAYS_INPUT}
		# Launch doxygen
		COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_LUXRAYS_INPUT}
		DEPENDS ${DOXYGEN_LUXRAYS_TEMPLATE}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../..
	)

	add_custom_target(apidoc_luxrays DEPENDS ${DOXYGEN_LUXRAYS_OUTPUT})
endif(DOXYGEN_FOUND)

###########################################################################
#
# Generate LuxRays kernel files
#
###########################################################################

set(LUXRAYS_KERNELS
	${LuxRays_SOURCE_DIR}/include/luxrays/luxrays_types.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/color/color_types.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/color/color_funcs.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/accelerators/bvh_types.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/accelerators/bvh.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/accelerators/mbvh.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/accelerators/qbvh_types.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/accelerators/qbvh.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/accelerators/mqbvh.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/randomgen_types.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/randomgen_funcs.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/trianglemesh_types.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/trianglemesh_funcs.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/geometry/matrix4x4_types.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/geometry/matrix4x4_funcs.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/geometry/transform_types.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/geometry/transform_funcs.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/geometry/frame_types.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/geometry/frame_funcs.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/epsilon_types.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/epsilon_funcs.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/geometry/ray_types.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/geometry/ray_funcs.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/geometry/point_types.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/geometry/vector_types.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/geometry/vector_funcs.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/geometry/normal_types.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/geometry/quaternion_types.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/geometry/quaternion_funcs.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/geometry/bbox_types.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/geometry/bbox_funcs.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/geometry/motionsystem_types.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/geometry/motionsystem_funcs.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/geometry/triangle_types.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/geometry/triangle_funcs.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/geometry/uv_types.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/core/utils_funcs.cl
	${LuxRays_SOURCE_DIR}/include/luxrays/utils/mc_funcs.cl
)

PreprocessOCLKernels(${LuxRays_SOURCE_DIR}/src/luxrays/kernels luxrays ${LUXRAYS_KERNELS})

###########################################################################
#
# Library project files
#
###########################################################################

set(LUXRAYS_SRCS
	${LuxRays_SOURCE_DIR}/src/luxrays/accelerators/bvhaccel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/accelerators/embreeaccel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/accelerators/mqbvhaccel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/accelerators/qbvhaccel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/accelerators/mbvhaccel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/core/color/color.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/core/color/spd.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/core/color/spds/blackbodyspd.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/core/color/spds/equalspd.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/core/color/spds/frequencyspd.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/core/color/spds/gaussianspd.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/core/color/spds/irregular.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/core/color/spds/regular.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/core/color/spds/rgbillum.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/core/color/spds/rgbrefl.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/core/color/spectrumwavelengths.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/core/color/swcspectrum.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/core/accelerator.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/core/context.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/core/dataset.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/core/device.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/core/epsilon.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/core/exttrianglemesh.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/core/trianglemesh.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/core/geometry/bbox.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/core/geometry/matrix4x4.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/core/geometry/motionsystem.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/core/geometry/quaternion.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/core/geometry/transform.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/idevices/openclidevice.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/idevices/nativeidevice.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/idevices/virtualidevice.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/utils/mc.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/utils/ocl.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/utils/ply/rply.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/utils/properties.cpp
)
SOURCE_GROUP("Source Files\\LuxRays Library" FILES ${LUXRAYS_SRCS})

set(LUXRAYS_SRCS_KERNELS
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/luxrays_types_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/bvh_types_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/bvh_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/mbvh_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/qbvh_types_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/qbvh_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/mqbvh_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/randomgen_types_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/randomgen_funcs_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/trianglemesh_types_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/trianglemesh_funcs_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/matrix4x4_types_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/matrix4x4_funcs_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/transform_types_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/transform_funcs_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/frame_types_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/frame_funcs_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/epsilon_types_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/epsilon_funcs_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/ray_types_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/ray_funcs_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/point_types_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/vector_types_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/vector_funcs_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/normal_types_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/quaternion_types_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/quaternion_funcs_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/bbox_types_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/bbox_funcs_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/motionsystem_types_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/motionsystem_funcs_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/triangle_types_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/triangle_funcs_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/color_types_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/color_funcs_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/uv_types_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/mc_funcs_kernel.cpp
	${LuxRays_SOURCE_DIR}/src/luxrays/kernels/utils_funcs_kernel.cpp
)
SOURCE_GROUP("Source Files\\LuxRays Kernels" FILES ${LUXRAYS_SRCS_KERNELS})

set(LUXRAYS_LIB_SRCS
	${LUXRAYS_SRCS}
	${LUXRAYS_SRCS_KERNELS}
)

add_library(luxrays STATIC ${LUXRAYS_LIB_SRCS})

IF(GCC AND NOT APPLE)
	# These accelerators needs to be compiled with much reduced optimizations
	# otherwise gcc produces incorrect code and ruins the render on 64bits machines
	SET_SOURCE_FILES_PROPERTIES(${LuxRays_SOURCE_DIR}/src/luxrays/accelerators/qbvhaccel.cpp COMPILE_FLAGS "-O2")
	SET_SOURCE_FILES_PROPERTIES(${LuxRays_SOURCE_DIR}/src/luxrays/accelerators/mqbvhaccel.cpp COMPILE_FLAGS "-O2")
ENDIF(GCC AND NOT APPLE)

TARGET_LINK_LIBRARIES(luxrays ${Boost_LIBRARIES})
TARGET_LINK_LIBRARIES(luxrays ${OPENIMAGEIO_LIBRARIES})

IF (NOT LUXRAYS_DISABLE_OPENCL)
	TARGET_LINK_LIBRARIES(luxrays ${OPENGL_gl_LIBRARY})
	TARGET_LINK_LIBRARIES(luxrays ${OPENCL_LIBRARIES})
ENDIF(NOT LUXRAYS_DISABLE_OPENCL)

###########################################################################
#
# Predefines
#
###########################################################################

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "../lib")
