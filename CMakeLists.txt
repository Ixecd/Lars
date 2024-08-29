cmake_minimum_required(VERSION 3.28)

project(lars_reactor LANGUAGES C CXX)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_PREFIX_PATH "/usr/local/protobuf" CACHE STRING "Path to the protobuf library")

add_subdirectory(base)
add_subdirectory(lars_reactor)
add_subdirectory(lars_dns)
add_subdirectory(lars_reporter)
add_subdirectory(lars_loadbalance_agent)
# add_subdirectory(co_async)
