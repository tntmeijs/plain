cmake_minimum_required(VERSION 3.12..3.24)

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

add_subdirectory("src/network")
add_subdirectory("src/core")
add_dependencies(plain network spdlog)

target_include_directories(plain PRIVATE dependencies/spdlog/include)
target_include_directories(network PRIVATE dependencies/spdlog/include)

# When using Visual Studio, set the core executable as the startup project
if (${MSVC})
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT plain)
endif()
