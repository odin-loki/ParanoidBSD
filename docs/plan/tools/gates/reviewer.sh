#!/usr/bin/env bash
# reviewer gate: the reviewer approves only a tree that builds and passes its tests.
. "$(dirname "$0")/lib.sh"
gate_main "$1" "$2" build test
