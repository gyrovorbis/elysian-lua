cmake_minimum_required(VERSION 3.10)

project(ElysianLuaTest VERSION ${ELYSIAN_LUA_VERSION} DESCRIPTION "ElysianLua Tests" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTO_RCC ON)
set(CMAKE_AUTOIC ON)

find_package(Qt5 COMPONENTS Core Test REQUIRED)

add_executable(ElysianLuaTests
#    main.cpp
    test_thread_view.cpp)

target_link_libraries(ElysianLuaTests
    libLua
    libElysianLua
    Qt5::Core
    Qt5::Test)

