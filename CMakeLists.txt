cmake_minimum_required(VERSION 3.2)

project(asampl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wno-sign-compare")

if(UNIX)
    include(FindPkgConfig)
    pkg_search_module(OpenCV REQUIRED opencv)
else()
    set(OPENCV_DIR ${PROJECT_SOURCE_DIR}/opencv)
    find_package(OPENCV REQUIRED NAMES OpenCV)
endif()

add_subdirectory(deps/dynalo)

add_subdirectory(src)

# executable
add_executable(asampl Asampl.cpp)

target_link_libraries(asampl asampl-lib)
