#!/usr/bin/env bash
# prover gate: build, then the PRISM proof check.
. "$(dirname "$0")/lib.sh"
gate_main "$1" "$2" build proof
