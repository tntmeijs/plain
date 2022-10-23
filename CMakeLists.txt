cmake_minimum_required(VERSION 3.16..3.24)

project(
    Plain
    HOMEPAGE_URL https://github.com/tntmeijs/plain
    VERSION 0.0.1
    LANGUAGES CXX
)

# Configure Vulkan
find_package(Vulkan REQUIRED)

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

# Configure GLFW
set(GLFW_BUILD_DOCS false)
set(GLFW_INSTALL false)
set(GLFW_BUILD_EXAMPLES false)
set(GLFW_BUILD_TESTS false)
add_subdirectory("${PROJECT_SOURCE_DIR}/dependencies/glfw")

# Configure GLM
add_subdirectory("${PROJECT_SOURCE_DIR}/dependencies/glm")

# Plain projects
add_subdirectory("src/graphics")
add_subdirectory("src/network")
add_subdirectory("src/core")

# Ensure the projects build in the correct order
add_dependencies(network spdlog)
add_dependencies(graphics spdlog glm glfw)
add_dependencies(plain network graphics spdlog)

# Ensure all projects can find the spdlog header files
target_include_directories(plain PRIVATE dependencies/spdlog/include)
target_include_directories(network PRIVATE dependencies/spdlog/include)
target_include_directories(graphics PRIVATE dependencies/spdlog/include ${Vulkan_INCLUDE_DIRS})

target_link_libraries(plain PRIVATE network graphics)
target_link_libraries(plain PRIVATE graphics)
target_link_libraries(graphics PRIVATE glfw glm::glm ${Vulkan_LIBRARIES})

# When using Visual Studio, set the core executable as the startup project
if (${MSVC})
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT plain)
endif()
