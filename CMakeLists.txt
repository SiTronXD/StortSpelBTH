cmake_minimum_required(VERSION 3.23)
project(3D_VEngine_GameDemo VERSION 0.0.1
                    DESCRIPTION "Demo using 3D_Vengine library"
                    LANGUAGES CXX)

include(cmake/program_dependencies.cmake)
include(cmake/setup_conan.cmake)
include(cmake/helper_funcs.cmake)

# Setup Conan, fetch required dependencies
find_program_dependencies()


# Compile with CCache if available to speed up compilation
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CUDA_COMPILER_LAUNCHER "${CCACHE_PROGRAM}") # CMake 3.9+
endif()

# Setup C++ settings
set(USE_COMPILE_COMMANDS ON)

add_executable(3D_VEngine_GameDemo main.cpp ) #${SOURCES} 

setup_conan_dependencies(3D_VEngine_GameDemo)

set_property(TARGET 3D_VEngine_GameDemo PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time") # measure compiletime
target_compile_features(3D_VEngine_GameDemo PUBLIC cxx_std_20)

set_target_properties(3D_VEngine_GameDemo PROPERTIES
                #CXX_STANDARD ${CPP_LANGUANGE_STANDARD}
                CXX_EXTENSIONS OFF                      
                EXPORT_COMPILE_COMMANDS ${USE_COMPILE_COMMANDS} 
                INTERPROCEDURAL_OPTIMIZATION TRUE       # Will do link time optimizations if available
                LINK_WHAT_YOU_USE TRUE                          
                ) 

find_package(Vulkan REQUIRED)
#vengine_find_module(vengine_imgui deps/imgui imgui_lib)

#################################


FILE(GLOB IMGUI_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/deps/imgui/src/*.cpp        
    ${CMAKE_CURRENT_LIST_DIR}/deps/imgui/src/backends/imgui_impl_vulkan.cpp        
    ${CMAKE_CURRENT_LIST_DIR}/deps/imgui/src/backends/imgui_impl_sdl.cpp)

FILE(GLOB IMGUI_HEADERS CONFIGURE_DEPENDS            
    ${CMAKE_CURRENT_LIST_DIR}/deps/imgui/src/backends/imgui_impl_vulkan.h            
    ${CMAKE_CURRENT_LIST_DIR}/deps/imgui/src/backends/imgui_impl_sdl.h
    ${CMAKE_CURRENT_LIST_DIR}/deps/imgui/src/*.h
)


add_library(imgui_lib STATIC)

set_target_properties(
    imgui_lib
    PROPERTIES VERSION ${PROJECT_VERSION}
            SOVERSION ${PROJECT_VERSION_MAJOR}
)
target_compile_features(imgui_lib PUBLIC cxx_std_20)

target_sources(imgui_lib PRIVATE "${IMGUI_SOURCES}" "${IMGUI_HEADERS}")


set(imgui_include_dirs "")
list(APPEND imgui_include_dirs ${CMAKE_CURRENT_LIST_DIR}/deps/imgui/src)
list(APPEND imgui_include_dirs ${CMAKE_CURRENT_LIST_DIR}/deps/imgui/src/backends)
target_include_directories(imgui_lib PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/deps/imgui/src> 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/deps/imgui/src/backends> 
)

target_link_libraries(imgui_lib PRIVATE SDL2::SDL2)
target_link_libraries(imgui_lib PRIVATE ${Vulkan_LIBRARY})
target_include_directories(imgui_lib PUBLIC ${Vulkan_INCLUDE_DIR})
target_include_directories(imgui_lib PUBLIC ${SDL2_INCLUDE_DIRS})

set_property(
    TARGET imgui_lib
    PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE
)
#################################


target_link_libraries(
    3D_VEngine_GameDemo
    PUBLIC
    Vulkan::Vulkan    
    #${vengine_imgui}
    ${imgui_lib}
    ${SDL2_LIBRARIES}
    ${CMAKE_CURRENT_LIST_DIR}/install/lib/libassimpd.so
    ${CMAKE_CURRENT_LIST_DIR}/install/lib/libTracyClientd.so
    ${CMAKE_CURRENT_LIST_DIR}/libvengined.a
    ${CMAKE_CURRENT_LIST_DIR}/install/lib/libimgui_libd.a
)              
target_include_directories(3D_VEngine_GameDemo PUBLIC ${imgui_include_dirs})  
target_include_directories(
    3D_VEngine_GameDemo
    PUBLIC
    install/include

    eng
)

# == FETCHING VENGINE ==
# Everything under this line is everyhing needed to fetch and use VEngine (except the downloading model part...)
# include(FetchContent)
# FetchContent_Declare(
#     vengine
#     GIT_REPOSITORY  https://github.com/SiTronXD/StortSpelBTH_Engine.git
#     GIT_TAG         cmake_fixes                                
#     GIT_SHALLOW     TRUE
#     GIT_PROGRESS    TRUE
# )
# set(VENGINE_INSTALL_ASSETS_PATH          ${CMAKE_SOURCE_DIR}        CACHE INTERNAL "Installation location of the assets folder")
# set(VENGINE_SYMLINK_TO_ASSETS_PATH       ${CMAKE_BINARY_DIR}        CACHE INTERNAL "Creates a symlink from the assets path to the build dir, so no copying of assets is needed")
# set(VENGINE_INSTALL_VENGINE_HELPER_PATH  ${CMAKE_SOURCE_DIR}/cmake  CACHE INTERNAL "Installation of vengine_helpers.cmake which contains cmake functions to build shaders.")
# FetchContent_MakeAvailable (vengine)
# list(APPEND CMAKE_MODULE_PATH ${vengine_SOURCE_DIR}/cmake)

# include(cmake/vengine_helpers.cmake)    # This file will be installed to VENGINE_INSTALL_VENGINE_HELPER_PATH. It will first be available after cmake has done a configure...
# build_shaders(3D_VEngine_GameDemo ${CMAKE_SOURCE_DIR}/assets ${CMAKE_SOURCE_DIR}/assets) # Build shaders, source files and compiled files will be in the same folder
    
#target_link_libraries(3D_VEngine_GameDemo PUBLIC vengine::vengine)




