cmake_minimum_required(VERSION 4.0.0)

project(micronot VERSION 0.1)

# clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

add_executable(unot
    src/main.c
    src/notification.c
    src/list.c
    src/unotd.c)

include_directories (/usr/include/freetype2)

target_link_libraries(unot
    X11
    Xft
    fontconfig)
