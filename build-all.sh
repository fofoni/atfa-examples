#!/bin/bash

directs=(*/)

if [[ $# != 0  ||  ${#directs[@]} == 0 ]]; then
    printf "Usage: %s\n" "$0" >&2
    printf "This script will run build.py on each subdirectory of the\n" >&2
    printf "        current working directory. Each subdir must contain\n" >&2
    printf "        the source files of an implementation of an adaptive\n" >&2
    printf "        filtering algorithm. For details, see the README.\n" >&2
    exit 1
fi

for d in "${directs[@]}"; do
    title="${d%/}"
    title="${title^^}"
    printf "////////////////////////////////////////////////////\n"
    printf "/////\n"
    printf "/////    %s\n\n" "${title}"
    (cd "$d"; ../build.py --title "${title}")
    printf "\n\n"
done
