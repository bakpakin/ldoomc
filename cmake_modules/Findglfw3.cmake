# Locate the glfw library
# This module defines the following variables:
# GLFW3_LIBRARY, the name of the library;
# GLFW3_INCLUDE_DIR, where to find glfw include files.
# GLFW3_FOUND, true if both the GLFW_LIBRARY and GLFW_INCLUDE_DIR have been found.
#
# To help locate the library and include file, you could define an environment variable called
# GLFW3_ROOT which points to the root of the glfw library installation. This is pretty useful
# on a Windows platform.
#
#
# Usage example to compile an "executable" target to the glfw library:
#
# FIND_PACKAGE (glfw3 REQUIRED)
# INCLUDE_DIRECTORIES (${GLFW3_INCLUDE_DIRS})
# ADD_EXECUTABLE (executable ${EXECUTABLE_SRCS})
# TARGET_LINK_LIBRARIES (executable ${GLFW3_LIBRARIES})
#
# TODO:
# Allow the user to select to link to a shared library or to a static library.

#Search for the include file...
FIND_PATH(GLFW3_INCLUDE_DIRS GLFW/glfw3.h DOC "Path to GLFW include directory."
  HINTS
  $ENV{GLFW3_ROOT}
  PATH_SUFFIX include #For finding the include file under the root of the glfw expanded archive, typically on Windows.
  PATHS
  /usr/include/
  /usr/local/include/
  # By default headers are under GLFW subfolder
  /usr/include/GLFW
  /usr/local/include/GLFW
  ${GLFW3_ROOT_DIR}/include/ # added by ptr
)

SET(GLFW3_LIB_NAMES libglfw3.a glfw3 glfw GLFW3.lib)

FIND_LIBRARY(GLFW3_LIBRARIES DOC "Absolute path to GLFW library."
  NAMES ${GLFW3_LIB_NAMES}
  HINTS
  $ENV{GLFW3_ROOT}
  PATH_SUFFIXES lib/win32 #For finding the library file under the root of the glfw expanded archive, typically on Windows.
  PATHS
  /usr/local/lib
  /usr/lib
  ${GLFW3_ROOT_DIR}/lib-msvc100/release # added by ptr
)
IF( APPLE )
    find_library(IOKIT NAMES IOKit)
    #find_library(OpenGL NAMES OpenGL)
    find_library(COREVIDEO NAMES CoreVideo)
    find_library(COCOA NAMES Cocoa)
    SET(GLFW_LIBRARIES ${GLFW3_LIBRARIES} ${IOKIT} ${COREVIDEO} ${COCOA})
endif( APPLE )

IF(GLFW3_LIBRARIES AND GLFW3_INCLUDE_DIRS)
  SET(GLFW3_FOUND TRUE)
  message(STATUS "Found GLFW3: ${GLFW3_LIBRARIES}")
ELSE()
  message(STATUS "GLFW3 NOT found!")
ENDIF(GLFW3_LIBRARIES AND GLFW3_INCLUDE_DIRS)

if(GLFW3_FOUND)
  MARK_AS_ADVANCED(GLFW3_INCLUDE_DIRS GLFW3_LIBRARIES)
endif(GLFW3_FOUND)
