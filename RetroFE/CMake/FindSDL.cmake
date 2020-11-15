# - Find SDL library and headers
# 
# Find module for SDL 2.0 (http://www.libsdl.org/).
# It defines the following variables:
#  SDL_INCLUDE_DIRS - The location of the headers, e.g., SDL.h.
#  SDL_LIBRARIES - The libraries to link against to use SDL.
#  SDL_FOUND - If false, do not try to use SDL.
#  SDL_VERSION_STRING - Human-readable string containing the version of SDL.
#
# This module responds to the the flag:
#  SDL_BUILDING_LIBRARY
#    If this is defined, then no SDL_main will be linked in because
#    only applications need main().
#    Otherwise, it is assumed you are building an application and this
#    module will attempt to locate and set the the proper link flags
#    as part of the returned SDL_LIBRARIES variable.
#
# Also defined, but not for general use are:
#   SDL_INCLUDE_DIR - The directory that contains SDL.h.
#   SDL_LIBRARY - The location of the SDL library.
#   SDLMAIN_LIBRARY - The location of the SDLmain library.
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
pkg_check_modules(PC_SDL QUIET sdl)

find_path(SDL_INCLUDE_DIR
  NAMES SDL/SDL.h
  HINTS
    ${SDL_ROOT}/include
)

find_library(SDL_LIBRARY
  NAMES SDL
  HINTS
    ${SDL_ROOT}/lib
  PATH_SUFFIXES x86
)

if(NOT SDL_BUILDING_LIBRARY)
  find_library(SDLMAIN_LIBRARY
    NAMES SDLmain
    HINTS
    ${SDL_ROOT}/lib
  PATH_SUFFIXES x86
  )
endif()

if(SDL_INCLUDE_DIR AND EXISTS "${SDL_INCLUDE_DIR}/SDL/SDL_version.h")
  file(STRINGS "${SDL_INCLUDE_DIR}/SDL/SDL_version.h" SDL_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL_MAJOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL_INCLUDE_DIR}/SDL/SDL_version.h" SDL_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL_MINOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL_INCLUDE_DIR}/SDL/SDL_version.h" SDL_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL_PATCHLEVEL[ \t]+[0-9]+$")
  string(REGEX REPLACE "^#define[ \t]+SDL_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL_VERSION_MAJOR "${SDL_VERSION_MAJOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL_VERSION_MINOR "${SDL_VERSION_MINOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDL_VERSION_PATCH "${SDL_VERSION_PATCH_LINE}")
  set(SDL_VERSION_STRING ${SDL_VERSION_MAJOR}.${SDL_VERSION_MINOR}.${SDL_VERSION_PATCH})
  unset(SDL_VERSION_MAJOR_LINE)
  unset(SDL_VERSION_MINOR_LINE)
  unset(SDL_VERSION_PATCH_LINE)
  unset(SDL_VERSION_MAJOR)
  unset(SDL_VERSION_MINOR)
  unset(SDL_VERSION_PATCH)
endif()

set(SDL_INCLUDE_DIRS ${SDL_INCLUDE_DIR} ${SDL_INCLUDE_DIR}/SDL)
set(SDL_LIBRARIES ${SDLMAIN_LIBRARY} ${SDL_LIBRARY})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(SDL
                                  REQUIRED_VARS SDL_INCLUDE_DIR SDL_LIBRARY
                                  VERSION_VAR SDL_VERSION_STRING)

mark_as_advanced(SDL_INCLUDE_DIR SDL_LIBRARY)
