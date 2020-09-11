CMAKE_MINIMUM_REQUIRED(VERSION 3.7)

MESSAGE(STATUS "OpenCV_LIBS: ${OpenCV_LIBS}")
PROJECT(isp)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

FIND_PACKAGE( OpenCV REQUIRED )

include_directories(/usr/include/python3.6m/)
include_directories(~/.local/lib/python3.6/site-packages/numpy/core/include/)
link_directories(~/usr/lib)
link_directories(/usr/lib/python3.6/config-3.6m-x86_64-linux-gnu)

add_definitions(-DMATPLOTLIBCPP_PYTHON_HEADER=Python.h)


# Declare the target (an executable)
ADD_EXECUTABLE(
isp fetch_raw.cpp 
isp_pipeline.cpp 
blc.cpp 
raw_hist.cpp 
isp_util.cpp 
raw_denoise.cpp 
sony_dol_hdr.cpp 
add_noise.cpp
plot_demo.cpp
bm3d_image_denoising.cpp
yuv_convert.cpp
matplot_demo.cpp
cfa.cpp
)

TARGET_LINK_LIBRARIES(isp ${OpenCV_LIBS} python3.6m)

MESSAGE(STATUS "OpenCV_LIBS: ${OpenCV_LIBS}")
MESSAGE(STATUS "OpenCV_DIR: ${OpenCV_DIR}")

