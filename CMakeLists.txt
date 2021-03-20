cmake_minimum_required(VERSION 3.1.0)
project(pifan)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

file(STRINGS "${PROJECT_SOURCE_DIR}/VERSION" VERSION)
string(REGEX REPLACE "([0-9]+)\\.[0-9]+\\.[0-9]+.*" "\\1" VERSION_MAJOR ${VERSION})
string(REGEX REPLACE "[0-9]+\\.([0-9])+\\.[0-9]+.*" "\\1" VERSION_MINOR ${VERSION})
string(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH ${VERSION})

message(STATUS "${PROJECT_NAME} version: ${VERSION}")
configure_file("${PROJECT_SOURCE_DIR}/version.h.in" "${PROJECT_SOURCE_DIR}/src/version.h")

include(FindPkgConfig)

find_package(GPIOD REQUIRED)
include_directories(${GPIOD_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} src/main.c)
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} ${GPIOD_LIBRARIES} pthread)

install(TARGETS ${PROJECT_NAME} DESTINATION /sbin)
install(FILES ${CMAKE_SOURCE_DIR}/resources/${PROJECT_NAME}.service DESTINATION /usr/lib/systemd/system)
