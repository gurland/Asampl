cmake_minimum_required(VERSION 3.2)

project(asampl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wno-sign-compare -g")

if(UNIX)
    find_package(OpenCV REQUIRED)
    include_directories(${OpenCV_INCLUDE_DIRS})
else()
    set(OPENCV_DIR ${PROJECT_SOURCE_DIR}/opencv)
    find_package(OPENCV REQUIRED NAMES OpenCV)
endif()

add_subdirectory(deps/dynalo)

add_library(asampl-interface INTERFACE)
target_include_directories(asampl-interface INTERFACE include)

add_subdirectory(src)

# executable
add_executable(asampl Asampl.cpp)

target_link_libraries(asampl asampl-lib)

#handlers
file(GLOB HANDLERS handlers/*)
foreach(HANDLER_DIR ${HANDLERS})
    add_subdirectory(${HANDLER_DIR})
endforeach()
