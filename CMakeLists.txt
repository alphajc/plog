cmake_minimum_required(VERSION 3.6)
project(plog)
INCLUDE_DIRECTORIES(
        include
)
link_libraries(pthread)
set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES main.c src/mmgr.c src/lib.c src/config.c src/files_handle.c include/files_handle.h)
add_executable(plog ${SOURCE_FILES})