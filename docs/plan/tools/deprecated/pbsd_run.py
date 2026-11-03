#!/usr/bin/env python3
# DEPRECATED: replaced by the C++ runner in docs/plan/tools/runner (pbsd_run). Kept for reference only.
"""PBSD runner and scheduler for one PC (docs/plan/AGENTS.md section 8).

Loads docs/plan/tasks.json into a SQLite task database (WAL), leases ready
tasks to DeepSeek Flash agents, runs build/proof/QEMU jobs in NUMA-pinned
slots, throttles on CPU/RAM/IO pressure, pauses API calls at peak hours, and
shows a live progress bar with ETA and cost so far.

  pbsd_run.py init            # create state DB from tasks.json (idempotent)
  pbsd_run.py run             # resume or start the run
  pbsd_run.py status          # one-shot progress report
Environment: DEEPSEEK_API_KEY (and optional DEEPSEEK_API_KEY_2), PBSD_STATE (default ~/.pbsd).
"""
import json, os, sys, time, sqlite3, random, signal, shutil, subprocess, threading, datetime as dt
from collections import deque
from urllib import request, error

ROOT = os.path.dirname(os.path.abspath(__file__))
TASKS = os.path.join(ROOT, "..", "tasks.json")
STATE = os.path.expanduser(os.environ.get("PBSD_STATE", "~/.pbsd"))
DB = os.path.join(STATE, "tasks.db")
API = "https://api.deepseek.com/chat/completions"
MODEL = "deepseek-flash"
# USD per 1M tokens, off-peak (peak is double)
PRICE = {"hit": 0.003, "miss": 0.15, "out": 0.60}
PEAK_UTC = [(1, 4), (6, 10)]          # weekdays; 12-15 and 17-21 Sydney (AEDT)

# Slots: (count, cores each, RAM GB each, NUMA node). Edit to suit the machine.
SLOTS = {
    "agent":   (48, 0, 0.3, None),    # waits on the API; negligible CPU
    "compile": (12, 1, 2.0, 0),
    "qemu":    (3,  2, 4.0, 0),
    "proof":   (6,  2, 12.0, 1),
}
LIMITS = {"load": 30, "ram_admit": 0.85, "ram_pause": 0.92, "io_some_avg10": 40.0}
WATCHDOG = {"agent": 20 * 60, "compile": 30 * 60, "qemu": 60 * 60}  # proof uses its task budget
LEASE = 45 * 60
STATES = ["blocked", "ready", "leased", "submitted", "passed", "failed", "escalated", "quarantined"]

# ---------------------------------------------------------------- database
def db():
    c = sqlite3.connect(DB, timeout=60, isolation_level=None)
    c.execute("PRAGMA journal_mode=WAL"); c.execute("PRAGMA synchronous=FULL")
    return c

def init():
    os.makedirs(STATE, exist_ok=True)
    c = db()
    c.executescript("""
    CREATE TABLE IF NOT EXISTS task(id TEXT PRIMARY KEY, wp TEXT, phase INT, role TEXT,
      priority INT, state TEXT, attempts INT DEFAULT 0, crashes INT DEFAULT 0,
      lease_until REAL, started REAL, finished REAL, body TEXT);
    CREATE TABLE IF NOT EXISTS dep(task TEXT, dep TEXT);
    CREATE TABLE IF NOT EXISTS event(ts REAL, task TEXT, kind TEXT, data TEXT);
    CREATE TABLE IF NOT EXISTS cost(ts REAL, task TEXT, hit INT, miss INT, out INT, usd REAL);
    CREATE INDEX IF NOT EXISTS ix_state ON task(state, priority, phase);
    CREATE INDEX IF NOT EXISTS ix_dep ON dep(task);
    """)
    tasks = json.load(open(TASKS))
    c.execute("BEGIN")
    for t in tasks:
        pri = 0 if t.get("month1", True) else 1          # month1 is ordering only
        c.execute("INSERT OR IGNORE INTO task(id,wp,phase,role,priority,state,body) VALUES(?,?,?,?,?,?,?)",
                  (t["id"], t["wp"], int(str(t["phase"]).lstrip("P") or 0), t["role"], pri,
                   "blocked" if t["deps"] else "ready", json.dumps(t)))
        for d in t["deps"]:
            c.execute("INSERT INTO dep SELECT ?,? WHERE NOT EXISTS(SELECT 1 FROM dep WHERE task=? AND dep=?)",
                      (t["id"], d, t["id"], d))
    c.execute("COMMIT")
    print(f"{len(tasks)} tasks in {DB}")

def log(c, task, kind, data=""):
    c.execute("INSERT INTO event VALUES(?,?,?,?)", (time.time(), task, kind, json.dumps(data)))

def recover(c):
    """Resume after crash, power loss or OOM: expired leases go back to ready."""
    n = c.execute("UPDATE task SET state='ready' WHERE state='leased' AND lease_until<?", (time.time(),)).rowcount
    c.execute("UPDATE task SET state='ready' WHERE state='submitted'")   # gates are idempotent: rerun
    if n: log(c, None, "recovered", n)
    ok = c.execute("PRAGMA integrity_check").fetchone()[0]
    if ok != "ok": sys.exit(f"task DB integrity check failed: {ok}")

def unblock(c):
    c.execute("""UPDATE task SET state='ready' WHERE state='blocked' AND NOT EXISTS(
      SELECT 1 FROM dep JOIN task d ON d.id=dep.dep WHERE dep.task=task.id AND d.state NOT IN ('passed'))""")

# ---------------------------------------------------------------- machine pressure
def meminfo():
    m = {l.split(":")[0]: int(l.split()[1]) for l in open("/proc/meminfo")}
    return 1 - m["MemAvailable"] / m["MemTotal"]

def io_pressure():
    try:
        return float(open("/proc/pressure/io").read().split()[1].split("=")[1])
    except Exception:
        return 0.0

def peak(now=None):
    now = now or dt.datetime.now(dt.timezone.utc)
    return now.weekday() < 5 and any(a <= now.hour < b for a, b in PEAK_UTC)

def can_admit(kind):
    if kind == "agent" and peak(): return False
    return (os.getloadavg()[0] < LIMITS["load"] and meminfo() < LIMITS["ram_admit"]
            and io_pressure() < LIMITS["io_some_avg10"])

# ---------------------------------------------------------------- API client
class Api:
    def __init__(s):
        s.keys = [k for k in (os.environ.get("DEEPSEEK_API_KEY"), os.environ.get("DEEPSEEK_API_KEY_2")) if k]
        s.fails = deque(); s.open_until = 0; s.lock = threading.Lock()
    def call(s, messages, tools=None):
        for attempt in range(8):
            while time.time() < s.open_until: time.sleep(5)
            body = {"model": MODEL, "messages": messages}
            if tools: body["tools"] = tools
            req = request.Request(API, json.dumps(body).encode(), {"Content-Type": "application/json",
                                  "Authorization": "Bearer " + s.keys[0]})
            try:
                return json.load(request.urlopen(req, timeout=900))
            except error.HTTPError as e:
                if e.code == 402: raise SystemExit("API balance exhausted; run stopped cleanly")
                if e.code == 401 and len(s.keys) > 1: s.keys.pop(0); continue
                wait = float(e.headers.get("Retry-After") or 0)
            except Exception:
                wait = 0
            with s.lock:
                s.fails.append(time.time())
                while s.fails and s.fails[0] < time.time() - 60: s.fails.popleft()
                if len(s.fails) > 20: s.open_until = time.time() + 300   # circuit breaker
            time.sleep(max(wait, min(600, 2 ** attempt) * (0.5 + random.random())))
        raise RuntimeError("API failed after 8 tries")

def usd(u, is_peak):
    hit = u.get("prompt_cache_hit_tokens", 0); miss = u.get("prompt_cache_miss_tokens", u.get("prompt_tokens", 0) - hit)
    out = u.get("completion_tokens", 0); m = 2 if is_peak else 1
    return hit, miss, out, m * (hit * PRICE["hit"] + miss * PRICE["miss"] + out * PRICE["out"]) / 1e6

# ---------------------------------------------------------------- jobs
def numa(cmd, node):
    return (["numactl", f"--cpunodebind={node}", f"--membind={node}"] + cmd) if node is not None and shutil.which("numactl") else cmd

def run_job(kind, cmd, timeout, cwd=None):
    """Run a build/test/proof/QEMU job pinned to its NUMA node; kill on watchdog timeout."""
    node = SLOTS[kind][3]
    p = subprocess.Popen(numa(cmd, node), cwd=cwd, start_new_session=True,
                         stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
                         preexec_fn=lambda: open("/proc/self/oom_score_adj", "w").write("500"))
    try:
        out, _ = p.communicate(timeout=timeout); return p.returncode, out[-20000:]
    except subprocess.TimeoutExpired:
        os.killpg(p.pid, signal.SIGKILL); return 124, "watchdog timeout"

def work(task, api):
    """One task turn loop. The harness (pbsd.py in the repo) supplies tools for
    reading/writing the task's tmpfs worktree and calling run_job for gates."""
    from pbsd_harness import run_task   # role prompts and gates from AGENTS.md sections 5-6
    return run_task(task, api, run_job)

# ---------------------------------------------------------------- progress
class Progress:
    def __init__(s, c):
        s.c = c; s.done_times = deque(maxlen=500); s.t0 = time.time()
    def snapshot(s):
        c = s.c
        tot = dict(c.execute("SELECT state,COUNT(*) FROM task GROUP BY state").fetchall())
        ph = c.execute("SELECT phase,COUNT(*),SUM(state='passed') FROM task GROUP BY phase ORDER BY phase").fetchall()
        cost = c.execute("SELECT COALESCE(SUM(usd),0) FROM cost").fetchone()[0]
        rate = c.execute("SELECT COUNT(*) FROM event WHERE kind='passed' AND ts>?", (time.time() - 86400,)).fetchone()[0]
        return tot, ph, cost, rate
    @staticmethod
    def bar(frac, width=40):
        n = int(frac * width); return "█" * n + "░" * (width - n)
    def render(s, running=None):
        tot, ph, cost, rate24 = s.snapshot()
        all_ = sum(tot.values()); done = tot.get("passed", 0)
        left = all_ - done - tot.get("escalated", 0) - tot.get("quarantined", 0)
        # rolling throughput: tasks passed in the last 24 h, else since start
        rate = rate24 / 24 if rate24 else done / max(1e-9, (time.time() - s.t0) / 3600)
        eta = left / rate if rate > 0 else float("inf")
        spend_rate = cost / max(done, 1)
        lines = [f"PBSD  {s.bar(done / all_)} {done:,}/{all_:,} ({100 * done / all_:.1f}%)",
                 f"  running {tot.get('leased', 0) + tot.get('submitted', 0):,}  queued {tot.get('ready', 0):,}  "
                 f"blocked {tot.get('blocked', 0):,}  failed {tot.get('failed', 0):,}  "
                 f"escalated {tot.get('escalated', 0):,}  quarantined {tot.get('quarantined', 0):,}",
                 f"  throughput {rate:.1f} tasks/h   time left {fmt(eta)}   ETA {eta_clock(eta)}",
                 f"  cost so far ${cost:,.2f}   projected total ${cost + spend_rate * left:,.0f}"
                 + ("   [API paused: peak hours]" if peak() else "")]
        for p, n, d in ph:
            lines.append(f"  P{p} {s.bar((d or 0) / n, 30)} {d or 0:,}/{n:,}")
        if running: lines.append("  slots " + "  ".join(f"{k} {v}/{SLOTS[k][0]}" for k, v in running.items()))
        return "\n".join(lines)

def fmt(h):
    if h == float("inf"): return "unknown (no throughput yet)"
    d, r = divmod(int(h * 60), 1440); return f"{d}d {r // 60}h {r % 60}m"

def eta_clock(h):
    if h == float("inf"): return "-"
    return (dt.datetime.now() + dt.timedelta(hours=h)).strftime("%a %d %b %H:%M %Z").strip()

# ---------------------------------------------------------------- scheduler
def lease(c, n):
    c.execute("BEGIN IMMEDIATE")
    rows = c.execute("SELECT id,body FROM task WHERE state='ready' ORDER BY priority,phase,id LIMIT ?", (n,)).fetchall()
    for tid, _ in rows:
        c.execute("UPDATE task SET state='leased',lease_until=?,started=?,attempts=attempts+1 WHERE id=?",
                  (time.time() + LEASE, time.time(), tid)); log(c, tid, "leased")
    c.execute("COMMIT"); return [json.loads(b) for _, b in rows]

def finish(c, tid, result):
    c.execute("BEGIN IMMEDIATE")
    att, crashes = c.execute("SELECT attempts,crashes FROM task WHERE id=?", (tid,)).fetchone()
    if result == "passed": st = "passed"
    elif result == "crash":
        crashes += 1; st = "quarantined" if crashes >= 3 else "ready"
    else: st = "escalated" if att >= 8 else "ready"     # fresh context at 3, split at 5 (harness)
    c.execute("UPDATE task SET state=?,crashes=?,finished=? WHERE id=?", (st, crashes, time.time(), tid))
    log(c, tid, st); c.execute("COMMIT")

def run():
    os.makedirs(STATE, exist_ok=True)
    c = db(); recover(c); api = Api(); prog = Progress(c)
    running = {k: 0 for k in SLOTS}; lock = threading.Lock(); tty = sys.stdout.isatty()

    def worker(t):
        c2 = db()
        try:
            res, usage = work(t, api)
            for u, pk in usage:
                hit, miss, out, cost = usd(u, pk)
                c2.execute("INSERT INTO cost VALUES(?,?,?,?,?,?)", (time.time(), t["id"], hit, miss, out, cost))
        except SystemExit: raise
        except MemoryError: res = "crash"
        except Exception as e: log(c2, t["id"], "error", repr(e)); res = "crash"
        finish(c2, t["id"], res)
        with lock: running["agent"] -= 1

    last = 0
    while True:
        unblock(c)
        if meminfo() > LIMITS["ram_pause"]: time.sleep(10); continue
        free = SLOTS["agent"][0] - running["agent"]
        if free > 0 and can_admit("agent"):
            for t in lease(c, free):
                with lock: running["agent"] += 1
                threading.Thread(target=worker, args=(t,), daemon=True).start()
        if time.time() - last > 2:
            out = prog.render(running); last = time.time()
            if tty: sys.stdout.write("\033[H\033[J" + out + "\n")
            else: print(out, flush=True)
        left = c.execute("SELECT COUNT(*) FROM task WHERE state NOT IN ('passed','escalated','quarantined')").fetchone()[0]
        if not left and not running["agent"]: print(prog.render()); print("run complete"); return
        time.sleep(1)

if __name__ == "__main__":
    cmd = sys.argv[1] if len(sys.argv) > 1 else "status"
    if cmd == "init": init()
    elif cmd == "run": init(); run()
    elif cmd == "status": print(Progress(db()).render())
    else: sys.exit(__doc__)
