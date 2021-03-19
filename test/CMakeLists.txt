cmake_minimum_required(VERSION 3.10)

project(ElysianLuaTest VERSION ${ELYSIAN_LUA_VERSION} DESCRIPTION "ElysianLua Tests" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTO_RCC ON)
set(CMAKE_AUTOIC ON)

find_package(Qt6 COMPONENTS Core Test REQUIRED)

add_executable(ElysianLuaTests
    elysian_qtest.hpp
    test_base.hpp
    test_stack.hpp
    test_globals_table.hpp
    test_table.hpp
    test_table_proxy.hpp
    test_function.hpp
    test_buffer.hpp
    test_operators.hpp
    test_thread_view.hpp
    test_variant.hpp
    test_stateless_reference.hpp
    test_reference.hpp
    main.cpp)

target_link_libraries(ElysianLuaTests
    libLua
    libElysianLua
    Qt6::Core
    Qt6::Test)

add_test(NAME ElysianLuaTests COMMAND ElysianLuaTests)
