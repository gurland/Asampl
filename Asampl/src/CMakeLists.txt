file(GLOB ASAMPL_SOURCES *.cpp)

add_library(asampl-lib STATIC
    ${ASAMPL_SOURCES})

target_include_directories(
    asampl-lib
    PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(
    asampl-lib

    dynalo
    ${OpenCV_LIBRARIES})
