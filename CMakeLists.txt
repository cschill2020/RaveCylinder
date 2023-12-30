cmake_minimum_required(VERSION 3.5.0)
project(RaveCylinder VERSION 0.1.0 LANGUAGES C CXX)
configure_file(RaveCylinder.h.in RaveCylinder.h)

SET(SOURCES
    Color.h 
    ColorPalettes.h ColorPalettes.cpp
    ColorUtils.h ColorUtils.cpp 
    DDPOutput.h DDPOutput.cpp
    Pixel.h 
    PixelMath.h PixelMath.cpp 
    PixelUtils.h 
    UDPClient.h 
    WLED_colors.cpp
    WLED_const.h
    WLED_fcn_declare.h
    WLED_FX_2Dfcn.cpp WLED_FX_fcn.cpp WLED_FX.cpp WLED_FX.h  
    WLED_noise.cpp
    WLED_palettes.h
    WLED.cpp WLED.h 
    main.cpp
   )

add_executable(RaveCylinder ${SOURCES})

target_include_directories(RaveCylinder PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )

target_compile_features(RaveCylinder PRIVATE cxx_std_17)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
