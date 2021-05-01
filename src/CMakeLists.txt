if(UNIX)
    find_package(OpenCV REQUIRED)
    include_directories(${OpenCV_INCLUDE_DIRS})
else()
    set(OPENCV_DIR ${PROJECT_SOURCE_DIR}/opencv)
    find_package(OPENCV REQUIRED NAMES OpenCV)
endif()

file(GLOB ASAMPL_SOURCES *.cpp **/*.cpp)

add_library(asampl-lib STATIC
    ${ASAMPL_SOURCES})

target_link_libraries(
    asampl-lib

    PUBLIC asampl-ffi
    PUBLIC asampl-interface
    PUBLIC dynalo
    PUBLIC ${OpenCV_LIBRARIES})

if(${ENABLE_PYTHON_SUPPORT})
    find_package(Boost REQUIRED COMPONENTS python numpy)
    find_package(Python COMPONENTS Interpreter Development NumPy)

    target_compile_definitions(asampl-lib PUBLIC ASAMPL_ENABLE_PYTHON)
    target_include_directories(asampl-lib
        PRIVATE ${Python_INCLUDE_DIRS})
    target_link_libraries(asampl-lib
        PRIVATE Boost::python
        PRIVATE Boost::numpy
        PRIVATE ${Python_LIBRARIES})
endif()
