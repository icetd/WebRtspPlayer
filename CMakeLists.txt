project (decoder)
cmake_minimum_required(VERSION 3.5.1)

set (SOURCE_DIR 	    "${CMAKE_SOURCE_DIR}/src")
set (INCLUDE_DIR 	    "${CMAKE_SOURCE_DIR}/src")
set (FFMPEG_INC         "${CMAKE_SOURCE_DIR}/thirdparty/ffmpeg-emcc/include")
set (FFMPEG_LIB         "${CMAKE_SOURCE_DIR}/thirdparty/ffmpeg-emcc/lib")

link_directories(
    ${FFMPEG_LIB}
)
include_directories(
    ${INCLUDE_DIR}
    ${FFMPEG_INC}
)
file(GLOB_RECURSE SRC_FILES
    "${SOURCE_DIR}/*.c"
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 ")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s WASM=1 -s TOTAL_MEMORY=1677721600")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s EXPORTED_FUNCTIONS='[\"_main\",\"_init_decoder\",\"_decoder_raw\",\"_free_decoder\",\"_malloc\",\"_free\"]' ")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s EXPORTED_RUNTIME_METHODS='[\"cwrap\"]' ")
set(CMAKE_EXECUTABLE_SUFFIX ".js")


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/www)
add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} avformat avutil avcodec)

file(COPY ${CMAKE_SOURCE_DIR}/www DESTINATION ${CMAKE_BINARY_DIR})
