# Configure Vulkan
find_package(Vulkan COMPONENTS shaderc_combined REQUIRED)

# Configure GLFW
set(GLFW_BUILD_DOCS false)
set(GLFW_INSTALL false)
set(GLFW_BUILD_EXAMPLES false)
set(GLFW_BUILD_TESTS false)
add_subdirectory("${PROJECT_SOURCE_DIR}/dependencies/glfw" "glfw")

# Configure GLM
add_subdirectory("${PROJECT_SOURCE_DIR}/dependencies/glm" "glm")

set(
    SOURCE_FILES
    source/window/window.cpp
    source/renderer/renderer.cpp
    source/renderer/shader_module.cpp
    source/renderer/vulkan/instance.cpp
)

set(
    HEADER_FILES
    include/graphics/window/window.hpp
    include/graphics/renderer/renderer.hpp
    include/graphics/renderer/shader_module.hpp
    include/graphics/renderer/vulkan/instance.hpp
)

set(ALL_FILES ${SOURCE_FILES} ${HEADER_FILES})

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${ALL_FILES})

add_library(graphics ${ALL_FILES})

target_compile_features(graphics PRIVATE cxx_std_20)
target_include_directories(graphics PUBLIC include ${Vulkan_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/dependencies/spdlog/include)
target_link_libraries(graphics PRIVATE glfw glm::glm Vulkan::Vulkan Vulkan::shaderc_combined)

set_target_properties(graphics PROPERTIES CXX_EXTENSIONS OFF)

if(MSVC)
    target_compile_options(graphics PRIVATE /W4 /WX)
else()
    target_compile_options(graphics PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_dependencies(graphics spdlog glm glfw)
