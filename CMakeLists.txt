cmake_minimum_required(VERSION 4.0.0)

project(micronot VERSION 0.1)

# clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

add_executable(
    unotd
    src/daemon/main.c
    src/daemon/unotd.c
    src/daemon/config.c
    src/daemon/utils.c
    src/daemon/nlist.c
    src/daemon/notification.c
    src/daemon/indicator.c
    src/daemon/server.c
    src/daemon/parser.c
    src/daemon/command.c
    src/protocol.c
)

add_library(unotclient SHARED src/client/client.c src/protocol.c)
target_include_directories(unotclient
    PUBLIC ${PROJECT_SOURCE_DIR}/include/
)

include_directories (/usr/include/freetype2)

target_link_libraries(unotd
    X11
    Xft
    fontconfig
)
