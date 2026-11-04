#!/usr/bin/env bash
# prover gate: build, then the tiered proof check (proof_check.sh; tier from the task).
. "$(dirname "$0")/lib.sh"
gate_main "$1" "$2" build proof
