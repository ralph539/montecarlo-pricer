cmake_minimum_required(VERSION 3.20)
project(montecarlo_hedging CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif ()

# PNL: numerical library (vectors, matrices, RNG, Cholesky).
# Point CMAKE_PREFIX_PATH at the PNL build directory.
find_package(Pnl REQUIRED)
include_directories(${PNL_INCLUDE_DIRS})
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DPNL_RANGE_CHECK_OFF)
endif ()

# nlohmann_json: fetched if not installed system-wide.
find_package(nlohmann_json QUIET)
if (NOT nlohmann_json_FOUND)
    include(FetchContent)
    FetchContent_Declare(json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(json)
endif ()

add_library(engine STATIC
    src/Option.cpp
    src/BasketOption.cpp
    src/AsianOption.cpp
    src/PerformanceOption.cpp
    src/BlackScholes.cpp
    src/MonteCarlo.cpp
    src/Hedging.cpp
    src/Portfolio.cpp
    src/Parameters.cpp
)
target_include_directories(engine PUBLIC src)
target_link_libraries(engine PUBLIC ${PNL_LIBRARIES} nlohmann_json::nlohmann_json)
target_compile_options(engine PRIVATE -Wall -Wextra)

add_executable(price src/price.cpp)
target_link_libraries(price engine)
pnl_add_postbuild(price)

add_executable(hedge src/hedge.cpp)
target_link_libraries(hedge engine)
pnl_add_postbuild(hedge)

enable_testing()
foreach (t test_options test_montecarlo test_hedging)
    add_executable(${t} tests/${t}.cpp)
    target_link_libraries(${t} engine)
    target_compile_options(${t} PRIVATE -UNDEBUG)
    pnl_add_postbuild(${t})
    add_test(NAME ${t} COMMAND ${t} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endforeach ()
