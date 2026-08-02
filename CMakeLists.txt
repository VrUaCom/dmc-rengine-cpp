cmake_minimum_required(VERSION 3.25)

project(DMCRengine VERSION 0.1.0 LANGUAGES CXX)

option(DMC_RENGINE_BUILD_TESTS "Build DMC Rengine tests" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(dmc_rengine_core
    src/core/version.cpp
    src/gdspaces/resource_id.cpp
)

add_library(DMCRengine::Core ALIAS dmc_rengine_core)

target_include_directories(dmc_rengine_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(dmc_rengine_core PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(dmc_rengine_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(dmc_rengine_core PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()

add_executable(dmc-rengine src/cli/main.cpp)
target_link_libraries(dmc-rengine PRIVATE DMCRengine::Core)

include(CTest)
if(BUILD_TESTING AND DMC_RENGINE_BUILD_TESTS)
    add_executable(dmc_rengine_tests tests/core_tests.cpp)
    target_link_libraries(dmc_rengine_tests PRIVATE DMCRengine::Core)
    add_test(NAME dmc_rengine_core_tests COMMAND dmc_rengine_tests)
endif()
