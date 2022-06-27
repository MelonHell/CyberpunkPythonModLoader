cmake_minimum_required(VERSION 3.21)

# Change the project name here!
project(CyberpunkPythonModLoader LANGUAGES CXX)

# Properties.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Variables.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/libs")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

foreach(configuration ${CMAKE_CONFIGURATION_TYPES})
    string(TOLOWER ${configuration} configuration_lower)
    string(TOUPPER ${configuration} configuration_upper)

    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${configuration_upper} "${CMAKE_BINARY_DIR}/${configuration_lower}/libs")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${configuration_upper} "${CMAKE_BINARY_DIR}/${configuration_lower}/bin")
endforeach()

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# Python
#find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
#include_directories(${Python3_INCLUDE_DIRS})
#link_libraries(${Python3_LIBRARIES})

# Pybind11
include_directories(libs/pybind11/include)
add_subdirectory(libs/pybind11)
link_libraries(pybind11::module)

# Minhook
include_directories(libs/MinHook/include)
add_subdirectory(libs/MinHook)
link_libraries(minhook)

# Red4Ext
include_directories("libs/RED4ext.SDK/include")
#file(GLOB_RECURSE RED4EXT_HEADERS libs/RED4ext.SDK/include/RED4ext/*.hpp)
#file(GLOB_RECURSE RED4EXT_SOURCES libs/RED4ext.SDK/src/*.cpp)
#add_library(RED4ext.SDK STATIC "")
#target_include_directories(RED4ext.SDK PRIVATE libs/RED4ext.SDK/include)
#target_include_directories(RED4ext.SDK INTERFACE libs/RED4ext.SDK/include)
#target_sources(RED4ext.SDK PRIVATE ${RED4EXT_HEADERS} ${RED4EXT_SOURCES})
#link_libraries(RED4ext.SDK)

#add_compile_definitions(RED4EXT_STATIC_LIB)
add_compile_definitions(WIN32_LEAN_AND_MEAN)

add_subdirectory(src)
