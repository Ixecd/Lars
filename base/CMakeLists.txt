file(GLOB_RECURSE srcs CONFIGURE_DEPENDS proto/*.cc proto/*.h)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/lib)
add_library(base STATIC ${srcs})
target_include_directories(base PUBLIC proto)

