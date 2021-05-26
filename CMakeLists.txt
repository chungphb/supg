cmake_minimum_required(VERSION 3.12)
project(supg)

set(CMAKE_CXX_STANDARD 14)

include_directories(modules/tinytoml/include)
include_directories(modules/date/include)
include_directories(modules/spdlog/include)
include_directories(include)

set(SUPG_HEADERS
    include/supg/core/simulator.h
    include/supg/core/device.h
    include/supg/core/gateway.h
    include/supg/core/rxpk.h
    include/supg/core/payload.h
    include/supg/util/helper.h
    include/supg/util/config.h
    include/supg/util/helper.h
)
set(SUPG_SOURCES
    src/core/simulator.cpp
    src/core/device.cpp
    src/core/gateway.cpp
    src/core/rxpk.cpp
    src/core/payload.cpp
    src/util/helper.cpp
)
add_library(supg ${SUPG_HEADERS} ${SUPG_SOURCES})
target_link_libraries(supg PUBLIC date::date)

add_executable(main apps/main.cpp)
target_link_libraries(main PRIVATE supg)

add_subdirectory(modules)