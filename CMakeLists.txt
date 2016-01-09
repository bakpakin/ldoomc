cmake_minimum_required(VERSION 3.0)
project(Ldoom)

# Set Some Variables
set(TARGET_NAME ${PROJECT_NAME})

# Import our CMake scripts
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${${TARGET_NAME}_SOURCE_DIR}/cmake_modules)

# Get Sources
set(SOURCES "")

## Main
list(APPEND SOURCES
src/main.c
src/util.c
src/gamestate.c
src/opool.c
src/grid3d.c
src/geom.c
src/shader.c
src/mesh.c
src/texture.c
src/camera.c
src/fntdraw.c

## Thirdparty static libs
src/lodepng.c
src/stb_vorbis.c
)

## Includes
include_directories(src)

# Set Executable Target
add_executable(${TARGET_NAME} ${SOURCES})

# Add custom flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")

# Set up Libraries
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
include_directories(${GLFW3_INCLUDE_DIRS}
                    ${OPENGL_INCLUDE_DIR})
target_link_libraries(${TARGET_NAME} 
                      ${GLFW3_LIBRARIES}
                      ${OPENGL_gl_LIBRARY})
