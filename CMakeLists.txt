cmake_minimum_required(VERSION 3.1.0)
project(pifan)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(FindPkgConfig)

find_package(GPIOD REQUIRED)
include_directories(${GPIOD_INCLUDE_DIRS})

add_executable(pifan src/main.c)
target_link_libraries(pifan ${GPIOD_LIBRARIES} pthread)

install(TARGETS pifan DESTINATION /sbin)
install(FILES ${CMAKE_SOURCE_DIR}/resources/pifan.service DESTINATION /usr/lib/systemd/system)
