cmake_minimum_required(VERSION 4.0.0)

project(micronot VERSION 0.1)

# clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

add_executable(
    unotd
    src/main.c
    src/unotd.c
    src/utils.c
    src/nlist.c
    src/notification.c
    src/server.c
    src/protocol.c
)

include_directories (/usr/include/freetype2)

target_link_libraries(unotd
    X11
    Xft
    fontconfig)
