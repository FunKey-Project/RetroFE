# Downloaded from http://cmake.3232098.n2.nabble.com/Find-modules-for-SDL-td7585211.html and adapted to SDL_ttf
#
# - Find SDL_ttf library and headers
# 
# Find module for SDL_ttf 2.0 (http://www.libsdl.org/projects/SDL_ttf/).
# It defines the following variables:
#  SDL_TTF_INCLUDE_DIRS - The location of the headers, e.g., SDL_ttf.h.
#  SDL_TTF_LIBRARIES - The libraries to link against to use SDL_ttf.
#  SDL_TTF_FOUND - If false, do not try to use SDL_ttf.
#  SDL_TTF_VERSION_STRING
#    Human-readable string containing the version of SDL_ttf.
#
# Also defined, but not for general use are:
#   SDL_TTF_INCLUDE_DIR - The directory that contains SDL_ttf.h.
#   SDL_TTF_LIBRARY - The location of the SDL_ttf library.
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
pkg_check_modules(PC_SDL_TTF QUIET SDL_ttf)

find_path(SDL_TTF_INCLUDE_DIR
  NAMES SDL/SDL_ttf.h
  HINTS
    ${SDL_TTF_ROOT}
  PATH_SUFFIXES include
)

find_library(SDL_TTF_LIBRARY
  NAMES SDL_ttf
  HINTS
    ${SDL_TTF_ROOT}/lib
  PATH_SUFFIXES x86
)

if(SDL_TTF_INCLUDE_DIR AND EXISTS "${SDL_TTF_INCLUDE_DIR}/SDL/SDL_ttf.h")
  file(STRINGS "${SDL_TTF_INCLUDE_DIR}/SDL/SDL_ttf.h" SDL_TTF_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL_TTF_MAJOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL_TTF_INCLUDE_DIR}/SDL/SDL_ttf.h" SDL_TTF_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL_TTF_MINOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL_TTF_INCLUDE_DIR}/SDL/SDL_ttf.h" SDL_TTF_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL_TTF_PATCHLEVEL[ \t]+[0-9]+$")
  string(REGEX REPLACE "^#define[ \t]+SDL_TTF_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL_TTF_VERSION_MAJOR "${SDL_TTF_VERSION_MAJOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_TTF_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL_TTF_VERSION_MINOR "${SDL_TTF_VERSION_MINOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_TTF_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDL_TTF_VERSION_PATCH "${SDL_TTF_VERSION_PATCH_LINE}")
  set(SDL_TTF_VERSION_STRING ${SDL_TTF_VERSION_MAJOR}.${SDL_TTF_VERSION_MINOR}.${SDL_TTF_VERSION_PATCH})
  unset(SDL_TTF_VERSION_MAJOR_LINE)
  unset(SDL_TTF_VERSION_MINOR_LINE)
  unset(SDL_TTF_VERSION_PATCH_LINE)
  unset(SDL_TTF_VERSION_MAJOR)
  unset(SDL_TTF_VERSION_MINOR)
  unset(SDL_TTF_VERSION_PATCH)
endif()

set(SDL_TTF_INCLUDE_DIRS ${SDL_TTF_INCLUDE_DIR} ${SDL_TTF_INCLUDE_DIR}/SDL)
set(SDL_TTF_LIBRARIES ${SDL_TTF_LIBRARY})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(SDL_ttf
                                  REQUIRED_VARS SDL_TTF_INCLUDE_DIRS SDL_TTF_LIBRARIES
                                  VERSION_VAR SDL_TTF_VERSION_STRING)

mark_as_advanced(SDL_TTF_INCLUDE_DIR SDL_TTF_LIBRARY)
