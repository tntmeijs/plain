set(
    SOURCE_FILES
    source/window/window.cpp
    source/renderer/renderer.cpp
)

set(
    HEADER_FILES
    include/graphics/window/window.hpp
    include/graphics/renderer/renderer.hpp
)

set(ALL_FILES ${SOURCE_FILES} ${HEADER_FILES})

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${ALL_FILES})

add_library(graphics ${ALL_FILES})

target_compile_features(graphics PRIVATE cxx_std_20)
target_include_directories(graphics PUBLIC include)
set_target_properties(graphics PROPERTIES CXX_EXTENSIONS OFF)

if(MSVC)
    target_compile_options(graphics PRIVATE /W4 /WX)
else()
    target_compile_options(graphics PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
