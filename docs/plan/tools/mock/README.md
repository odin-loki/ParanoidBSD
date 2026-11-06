# Mock pilot

Runs the real runner, gate scripts and PRISM on 17 real P0-02 tasks (IF, three units with SPEC/IMPL/TEST/PROOF/REVIEW at T2/T3/T4, INT)
against a scripted OpenAI-compatible model (`mock_model.py`), in a scratch repo under `/tmp/mock`. No API key or spend.
Needs a built `pbsd_run` (`~/pbsd-run-build/pbsd_run`, built with `-DPBSD_STATIC=OFF`), PRISM and its analyzers (`tools/cloud/build-prism.sh`), jq, python3.

    tools/mock/setup.sh                      # scratch repo, mini task list, config pointing at the mock model
    tools/mock/run.sh P0-02-U002-IMPL        # run; the named IMPL task is broken once, so the build gate rejects it and the retry fixes it
    tools/mock/neg_proof_gate.sh             # proof_check.sh against cheating cases (assume macro, code change, signed overflow, ...)

6 Nov 2026: 17 of 17 tasks pass, and `neg_proof_gate.sh` shows the gate accepting the genuine proof and rejecting every cheat (assume macro, code change, vacuous `requires`, signed overflow, empty obligations); exit 3 for a missing tier or PRISM.
