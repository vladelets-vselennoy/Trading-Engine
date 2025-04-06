#!/usr/bin/env bash

# ==============================================================================
# Build and Run Script for the Deribit C++ Client
#
# This script automates the process of building the project using CMake/Make
# and running the resulting executable.
#
# Usage:
#   ./build_and_run.sh [options] [-- [executable_arguments...]]
#
# Options:
#   --clean          Remove the build directory before building.
#   --build-type TYPE Set CMake build type (Debug, Release, RelWithDebInfo, MinSizeRel).
#                    Default: Release
#   --jobs N         Number of parallel jobs for make (e.g., --jobs 4).
#                    Default: Auto-detect using getconf or nproc.
#   --help           Display this help message and exit.
#
# Arguments after '--' are passed directly to the DeribitClient executable.
#
# Prerequisites:
#   - CMake (version 3.16+)
#   - Make
#   - C++17 compliant compiler (GCC 7+, Clang 5+)
#   - Boost libraries (System, Thread, Asio, Beast, SSL - Development Headers)
#   - OpenSSL (Development Libraries)
#   - spdlog (Development Headers/Library)
#   - fmt (Development Headers/Library)
#   - nlohmann/json (Header-only)
# ==============================================================================

# --- Configuration ---
BUILD_DIR="build"                   # Name of the build directory
EXECUTABLE_NAME="DeribitClient"     # Executable name defined in CMakeLists.txt
DEFAULT_BUILD_TYPE="Release"        # Default CMake build type
DEFAULT_JOBS=$(getconf _NPROCESSORS_ONLN 2>/dev/null || nproc 2>/dev/null || echo 1) # Auto-detect cores or default to 1

# --- Script Setup ---
# Exit immediately if a command exits with a non-zero status.
set -e
# Treat unset variables as an error when substituting.
# set -u # Can be too strict sometimes, enable if needed
# Pipe failures should exit the script
set -o pipefail

# Get the directory where the script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$SCRIPT_DIR" # Assume script is in project root

# --- Helper Functions ---
info() {
    echo "[INFO] $1"
}

warn() {
    echo "[WARN] $1" >&2
}

error() {
    echo "[ERROR] $1" >&2
    exit 1
}

show_help() {
    grep '^# Usage:' "$0" | cut -c3-
    echo ""
    grep '^# Options:' "$0" | cut -c3-
    grep '^#   --' "$0" | cut -c3-
    echo ""
    grep '^# Arguments after' "$0" | cut -c3-
    echo ""
    grep '^# Prerequisites:' "$0" | cut -c3-
    grep '^#   -' "$0" | cut -c3-
}

clean_build() {
    if [ -d "$BUILD_DIR_PATH" ]; then
        info "Cleaning build directory: $BUILD_DIR_PATH"
        rm -rf "$BUILD_DIR_PATH"
    else
        info "Build directory '$BUILD_DIR_PATH' does not exist. Nothing to clean."
    fi
}

# --- Argument Parsing ---
CLEAN_REQUESTED=false
BUILD_TYPE="$DEFAULT_BUILD_TYPE"
NUM_JOBS="$DEFAULT_JOBS"
EXECUTABLE_ARGS=()
PASS_THROUGH_MODE=false

while [[ $# -gt 0 ]]; do
    if $PASS_THROUGH_MODE; then
         EXECUTABLE_ARGS+=("$1")
         shift
         continue
    fi

    case $1 in
        --clean)
            CLEAN_REQUESTED=true
            shift
            ;;
        --build-type)
            if [[ -z "$2" || "$2" == -* ]]; then
                error "Option --build-type requires an argument (e.g., Debug, Release)."
            fi
            BUILD_TYPE="$2"
            shift 2
            ;;
        --jobs)
             if [[ -z "$2" || "$2" == -* ]]; then
                error "Option --jobs requires a number argument."
            fi
            if ! [[ "$2" =~ ^[0-9]+$ ]]; then
                error "Invalid argument for --jobs: '$2'. Must be a positive integer."
            fi
            NUM_JOBS="$2"
            shift 2
            ;;
        --help)
            show_help
            exit 0
            ;;
        --) # Separator for pass-through arguments
            PASS_THROUGH_MODE=true
            shift
            ;;
        -*) # Unknown option
            error "Unknown option: $1"
            ;;
        *) # Positional argument (unexpected)
            error "Unexpected positional argument: $1"
            ;;
    esac
done

# --- Sanity Checks & Prerequisite Check ---
info "Checking prerequisites..."
command -v cmake >/dev/null 2>&1 || error "cmake is not installed or not in PATH."
command -v make >/dev/null 2>&1 || error "make is not installed or not in PATH."
info "Prerequisites (cmake, make) found."
info "Reminder: Ensure C++17 compiler and required libraries (Boost, OpenSSL, spdlog, fmt, nlohmann_json) are installed."

# --- Main Logic ---
cd "$PROJECT_ROOT"
BUILD_DIR_PATH="$PROJECT_ROOT/$BUILD_DIR"

# Clean if requested
if $CLEAN_REQUESTED; then
    clean_build
fi

# Configure (CMake)
info "Configuring project using CMake..."
info "Build Type: $BUILD_TYPE"
info "Build Directory: $BUILD_DIR_PATH"

# Create build directory if it doesn't exist
mkdir -p "$BUILD_DIR_PATH"

# Run CMake
cmake -S "$PROJECT_ROOT" -B "$BUILD_DIR_PATH" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON || error "CMake configuration failed."
info "CMake configuration successful."

# Build (Make)
info "Building project using make (using $NUM_JOBS parallel jobs)..."
make -C "$BUILD_DIR_PATH" -j"$NUM_JOBS" || error "Make build failed."
info "Build successful."

# Run
EXECUTABLE_PATH="$BUILD_DIR_PATH/$EXECUTABLE_NAME"
info "Attempting to run executable..."

if [ ! -f "$EXECUTABLE_PATH" ]; then
    error "Executable '$EXECUTABLE_PATH' not found after build."
fi

if [ ! -x "$EXECUTABLE_PATH" ]; then
    error "Executable '$EXECUTABLE_PATH' is not executable. Check permissions."
fi

info "Executing: $EXECUTABLE_PATH ${EXECUTABLE_ARGS[*]}"
echo "------------------------------------------------------" # Separator before execution output

# Execute the client, passing any arguments that came after '--'
"$EXECUTABLE_PATH" "${EXECUTABLE_ARGS[@]}"

EXIT_CODE=$?
echo "------------------------------------------------------" # Separator after execution output

if [ $EXIT_CODE -ne 0 ]; then
    warn "Executable exited with non-zero status code: $EXIT_CODE"
fi

info "Script finished."
exit $EXIT_CODE