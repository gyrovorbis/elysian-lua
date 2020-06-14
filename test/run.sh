#!/bin/bash
# Builds and executes the test suite with different compilers.
set -euo pipefail

main() {
    if [[ "$#" -eq 0 ]]; then
        echo >&2 "Usage: $0 build_dir [compiler...]"
        return 1
    fi
    local build_dir=$1
    shift
    if [[ "$#" -eq 0 ]]; then
        set -- g++ clang++
    fi
    local compiler
    for compiler; do
        execute "$build_dir" "$compiler"
    done
}

execute() {
    local build_dir=$1 compiler=$2
    local dir=$build_dir/$compiler
    cmake -B "$dir" -D CMAKE_BUILD_TYPE=Debug -D "CMAKE_CXX_COMPILER=$compiler"
    make -C "$dir" all test
}

main "$@"
