cmake_minimum_required(VERSION 3.12)
project(supg)

set(CMAKE_CXX_STANDARD 14)

include_directories(modules/tinytoml/include)
include_directories(modules/date/include)
include_directories(modules/spdlog/include)
include_directories(modules/cryptopp)
include_directories(modules/json11)
include_directories(include)

set(supg_hdrs 
        include/supg/core/simulator.h
        include/supg/core/device.h
        include/supg/core/gateway.h
        include/supg/core/rxpk.h
        include/supg/core/payload.h
        include/supg/core/frame.h
        include/supg/core/config.h
        include/supg/util/helper.h
        include/supg/util/channel.h
        include/supg/util/data_types.h)
set(supg_srcs
        src/core/simulator.cpp
        src/core/device.cpp
        src/core/gateway.cpp
        src/core/rxpk.cpp
        src/core/payload.cpp
        src/util/helper.cpp)
add_executable(supg
        apps/supg.cpp
        ${supg_hdrs}
        ${supg_srcs})
target_link_libraries(supg
        PUBLIC date::date
        PUBLIC cryptopp-shared
        PUBLIC json11)

add_subdirectory(modules)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/apps/supg.toml DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)