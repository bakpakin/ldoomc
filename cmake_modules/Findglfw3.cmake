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

#Search for the include file...
FIND_PATH(GLFW3_INCLUDE_DIRS GLFW/glfw3.h DOC "Path to GLFW include directory."
  
  HINTS
	${GLFW3_ROOT}
  
  PATHS
	/usr/include/
	/usr/local/include/
	/usr/include/GLFW
	/usr/local/include/GLFW
	${GLFW3_ROOT}/include
)

FIND_LIBRARY(GLFW3_LIBRARIES DOC "Absolute path to GLFW library."

  NAMES 
    libglfw3.so
    libglfw3
    glfw3.so
    glfw3
  
  HINTS
	${GLFW3_ROOT}
  
  PATHS
	/usr/local/lib
	/usr/lib
	${GLFW3_ROOT}/lib-vc2015
)
IF( APPLE )
    find_library(COREVIDEO NAMES CoreVideo)
    find_library(COCOA NAMES Cocoa)
    find_library(IOKIT NAMES IOKit)
    SET(GLFW_LIBRARIES ${GLFW3_LIBRARIES} ${IOKIT} ${COREVIDEO} ${COCOA})
ENDIF( APPLE )

IF(GLFW3_LIBRARIES AND GLFW3_INCLUDE_DIRS)
  SET(GLFW3_FOUND TRUE)
  message(STATUS "Found GLFW3> Libraries: ${GLFW3_LIBRARIES}")
  message(STATUS "Found GLFW3> Headers: ${GLFW3_INCLUDE_DIRS}")
ELSE(GLFW3_LIBRARIES AND GLFW3_INCLUDE_DIRS)
  message(STATUS "GLFW3 NOT found!")
ENDIF(GLFW3_LIBRARIES AND GLFW3_INCLUDE_DIRS)

if(GLFW3_FOUND)
  MARK_AS_ADVANCED(GLFW3_INCLUDE_DIRS GLFW3_LIBRARIES)
endif(GLFW3_FOUND)
