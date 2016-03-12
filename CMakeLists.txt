# LDoom
# Copyright 2015-2016 Calvin Rose

cmake_minimum_required(VERSION 2.8)
project(Ldoom)

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
src/log.c
src/opool.c
src/geom.c
src/shader.c
src/mesh.c
src/texture.c
src/camera.c
src/fntdraw.c
src/arenastate.c
src/quickdraw.c
src/scene.c
src/mob.c
src/console.c
src/sky.c

## Thirdparty static libs
src/lodepng.c
src/stb_vorbis.c
)

## Includes
include_directories(src)

## Game resources
set(RESOURCES
resources/basic.glsl
resources/diffuseshader.glsl
resources/consolefont.png
resources/consolefont.txt
resources/sky.glsl
resources/fancy.txt
resources/fancy.png
resources/diffuse.png
resources/grass.png
resources/reflectshader.glsl
resources/skytop.png
resources/skybottom.png
resources/skyleft.png
resources/skyright.png
resources/skyfront.png
resources/skyback.png
)

# Set Executable Target
if(APPLE)

    # Create an app bundle for apple
    set(ICON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/osx/icon.icns)
    set(MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/osx/Info.plist)
    set(MACOSX_BUNDLE_BUNDLE_NAME ${TARGET_NAME})
    set(MACOSX_BUNDLE_ICON_FILE icon.icns) 
    set_source_files_properties(${ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    # Add each resource to resources bundle in the app
    foreach(res ${RESOURCES})
        set_source_files_properties(${res} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    endforeach(res)

    # Make the executable
    add_executable(${TARGET_NAME} MACOSX_BUNDLE ${ICON_FILE} ${SOURCES} ${RESOURCES})
    set_target_properties(${TARGET_NAME} PROPERTIES MACOSX_BUNDLE TRUE)

    # Link any OSX specific frameworks
    target_link_libraries(${TARGET_NAME} "-framework CoreFoundation")

else(APPLE)
    
    add_executable(${TARGET_NAME} ${SOURCES})

endif(APPLE)

# Add custom flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-function -Wno-unused-parameter")

# Include Library Headers and ste up linking
add_subdirectory("glfw")
include_directories("glfw/include")
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
target_link_libraries(
    ${TARGET_NAME} 
    glfw
    ${GLFW_LIBRARIES}
    ${OPENGL_LIBRARIES})

