#!/bin/bash

# Define source files
SRC_FILES="src/main_drawonly.cpp"
OUTPUT_EXE="neural_network_draw.exe"

# Include directories
INCLUDE_DIRS="-I./include -I./include/raylib -I./include/mnist"

# Library directories
LIB_DIRS="-L./lib"

# Libraries to link
LIBS="-lraylib -lopengl32 -lgdi32 -lwinmm"

# Compile command
g++ $SRC_FILES -o $OUTPUT_EXE $INCLUDE_DIRS $LIB_DIRS $LIBS -static -O3

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "[✓] Build successful: $OUTPUT_EXE"
    echo "Running..."
    ./$OUTPUT_EXE
else
    echo "[✗] Build failed"
fi
