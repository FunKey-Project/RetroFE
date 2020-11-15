# - Find SDL_image library and headers
# 
# Find module for SDL_image 2.0 (http://www.libsdl.org/projects/SDL_image/).
# It defines the following variables:
#  SDL_IMAGE_INCLUDE_DIRS - The location of the headers, e.g., SDL_image.h.
#  SDL_IMAGE_LIBRARIES - The libraries to link against to use SDL_image.
#  SDL_IMAGE_FOUND - If false, do not try to use SDL_image.
#  SDL_IMAGE_VERSION_STRING
#    Human-readable string containing the version of SDL_image.
#
# Also defined, but not for general use are:
#   SDL_IMAGE_INCLUDE_DIR - The directory that contains SDL_image.h.
#   SDL_IMAGE_LIBRARY - The location of the SDL_image library.
#

#=============================================================================
# Copyright 2013 Benjamin Eikel
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

find_package(PkgConfig QUIET)
pkg_check_modules(PC_SDL_IMAGE QUIET SDL_image)

find_path(SDL_IMAGE_INCLUDE_DIR
  NAMES SDL/SDL_image.h
  HINTS
    ${SDL_IMAGE_ROOT}/include
)

find_library(SDL_IMAGE_LIBRARY
  NAMES SDL_image
  HINTS
    ${SDL_IMAGE_ROOT}/lib
  PATH_SUFFIXES x86
)

if(SDL_IMAGE_INCLUDE_DIR AND EXISTS "${SDL_IMAGE_INCLUDE_DIR}/SDL/SDL_image.h")
  file(STRINGS "${SDL_IMAGE_INCLUDE_DIR}/SDL/SDL_image.h" SDL_IMAGE_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL_IMAGE_MAJOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL_IMAGE_INCLUDE_DIR}/SDL/SDL_image.h" SDL_IMAGE_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL_IMAGE_MINOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL_IMAGE_INCLUDE_DIR}/SDL/SDL_image.h" SDL_IMAGE_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL_IMAGE_PATCHLEVEL[ \t]+[0-9]+$")
  string(REGEX REPLACE "^#define[ \t]+SDL_IMAGE_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL_IMAGE_VERSION_MAJOR "${SDL_IMAGE_VERSION_MAJOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_IMAGE_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL_IMAGE_VERSION_MINOR "${SDL_IMAGE_VERSION_MINOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_IMAGE_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDL_IMAGE_VERSION_PATCH "${SDL_IMAGE_VERSION_PATCH_LINE}")
  set(SDL_IMAGE_VERSION_STRING ${SDL_IMAGE_VERSION_MAJOR}.${SDL_IMAGE_VERSION_MINOR}.${SDL_IMAGE_VERSION_PATCH})
  unset(SDL_IMAGE_VERSION_MAJOR_LINE)
  unset(SDL_IMAGE_VERSION_MINOR_LINE)
  unset(SDL_IMAGE_VERSION_PATCH_LINE)
  unset(SDL_IMAGE_VERSION_MAJOR)
  unset(SDL_IMAGE_VERSION_MINOR)
  unset(SDL_IMAGE_VERSION_PATCH)
endif()

set(SDL_IMAGE_INCLUDE_DIRS ${SDL_IMAGE_INCLUDE_DIR} ${SDL_IMAGE_INCLUDE_DIR}/SDL)
set(SDL_IMAGE_LIBRARIES ${SDL_IMAGE_LIBRARY})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(SDL_image
                                  REQUIRED_VARS SDL_IMAGE_INCLUDE_DIRS SDL_IMAGE_LIBRARIES
                                  VERSION_VAR SDL_IMAGE_VERSION_STRING)

mark_as_advanced(SDL_IMAGE_INCLUDE_DIR SDL_IMAGE_LIBRARY)
