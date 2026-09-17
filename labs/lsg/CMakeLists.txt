cmake_minimum_required(VERSION 3.22.1)
project(RengineLSGPrototype VERSION 0.9.0 LANGUAGES C CXX)

option(RENGINE_LSG_BUILD_TESTS "Build LSG unit tests" ON)
option(RENGINE_LSG_BUILD_VIEWER "Build Windows Vulkan viewer when Vulkan is available" ON)
option(RENGINE_LSG_BUILD_MESH_TOOL "Build OBJ/GLB/glTF to experimental RMS0 mesh preparation tool" ON)
option(RENGINE_LSG_BUILD_PROFILE_TOOL "Build strict JSON to LSG0 profile compiler" ON)
option(RENGINE_LSG_BUILD_REPORT_TOOL "Build measurable LSG storage-contract reporter" ON)

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
  src/runtime.cpp
  src/rmesh.cpp
  src/camera.cpp
  src/derived_character.cpp
  src/anatomical_field.cpp
  src/extension_contract.cpp)
target_include_directories(rengine_lsg_core PUBLIC include)
target_compile_features(rengine_lsg_core PUBLIC cxx_std_23)
set_target_properties(rengine_lsg_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
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

if((RENGINE_LSG_BUILD_MESH_TOOL OR RENGINE_LSG_BUILD_PROFILE_TOOL) AND NOT ANDROID)
  include(FetchContent)
endif()

if(RENGINE_LSG_BUILD_MESH_TOOL AND NOT ANDROID)
  set(FASTGLTF_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
  set(FASTGLTF_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(FASTGLTF_ENABLE_DOCS OFF CACHE BOOL "" FORCE)
  set(FASTGLTF_COMPILE_AS_CPP20 ON CACHE BOOL "" FORCE)
  FetchContent_Declare(fastgltf GIT_REPOSITORY https://github.com/spnda/fastgltf.git GIT_TAG v0.9.0 GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(fastgltf)
  add_executable(lsg_prepare_mesh tools/lsg_prepare_mesh/main.cpp)
  target_link_libraries(lsg_prepare_mesh PRIVATE rengine_lsg_core fastgltf::fastgltf)
  target_compile_features(lsg_prepare_mesh PRIVATE cxx_std_23)
  add_executable(lsg_extract_obj_group tools/lsg_extract_obj_group/main.cpp)
  target_compile_features(lsg_extract_obj_group PRIVATE cxx_std_23)
  add_executable(lsg_mesh_audit tools/lsg_mesh_audit/main.cpp)
  target_link_libraries(lsg_mesh_audit PRIVATE rengine_lsg_core)
  target_compile_features(lsg_mesh_audit PRIVATE cxx_std_23)
endif()

if(RENGINE_LSG_BUILD_PROFILE_TOOL AND NOT ANDROID)
  set(JSON_BuildTests OFF CACHE INTERNAL "")
  set(JSON_Install OFF CACHE INTERNAL "")
  FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG 55f93686c01528224f448c19128836e7df245f72
    GIT_SHALLOW FALSE)
  FetchContent_MakeAvailable(nlohmann_json)
  add_executable(lsg_compile tools/lsg_compile/main.cpp)
  target_link_libraries(lsg_compile PRIVATE rengine_lsg_core nlohmann_json::nlohmann_json)
  target_compile_features(lsg_compile PRIVATE cxx_std_23)
  if(RENGINE_LSG_BUILD_TESTS)
    add_dependencies(rengine_lsg_tests lsg_compile)
    add_test(NAME lsg_compile_character_0
      COMMAND lsg_compile ${CMAKE_CURRENT_SOURCE_DIR}/assets/profiles/character_0.lsg.json
                          ${CMAKE_CURRENT_BINARY_DIR}/character_0.lsg)
    add_test(NAME lsg_compile_character_1
      COMMAND lsg_compile ${CMAKE_CURRENT_SOURCE_DIR}/assets/profiles/character_1.lsg.json
                          ${CMAKE_CURRENT_BINARY_DIR}/character_1.lsg)
  endif()
endif()

if(RENGINE_LSG_BUILD_REPORT_TOOL AND NOT ANDROID)
  add_executable(lsg_report tools/lsg_report/main.cpp)
  target_link_libraries(lsg_report PRIVATE rengine_lsg_core)
  target_compile_features(lsg_report PRIVATE cxx_std_23)
endif()

if(ANDROID)
  add_library(android_native_app_glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
  target_include_directories(android_native_app_glue PUBLIC ${ANDROID_NDK}/sources/android/native_app_glue)
  set_target_properties(android_native_app_glue PROPERTIES POSITION_INDEPENDENT_CODE ON)
  add_library(rengine_lsg SHARED viewer/android/app/src/main/cpp/native_main.cpp renderer/vulkan_renderer.cpp)
  target_include_directories(rengine_lsg PRIVATE renderer include)
  target_compile_features(rengine_lsg PRIVATE cxx_std_23)
  target_compile_definitions(rengine_lsg PRIVATE VK_USE_PLATFORM_ANDROID_KHR=1)
  target_link_options(rengine_lsg PRIVATE "-uANativeActivity_onCreate")
  target_link_libraries(rengine_lsg PRIVATE rengine_lsg_core android_native_app_glue android log vulkan)
elseif(WIN32 AND RENGINE_LSG_BUILD_VIEWER)
  find_package(Vulkan REQUIRED)
  if(NOT Vulkan_GLSLC_EXECUTABLE)
    message(FATAL_ERROR "Vulkan glslc is required for the LSG Windows viewer")
  endif()
  set(LSG_SHADER_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")
  set(LSG_VERTEX_SPV "${LSG_SHADER_BUILD_DIR}/human.vert.spv")
  set(LSG_FRAGMENT_SPV "${LSG_SHADER_BUILD_DIR}/human.frag.spv")
  add_custom_command(OUTPUT ${LSG_VERTEX_SPV} ${LSG_FRAGMENT_SPV}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${LSG_SHADER_BUILD_DIR}
      COMMAND ${Vulkan_GLSLC_EXECUTABLE} --target-env=vulkan1.2 ${CMAKE_CURRENT_SOURCE_DIR}/shaders/human.vert -o ${LSG_VERTEX_SPV}
      COMMAND ${Vulkan_GLSLC_EXECUTABLE} --target-env=vulkan1.2 ${CMAKE_CURRENT_SOURCE_DIR}/shaders/human.frag -o ${LSG_FRAGMENT_SPV}
      DEPENDS shaders/human.vert shaders/human.frag VERBATIM)
  add_custom_target(rengine_lsg_shaders DEPENDS ${LSG_VERTEX_SPV} ${LSG_FRAGMENT_SPV})
  add_executable(rengine_lsg_viewer viewer/windows/main.cpp renderer/vulkan_renderer.cpp)
  add_dependencies(rengine_lsg_viewer rengine_lsg_shaders)
  target_compile_definitions(rengine_lsg_viewer PRIVATE VK_USE_PLATFORM_WIN32_KHR=1)
  target_include_directories(rengine_lsg_viewer PRIVATE renderer include)
  target_link_libraries(rengine_lsg_viewer PRIVATE rengine_lsg_core Vulkan::Vulkan user32)
  target_compile_features(rengine_lsg_viewer PRIVATE cxx_std_23)
  add_custom_command(TARGET rengine_lsg_viewer POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:rengine_lsg_viewer>/shaders"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LSG_VERTEX_SPV} "$<TARGET_FILE_DIR:rengine_lsg_viewer>/shaders/human.vert.spv"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LSG_FRAGMENT_SPV} "$<TARGET_FILE_DIR:rengine_lsg_viewer>/shaders/human.frag.spv")
elseif(RENGINE_LSG_BUILD_VIEWER)
  message(STATUS "LSG desktop viewer currently targets Win32; non-Windows viewer skipped")
endif()
