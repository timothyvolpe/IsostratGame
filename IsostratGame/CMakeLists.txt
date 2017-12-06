cmake_minimum_required(VERSION 3.0)
project(IsostratGame)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)
ADD_DEFINITIONS(-DGLEW_STATIC)
ADD_DEFINITIONS(-DSDL_MAIN_HANDLED)

# Version
set(GameConfig_VERSION_MAJOR 0)
set(GameConfig_VERSION_MINOR 17)

# Include
file(GLOB Isostrat_INC "${PROJECT_SOURCE_DIR}/include/*.h" )
file(GLOB_RECURSE Isostrat_INC_SCRIPT "${PROJECT_SOURCE_DIR}/include/script/*.h")
file(GLOB_RECURSE Isostrat_INC_SHADER "${PROJECT_SOURCE_DIR}/include/shader/*.h")
file(GLOB_RECURSE Isostrat_INC_UI "${PROJECT_SOURCE_DIR}/include/ui/*.h")
file(GLOB_RECURSE Isostrat_INC_WORLD "${PROJECT_SOURCE_DIR}/include/world/*.h")
# Source
file(GLOB Isostrat_SRC "${PROJECT_SOURCE_DIR}/source/*.cpp" )
file(GLOB_RECURSE Isostrat_SRC_SCRIPT "${PROJECT_SOURCE_DIR}/source/script/*.cpp")
file(GLOB_RECURSE Isostrat_SRC_SHADER "${PROJECT_SOURCE_DIR}/source/shader/*.cpp")
file(GLOB_RECURSE Isostrat_SRC_UI "${PROJECT_SOURCE_DIR}/source/ui/*.cpp")
file(GLOB_RECURSE Isostrat_SRC_WORLD "${PROJECT_SOURCE_DIR}/source/world/*.cpp")

configure_file(
	"${PROJECT_SOURCE_DIR}/GameConfig.h.in"
    "${PROJECT_BINARY_DIR}/include/GameConfig.h"
	)
	
include_directories("${PROJECT_BINARY_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/include")

source_group("Header Files\\script" FILES ${Isostrat_INC_SCRIPT} )
source_group("Header Files\\shader" FILES ${Isostrat_INC_SHADER} )
source_group("Header Files\\ui" FILES ${Isostrat_INC_UI} )
source_group("Header Files\\world" FILES ${Isostrat_INC_WORLD} )

source_group("Source Files\\script"	FILES ${Isostrat_SRC_SCRIPT} )
source_group("Source Files\\shader"	FILES ${Isostrat_SRC_SHADER})
source_group("Source Files\\ui"		FILES ${Isostrat_SRC_UI})
source_group("Source Files\\world"	FILES ${Isostrat_SRC_WORLD})

# Boost
set( BOOST_INCLUDE_DIR "NOT-FOUND" CACHE PATH "" )
set( BOOST_LIBRARY_DIR "NOT-FOUND" CACHE PATH "" )

if( NOT BOOST_INCLUDE_DIR STREQUAL "NOT-FOUND" )
	include_directories( ${BOOST_INCLUDE_DIR} )
endif( NOT BOOST_INCLUDE_DIR STREQUAL "NOT-FOUND" )
if( NOT BOOST_LIBRARY_DIR STREQUAL "NOT-FOUND" )
	link_directories( ${BOOST_LIBRARY_DIR} )
endif( NOT BOOST_LIBRARY_DIR STREQUAL "NOT-FOUND" )

add_executable(IsostratGame 
	${Isostrat_SRC} ${Isostrat_SRC_SCRIPT} ${Isostrat_SRC_SHADER} ${Isostrat_SRC_UI} ${Isostrat_SRC_WORLD} 
	${Isostrat_INC} ${Isostrat_INC_SCRIPT} ${Isostrat_INC_SHADER} ${Isostrat_INC_UI} ${Isostrat_INC_WORLD}
)

# Output

if ( MSVC )
    set_target_properties( IsostratGame PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../build )
    set_target_properties( IsostratGame PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/../build )
    set_target_properties( IsostratGame PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/../build )
	
	set_target_properties( IsostratGame PROPERTIES DEBUG_POSTFIX DEBUG )
	set_target_properties( IsostratGame PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/../build )
endif ( MSVC )

# Third Party Libraries

# OpenGL 

find_package( OpenGL REQUIRED )
if ( OPENGL_FOUND )
    include_directories( ${OPENGL_INCLUDE_DIR} )
    target_link_libraries( IsostratGame ${OPENGL_LIBRARIES} )
endif( OPENGL_FOUND )

# SDL2 2.0.7
find_package( SDL2 REQUIRED )
if ( SDL2_FOUND )
    include_directories( ${SDL2_INCLUDE_DIR} )
    target_link_libraries( IsostratGame ${SDL2_LIBRARY} )
endif( SDL2_FOUND )

# Lua 5.3.4
set( LUA_INCLUDE_DIR "NOT-FOUND" CACHE PATH "" )
set( LUA_LIBRARY_DIR "NOT-FOUND" CACHE FILEPATH "" )

if( NOT LUA_INCLUDE_DIR STREQUAL "NOT-FOUND" )
	include_directories( ${LUA_INCLUDE_DIR} )
endif( NOT LUA_INCLUDE_DIR STREQUAL "NOT-FOUND" )
if( NOT LUA_LIBRARY_DIR STREQUAL "NOT-FOUND" )
	target_link_libraries( IsostratGame ${LUA_LIBRARY_DIR} )
endif( NOT LUA_LIBRARY_DIR STREQUAL "NOT-FOUND" )

# GLM 0.9.9-a2
set( GLM_INCLUDE_DIR "NOT-FOUND" CACHE PATH "" )

if( NOT GLM_INCLUDE_DIR STREQUAL "NOT-FOUND" )
	include_directories( ${GLM_INCLUDE_DIR} )
endif( NOT GLM_INCLUDE_DIR STREQUAL "NOT-FOUND" )

# GLEW 2.1.0
set( GLEW_INCLUDE_DIR "NOT-FOUND" CACHE PATH "" )
set( GLEW_LIBRARY_DIR "NOT-FOUND" CACHE FILEPATH "" )

if( NOT GLEW_INCLUDE_DIR STREQUAL "NOT-FOUND" )
	include_directories( ${GLEW_INCLUDE_DIR} )
endif(  NOT GLEW_INCLUDE_DIR STREQUAL "NOT-FOUND" )
if( NOT GLEW_LIBRARY_DIR STREQUAL "NOT-FOUND" )
	target_link_libraries( IsostratGame ${GLEW_LIBRARY_DIR} )
endif( NOT GLEW_LIBRARY_DIR STREQUAL "NOT-FOUND" )

# FreeType
set( FREETYPE_INCLUDE_DIR "NOT-FOUND" CACHE PATH "" )
set( FREETYPE_LIBRARY_DIR "NOT-FOUND" CACHE FILEPATH "" )

if( NOT FREETYPE_INCLUDE_DIR STREQUAL "NOT-FOUND" )
	include_directories( ${FREETYPE_INCLUDE_DIR} )
endif( NOT FREETYPE_INCLUDE_DIR STREQUAL "NOT-FOUND" )
if( NOT FREETYPE_LIBRARY_DIR STREQUAL "NOT-FOUND" )
	target_link_libraries( IsostratGame ${FREETYPE_LIBRARY_DIR} )
endif( NOT FREETYPE_LIBRARY_DIR STREQUAL "NOT-FOUND" )