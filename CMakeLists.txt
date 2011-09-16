######################################
##### CMake environment setup    #####
######################################
project(filewalk)
cmake_minimum_required(VERSION 2.6.4)
set(CMAKE_MODULE_PATH ${${PROJECT_NAME}_SOURCE_DIR}/cmake)
set(EXECUTABLE_OUTPUT_PATH ${${PROJECT_NAME}_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${${PROJECT_NAME}_BINARY_DIR})
include_directories(${${PROJECT_NAME}_SOURCE_DIR})


find_package(GLIB REQUIRED)
include_directories(SYSTEM ${GLIB_INCLUDE_DIRS})

add_library(filetree filetree.c)
target_link_libraries(filetree ${GLIB_LIBRARIES})

add_executable(walk main.c)
target_link_libraries(walk filetree input)
