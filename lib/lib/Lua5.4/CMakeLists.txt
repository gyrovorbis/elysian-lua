cmake_minimum_required(VERSION 3.10)

option(LUA_USE_APICHECK "Enable Lua API argument validation" OFF)

project(Lua5_4 VERSION 5.4 DESCRIPTION "Lua 5.4 VM" LANGUAGES C)

set(LUA_SOURCES
	source/lapi.c 
    source/lauxlib.c 
    source/lbaselib.c 
    source/lcode.c 
    source/lcorolib.c 
    source/lctype.c 
    source/ldblib.c 
    source/ldebug.c 
    source/ldo.c 
    source/ldump.c 
    source/lfunc.c 
    source/lgc.c 
    source/linit.c 
    source/liolib.c 
    source/llex.c 
    source/lmathlib.c 
    source/lmem.c 
    source/loadlib.c 
    source/lobject.c 
    source/lopcodes.c 
    source/loslib.c 
    source/lparser.c 
    source/lstate.c 
    source/lstring.c 
    source/lstrlib.c 
    source/ltable.c 
    source/ltablib.c 
    source/ltm.c 
    source/lundump.c 
    source/lutf8lib.c 
    source/lvm.c 
    source/lzio.c)

#set(LUA_INCLUDES
#	api/ElysianLua/elysian_lua_vm.hpp)


#option(ELYSIAN_LUA_LUA_CMAKE_PATH, "Path to Lua Interpreter CMake Project Root" CACHE STRING "lib/Lua5.4")

add_library(libLua STATIC
    ${LUA_SOURCES}
    ${LUA_INCLUDES})

#[[
set_target_properties(libgyro PROPERTIES
    ${LIBGYRO_TARGET_PROPERTIES})
]]#

target_include_directories(libLua
 	PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/lua
)

if(LUA_USE_APICHECK)
    target_compile_definitions(libLua
        PUBLIC
            LUA_USE_APICHECK
    )
endif()
