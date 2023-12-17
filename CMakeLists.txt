cmake_minimum_required(VERSION 3.5.0)
project(RaveCylinder VERSION 0.1.0 LANGUAGES C CXX)
configure_file(RaveCylinder.h.in RaveCylinder.h)

SET(SOURCES
    Color.h ColorUtils.h ColorUtils.cpp DDPOutput.h DDPOutput.cpp
    Pixel.h PixelMath.h PixelMath.cpp PixelUtils.h UDPClient.h main.cpp
   )

add_executable(RaveCylinder ${SOURCES})

target_include_directories(RaveCylinder PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )

target_compile_features(RaveCylinder PRIVATE cxx_std_17)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
