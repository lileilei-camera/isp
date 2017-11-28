PROJECT(isp)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

FIND_PACKAGE( OpenCV REQUIRED )

# Declare the target (an executable)
ADD_EXECUTABLE(isp  fetch_raw.c)

TARGET_LINK_LIBRARIES(isp ${OpenCV_LIBS})

MESSAGE(STATUS "OpenCV_LIBS: ${OpenCV_LIBS}")
