#!/bin/bash

# Compiler options
CC=em++
CFLAGS="-std=c++17 -Os -Wall"
EMFLAGS="-s USE_GLFW=3 --shell-file shell.html -DPLATFORM_WEB"

# Paths
INCLUDE_DIR="include/"
LIB_DIR="llib/"
OUTPUT="index.html"

# Build command
$CC -o $OUTPUT main.cpp $LIB_DIR/libraylib.a $CFLAGS -I. -I $INCLUDE_DIR -L. -L $LIB_DIR $EMFLAGS