cmake_minimum_required (VERSION 2.8)

project (retrofe)

set(RETROFE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(RETROFE_THIRD_PARTY_DIR "${RETROFE_DIR}/ThirdParty")
list(APPEND CMAKE_MODULE_PATH "${RETROFE_DIR}/CMake")

##############################################################
# Setup some variables to help find external libraries
##############################################################

set(SQLITE3_ROOT "${RETROFE_THIRD_PARTY_DIR}/sqlite3")
set(RAPIDXML_ROOT "${RETROFE_THIRD_PARTY_DIR}/rapidxml-1.13")
set(LUA_ROOT "${RETROFE_THIRD_PARTY_DIR}/lua-5.3.0/src")

if(WIN32)
if(MSVC)
	set(SDL2_ROOT "${RETROFE_THIRD_PARTY_DIR}/SDL2-2.0.3")
	set(SDL2_IMAGE_ROOT "${RETROFE_THIRD_PARTY_DIR}/SDL2_image-2.0.0")
	set(SDL2_MIXER_ROOT "${RETROFE_THIRD_PARTY_DIR}/SDL2_mixer-2.0.0")
	set(SDL2_TTF_ROOT "${RETROFE_THIRD_PARTY_DIR}/SDL2_ttf-2.0.12")
elseif(MINGW)
	set(SDL2_ROOT "${RETROFE_THIRD_PARTY_DIR}/SDL2-2.0.3-mingw/i686-w64-mingw32")
	set(SDL2_IMAGE_ROOT "${RETROFE_THIRD_PARTY_DIR}/SDL2_image-2.0.0-mingw/i686-w64-mingw32")
	set(SDL2_MIXER_ROOT "${RETROFE_THIRD_PARTY_DIR}/SDL2_mixer-2.0.0-mingw/i686-w64-mingw32")
	set(SDL2_TTF_ROOT "${RETROFE_THIRD_PARTY_DIR}/SDL2_ttf-2.0.12-mingw/i686-w64-mingw32")
endif()

	set(ZLIB_ROOT "${RETROFE_THIRD_PARTY_DIR}/zlib128-dll")

	set(GSTREAMER_ROOT "C:/gstreamer/1.0/x86" CACHE STRING "location of where your gstreamer include and lib folders reside")
	set(GLIB2_ROOT "${GSTREAMER_ROOT}")
  
if(MSVC)  
	set(DIRENT_INCLUDE_DIR "${RETROFE_THIRD_PARTY_DIR}/dirent-1.20.1/include")
endif()
endif()

if(WIN32)
	find_package(Glib2 REQUIRED)
	find_package(GStreamer REQUIRED)
	find_package(SDL2 REQUIRED)
	find_package(SDL2_image REQUIRED)
	find_package(SDL2_mixer REQUIRED)
	find_package(SDL2_ttf REQUIRED)
	find_package(ZLIB REQUIRED)
else()
	include(FindPkgConfig)
	pkg_search_module(SDL2 REQUIRED sdl2)
	pkg_search_module(SDL2_IMAGE REQUIRED SDL2_image)
	pkg_search_module(SDL2_MIXER REQUIRED SDL2_mixer)
	pkg_search_module(SDL2_TTF REQUIRED SDL2_ttf)
	pkg_search_module(ZLIB REQUIRED zlib)
	pkg_search_module(GSTREAMER REQUIRED gstreamer-1.0 gstbase-1.0)
	pkg_search_module(Glib2 REQUIRED glib-2.0 gobject-2.0 gthread-2.0 gmodule-2.0)
	find_package(Threads REQUIRED)
endif()

set(RETROFE_INCLUDE_DIRS
	"${GLIB2_INCLUDE_DIRS}"
	"${GSTREAMER_INCLUDE_DIRS}"
	"${SDL2_INCLUDE_DIRS}"
	"${SDL2_IMAGE_INCLUDE_DIRS}"
	"${SDL2_MIXER_INCLUDE_DIRS}"
	"${SDL2_TTF_INCLUDE_DIRS}"
	"${ZLIB_INCLUDE_DIRS}"
	"${SQLITE3_ROOT}"
	"${RAPIDXML_ROOT}"
  "${LUA_ROOT}"
  
)

if(MSVC)
LIST(APPEND RETROFE_INCLUDE_DIRS 	"${DIRENT_INCLUDE_DIR}")
endif()

set(RETROFE_LIBRARIES
	${GLIB2_LIBRARIES}
	${GSTREAMER_LIBRARIES}
	${SDL2_LIBRARIES}
	${SDL2_IMAGE_LIBRARIES}
	${SDL2_MIXER_LIBRARIES}
	${SDL2_TTF_LIBRARIES}
	${ZLIB_LIBRARIES}
  LUA
)

if(NOT WIN32)
	LIST(APPEND RETROFE_LIBRARIES ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
endif()

set(RETROFE_HEADERS
	"${RETROFE_DIR}/Source/Database/Configuration.h"
	"${RETROFE_DIR}/Source/Graphics/Component/Image.h"
	"${RETROFE_DIR}/Source/Graphics/Component/Component.h"
	"${RETROFE_DIR}/Source/Graphics/Component/ComponentData.h"
	"${RETROFE_DIR}/Source/Graphics/Component/ComponentFactory.h"
	"${RETROFE_DIR}/Source/Lua/Lua.h"
	"${RETROFE_DIR}/Source/Lua/LuaDisplay.h"
	"${RETROFE_DIR}/Source/Lua/LuaEvent.h"
	"${RETROFE_DIR}/Source/Lua/LuaImage.h"
	"${RETROFE_DIR}/Source/Lua/LuaLog.h"
	"${RETROFE_DIR}/Source/RetroFE.h"
	"${RETROFE_DIR}/Source/StateMachine.h"
	"${RETROFE_DIR}/Source/SDL.h"
	"${RETROFE_DIR}/Source/Version.h"
)

set(RETROFE_SOURCES
	"${RETROFE_DIR}/Source/Database/Configuration.cpp"
	"${RETROFE_DIR}/Source/Graphics/Component/Image.cpp"
	"${RETROFE_DIR}/Source/Graphics/Component/Component.cpp"
	"${RETROFE_DIR}/Source/Graphics/Component/ComponentFactory.cpp"
	"${RETROFE_DIR}/Source/Utility/Log.cpp"
	"${RETROFE_DIR}/Source/Utility/Utils.cpp"
	"${RETROFE_DIR}/Source/Lua/Lua.cpp"
	"${RETROFE_DIR}/Source/Lua/LuaDisplay.cpp"
	"${RETROFE_DIR}/Source/Lua/LuaImage.cpp"
	"${RETROFE_DIR}/Source/Lua/LuaEvent.cpp"
	"${RETROFE_DIR}/Source/Lua/LuaLog.cpp"
	"${RETROFE_DIR}/Source/Main.cpp"
	"${RETROFE_DIR}/Source/RetroFE.cpp"
	"${RETROFE_DIR}/Source/StateMachine.cpp"
	"${RETROFE_DIR}/Source/SDL.cpp"
	"${RETROFE_DIR}/Source/Version.cpp"
)


set(EXECUTABLE_OUTPUT_PATH "${RETROFE_DIR}/Build" CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH "${RETROFE_DIR}/Build" CACHE PATH "Build directory" FORCE)


include_directories(${RETROFE_INCLUDE_DIRS})
add_executable(retrofe  ${RETROFE_SOURCES} ${RETROFE_HEADERS})
target_link_libraries(retrofe ${RETROFE_LIBRARIES})
set_target_properties(retrofe PROPERTIES LINKER_LANGUAGE CXX)
if(MINGW)
  set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -lmingw32 -mwindows")
endif()

file(GLOB LUA_SRC ${LUA_ROOT}/*.c)
add_library( LUA STATIC ${LUA_SRC})

add_definitions(-DRETROFE_VERSION_MAJOR=${VERSION_MAJOR})
add_definitions(-DRETROFE_VERSION_MINOR=${VERSION_MINOR})
add_definitions(-DRETROFE_VERSION_BUILD=${VERSION_BUILD})


if(MSVC)
	set(CMAKE_DEBUG_POSTFIX "d")
	add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
	add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /WX /MD")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP /WX /MD")

	set_target_properties(retrofe PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
  set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()
