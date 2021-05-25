cmake_minimum_required(VERSION 3.12)
project(supg)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

set(SUPG_HEADERS
    include/supg/core/simulator.h
    include/supg/core/device.h
    include/supg/core/gateway.h
    include/supg/core/rxpk.h
    include/supg/core/payload.h
    include/supg/util/data_types.h
    include/supg/util/functions.h
)
set(SUPG_SOURCES
    src/core/simulator.cpp
    src/core/device.cpp
    src/core/gateway.cpp
    src/core/rxpk.cpp
    src/core/payload.cpp
    src/util/functions.cpp
)
add_library(supg ${SUPG_HEADERS} ${SUPG_SOURCES})

add_executable(main apps/main.cpp)
target_link_libraries(main PRIVATE supg)