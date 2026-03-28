# - Try to find libgpiod
# Once done this will define
#  GPIOD_FOUND - System has libgpiod
#  GPIOD_INCLUDE_DIRS - The libgpiod include directories
#  GPIOD_LIBRARIES - The libraries needed to use libgpiod

find_package(PkgConfig QUIET)

pkg_check_modules(PC_GPIOD QUIET libgpiod)

find_library(GPIOD_LIBRARY 
    NAMES gpiod
    HINTS ${PC_GPIOD_LIBDIR} ${PC_GPIOD_LIBRARY_DIRS}
)
mark_as_advanced(GPIOD_LIBRARY)

find_path(GPIOD_INCLUDE_DIR
    NAMES gpiod.h
    HINTS ${PC_GPIOD_INCLUDEDIR} ${PC_GPIOD_INCLUDE_DIRS}
)
mark_as_advanced(GPIOD_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(gpiod
    REQUIRED_VARS GPIOD_LIBRARY GPIOD_INCLUDE_DIR
)

if(GPIOD_FOUND)
    set(GPIOD_LIBRARIES ${GPIOD_LIBRARY})
    set(GPIOD_INCLUDE_DIRS ${GPIOD_INCLUDE_DIR})
endif()
