# CMAKE generated file: DO NOT EDIT!
# Generated by CMake Version 3.28
cmake_policy(SET CMP0009 NEW)

# srcs at base/CMakeLists.txt:31 (file)
file(GLOB_RECURSE NEW_GLOB LIST_DIRECTORIES false "/home/qc/Lars/base/proto/*.cc")
set(OLD_GLOB
  "/home/qc/Lars/base/proto/lars.pb.cc"
  )
if(NOT "${NEW_GLOB}" STREQUAL "${OLD_GLOB}")
  message("-- GLOB mismatch!")
  file(TOUCH_NOCREATE "/home/qc/Lars/build/CMakeFiles/cmake.verify_globs")
endif()

# srcs at lars_reactor/CMakeLists.txt:25 (file)
file(GLOB_RECURSE NEW_GLOB LIST_DIRECTORIES false "/home/qc/Lars/lars_reactor/src/*.cc")
set(OLD_GLOB
  "/home/qc/Lars/lars_reactor/src/buf_pool.cc"
  "/home/qc/Lars/lars_reactor/src/config_file.cc"
  "/home/qc/Lars/lars_reactor/src/event_loop.cc"
  "/home/qc/Lars/lars_reactor/src/io_buf.cc"
  "/home/qc/Lars/lars_reactor/src/reactor.cc"
  "/home/qc/Lars/lars_reactor/src/tcp_client.cc"
  "/home/qc/Lars/lars_reactor/src/tcp_conn.cc"
  "/home/qc/Lars/lars_reactor/src/tcp_server.cc"
  "/home/qc/Lars/lars_reactor/src/thread_pool.cc"
  "/home/qc/Lars/lars_reactor/src/udp_client.cc"
  "/home/qc/Lars/lars_reactor/src/udp_server.cc"
  )
if(NOT "${NEW_GLOB}" STREQUAL "${OLD_GLOB}")
  message("-- GLOB mismatch!")
  file(TOUCH_NOCREATE "/home/qc/Lars/build/CMakeFiles/cmake.verify_globs")
endif()