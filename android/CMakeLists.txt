# Android entry point for the DMC Rengine runtime host layer.
#
# Gradle points `externalNativeBuild` here rather than at the repository root
# so the desktop CLI and the CTest suite stay out of the NDK build. Only the
# core library and the runtime shared object are produced.

cmake_minimum_required(VERSION 3.25)

project(DMCRengineAndroid LANGUAGES CXX)

set(DMC_RENGINE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(DMC_RENGINE_BUILD_CLI OFF CACHE BOOL "" FORCE)
set(DMC_RENGINE_BUILD_RUNTIME ON CACHE BOOL "" FORCE)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/.." dmc-rengine)
