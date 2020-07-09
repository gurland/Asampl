file(GLOB ASAMPL_SOURCES *.cpp)

set(COMPILER_EXECUTABLE asampl)

add_executable(
    ${COMPILER_EXECUTABLE}
    ${ASAMPL_SOURCES})

target_include_directories(
    ${COMPILER_EXECUTABLE}
    PRIVATE ${PROJECT_SOURCE_DIR}/include
    PRIVATE ${OpenCV_INCLUDE_DIRS})

target_link_libraries(
    ${COMPILER_EXECUTABLE}
    PRIVATE ${OpenCV_LIBRARIES})
