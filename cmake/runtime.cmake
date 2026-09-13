# DMC Rengine runtime host layer.
#
# The runtime is kept in its own target so it can move ahead of the core
# library's C++20 baseline (Constitution, Article VI) without forcing every
# existing translation unit onto a newer standard. Consumers that only need
# read-side format authority keep linking DMCRengine::Core alone.

if(TARGET dmc_rengine_runtime)
    return()
endif()

get_filename_component(DMC_RENGINE_RUNTIME_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

set(DMC_RENGINE_RUNTIME_CXX_STANDARD "23"
    CACHE STRING "C++ standard for the runtime host layer: 20, 23 or 26")
set_property(CACHE DMC_RENGINE_RUNTIME_CXX_STANDARD PROPERTY STRINGS 20 23 26)

set(_dmc_runtime_requested "${DMC_RENGINE_RUNTIME_CXX_STANDARD}")
if(NOT _dmc_runtime_requested MATCHES "^(20|23|26)$")
    message(WARNING
        "DMC_RENGINE_RUNTIME_CXX_STANDARD='${_dmc_runtime_requested}' is not one of 20/23/26; using 23.")
    set(_dmc_runtime_requested "23")
endif()

# Ask for the requested standard, then step down until the toolchain actually
# advertises one. A missing C++26 compiler degrades to C++23 rather than
# failing the build; the feature-detection macros in status.hpp and
# application.hpp pick matching library facilities at compile time.
if(_dmc_runtime_requested STREQUAL "26")
    set(_dmc_runtime_candidates 26 23 20)
elseif(_dmc_runtime_requested STREQUAL "23")
    set(_dmc_runtime_candidates 23 20)
else()
    set(_dmc_runtime_candidates 20)
endif()

set(_dmc_runtime_standard "")
foreach(_candidate IN LISTS _dmc_runtime_candidates)
    if("cxx_std_${_candidate}" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
        set(_dmc_runtime_standard "${_candidate}")
        break()
    endif()
endforeach()

if(_dmc_runtime_standard STREQUAL "")
    message(FATAL_ERROR
        "The C++ compiler advertises no usable standard from '${_dmc_runtime_candidates}'. "
        "DMC Rengine runtime requires at least C++20.")
endif()

if(NOT _dmc_runtime_standard STREQUAL _dmc_runtime_requested)
    message(STATUS
        "DMC Rengine runtime: C++${_dmc_runtime_requested} unavailable, building as C++${_dmc_runtime_standard}.")
endif()

set(DMC_RENGINE_RUNTIME_EFFECTIVE_CXX_STANDARD "${_dmc_runtime_standard}"
    CACHE INTERNAL "Resolved C++ standard for the runtime host layer")

file(GLOB_RECURSE DMC_RENGINE_RUNTIME_SOURCES CONFIGURE_DEPENDS
    "${DMC_RENGINE_RUNTIME_ROOT}/src/runtime/*.cpp")

if(NOT ANDROID)
    list(FILTER DMC_RENGINE_RUNTIME_SOURCES EXCLUDE REGEX "/src/runtime/jni/")
    list(FILTER DMC_RENGINE_RUNTIME_SOURCES EXCLUDE REGEX "/src/runtime/platform/android_")
endif()

if(ANDROID)
    # Gradle packages libdmc_rengine_runtime.so; the JNI entry points
    # only exist in a shared object.
    add_library(dmc_rengine_runtime SHARED ${DMC_RENGINE_RUNTIME_SOURCES})
else()
    add_library(dmc_rengine_runtime ${DMC_RENGINE_RUNTIME_SOURCES})
endif()
add_library(DMCRengine::Runtime ALIAS dmc_rengine_runtime)

target_include_directories(dmc_rengine_runtime
    PUBLIC
        $<BUILD_INTERFACE:${DMC_RENGINE_RUNTIME_ROOT}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(dmc_rengine_runtime PUBLIC DMCRengine::Core)
target_compile_features(dmc_rengine_runtime PUBLIC cxx_std_${_dmc_runtime_standard})

if(COMMAND dmc_rengine_enable_warnings)
    dmc_rengine_enable_warnings(dmc_rengine_runtime)
endif()

if(ANDROID)
    find_library(DMC_RENGINE_ANDROID_LOG_LIB log)
    target_link_libraries(dmc_rengine_runtime PUBLIC ${DMC_RENGINE_ANDROID_LOG_LIB} android)
endif()

if(NOT ANDROID)
    add_executable(dmc-rengine-runtime-probe
        "${DMC_RENGINE_RUNTIME_ROOT}/src/runtime_probe/main.cpp")
    target_link_libraries(dmc-rengine-runtime-probe PRIVATE DMCRengine::Runtime)
    if(COMMAND dmc_rengine_enable_warnings)
        dmc_rengine_enable_warnings(dmc-rengine-runtime-probe)
    endif()
endif()

message(STATUS "DMC Rengine runtime: C++${_dmc_runtime_standard}, "
               "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

unset(_dmc_runtime_requested)
unset(_dmc_runtime_candidates)
unset(_dmc_runtime_standard)
unset(_candidate)
