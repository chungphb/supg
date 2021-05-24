cmake_minimum_required(VERSION 3.19)
project(supg)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

set(SUPG_HEADERS
    include/supg/core/gateway.h
    include/supg/util/payload.h
    include/supg/util/data_types.h
)
set(SUPG_SOURCES
    src/core/gateway.cpp
)
add_library(supg ${SUPG_HEADERS} ${SUPG_SOURCES})

add_executable(main apps/main.cpp)
target_link_libraries(main PRIVATE supg)