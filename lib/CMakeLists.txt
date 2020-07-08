cmake_minimum_required(VERSION 3.10)

set(ELYSIAN_LUA_VERSION_MAJOR 0)
set(ELYSIAN_LUA_VERSION_MINOR 0)
set(ELYSIAN_LUA_VERSION_PATCH 1)
set(ELYSIAN_LUA_VERSION
  ${ELYSIAN_LUA_VERSION_MAJOR}.${ELYSIAN_LUA_VERSION_MINOR}.${ELYSIAN_LUA_VERSION_PATCH})

project(ElysianLua VERSION ${ELYSIAN_LUA_VERSION} DESCRIPTION "Elysian Shadows Lua Runtime Framework" LANGUAGES CXX)

set(ELYSIAN_LUA_STACK_GUARD_OPTIONS
    ELYSIAN_LUA_STACK_GUARD_LUA_ERROR
    ELYSIAN_LUA_STACK_GUARD_LUA_WARNING
    ELYSIAN_LUA_STACK_GUARD_ASSERT
    ELYSIAN_LUA_STACK_GUARD_DISABLED)

set(ELYSIAN_LUA_STACK_GUARD_OPTION ELYSIAN_LUA_STACK_GUARD_ASSERT CACHE STRING "Select an option")

set_property(CACHE ELYSIAN_LUA_STACK_GUARD_OPTION
    PROPERTY STRINGS ${ELYSIAN_LUA_STACK_GUARD_OPTIONS})

# option(ELYSIAN_LUA_DEBUG "Enable Debug" ON)
option(ELYSIAN_LUA_USE_STD_STRING "Compile with std::string Support" OFF)
# option(ELYSIAN_LUA_PUSH_NULL_AS_NIL "Push any pointer type with a value of NULL as nil" ON)
option(ELYSIAN_LUA_ENABLE_CONCEPTS "Use C++20 Concepts" ON)

if (NOT ELYSIAN_LUA_STACK_GUARD_OPTION IN_LIST ELYSIAN_LUA_STACK_GUARD_OPTIONS)
  message(FATAL_ERROR "Specify ELYSIAN_LUA_STACK_GUARD_OPTION. Must be one of ${ELYSIAN_LUA_STACK_GUARD_OPTIONS}")
endif()


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ELYSIAN_LUA_SOURCES
    source/elysian_lua_vm.cpp
    source/elysian_lua_thread_view.cpp
    source/elysian_lua_thread.cpp
    source/elysian_lua_reference.cpp
    source/elysian_lua_buffer.cpp
    source/elysian_lua_protected_block.cpp
    source/elysian_lua_stack_monitor.cpp)

set(ELYSIAN_LUA_INCLUDES
    api/ElysianLua/elysian_lua_forward_declarations.hpp
    api/ElysianLua/elysian_lua_vm.hpp
    api/ElysianLua/elysian_lua_alloc_stats.hpp
    api/ElysianLua/elysian_lua_thread_view.hpp
    api/ElysianLua/elysian_lua_thread.hpp
    api/ElysianLua/elysian_lua_object.hpp
    api/ElysianLua/elysian_lua_table_accessible.hpp
    api/ElysianLua/elysian_lua_table.hpp
    api/ElysianLua/elysian_lua_callable.hpp
    api/ElysianLua/elysian_lua_function.hpp
    api/ElysianLua/elysian_lua_reference.hpp
    api/ElysianLua/elysian_lua_proxy.hpp
    api/ElysianLua/elysian_lua_table_proxy.hpp
    api/ElysianLua/elysian_lua_function_result.hpp
    api/ElysianLua/elysian_lua_operator_proxy.hpp
    api/ElysianLua/elysian_lua_stack_monitor.hpp
    api/ElysianLua/elysian_lua_protected_block.hpp
    api/ElysianLua/elysian_lua_traits.hpp
    api/ElysianLua/elysian_lua_stack.hpp
    api/ElysianLua/elysian_lua_tuple.hpp
    api/ElysianLua/elysian_lua_thread_view_base.hpp
    api/ElysianLua/elysian_lua_buffer.hpp
    api/ElysianLua/elysian_lua_metatable.hpp
    api/ElysianLua/elysian_lua_stack_frame.hpp
    api/ElysianLua/elysian_lua_variant.hpp)

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

target_compile_definitions(libElysianLua
    PUBLIC
        ELYSIAN_LUA_STACK_GUARD_OPTION=${ELYSIAN_LUA_STACK_GUARD_OPTION}
)

if(ELYSIAN_LUA_USE_STD_STRING)
    target_compile_definitions(libElysianLua
        PRIVATE
            ELYSIAN_LUA_USE_STD_STRING
    )
endif()

if(ELYSIAN_LUA_ENABLE_CONCEPTS)
    target_compile_definitions(libElysianLua
        PRIVATE
            ELYSIAN_LUA_ENABLE_CONCEPTS
    )
endif()


target_link_libraries(libElysianLua
    libLua)

