# - Find ONNX Runtime
# Find the ONNX Runtime library
#
# This module defines the following variables:
#  ONNXRuntime_FOUND - true if ONNX Runtime was found
#  ONNXRuntime_INCLUDE_DIRS - include path
#  ONNXRuntime_LIBRARIES - libraries to link against

find_path(ONNXRuntime_INCLUDE_DIR
    NAMES onnxruntime_c_api.h
    HINTS /opt/onnxruntime/include /usr/local/include /usr/include
    PATH_SUFFIXES onnxruntime
)

find_library(ONNXRuntime_LIBRARY
    NAMES onnxruntime
    HINTS /opt/onnxruntime/lib /usr/local/lib /usr/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ONNXRuntime DEFAULT_MSG
    ONNXRuntime_LIBRARY ONNXRuntime_INCLUDE_DIR)

if(ONNXRuntime_FOUND)
    set(ONNXRuntime_INCLUDE_DIRS ${ONNXRuntime_INCLUDE_DIR})
    set(ONNXRuntime_LIBRARIES ${ONNXRuntime_LIBRARY})
endif()

mark_as_advanced(ONNXRuntime_INCLUDE_DIR ONNXRuntime_LIBRARY)
