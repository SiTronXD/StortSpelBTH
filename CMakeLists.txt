cmake_minimum_required(VERSION 3.23)
project(3D_VEngine_GameDemo VERSION 0.0.1
                    DESCRIPTION "Demo using 3D_Vengine library"
                    LANGUAGES CXX)

include(cmake/helper_funcs.cmake)

# Show download status during fetch
Set(FETCHCONTENT_QUIET FALSE)

# Compile with CCache if available to speed up compilation
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CUDA_COMPILER_LAUNCHER "${CCACHE_PROGRAM}") # CMake 3.9+
endif()

# Setup C++ settings
set(USE_COMPILE_COMMANDS ON)


# all the source files
FILE(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS
    game/*.cpp 
    game/*.h 
    game/*.hpp
)
# all the source files
FILE(GLOB_RECURSE GAME_HEADERS CONFIGURE_DEPENDS    
    game/*.h 
    game/*.hpp 
)




add_executable(3D_VEngine_GameDemo  ) #${SOURCES} main.cpp

target_sources(3D_VEngine_GameDemo PRIVATE ${GAME_SOURCES})

set_property(TARGET 3D_VEngine_GameDemo PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time") # measure compiletime
target_compile_features(3D_VEngine_GameDemo PUBLIC cxx_std_20)

set_target_properties(3D_VEngine_GameDemo PROPERTIES                
                CXX_EXTENSIONS OFF                      
                EXPORT_COMPILE_COMMANDS ${USE_COMPILE_COMMANDS} 
                INTERPROCEDURAL_OPTIMIZATION TRUE       # Will do link time optimizations if available
                LINK_WHAT_YOU_USE TRUE                          
                ) 


#

if(${CMAKE_BUILD_TYPE} MATCHES "Debug" OR ${CMAKE_BUILD_TYPE} MATCHES "Release" )
target_compile_definitions(3D_VEngine_GameDemo PRIVATE "_CONSOLE" )
target_compile_definitions(3D_VEngine_GameDemo PRIVATE "NO_MOUSE_GRAB" )
target_compile_definitions(3D_VEngine_GameDemo PRIVATE "_DEBUG" )
endif()

if(${CMAKE_BUILD_TYPE} MATCHES "Debug" )
target_compile_definitions(3D_VEngine_GameDemo PRIVATE "_DEBUG" )
endif()

set(VENGINE_USE_PROFILING true)

add_subdirectory(deps/vengine/src)

target_include_directories(3D_VEngine_GameDemo PUBLIC  deps/vengine/src/vengine)           # Needed for vengine.h
target_include_directories(3D_VEngine_GameDemo PUBLIC  deps/vengine/src/vengine/pch)       # Needed for vengine.h
target_include_directories(3D_VEngine_GameDemo PUBLIC  deps/vengine/src/vengine/vengine)
target_include_directories(3D_VEngine_GameDemo PUBLIC  deps/vengine/src/vengine/vengine/test) # Get testscene...? 
target_link_libraries(3D_VEngine_GameDemo PUBLIC vengine::vengine) 

create_symlink_now(${CMAKE_SOURCE_DIR}/deps/vengine/src/vengine/vengine_assets ${CMAKE_SOURCE_DIR}/game/vengine_assets)
#create_symlink_now(${CMAKE_SOURCE_DIR}/ ${CMAKE_SOURCE_DIR}/game/vengine_assets)


# Ignore FETCHING variant for now... 
set(FETCHING FALSE)
if(${FETCHING})
    # == FETCHING VENGINE ==
    # Everything under this line is everyhing needed to fetch and use VEngine (except the downloading model part...)
    include(FetchContent)
    FetchContent_Declare(
        vengine_fetch
        GIT_REPOSITORY  https://github.com/SiTronXD/StortSpelBTH_Engine.git
        GIT_TAG         main                               
        GIT_SHALLOW     TRUE
        GIT_PROGRESS    TRUE
    )
    set(VENGINE_USE_PROFILING true)
    set(VENGINE_INSTALL_ASSETS_PATH          ${CMAKE_SOURCE_DIR}        CACHE INTERNAL "Installation location of the assets folder")
    set(VENGINE_SYMLINK_TO_ASSETS_PATH       ${CMAKE_BINARY_DIR}        CACHE INTERNAL "Creates a symllink from the assets path to the build dir, so no copying of assets is needed")
    set(VENGINE_INSTALL_VENGINE_HELPER_PATH  ${CMAKE_SOURCE_DIR}/cmake  CACHE INTERNAL "Installation of vengine_helpers.cmake which contains cmake functions to build shaders.")
    FetchContent_MakeAvailable (vengine_fetch)
    list(APPEND CMAKE_MODULE_PATH ${vengine_SOURCE_DIR}/cmake)

    include(cmake/vengine_helpers.cmake)    # This file will be installed to VENGINE_INSTALL_VENGINE_HELPER_PATH. It will first be available after cmake has done a configure...
    build_shaders(3D_VEngine_GameDemo ${CMAKE_SOURCE_DIR}/assets ${CMAKE_SOURCE_DIR}/assets) # Build shaders, source files and compiled files will be in the same folder

    target_link_libraries(3D_VEngine_GameDemo PUBLIC vengine::vengine) 

    if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

        # Command to copy dll-files to build folder will begin after build...
        add_custom_command(TARGET 3D_VEngine_GameDemo POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:vengine> $<TARGET_FILE_DIR:3D_VEngine_GameDemo>
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:3D_VEngine_GameDemo> $<TARGET_FILE_DIR:3D_VEngine_GameDemo>
            COMMENT "Copying dll-files to location of executable..."
            COMMAND_EXPAND_LISTS
            )

        set_target_properties(3D_VEngine_GameDemo PROPERTIES  
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")        
    endif()


    add_test(NAME sanitizer_test COMMAND vengine)
    set_tests_properties(sanitizer_test PROPERTIES WILL_FAIL True)
endif()