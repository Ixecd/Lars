cmake_minimum_required(VERSION 3.28)

# 设置项目名称
project(base)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/lib)

# 设置 CMAKE_PREFIX_PATH,这里将protobuf安装在/usr/local/protobuf下了
set(CMAKE_PREFIX_PATH "/usr/local/protobuf" CACHE STRING "Path to the protobuf library")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/lib)

# 查找 pthread 库
find_package(Threads REQUIRED)
# 查找 protobuf
find_package(Protobuf)
if (PROTOBUF_FOUND)
    message("protobuf found")
else ()
    message(FATAL_ERROR "Cannot find Protobuf")
endif ()


# CMAKE_SOURCE_DIR / CMAKE_CURRENT_LIST_DIR

# 添加头文件搜索路径
include_directories(${CMAKE_CURRENT_LIST_DIR}/proto)

file(GLOB_RECURSE srcs CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/proto/*.cc)


foreach(src ${srcs})
    message(STATUS "Found source file: ${src}")
endforeach()

add_library(base STATIC ${srcs})

target_include_directories(base PUBLIC .)

