cmake_minimum_required(VERSION 3.10)
project(filewalk C)

include_directories(${${PROJECT_NAME}_SOURCE_DIR})

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(GLIB20 glib-2.0)
endif()

if(GLIB20_FOUND AND NOT DISABLE_GLIB20)
  include_directories(SYSTEM ${GLIB20_INCLUDE_DIRS})
  link_directories(${GLIB20_LIBRARY_DIRS})
  add_definitions(${GLIB20_CFLAGS_OTHER})

  add_library(filetree filetree.c)
  target_link_libraries(filetree ${GLIB20_LIBRARIES})

  install(TARGETS filetree DESTINATION lib)
endif()
