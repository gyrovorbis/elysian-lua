cmake_minimum_required(VERSION 3.10)

set(ELYSIAN_LUA_VERSION_MAJOR 0)
set(ELYSIAN_LUA_VERSION_MINOR 0)
set(ELYSIAN_LUA_VERSION_PATCH 1)
set(ELYSIAN_LUA_VERSION
  ${ELYSIAN_LUA_VERSION_MAJOR}.${ELYSIAN_LUA_VERSION_MINOR}.${ELYSIAN_LUA_VERSION_PATCH})

project(ElysianLua VERSION ${ELYSIAN_LUA_VERSION} DESCRIPTION "Elysian Shadows Lua Runtime Framework" LANGUAGES CXX)

option(ELYSIAN_LUA_DEBUG "Enable Debug" ON)
option(ELYSIAN_LUA_USE_STD_STRING "Compile with std::string Support" OFF)
option(ELYSIAN_LUA_PUSH_NULL_AS_NIL "Push any pointer type with a value of NULL as nil" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ELYSIAN_LUA_SOURCES
    source/elysian_lua_vm.cpp
    source/elysian_lua_thread_view.cpp
    source/elysian_lua_thread.cpp
    source/elysian_lua_reference.cpp
    source/elysian_lua_buffer.cpp)

set(ELYSIAN_LUA_INCLUDES
    api/ElysianLua/elysian_lua_forward_declarations.hpp
    api/ElysianLua/elysian_lua_vm.hpp
    api/ElysianLua/elysian_lua_alloc_stats.hpp
    api/ElysianLua/elysian_lua_thread_view.hpp
    api/ElysianLua/elysian_lua_thread.hpp
    api/ElysianLua/elysian_lua_object.hpp
    api/ElysianLua/elysian_lua_table.hpp
    api/ElysianLua/elysian_lua_function.hpp
    api/ElysianLua/elysian_lua_reference.hpp
    api/ElysianLua/elysian_lua_proxy.hpp
    api/ElysianLua/elysian_lua_table_proxy.hpp
    api/ElysianLua/elysian_lua_function_result.hpp
    api/ElysianLua/elysian_lua_operator_proxy.hpp
    api/ElysianLua/elysian_lua_traits.hpp
    api/ElysianLua/elysian_lua_stack.hpp
    api/ElysianLua/elysian_lua_tuple.hpp
    api/ElysianLua/elysian_lua_thread_view_base.hpp
    api/ElysianLua/elysian_lua_buffer.hpp
    api/ElysianLua/elysian_lua_metatable.hpp)

set(ELYSIAN_LUA_CMAKE_PATH "lib/Lua5.4" CACHE STRING "CMake Project Path for Lua VM")
#set_property(ELYSIAN_LUA_CMAKE_PATH PROPERTY VALUE)

add_subdirectory(${ELYSIAN_LUA_CMAKE_PATH})

add_library(libElysianLua STATIC
    ${ELYSIAN_LUA_SOURCES}
    ${ELYSIAN_LUA_INCLUDES})

target_include_directories(libElysianLua
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/api>
        $<INSTALL_INTERFACE:api>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/api
)

target_link_libraries(libElysianLua
    libLua)

