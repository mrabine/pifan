cmake_minimum_required(VERSION 3.22.1)
project(pifan)

file(STRINGS "${PROJECT_SOURCE_DIR}/VERSION" VERSION)
string(REGEX REPLACE "([0-9]+)\\.[0-9]+\\.[0-9]+.*" "\\1" VERSION_MAJOR ${VERSION})
string(REGEX REPLACE "[0-9]+\\.([0-9]+)\\.[0-9]+.*" "\\1" VERSION_MINOR ${VERSION})
string(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH ${VERSION})

message(STATUS "${PROJECT_NAME} version: ${VERSION}")
configure_file("${PROJECT_SOURCE_DIR}/version.h.in" "${PROJECT_SOURCE_DIR}/src/version.h")

include(FindPkgConfig)
pkg_check_modules(GPIOD REQUIRED libgpiod)

add_definitions(-D_POSIX_C_SOURCE=1)
add_executable(${PROJECT_NAME} src/main.c)
target_include_directories(${PROJECT_NAME} PRIVATE src ${GPIOD_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${GPIOD_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} ${GPIOD_LIBRARIES} pthread)

configure_file(
    ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}.service.in
    ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.service
    @ONLY
)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.service DESTINATION lib/systemd/system)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT "Mathieu Rabine")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Raspberry Pi fan controller daemon")
set(CPACK_PACKAGE_DESCRIPTION
"pifan starts a GPIO-controlled fan when the CPU temperature exceeds a
configurable upper threshold and stops it when the temperature drops
below a configurable lower threshold.
Supports Raspberry Pi 4 and Raspberry Pi 5.")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgpiod2 (>= 2.0)")
set(CPACK_DEBIAN_PACKAGE_SUGGESTS "gpiod")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "armhf")
else()
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${CMAKE_SYSTEM_PROCESSOR}")
endif()

include(CPack)
