cmake_minimum_required(VERSION 3.5)
project(Lars_reactor)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

# 添加源文件目录
file(GLOB SOURCES "src/*.cc")

# 添加头文件目录
include_directories(include)

# 生成动态库
add_library(${PROJECT_NAME} STATIC ${SOURCES})

set(INSTALL_LIB_DIR lib)
# 设置目标文件夹
set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib
)

