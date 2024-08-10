cmake_minimum_required(VERSION 3.28)

# 设置项目名称
project(lars_reporter)

add_compile_options(-fPIC)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 设置protobuf位置
set(PROTOBUF_SRC_INC ${CMAKE_CURRENT_LIST_DIR}/../base/proto)

# 设置 CMAKE_PREFIX_PATH,这里将protobuf安装在/usr/local/protobuf下了
set(CMAKE_PREFIX_PATH "/usr/local/protobuf" CACHE STRING "Path to the protobuf library")

# 添加头文件搜索路径
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/../lars_reactor/include)
include_directories(${CMAKE_SOURCE_DIR}/../base)
include_directories(/usr/include/mysql-connector-c/include)
include_directories(${CMAKE_SOURCE_DIR}/../co_async)

# 添加静态库目录
link_directories(${CMAKE_SOURCE_DIR}/../lars_reactor/lib)
link_directories(/usr/include/mysql-connector-c/lib)
link_directories(${CMAKE_SOURCE_DIR}/../base/lib)
link_directories(/usr/local/protobuf/lib)

# 查找 pthread 库
find_package(Threads REQUIRED)
# 查找 protobuf (这里可以不用find因为在build的时候就-D加上了地址)
find_package(Protobuf)
if (PROTOBUF_FOUND)
    message("protobuf found")
else ()
    message(FATAL_ERROR "Cannot find Protobuf")
endif ()

# 将 src 目录中的源文件添加到工程中
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cc")

# 创建可执行文件
add_executable(lars_reporter ${SOURCES})


# 链接静态库
target_link_libraries(lars_reporter PUBLIC Threads::Threads)
target_link_libraries(lars_reporter PUBLIC mysqlclient)
target_link_libraries(lars_reporter PUBLIC base)
target_link_libraries(lars_reporter PUBLIC protobuf)
target_link_libraries(lars_reporter PUBLIC lars_reactor)

target_include_directories(lars_reporter PUBLIC include)

