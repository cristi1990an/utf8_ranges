#!/usr/bin/env bash

set -euo pipefail

if [[ $# -lt 2 ]]; then
  echo "usage: $0 <compiler> <output-dir> [extra compiler flags...]" >&2
  exit 1
fi

cxx=$1
shift
out_dir=$1
shift

mkdir -p "$out_dir"

mapfile -t sources < <(find docs/examples -name '*.cpp' | sort)

for source in "${sources[@]}"; do
  rel="${source#docs/examples/}"
  stem="${rel%.cpp}"
  exe_name="${stem//\//__}"
  echo "Compiling $source"
  "$cxx" -std=c++23 -I. "$@" "$source" -o "$out_dir/$exe_name"
done
