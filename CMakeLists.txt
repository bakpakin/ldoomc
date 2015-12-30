cmake_minimum_required(VERSION 3.0)
project(Ldoom)

# Set Some Variables
set(TARGET_NAME ${PROJECT_NAME})
set(BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# Import our CMake scripts
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${${TARGET_NAME}_SOURCE_DIR}/cmake_modules)

# Get Sources
set(SOURCES "")

## Main
list(APPEND SOURCES
src/main.c
src/util.c
src/gamestate.c
src/ds/opool.c
src/ds/grid3d.c
src/math/geom.c
src/render/shader.c
src/render/mesh.c
src/render/texture.c
src/render/camera.c

## Thirdparty static libs
src/thirdparty/lodepng.c
src/thirdparty/stb_vorbis.c
)

## Includes
include_directories(
src
src/math
src/render
src/ds
src/thirdparty
)

# Set Executable Target
add_executable(${TARGET_NAME} ${SOURCES})

# Add custom flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")

# Set up Libraries
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(OpenGL REQUIRED)
include_directories(${SDL2_INCLUDE_DIR}
                    ${SDL2_TTF_INCLUDE_DIR}
                    ${OPENGL_INCLUDE_DIR})
target_link_libraries(${TARGET_NAME} ${SDL2_LIBRARY}
                             ${SDL2_TTF_LIBRARIES}
                             ${OPENGL_gl_LIBRARY})
