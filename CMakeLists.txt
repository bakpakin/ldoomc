cmake_minimum_required(VERSION 3.0)
project(Ldoom)

# Set Some Variables
set(TARGET_NAME ${PROJECT_NAME})

# Import our CMake scripts
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${${TARGET_NAME}_SOURCE_DIR}/cmake_modules)

set(SOURCES
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
src/arenastate.c

## Thirdparty static libs
src/lodepng.c
src/stb_vorbis.c
)

## Includes
include_directories(src)

# Set Executable Target
add_executable(${TARGET_NAME} ${SOURCES})

# Add custom flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

# Set up Libraries
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
include_directories(${GLFW3_INCLUDE_DIRS}
                    ${OPENGL_INCLUDE_DIR})
target_link_libraries(${TARGET_NAME} 
                      ${GLFW3_LIBRARIES}
                      ${OPENGL_gl_LIBRARY})

# Create bundle
if(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE icons/tempicon.icns)
    set_source_files_properties(icons/tempicon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    list(APPEND SOURCES icons/tempicon.icns)
endif(APPLE)

#--------------------------------------------------------------------------------
# Now the installation stuff below
#--------------------------------------------------------------------------------
set(plugin_dest_dir bin)
set(qtconf_dest_dir bin)
set(APPS "\${CMAKE_INSTALL_PREFIX}/bin/Ldoom")
if(APPLE)
    set(plugin_dest_dir Ldoom.app/Contents/MacOS)
    set(APPS "\${CMAKE_INSTALL_PREFIX}/Ldoom.app")
endif(APPLE)
if(WIN32)
  set(APPS "\${CMAKE_INSTALL_PREFIX}/bin/Ldoom.exe")
endif(WIN32)

#--------------------------------------------------------------------------------
INSTALL(TARGETS Ldoom 
    BUNDLE DESTINATION . COMPONENT Runtime
    RUNTIME DESTINATION bin COMPONENT Runtime
    )

# To Create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" on Mac OS X
# where CPackConfig.cmake is created by including CPack
# And then there's ways to customize this as well
set(CPACK_BINARY_DRAGNDROP ON)
include(CPack)
