cmake_minimum_required(VERSION 4.0)

project(micronot VERSION 0.1 LANGUAGES C)

# clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# third-party
add_library(ext_headers INTERFACE)
target_include_directories(ext_headers INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/ext
)

# compiler options
add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE
    -Wall
    -Wextra
)

# unotd
add_executable(unotd
    src/daemon/main.c
    src/daemon/config.c
    src/daemon/utils.c
    src/daemon/nlist.c
    src/daemon/notification.c
    src/daemon/indicator.c
    src/daemon/server.c
    src/daemon/command.c
)

find_package(Freetype REQUIRED)

target_link_libraries(unotd PRIVATE
    ext_headers
    project_warnings
    Freetype::Freetype
    X11
    Xft
    fontconfig
)

set_property(TARGET unotd PROPERTY
    INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
)

# unotclient
add_library(unotclient SHARED
    src/client/client.c
)

target_link_libraries(unotclient PRIVATE
    ext_headers
    project_warnings
)

target_include_directories(unotclient PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
