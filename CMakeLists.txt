cmake_minimum_required(VERSION 3.16..3.24)

project(
    Plain
    HOMEPAGE_URL https://github.com/tntmeijs/plain
    VERSION 0.0.1
    LANGUAGES CXX
)

# Configure Spdlog
include(ExternalProject)
ExternalProject_Add(spdlog
    PREFIX spdlog
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/dependencies/spdlog
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_INSTALL_PREFIX=${STAGING_DIR}
    -DSPDLOG_BUILD_SHARED=OFF
)

# Plain projects
add_subdirectory("src/graphics" "graphics")
add_subdirectory("src/network" "network")
add_subdirectory("src/plain" "plain")

# When using Visual Studio, set the startup project to make it easier to run the application
if (${MSVC})
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT plain)
endif()
