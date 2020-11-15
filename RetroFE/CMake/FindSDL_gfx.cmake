# Downloaded from http://cmake.3232098.n2.nabble.com/Find-modules-for-SDL-td7585211.html and adapted to SDL_gfx
#
# - Find SDL_gfx library and headers
# 
# Find module for SDL_gfx 2.0 (http://www.libsdl.org/projects/SDL_gfx/).
# It defines the following variables:
#  SDL_GFX_INCLUDE_DIRS - The location of the headers, e.g., SDL_gfx.h.
#  SDL_GFX_LIBRARIES - The libraries to link against to use SDL_gfx.
#  SDL_GFX_FOUND - If false, do not try to use SDL_gfx.
#  SDL_GFX_VERSION_STRING
#    Human-readable string containing the version of SDL_gfx.
#
# Also defined, but not for general use are:
#   SDL_GFX_INCLUDE_DIR - The directory that contains SDL_gfx.h.
#   SDL_GFX_LIBRARY - The location of the SDL_gfx library.
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
pkg_check_modules(PC_SDL_GFX QUIET SDL_gfx)

find_path(SDL_GFX_INCLUDE_DIR
  NAMES SDL/SDL_rotozoom.h
  HINTS
    ${SDL_GFX_ROOT}
  PATH_SUFFIXES include
)

find_library(SDL_GFX_LIBRARY
  NAMES SDL_gfx
  HINTS
    ${SDL_GFX_ROOT}/lib
  PATH_SUFFIXES x86
)

if(SDL_GFX_INCLUDE_DIR AND EXISTS "${SDL_GFX_INCLUDE_DIR}/SDL/SDL_gfx.h")
  file(STRINGS "${SDL_GFX_INCLUDE_DIR}/SDL/SDL_gfx.h" SDL_GFX_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL_GFX_MAJOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL_GFX_INCLUDE_DIR}/SDL/SDL_gfx.h" SDL_GFX_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL_GFX_MINOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL_GFX_INCLUDE_DIR}/SDL/SDL_gfx.h" SDL_GFX_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL_GFX_PATCHLEVEL[ \t]+[0-9]+$")
  string(REGEX REPLACE "^#define[ \t]+SDL_GFX_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL_GFX_VERSION_MAJOR "${SDL_GFX_VERSION_MAJOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_GFX_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL_GFX_VERSION_MINOR "${SDL_GFX_VERSION_MINOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_GFX_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDL_GFX_VERSION_PATCH "${SDL_GFX_VERSION_PATCH_LINE}")
  set(SDL_GFX_VERSION_STRING ${SDL_GFX_VERSION_MAJOR}.${SDL_GFX_VERSION_MINOR}.${SDL_GFX_VERSION_PATCH})
  unset(SDL_GFX_VERSION_MAJOR_LINE)
  unset(SDL_GFX_VERSION_MINOR_LINE)
  unset(SDL_GFX_VERSION_PATCH_LINE)
  unset(SDL_GFX_VERSION_MAJOR)
  unset(SDL_GFX_VERSION_MINOR)
  unset(SDL_GFX_VERSION_PATCH)
endif()

set(SDL_GFX_INCLUDE_DIRS ${SDL_GFX_INCLUDE_DIR} ${SDL_GFX_INCLUDE_DIR}/SDL)
set(SDL_GFX_LIBRARIES ${SDL_GFX_LIBRARY})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(SDL_gfx
                                  REQUIRED_VARS SDL_GFX_INCLUDE_DIRS SDL_GFX_LIBRARIES
                                  VERSION_VAR SDL_GFX_VERSION_STRING)

mark_as_advanced(SDL_GFX_INCLUDE_DIR SDL_GFX_LIBRARY)
