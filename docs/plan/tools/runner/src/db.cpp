#include "common.hpp"
#include <stdexcept>

namespace pbsd {
namespace {
struct Stmt {
  sqlite3_stmt* s = nullptr;
  Stmt(sqlite3* h, const char* sql) { if (sqlite3_prepare_v2(h, sql, -1, &s, nullptr) != SQLITE_OK) throw std::runtime_error(sqlite3_errmsg(h)); }
  ~Stmt() { sqlite3_finalize(s); }
  Stmt& b(int i, const std::string& v) { sqlite3_bind_text(s, i, v.c_str(), -1, SQLITE_TRANSIENT); return *this; }
  Stmt& b(int i, double v) { sqlite3_bind_double(s, i, v); return *this; }
  Stmt& b(int i, int64_t v) { sqlite3_bind_int64(s, i, v); return *this; }
  bool step() { int r = sqlite3_step(s); if (r == SQLITE_ROW) return true; if (r == SQLITE_DONE) return false; throw std::runtime_error(sqlite3_errmsg(sqlite3_db_handle(s))); }
  void reset() { sqlite3_reset(s); sqlite3_clear_bindings(s); }
  int64_t i(int c) { return sqlite3_column_int64(s, c); }
  double d(int c) { return sqlite3_column_double(s, c); }
  std::string t(int c) { auto p = sqlite3_column_text(s, c); return p ? reinterpret_cast<const char*>(p) : ""; }
};
}  // namespace

Db::Db(const std::string& path) {
  if (sqlite3_open(path.c_str(), &h_) != SQLITE_OK) throw std::runtime_error("cannot open " + path);
  sqlite3_busy_timeout(h_, 60000);
  exec("PRAGMA journal_mode=WAL"); exec("PRAGMA synchronous=FULL"); exec("PRAGMA foreign_keys=ON");
}
Db::~Db() { sqlite3_close(h_); }
void Db::exec(const std::string& sql) {
  char* e = nullptr;
  if (sqlite3_exec(h_, sql.c_str(), nullptr, nullptr, &e) != SQLITE_OK) { std::string m = e ? e : "?"; sqlite3_free(e); throw std::runtime_error(m); }
}
void Db::init_schema() {
  exec(R"(CREATE TABLE IF NOT EXISTS task(id TEXT PRIMARY KEY, wp TEXT, phase INT, role TEXT, priority INT,
      state TEXT, attempts INT DEFAULT 0, crashes INT DEFAULT 0, lease_until REAL, started REAL, finished REAL, body TEXT);
    CREATE TABLE IF NOT EXISTS dep(task TEXT, dep TEXT, PRIMARY KEY(task, dep));
    CREATE TABLE IF NOT EXISTS event(ts REAL, task TEXT, kind TEXT, data TEXT);
    CREATE TABLE IF NOT EXISTS cost(ts REAL, task TEXT, hit INT, miss INT, out INT, usd REAL);
    CREATE INDEX IF NOT EXISTS ix_state ON task(state, priority, phase);
    CREATE INDEX IF NOT EXISTS ix_dep ON dep(dep);
    CREATE INDEX IF NOT EXISTS ix_ev ON event(kind, ts);)");
}
static int phase_num(const json& p) {
  if (p.is_number()) return p.get<int>();
  std::string s = p.get<std::string>(); while (!s.empty() && !isdigit(s[0])) s.erase(0, 1);
  return s.empty() ? 0 : std::stoi(s);
}
size_t Db::load_tasks(const json& tasks) {
  std::lock_guard g(m_);
  exec("BEGIN IMMEDIATE");
  Stmt ins(h_, "INSERT OR IGNORE INTO task(id,wp,phase,role,priority,state,body) VALUES(?,?,?,?,?,?,?)");
  Stmt dep(h_, "INSERT OR IGNORE INTO dep VALUES(?,?)");
  for (auto& t : tasks) {
    int64_t pri = t.contains("priority") ? t["priority"].get<int64_t>() : (t.value("month1", true) ? 0 : 1);
    ins.b(1, t["id"].get<std::string>()).b(2, t["wp"].get<std::string>()).b(3, int64_t(phase_num(t["phase"])))
       .b(4, t["role"].get<std::string>()).b(5, pri).b(6, std::string(t["deps"].empty() ? "ready" : "blocked")).b(7, t.dump());
    ins.step(); ins.reset();
    for (auto& d : t["deps"]) { dep.b(1, t["id"].get<std::string>()).b(2, d.get<std::string>()); dep.step(); dep.reset(); }
  }
  exec("COMMIT");
  return tasks.size();
}
int Db::recover() {
  std::lock_guard g(m_);
  exec("BEGIN IMMEDIATE");
  Stmt s(h_, "UPDATE task SET state='ready' WHERE state IN ('leased','submitted') AND (lease_until IS NULL OR lease_until<?)");
  s.b(1, now()); s.step();
  int n = sqlite3_changes(h_);
  exec("COMMIT");
  return n;
}
bool Db::integrity_ok() { std::lock_guard g(m_); Stmt s(h_, "PRAGMA quick_check"); return s.step() && s.t(0) == "ok"; }
void Db::unblock() {
  std::lock_guard g(m_);
  exec(R"(UPDATE task SET state='ready' WHERE state='blocked' AND NOT EXISTS(
      SELECT 1 FROM dep JOIN task d ON d.id=dep.dep WHERE dep.task=task.id AND d.state<>'passed'))");
}
std::vector<json> Db::lease(int n, int lease_s) {
  std::lock_guard g(m_);
  std::vector<json> out;
  exec("BEGIN IMMEDIATE");
  {
    Stmt q(h_, "SELECT id, body FROM task WHERE state='ready' ORDER BY priority, phase, id LIMIT ?");
    q.b(1, int64_t(n));
    Stmt u(h_, "UPDATE task SET state='leased', lease_until=?, started=?, attempts=attempts+1 WHERE id=?");
    Stmt e(h_, "INSERT INTO event VALUES(?,?,'leased','')");
    while (q.step()) {
      std::string id = q.t(0);
      json j = json::parse(q.t(1));
      u.b(1, now() + lease_s).b(2, now()).b(3, id); u.step(); u.reset();
      e.b(1, now()).b(2, id); e.step(); e.reset();
      out.push_back(std::move(j));
    }
    Stmt a(h_, "SELECT id, attempts FROM task WHERE state='leased' AND started>=?");
  }
  exec("COMMIT");
  for (auto& j : out) {   // attempt number lets the harness use a fresh context at 3 and split at 5
    Stmt a(h_, "SELECT attempts FROM task WHERE id=?"); a.b(1, j["id"].get<std::string>());
    if (a.step()) j["_attempt"] = a.i(0);
  }
  return out;
}
void Db::heartbeat(const std::string& id, int lease_s) {
  std::lock_guard g(m_);
  Stmt s(h_, "UPDATE task SET lease_until=? WHERE id=? AND state='leased'"); s.b(1, now() + lease_s).b(2, id); s.step();
}
void Db::finish(const std::string& id, const std::string& result, const Config& c) {
  std::lock_guard g(m_);
  exec("BEGIN IMMEDIATE");
  Stmt q(h_, "SELECT attempts, crashes FROM task WHERE id=?"); q.b(1, id);
  int64_t att = 0, cr = 0; if (q.step()) { att = q.i(0); cr = q.i(1); }
  std::string st;
  if (result == "passed") st = "passed";
  else if (result == "crash") { ++cr; st = cr >= c.quarantine_after ? "quarantined" : "ready"; }
  else st = att >= c.max_attempts ? "escalated" : "ready";
  Stmt u(h_, "UPDATE task SET state=?, crashes=?, finished=? WHERE id=?");
  u.b(1, st).b(2, cr).b(3, now()).b(4, id); u.step();
  Stmt e(h_, "INSERT INTO event VALUES(?,?,?,?)"); e.b(1, now()).b(2, id).b(3, st).b(4, result); e.step();
  exec("COMMIT");
}
void Db::event(const std::string& task, const std::string& kind, const json& data) {
  std::lock_guard g(m_);
  Stmt e(h_, "INSERT INTO event VALUES(?,?,?,?)"); e.b(1, now()).b(2, task).b(3, kind).b(4, data.dump()); e.step();
}
void Db::cost(const std::string& task, int64_t hit, int64_t miss, int64_t out, double usd) {
  std::lock_guard g(m_);
  Stmt e(h_, "INSERT INTO cost VALUES(?,?,?,?,?,?)"); e.b(1, now()).b(2, task).b(3, hit).b(4, miss).b(5, out).b(6, usd); e.step();
}
Db::Snapshot Db::snapshot() {
  std::lock_guard g(m_);
  Snapshot s;
  { Stmt q(h_, "SELECT state, COUNT(*) FROM task GROUP BY state"); while (q.step()) s.states[q.t(0)] = q.i(1); }
  { Stmt q(h_, "SELECT phase, COUNT(*), SUM(state='passed') FROM task GROUP BY phase ORDER BY phase");
    while (q.step()) s.phases.emplace_back(int(q.i(0)), q.i(1), q.i(2)); }
  { Stmt q(h_, "SELECT COALESCE(SUM(usd),0) FROM cost"); if (q.step()) s.cost = q.d(0); }
  { Stmt q(h_, "SELECT COUNT(*) FROM event WHERE kind='passed' AND ts>?"); q.b(1, now() - 86400); if (q.step()) s.passed_24h = q.i(0); }
  { Stmt q(h_, "SELECT COALESCE(MIN(ts),0) FROM event WHERE kind='leased'"); if (q.step()) s.first_pass = q.d(0); }
  return s;
}
int64_t Db::remaining() {
  std::lock_guard g(m_);
  Stmt q(h_, "SELECT COUNT(*) FROM task WHERE state NOT IN ('passed','escalated','quarantined')"); q.step(); return q.i(0);
}
}  // namespace pbsd
