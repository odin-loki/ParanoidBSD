#!/usr/bin/env bash
# decomposer gate: the split must still build (no code yet, so this is quick).
. "$(dirname "$0")/lib.sh"
gate_main "$1" "$2" build
