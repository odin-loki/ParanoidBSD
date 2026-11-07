#!/bin/bash
# make_list.sh [tasks.json]   Print the C/C++ files behind the SPEC tasks' sources: hbsd/ paths, linux:<dir> directories (their .c files), one per line, no duplicates.
# Needs the repo checkout (hbsd/) at /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79 and the Linux reference tree at ~/pbsd-ref/linux.
T=${1:-/opt/pbsd/tasks.json}; R=/mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79; L=$HOME/pbsd-ref/linux
jq -r '.[]|select(.id|test("-SPEC$"))|.source|select(.!=null)' "$T" | sed 's/ part [0-9]*\/[0-9]*$//' | sort -u | while read -r src; do
  case "$src" in
    linux:*) d=${src#linux:}; if [ -d "$L/$d" ]; then find "$L/$d" -maxdepth 1 -type f -name '*.c' -size +1k -size -60k; elif [ -f "$L/$d" ]; then echo "$L/$d"; fi ;;
    hbsd/*)  [ -f "$R/$src" ] && echo "$R/$src" ;;
  esac
done | sort -u
