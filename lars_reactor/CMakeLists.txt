#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/lib)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/lib)

#file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cc include/*.hpp)
#add_library(lars_reactor STATIC ${srcs})


cmake_minimum_required(VERSION 3.28)

# 设置项目名称
project(lars_reactor)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/lib)

# 添加头文件搜索路径
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../co_async)

# 将 src 目录中的源文件添加到工程中
#file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_LIST_DIR}/src/*.cc")
file(GLOB_RECURSE srcs CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.cc")

# 创建静态库
add_library(lars_reactor STATIC ${srcs})

# 安装头文件和库文件
#install(TARGETS lars_reactor DESTINATION lib)
#install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/lars_reactor DESTINATION include)
target_include_directories(lars_reactor PUBLIC include)

