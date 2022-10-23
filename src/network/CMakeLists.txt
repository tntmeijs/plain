set(
    SOURCE_FILES
    source/http/request_message_builder.cpp
    source/http/request_message.cpp
    source/tcp/socket_factory.cpp
    source/tcp/socket.cpp
    source/tcp/windows/windows_socket.cpp
)

set(
    HEADER_FILES
    include/network/http/http_method.hpp
    include/network/http/header.hpp
    include/network/http/request_message_builder.hpp
    include/network/http/request_message.hpp
    include/network/tcp/socket_factory.hpp
    include/network/tcp/socket_state_change.hpp
    include/network/tcp/socket.hpp
    include/network/tcp/windows/windows_socket.hpp
)

set(ALL_FILES ${SOURCE_FILES} ${HEADER_FILES})

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${ALL_FILES})

add_library(network ${ALL_FILES})

target_compile_features(network PRIVATE cxx_std_20)
target_include_directories(network PUBLIC include)
set_target_properties(network PROPERTIES CXX_EXTENSIONS OFF)

if(MSVC)
    target_compile_options(network PRIVATE /W4 /WX)
else()
    target_compile_options(network PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

if(WIN32)
    target_link_libraries(network PRIVATE Ws2_32.lib)
endif()
