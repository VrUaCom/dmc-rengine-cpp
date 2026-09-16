cmake_minimum_required(VERSION 3.22.1)
project(RengineLSGPrototype VERSION 0.1.0 LANGUAGES C CXX)

option(RENGINE_LSG_BUILD_TESTS "Build LSG unit tests" ON)
option(RENGINE_LSG_BUILD_VIEWER "Build desktop Vulkan viewer when Vulkan is available" ON)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(rengine_lsg_core STATIC
    src/genome.cpp
    src/detail_scheduler.cpp
    src/physiology.cpp
    src/surface.cpp
    src/runtime.cpp)

target_include_directories(rengine_lsg_core PUBLIC include)
target_compile_features(rengine_lsg_core PUBLIC cxx_std_23)
if(MSVC)
  target_compile_options(rengine_lsg_core PRIVATE /W4 /permissive-)
else()
  target_compile_options(rengine_lsg_core PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()

if(RENGINE_LSG_BUILD_TESTS AND NOT ANDROID)
  enable_testing()
  add_executable(rengine_lsg_tests tests/lsg_tests.cpp)
  target_link_libraries(rengine_lsg_tests PRIVATE rengine_lsg_core)
  add_test(NAME rengine_lsg_tests COMMAND rengine_lsg_tests)
endif()

if(ANDROID)
  add_library(android_native_app_glue STATIC
      ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
  target_include_directories(android_native_app_glue PUBLIC
      ${ANDROID_NDK}/sources/android/native_app_glue)

  add_library(rengine_lsg SHARED
      viewer/android/app/src/main/cpp/native_main.cpp
      renderer/vulkan_renderer.cpp)
  target_include_directories(rengine_lsg PRIVATE renderer include)
  target_compile_features(rengine_lsg PRIVATE cxx_std_23)
  target_compile_definitions(rengine_lsg PRIVATE VK_USE_PLATFORM_ANDROID_KHR=1)
  target_link_libraries(rengine_lsg PRIVATE
      rengine_lsg_core android_native_app_glue android log vulkan)
else()
  if(RENGINE_LSG_BUILD_VIEWER)
    find_package(Vulkan QUIET)
    if(Vulkan_FOUND)
      add_executable(rengine_lsg_viewer
          viewer/windows/main.cpp
          renderer/vulkan_renderer.cpp)
      target_include_directories(rengine_lsg_viewer PRIVATE renderer include)
      target_link_libraries(rengine_lsg_viewer PRIVATE rengine_lsg_core Vulkan::Vulkan)
      target_compile_features(rengine_lsg_viewer PRIVATE cxx_std_23)
      if(WIN32)
        target_compile_definitions(rengine_lsg_viewer PRIVATE VK_USE_PLATFORM_WIN32_KHR=1)
      endif()
    else()
      message(STATUS "LSG desktop viewer skipped: Vulkan SDK/headers not found")
    endif()
  endif()
endif()
