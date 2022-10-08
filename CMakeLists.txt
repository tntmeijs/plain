cmake_minimum_required(VERSION 3.12..3.24)

project(
    Plain
    HOMEPAGE_URL https://github.com/tntmeijs/plain
    VERSION 0.0.1
    LANGUAGES CXX
)

add_subdirectory("src/network")
add_subdirectory("src/core")

# When using Visual Studio, set the core executable as the startup project
if (${MSVC})
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT plain)
endif()
