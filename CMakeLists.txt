cmake_minimum_required(VERSION 3.10)
project(coasync
    VERSION     0.0.1
    LANGUAGES   CXX
)

add_library(coasync INTERFACE)
add_library(coasync::coasync ALIAS coasync)

target_compile_features(coasync INTERFACE cxx_std_20)

target_include_directories(coasync INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

set(COASYNC_MASTER_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(COASYNC_MASTER_PROJECT ON)
endif()

option(COASYNC_TEST "Build the tests" ${COASYNC_MASTER_PROJECT})

# only override the warning options if we're build as the master
# project, in other cases leave them alone since we might be added
# to a project with laxer standards for dealing with warnings
if (COASYNC_MASTER_PROJECT)
    # msvc does not support normal options
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        # set warning level to 4
        target_compile_options(coasync INTERFACE /W4)
    else()
        # include helper to enable only options which are supported
        include(cmake/target_supported_compile_options.cmake)

        # try all the regular options and enable them if possible
        target_supported_compile_options(coasync INTERFACE -Wall)
        target_supported_compile_options(coasync INTERFACE -Wextra)
        target_supported_compile_options(coasync INTERFACE -Wdeprecated)
        target_supported_compile_options(coasync INTERFACE -Wdocumentation)
    endif()
endif()

install(
    DIRECTORY   include/coasync
    DESTINATION include
)

install(
    FILES       include/coasync.hpp
    DESTINATION include
)

install(
    TARGETS     coasync
    EXPORT      coasync-targets
    DESTINATION lib
)

install(
    EXPORT      coasync-targets
    NAMESPACE   coasync::
    DESTINATION lib/cmake/coasync
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/coasync/coasync-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

export(
    EXPORT      coasync-targets
    FILE        "${CMAKE_CURRENT_BINARY_DIR}/coasync/coasync-targets.cmake"
    NAMESPACE   coasync::
)

configure_file(cmake/coasync-config.cmake
    "${CMAKE_CURRENT_BINARY_DIR}/coasync/coasync-config.cmake"
    COPYONLY
)

install(
    FILES
        cmake/coasync-config.cmake
        "${CMAKE_CURRENT_BINARY_DIR}/coasync/coasync-config-version.cmake"
    DESTINATION
        lib/cmake/coasync
)

if (COASYNC_TEST)
    add_subdirectory(tests)
endif()
