#!/usr/bin/env bash
# integrator gate: full build, tests and a QEMU boot of the merged package (and a proof re-check of every unit the merge changed).
. "$(dirname "$0")/lib.sh"
gate_main "$1" "$2" build test proof qemu
