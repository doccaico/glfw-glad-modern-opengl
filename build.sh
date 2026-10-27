#!/bin/sh

if [ -z "$1" ]; then
    echo "build.sh SRC_DIR [\"\" or debug|release]"
    exit 1
fi

EXE_NAME="$1"
SRC_DIR="$1"

OUT_DIR="build"
OUT_DIR_BIN="${OUT_DIR}/bin"

C_FLAGS=""
C_FLAGS="${C_FLAGS} -std=c++26"
C_FLAGS="${C_FLAGS} -Wall -Wextra"
C_FLAGS="${C_FLAGS} -Wno-unused-function"
C_FLAGS="${C_FLAGS} -DUNICODE -D_UNICODE"
C_FLAGS="${C_FLAGS} -DGLAD_GL_IMPLEMENTATION"
C_FLAGS="${C_FLAGS} -DNDEBUG -D_NDEBUG"
C_FLAGS="${C_FLAGS} -Iinclude"
C_FLAGS="${C_FLAGS} -IVendor/glad/include -IVendor/glfw/include"

L_FLAGS=""
L_FLAGS="${L_FLAGS} -Lvendor/glfw/lib/lib-mingw-w64"
L_FLAGS="${L_FLAGS} -lglfw3 -lopengl32"
L_FLAGS="${L_FLAGS} -luser32 -lgdi32 -lshell32"
L_FLAGS="${L_FLAGS} -lstdc++exp"

TARGET_MODE="${2:-debug}"

if [ "$TARGET_MODE" = "debug" ]; then
    MODE="[DEBUG]"
    C_FLAGS="${C_FLAGS} -O0 -g"
elif [ "$TARGET_MODE" = "release" ]; then
    MODE="[RELEASE]"
    C_FLAGS="${C_FLAGS} -O2"
    L_FLAGS="${L_FLAGS} -mwindows"
else
    echo "build.sh SRC_DIR [\"\" or debug|release]" >&2
    exit 1
fi

mkdir -p "${OUT_DIR_BIN}"

g++ "${SRC_DIR}/main.cc" -o "${OUT_DIR_BIN}/${EXE_NAME}.exe" ${C_FLAGS} ${L_FLAGS}
if [ $? -ne 0 ]; then exit 1; fi

printf "\033[1;32m%s\033[0m An executable file has been generated: \033[1m%s\033[0m\n" "$MODE" "${OUT_DIR_BIN}/${EXE_NAME}"
