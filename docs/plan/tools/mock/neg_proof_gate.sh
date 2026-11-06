G=/tmp/mock/gates/proof_check.sh; rm -rf /tmp/neg; git clone -q /tmp/mock/repo /tmp/neg; cd /tmp/neg; git config user.email m@m; git config user.name m
git checkout -q --detach origin/t/P0-02-U001-PROOF; ORIG=$(git rev-parse HEAD); BASE=$(git rev-parse $(git log --diff-filter=A --format=%H -- proofs/P0-02/U001/obligations.json | tail -1)^)
export PRISM_BIN=$HOME/PRISM/build/prism PRISM_TOOLS_DIR=$HOME/.prism/tools PBSD_PRISM_UNWIND=8 PBSD_PROOF_TIMEOUT_S=300
U=pbsd/kernel/include/cap/sys_capability/unit.cpp
mk() { git reset -q --hard $ORIG; "$@"; git add -A; git commit -q -m "gate

PBSD-Base: $BASE"; }
run() { PBSD_PROOF_TIER=$2 bash $G P0-02-U001-PROOF /tmp/neg > /tmp/neg.out 2>&1; rc=$?; printf '%-36s tier=%s rc=%s  %s\n' "$1" "$2" $rc "$(grep -E '^(FAIL|infrastructure|PBSD-RESULT|T[234]:)' /tmp/neg.out | head -1 | cut -c1-120)"; }
e_none() { :; }
e_assume() { echo '#define A __ESBMC_assume(1)' >> $U; }
e_code() { sed -i "s/255u/127u/" $U; }
e_ann() { sed -i 's/requires x >= 0;/requires x >= 0 \&\& x < 100;/' $U; }
e_signed() { printf '%s\n' '/*@ requires a < 5 && b < 5; ensures result == a + b; */' 'int f_u001(int a, int b) { return a + b; }' > $U; }
e_empty() { echo "[]" > proofs/P0-02/U001/obligations.json; }
e_vac() { sed -i 's/requires x >= 0;/requires x >= 1 \&\& x < 1;/' $U; }
mk e_ann;    run "genuine proof (annotation-only change)" T2
mk e_assume; run "assume macro added" T2
mk e_code;   run "code changed, not annotation" T2
mk e_vac;    run "vacuous requires (x>=1 && x<1)" T2
mk e_signed; run "signed overflow" T4
mk e_empty;  run "empty obligation list" T2
mk e_none; env -u PBSD_PROOF_TIER bash $G P0-02-U001-PROOF /tmp/neg > /tmp/neg.out 2>&1; echo "no tier from runner                  rc=$? $(head -1 /tmp/neg.out)"
mk e_none; PRISM_BIN=/nonexistent PBSD_PROOF_TIER=T2 bash $G P0-02-U001-PROOF /tmp/neg > /tmp/neg.out 2>&1; echo "PRISM missing                        rc=$? $(head -1 /tmp/neg.out)"
mk e_none; PBSD_PROOF_TIER=T1 bash $G P0-02-U001-PROOF /tmp/neg > /tmp/neg.out 2>&1; echo "T1 with no model dir                 rc=$? $(head -1 /tmp/neg.out)"
mk e_none; PBSD_PROOF_TIER=T0 bash $G P0-02-U001-PROOF /tmp/neg > /tmp/neg.out 2>&1; echo "T0                                   rc=$? $(head -1 /tmp/neg.out)"
