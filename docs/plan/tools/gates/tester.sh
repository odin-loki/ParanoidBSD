#!/usr/bin/env bash
# tester gate: build, then unit and fault-injection tests.
. "$(dirname "$0")/lib.sh"
gate_main "$1" "$2" build test
