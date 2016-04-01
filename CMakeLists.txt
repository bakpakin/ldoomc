# LDoom
# Copyright 2015-2016 Calvin Rose

cmake_minimum_required(VERSION 2.8)
project(Ldoom)

# Options
option(LDOOMC_DEBUG "Debug build (on) or release build (off)." ON)
option(LDOOMC_AMALG "Build all c files together. Might make binary more efficient, or might not work at all." OFF)


# Set Some Variables
set(TARGET_NAME ${PROJECT_NAME})
if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        set (CMAKE_C_FLAGS "--std=gnu99 ${CMAKE_C_FLAGS}")
    endif ()
else ()
    set (CMAKE_C_STANDARD 99)
endif ()

set(SOURCES
src/main.c
src/platform.c
src/util.c
src/ldmath.c
src/log.c
src/geom.c
src/shader.c
src/mesh.c
src/texture.c
src/camera.c
src/fntdraw.c
src/arenastate.c
src/menustate.c
src/quickdraw.c
src/scene.c
src/mob.c
src/console.c
src/sky.c
src/glad.c
src/luainterop.c
## Thirdparty static libs
src/lodepng.c
src/stb_vorbis.c
)

# TODO: Auto-generate the c file used to make the amlagamated build.
if(${LDOOMC_AMLAG})
    set(SOURCES "src/amalg.c")
endif()

include_directories(src)

## Game resources
set(RESOURCES
resources/basic.glsl
resources/diffuseshader.glsl
resources/consolefont.png
resources/consolefont.txt
resources/hud.png
resources/hud.txt
resources/sky.glsl
resources/diffuse.png
resources/reflectshader.glsl
resources/skytop.png
resources/skybottom.png
resources/skyleft.png
resources/skyright.png
resources/skyfront.png
resources/skyback.png
## Scripts
resources/scripts/bootstrap.lua
)

# GLFW Reset default options
option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option(GLFW_INSTALL "Generate installation target" OFF)

# Build the executable
if(${LDOOMC_DEBUG})
    add_executable(${TARGET_NAME} ${SOURCES})
    add_custom_target(copy_resources)
    foreach(filename ${RESOURCES})
        set(SRC "${CMAKE_SOURCE_DIR}/${filename}")
        set(DST "${CMAKE_BINARY_DIR}/${filename}")
        add_custom_command(
            TARGET copy_resources
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SRC} ${DST}
            COMMENT "Copying file: ${filename}")
    endforeach(filename)
    add_dependencies(${TARGET_NAME} copy_resources)
else(${LDOOMC_DEBUG})
    if(APPLE)
        # Create an app bundle for apple
        set(ICON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/osx/icon.icns)
        set(MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/osx/Info.plist)
        set(MACOSX_BUNDLE_BUNDLE_NAME ${TARGET_NAME})
        set(MACOSX_BUNDLE_ICON_FILE icon.icns) 
        set_source_files_properties(${ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
        # Add each resource to resources bundle in the app
        foreach(res ${RESOURCES})
            get_filename_component(resdir ${res} DIRECTORY)
            string(REPLACE "resources" "" resdir ${resdir})
            set_source_files_properties(${res} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/${resdir}")
        endforeach(res)
        # Make the executable
        add_executable(${TARGET_NAME} MACOSX_BUNDLE ${ICON_FILE} ${SOURCES} ${RESOURCES})
        set_target_properties(${TARGET_NAME} PROPERTIES MACOSX_BUNDLE TRUE)
        # Add LuaJIT required link flags
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pagezero_size 10000 -image_base 100000000")
        # Link any OSX specific frameworks
        target_link_libraries(${TARGET_NAME} "-framework CoreFoundation")
    else(APPLE)
        message(FATAL_ERROR "Release build not yet implemented for this platform.")
    endif(APPLE)
endif(${LDOOMC_DEBUG})

# LuaJIT Compilation using external build system. 
# (make on Mac and linux, and custom build script on windows)
set(LUAJIT_LIB ${CMAKE_SOURCE_DIR}/luajit/src/libluajit.a)
add_custom_target(libluajit DEPENDS ${LUAJIT_LIB})
add_dependencies(${TARGET_NAME} libluajit)
add_custom_command(
    OUTPUT ${LUAJIT_LIB}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/luajit/src
    COMMAND make amalg
    VERBATIM
)

# Add custom flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter -O3 -s -ffast-math -fno-math-errno")

# Include Library Headers and set up linking
add_subdirectory("glfw")
find_package(OpenGL REQUIRED)
find_package(OpenAL REQUIRED)
include_directories("glfw/include" "luajit/src" ${OPENAL_INCLUDE_DIR})
target_link_libraries(
    ${TARGET_NAME} 
    ${LUAJIT_LIB}
    glfw
    ${GLFW_LIBRARIES}
    ${OPENGL_gl_LIBRARY}
    ${OPENAL_LIBRARY}
)
