cmake_minimum_required(VERSION 3.10)

set(ELYSIAN_LUA_VERSION_MAJOR 0)
set(ELYSIAN_LUA_VERSION_MINOR 0)
set(ELYSIAN_LUA_VERSION_PATCH 1)
set(ELYSIAN_LUA_VERSION
  ${ELYSIAN_LUA_VERSION_MAJOR}.${ELYSIAN_LUA_VERSION_MINOR}.${ELYSIAN_LUA_VERSION_PATCH})

project(ElysianLua VERSION ${ELYSIAN_LUA_VERSION} DESCRIPTION "Elysian Shadows Lua Runtime Framework" LANGUAGES CXX)

enable_testing()

add_subdirectory(lib)
add_subdirectory(test)
