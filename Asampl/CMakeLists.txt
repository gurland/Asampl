cmake_minimum_required(VERSION 3.2)

project(asampl)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Wall -Wno-sign-compare")

if(UNIX)
    include(FindPkgConfig)
    pkg_search_module(OpenCV REQUIRED opencv)
else()
    set(OPENCV_DIR ${PROJECT_SOURCE_DIR}/opencv)
    find_package(OPENCV REQUIRED NAMES OpenCV)
endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/src)
