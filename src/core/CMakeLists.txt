set(
    SOURCE_FILES
    main.cpp
)

set(
    HEADER_FILES
)

set(ALL_FILES ${SOURCE_FILES} ${HEADER_FILES})

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${ALL_FILES})

add_executable(plain ${ALL_FILES})
target_link_libraries(plain PRIVATE network)

target_compile_features(plain PRIVATE cxx_std_20)
set_target_properties(plain PROPERTIES CXX_EXTENSIONS OFF)

if(MSVC)
    target_compile_options(plain PRIVATE /W4 /WX)
else()
    target_compile_options(plain PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
