"""Per-task harness stub used by pbsd_run.py.
# DEPRECATED: replaced by the C++ runner in docs/plan/tools/runner (pbsd_run). Kept for reference only.

run_task(task, api, run_job) -> (result, usage_list)
  result: "passed" | "failed" | "crash"; usage_list: [(usage_dict, was_peak), ...]
It creates the tmpfs sparse worktree for task["owns"], builds the layered prompt
(global, role, package contracts, unit source, tail) from AGENTS.md section 6,
loops model turns with tools (read, write, run gate), commits to t/<id> and a
wip/<id> checkpoint every 10 minutes, and runs the acceptance gate in task["accept"]
(including fault-injection tests) via run_job. Wire it to the repo's pbsd.py.
"""
def run_task(task, api, run_job):
    raise NotImplementedError("connect to pbsd.py agent loop; see AGENTS.md section 5")
