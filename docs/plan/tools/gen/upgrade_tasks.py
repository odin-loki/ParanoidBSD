#!/usr/bin/env python3
"""Upgrade docs/plan/tasks.json and tasks.yaml in place (idempotent).

What it changes, and why (AGENTS.md section 1):
- owns becomes a list, and each unit task owns its own paths, so parallel tasks never write the
  same file: SPEC docs/plan/specs/<WP>/<U>.md, IMPL <package dir>/<unit>/, TEST tests/<WP>/<U>/,
  PROOF proofs/<WP>/<U>/ plus annotation-only edits to the unit's code.
- Each package merge (<WP>-INT) also waits for the merges of the packages it really depends on
  (PBSD_PLAN.md "Depends on"), so integration follows the real order instead of all at P6-01.
- PRISM is replaced by a tiered proof stack (proof_tier T0..T4); the core packages get a
  -MODEL task (Lean 4 or TLA+) whose locked theorem statements are written by the -IF task.
- turns_budget and max_turns reflect agent request counts (one turn = one API request).
- deps_one_day (Appendix A's flattened graph) is kept; the new -MODEL tasks get the same deps there.
"""
import json, re, sys, math, pathlib

HERE = pathlib.Path(__file__).resolve().parents[2]          # docs/plan
TASKS, YAML, PLAN = HERE / "tasks.json", HERE / "tasks.yaml", HERE / "PBSD_PLAN.md"

# Requests per task (mid case), by role or proof tier. AGENTS.md section 8.1 explains them.
TURNS = {"decomposer": 12, "decomposer-IF": 20, "implementer": 30, "tester": 25, "reviewer": 10,
         "integrator": 60, "P0-FREEZE": 80, "P6-01-INT": 200,
         "T0": 6, "T1": 400, "T2": 150, "T3": 50, "T4": 20}
PROOF_MIN = {"T0": 10, "T1": 30, "T2": 120, "T3": 30, "T4": 15}   # watchdog per checker run, minutes
MODEL_TOOL = {"P1-06": "TLA+ (Apalache inductive invariants)", "P2-01": "TLA+ (Apalache) for the ring protocol and Lean 4 for index arithmetic"}

FIX_OWNS = {   # free-text owns in the package list -> paths
    "CMake/ninja under pbsd/": ["pbsd/CMakeLists.txt", "pbsd/cmake/"],
    "toolchain flags": ["pbsd/cmake/toolchain/"],
    "pbsd/kernel/drivers/<class>/ (one package per class)": ["pbsd/kernel/drivers/"],
    "pbsd/userland": ["pbsd/userland/"], "pbsd/lib": ["pbsd/lib/"], "pbsd/bin…": ["pbsd/bin/"],
    "pbsd/bifrost": ["pbsd/bifrost/"], "pbsd/compositor": ["pbsd/compositor/"], "pbsd/theme": ["pbsd/theme/"],
    "branch pbsd-cutover": ["pbsd/", "tests/", "proofs/", "fragments/"],
    "CODEOWNERS": ["CODEOWNERS"],
}

def split_owns(s):
    if isinstance(s, list): return s
    out, cur, depth = [], "", 0
    for ch in s:
        depth += ch == "{"; depth -= ch == "}"
        if ch == "," and depth == 0: out.append(cur); cur = ""
        else: cur += ch
    out.append(cur)
    res = []
    for o in (x.strip() for x in out):
        if not o or o == "-": continue
        if o in FIX_OWNS: res += FIX_OWNS[o]; continue
        m = re.match(r"^(.*)\{([^}]*)\}(.*)$", o)
        res += [m.group(1) + p.strip() + m.group(3) for p in m.group(2).split(",")] if m else [o]
    return res

def package_deps():
    deps, cur = {}, None
    for line in PLAN.read_text().splitlines():
        m = re.match(r"^#### (P\d-\d\d) ", line)
        if m: cur = m.group(1)
        m = re.match(r"^- Depends on: (.*)$", line)
        if m and cur:
            deps[cur] = re.findall(r"P\d-\d\d", m.group(1)) if "all" not in m.group(1) else ["*"]
    return deps

def proof_tier(wp, src):
    src = src or ""
    if wp in ("P0-07", "P0-08", "P0-01"): return "T0"
    if wp in ("P0-02", "P0-06", "P1-02", "P1-03", "P1-04", "P4-01"): return "T2"
    if wp == "P1-05": return "T2" if "fair.c" in src else "T3"
    if wp == "P2-01": return "T2" if "io_uring.c" in src else "T3"
    if wp == "P2-03":
        if re.search(r"vfs_lookup|vfs_cache", src): return "T2"
        return "T3" if src.startswith("hbsd/src/sys/kern/") else "T4"
    if wp == "P3-02": return "T2" if re.search(r"iommu_gas|busdma_iommu|iommu/iommu\.c", src) else "T3"
    if wp in ("P3-05", "P3-06", "P4-04", "P4-06", "P5-02"): return "T4"
    if wp in ("P2-04", "P3-03"): return "T4" if src.startswith("linux:") else "T3"
    return "T3"

PROOF_ACCEPT = {
    "T0": "Static checks only (static_assert/consteval, dialect lint); reported as TESTED, never as proved.",
    "T1": "Every theorem in the package's locked statement files (proofs/{wp}/spec/, hash-pinned at P0-FREEZE) is proved with {tool}; "
          "Lean: no sorry/admit/axiom/native_decide/implemented_by/extern, #print axioms lists only propext, Classical.choice and Quot.sound, "
          "and the comparator confirms each proved statement is byte-identical to the locked one. Reported as PROVED.",
    "T2": "Every obligation in the unit's obligation list (the spec's contracts, plus the package model's refinement post-condition "
          "abs(s') == step(abs(s), op)) is verified by ESBMC in contract or k-induction mode with the locked flags; no __ESBMC_assume / "
          "__CPROVER_assume outside the locked harness skeleton; the vacuity check (assert(false) after the preconditions) fails as it must; "
          "zero-VC results rejected; annotation-only diff against the reviewed implementation. Reported as PROVED. An obligation that "
          "stays open is escalated, never turned into a runtime check.",
    "T3": "Memory safety, integer overflow and the unit's contracts verified by ESBMC (or CBMC for C files) with unwinding assertions "
          "and the locked flags; concurrency code also checked with GenMC (RC11/LKMM, 2-4 threads); every function the checker could not "
          "reach is listed; annotation-only diff. Reported as BOUNDED(k).",
    "T4": "A CBMC/ESBMC harness for every gateway-facing function of the unit (memory safety, overflow and the gateway contract) with "
          "unwinding assertions; matching runtime gateway checks generated; reported as BOUNDED(k) plus RUNTIME, never as proved.",
}

def accept_fix(s):
    s = s.replace("contracts parse in PRISM", "contracts expand in the runtime build (GCC 16 -fcontracts) and the verification build (ESBMC)")
    s = s.replace("PRISM parses contracts", "contracts expand in the runtime and verification builds")
    s = s.replace("PRISM parses every contract form", "every contract form expands in the runtime build and the verification build")
    s = s.replace("TLA+/PRISM model", "TLA+ model (Apalache inductive invariant)")
    s = re.sub(r"PRISM proves? ", "machine-checked proof (package model in Lean 4/TLA+, code by ESBMC contracts) that ", s)
    s = s.replace("PRISM invariant:", "Machine-checked invariant (Lean model + ESBMC on every PTE writer):")
    s = s.replace("PRISM proof obligations closed for every verified-core TU",
                  "every T1/T2 obligation PROVED and every T3/T4 result recorded as BOUNDED or RUNTIME in the proof report")
    s = s.replace("dispatcher contract verified in PRISM", "dispatcher totality and uniqueness checked at compile time (static_assert) and by ESBMC")
    s = s.replace("PRISM job green on P0 headers", "proof job (Lean, ESBMC, CBMC, GenMC) green on P0 headers")
    s = s.replace("Second Flash agent approves", "An independent reviewer agent approves")
    s = s.replace("PRISM", "the proof stack")
    return s

def slug(src, used):
    src = (src or "unit").split(" part ")[0]
    path = re.sub(r"^[a-z ]+:", "", src)
    comps = [c for c in re.split(r"[/,]\s*", path) if c]
    for n in range(1, len(comps) + 1):
        s = "_".join(re.sub(r"\.[A-Za-z]+$", "", c) for c in comps[-n:])
        s = re.sub(r"[^A-Za-z0-9_]+", "_", s).strip("_").lower() or "unit"
        if s not in used: return s
    return s

def main():
    tasks = json.loads(TASKS.read_text())
    if any(t["id"].endswith("-MODEL") for t in tasks): print("already upgraded"); return 0
    by_id = {t["id"]: t for t in tasks}
    pdeps = package_deps()
    pkg_owns = {t["wp"]: split_owns(t["owns"]) for t in tasks if t["id"].endswith("-IF")}
    # Unit slugs and directories, from the IMPL task of each unit.
    unit_dir, used = {}, {}
    for t in tasks:
        if t["role"] != "implementer": continue
        u = t["id"].rsplit("-", 1)[0]; wp = t["wp"]
        base = next((o for o in pkg_owns[wp] if o.endswith("/") and o.startswith(("pbsd/", "kde/"))), pkg_owns[wp][0])
        src = t.get("source") or ""
        part = re.search(r" part (\d+)/(\d+)$", src)
        used.setdefault(wp, set())
        s = slug(src, used[wp] if not part else set())
        if part: s += "_p%02d" % int(part.group(1))
        while s in used[wp]: s += "_"
        used[wp].add(s)
        unit_dir[u] = base.rstrip("/") + "/" + s + "/"
    model_wps = sorted({t["wp"] for t in tasks if t["role"] == "prover" and proof_tier(t["wp"], t.get("source")) == "T2"} | {"P1-06", "P4-07"})
    out = []
    for t in tasks:
        t = dict(t)
        wp, role, tid = t["wp"], t["role"], t["id"]
        u = tid.rsplit("-", 1)[0]
        kind = tid.rsplit("-", 1)[1]
        t["accept"] = accept_fix(t["accept"]); t["title"] = t["title"].replace("PRISM", "proof")
        if tid == "P0-FREEZE":
            t["owns"] = ["pbsd/kernel/include/", "proofs/spec-lock.json"]
            t["accept"] += "; every locked Lean/TLA+ statement file under proofs/*/spec/ is hash-pinned in proofs/spec-lock.json"
        elif kind == "IF":
            t["owns"] = pkg_owns[wp] + ["docs/plan/specs/%s/IF.md" % wp] + (["proofs/%s/spec/" % wp] if wp in model_wps else [])
            if wp in model_wps:
                t["accept"] += ("; the package's theorem statements are written in proofs/%s/spec/ (definitions and statements only, "
                                "no proofs) for the -MODEL task" % wp)
        elif kind == "SPEC": t["owns"] = ["docs/plan/specs/%s/%s.md" % (wp, u.split("-")[-1])]
        elif kind == "IMPL": t["owns"] = [unit_dir[u], "fragments/%s/%s/" % (wp, u.split("-")[-1])]
        elif kind == "TEST": t["owns"] = ["tests/%s/%s/" % (wp, u.split("-")[-1])]
        elif kind == "PROOF":
            tier = proof_tier(wp, by_id[u + "-IMPL"].get("source"))
            t["proof_tier"] = tier; t["proof_mode"] = {"T0": "static", "T2": "full", "T3": "bounded", "T4": "gateway"}[tier]
            t["proof_budget_minutes"] = PROOF_MIN[tier]
            t["owns"] = ["proofs/%s/%s/" % (wp, u.split("-")[-1]), unit_dir[u]]
            t["accept"] = PROOF_ACCEPT[tier].format(wp=wp, tool="")
            t["title"] = {"T0": "Static checks", "T2": "Prove", "T3": "Bounded checks", "T4": "Gateway harnesses"}[tier] + \
                         " for " + re.sub(r"^Discharge (the )?proof obligations for ", "", t["title"])
            if tier == "T2" and wp in model_wps:
                t["deps"] = t["deps"] + [wp + "-MODEL"]
                if "deps_one_day" in t: t["deps_one_day"] = t["deps_one_day"] + [wp + "-MODEL"]
        elif kind == "REVIEW": t["owns"] = []
        elif kind == "INT":
            t["owns"] = pkg_owns[wp] + ["tests/%s/" % wp, "proofs/%s/" % wp, "fragments/%s/" % wp] if wp != "P6-01" else FIX_OWNS["branch pbsd-cutover"]
            up = pdeps.get(wp, [])
            if up != ["*"]: t["deps"] = t["deps"] + [d + "-INT" for d in up if d + "-INT" in by_id and d + "-INT" not in t["deps"]]
            t["accept"] += ("; acceptance criteria that need a package not yet integrated are recorded as deferred and run in P6-01-INT")
        else: t["owns"] = split_owns(t["owns"])
        t["owns"] = [("tools/proof/" if o == "tools/prism/" else o) for o in t["owns"]]
        for k in ("source", "title"):
            if isinstance(t.get(k), str): t[k] = t[k].replace("Build, CI, PRISM pipeline", "Build, CI, proof pipeline")
        if "reuse" in t: t["reuse"] = json.loads(json.dumps(t["reuse"]).replace("PRISM boundary spec generator", "gateway contract-to-check generator"))
        key = tid if tid in TURNS else ("decomposer-IF" if kind == "IF" else t.get("proof_tier", role))
        t["turns_budget"] = TURNS.get(key, TURNS.get(role, 30))
        t["max_turns"] = int(math.ceil(t["turns_budget"] * 2))
        out.append(t)
    # Package models (T1) for the verified core.
    for wp in model_wps:
        name = next(t["title"] for t in tasks if t["id"] == wp + "-IF").replace("Write contract headers and stubs for ", "")
        tool = MODEL_TOOL.get(wp, "Lean 4")
        out.append({"id": wp + "-MODEL", "wp": wp, "phase": int(wp[1]), "role": "prover",
                    "title": "Prove the package model theorems for " + name, "source": None,
                    "owns": ["proofs/%s/model/" % wp], "deps": [wp + "-IF", "P0-FREEZE"], "deps_one_day": [wp + "-IF", "P0-FREEZE"],
                    "accept": PROOF_ACCEPT["T1"].format(wp=wp, tool=tool), "proof_tier": "T1", "proof_mode": "model",
                    "proof_budget_minutes": PROOF_MIN["T1"], "turns_budget": TURNS["T1"], "max_turns": TURNS["T1"] * 2,
                    "fault_tolerance": "n/a (model)", "month1": True, "idempotent": True, "priority": 0})
    TASKS.write_text(json.dumps(out, indent=1, ensure_ascii=False) + "\n")
    try:
        import yaml
        YAML.write_text(yaml.safe_dump(out, sort_keys=False, allow_unicode=True, width=1000))
    except ImportError:
        print("PyYAML missing: tasks.yaml not rewritten", file=sys.stderr)
    tiers = {}
    for t in out:
        if t.get("proof_tier"): tiers[t["proof_tier"]] = tiers.get(t["proof_tier"], 0) + 1
    print(len(out), "tasks; proof tiers", dict(sorted(tiers.items())), "; models", model_wps)
    return 0

if __name__ == "__main__":
    sys.exit(main())
