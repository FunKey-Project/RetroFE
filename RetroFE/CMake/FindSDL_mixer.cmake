# Downloaded from http://cmake.3232098.n2.nabble.com/Find-modules-for-SDL-td7585211.html and adapted to SDL_mixer
#
# - Find SDL_mixer library and headers
# 
# Find module for SDL_mixer 2.0 (http://www.libsdl.org/projects/SDL_mixer/).
# It defines the following variables:
#  SDL_MIXER_INCLUDE_DIRS - The location of the headers, e.g., SDL_mixer.h.
#  SDL_MIXER_LIBRARIES - The libraries to link against to use SDL_mixer.
#  SDL_MIXER_FOUND - If false, do not try to use SDL_mixer.
#  SDL_MIXER_VERSION_STRING
#    Human-readable string containing the version of SDL_mixer.
#
# Also defined, but not for general use are:
#   SDL_MIXER_INCLUDE_DIR - The directory that contains SDL_mixer.h.
#   SDL_MIXER_LIBRARY - The location of the SDL_mixer library.
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
pkg_check_modules(PC_SDL_MIXER QUIET SDL_mixer)

find_path(SDL_MIXER_INCLUDE_DIR
  NAMES SDL/SDL_mixer.h
  HINTS
    ${SDL_MIXER_ROOT}/include
)

find_library(SDL_MIXER_LIBRARY
  NAMES SDL_mixer
  HINTS
    ${SDL_MIXER_ROOT}/lib
  PATH_SUFFIXES x86
)

if(SDL_MIXER_INCLUDE_DIR AND EXISTS "${SDL_MIXER_INCLUDE_DIR}/SDL/SDL_mixer.h")
  file(STRINGS "${SDL_MIXER_INCLUDE_DIR}/SDL/SDL_mixer.h" SDL_MIXER_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL_MIXER_MAJOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL_MIXER_INCLUDE_DIR}/SDL/SDL_mixer.h" SDL_MIXER_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL_MIXER_MINOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL_MIXER_INCLUDE_DIR}/SDL/SDL_mixer.h" SDL_MIXER_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL_MIXER_PATCHLEVEL[ \t]+[0-9]+$")
  string(REGEX REPLACE "^#define[ \t]+SDL_MIXER_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL_MIXER_VERSION_MAJOR "${SDL_MIXER_VERSION_MAJOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_MIXER_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL_MIXER_VERSION_MINOR "${SDL_MIXER_VERSION_MINOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_MIXER_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDL_MIXER_VERSION_PATCH "${SDL_MIXER_VERSION_PATCH_LINE}")
  set(SDL_MIXER_VERSION_STRING ${SDL_MIXER_VERSION_MAJOR}.${SDL_MIXER_VERSION_MINOR}.${SDL_MIXER_VERSION_PATCH})
  unset(SDL_MIXER_VERSION_MAJOR_LINE)
  unset(SDL_MIXER_VERSION_MINOR_LINE)
  unset(SDL_MIXER_VERSION_PATCH_LINE)
  unset(SDL_MIXER_VERSION_MAJOR)
  unset(SDL_MIXER_VERSION_MINOR)
  unset(SDL_MIXER_VERSION_PATCH)
endif()

set(SDL_MIXER_INCLUDE_DIRS ${SDL_MIXER_INCLUDE_DIR} ${SDL_MIXER_INCLUDE_DIR}/SDL)
set(SDL_MIXER_LIBRARIES ${SDL_MIXER_LIBRARY})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(SDL_mixer
                                  REQUIRED_VARS SDL_MIXER_INCLUDE_DIRS SDL_MIXER_LIBRARIES
                                  VERSION_VAR SDL_MIXER_VERSION_STRING)

mark_as_advanced(SDL_MIXER_INCLUDE_DIR SDL_MIXER_LIBRARY)
