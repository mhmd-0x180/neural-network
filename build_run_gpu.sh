#!/bin/bash

# === CONFIGURATION ===
PROJECT_NAME="neural_network_gpu" # don't use spaces

COMPILER_FLAGS="-O2"

# Compile main_gpu.cpp instead of main.cpp
SRC_FILE="src/main_gpu.cpp" 
INCLUDE_DIR="./include"  # path to raylib include folder
LIB_DIR="./lib"          # path to raylib .a files (static)

# Additional required system libraries
SYSTEM_LIBS="-lopengl32 -lgdi32 -lwinmm"
# === BUILD COMMAND ===
g++ "$SRC_FILE" -o "$PROJECT_NAME.exe" "$COMPILER_FLAGS" \
    -I"$INCLUDE_DIR" -L"$LIB_DIR" -lraylib $SYSTEM_LIBS \
    -static -static-libgcc -static-libstdc++

# === OUTPUT ===
if [ $? -eq 0 ]; then
    echo "[✓] Build successful: $PROJECT_NAME.exe"
    echo "Running..."
    ./"$PROJECT_NAME.exe"
else
    echo "[✗] Build failed"
fi
