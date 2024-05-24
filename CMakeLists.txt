cmake_minimum_required(VERSION 3.5.0)
project(RaveCylinder VERSION 0.1.0 LANGUAGES C CXX)
configure_file(RaveCylinder.h.in RaveCylinder.h)

set(JSON_BuildTests OFF CACHE INTERNAL "")

find_package(PkgConfig REQUIRED)
find_package(nlohmann_json 3.1.3 REQUIRED)
pkg_check_modules(PC_HTTPSERVER REQUIRED IMPORTED_TARGET
				  libhttpserver>=0.19)
pkg_check_modules(PC_MICROHTTPD REQUIRED IMPORTED_TARGET
				  libmicrohttpd>=0.9.52)
                  
SET(SOURCES
    Color.h 
    ColorPalettes.h ColorPalettes.cpp
    ColorUtils.h ColorUtils.cpp 
    DDPOutput.h DDPOutput.cpp
    Pixel.h 
    PixelMath.h PixelMath.cpp 
    PixelUtils.h 
    UDPClient.h 
    WLED_cfg.cpp
    WLED_colors.cpp
    WLED_const.h
    WLED_fcn_declare.h
    WLED_file.cpp
    WLED_FX_2Dfcn.cpp WLED_FX_fcn.cpp WLED_FX.cpp WLED_FX.h  
    WLED_json.cpp
    WLED_led.cpp
    WLED_noise.cpp
    WLED_palettes.h
    WLED_playlist.cpp
    WLED_presets.cpp
    WLED.cpp WLED.h 
    WLED_set.cpp
    WLED_server.cpp
    WLED_xml.cpp
    main.cpp
   )
add_executable(RaveCylinder ${SOURCES})

target_include_directories(RaveCylinder 
    PUBLIC
        "${PROJECT_BINARY_DIR}"
)
                          
target_compile_features(RaveCylinder PRIVATE cxx_std_17)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)

target_link_libraries(RaveCylinder
    PRIVATE
    PkgConfig::PC_HTTPSERVER
    PkgConfig::PC_MICROHTTPD
    nlohmann_json::nlohmann_json
    -fsanitize=address
    -fno-stack-protector
)
