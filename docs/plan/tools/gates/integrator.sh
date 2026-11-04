#!/usr/bin/env bash
# integrator gate: full build, tests, proofs and a QEMU boot of the merged tree.
. "$(dirname "$0")/lib.sh"
gate_main "$1" "$2" build test proof qemu
