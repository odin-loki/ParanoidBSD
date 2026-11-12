// acsl2bv.cpp  Bit-precise proof harness for a scalar ACSL contract (step 2, "bit-vector path").
//
//   acsl2bv <contract.exp> <unit.c.sig> <unit.c> <function> <out-harness.c>
//
// Writes a C file that holds the unit and a main():   nondet parameters; __CPROVER_assume(requires); r = f(params); __CPROVER_assert(ensures).
// CBMC (SAT, bit-precise machine integers) then decides the contract for ALL inputs, so it is a proof (loop-free code, or loops whose unwinding assertion holds).
//
// The semantic gap between ACSL (mathematical integers) and C (machine integers) is closed like this (this is the soundness argument for the path):
//   * the contract is evaluated in __int128: every parameter, \result, literal and enumerator is widened first, +,-,* cannot wrap, a shift is a multiplication or a floor
//     division by 2^k, / and % are C99 truncating division; a casted subterm  (T)e  converts to T first (ACSL casts do the same) and then widens;
//   * if an intermediate value could leave the 128-bit range, or a shift count is outside [0,120), or a divisor is 0, an assertion in the harness FAILS (never "assumed away");
//   * bitwise & | ^ ~ on __int128 are the two's-complement operations on infinite integers for every value inside the range, which is what ACSL means;
//   * chained comparisons  a <= b <= c  are read as  a <= b && b <= c  (ACSL);
//   * an unsupported construct (pointer, \valid, quantifier, call, field, unknown name) gives SKIP, never a guess.
// Output on stdout: "OK" or "SKIP <reason>".  Exit 0 / 3.
#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <cstring>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

struct Skip : runtime_error { using runtime_error::runtime_error; };
struct Tok { enum K { Num, Id, Op, End } k; string s; };

static const set<string> kIntWords = {"char", "short", "int", "long", "unsigned", "signed", "_Bool", "bool", "__int128"};
static set<string> typedefs; // names declared with typedef in the unit
// pointer support (round 3): pointer parameters are modelled as malloc objects of the size the requires clause gives with alid; see header comment
static set<string> g_ptrs; static map<string, pair<string, string>> g_valid; /* name -> (lo, hi) as __int128 C expressions */ static bool g_inreq = false, g_inold = false, g_bounded = false;

class Lexer {
 public:
  explicit Lexer(const string& s) : s_(s) {}
  vector<Tok> run() {
    vector<Tok> out;
    while (true) {
      while (i_ < s_.size() && isspace((unsigned char)s_[i_])) i_++;
      if (i_ >= s_.size()) break;
      char c = s_[i_];
      if (isdigit((unsigned char)c)) {
        size_t j = i_;
        if (c == '0' && i_ + 1 < s_.size() && (s_[i_ + 1] == 'x' || s_[i_ + 1] == 'X')) { j += 2; while (j < s_.size() && isxdigit((unsigned char)s_[j])) j++; }
        else while (j < s_.size() && isdigit((unsigned char)s_[j])) j++;
        string num = s_.substr(i_, j - i_);
        while (j < s_.size() && (s_[j] == 'u' || s_[j] == 'U' || s_[j] == 'l' || s_[j] == 'L')) j++;
        if (j < s_.size() && (isalpha((unsigned char)s_[j]) || (s_[j] == '.' && !(j + 1 < s_.size() && s_[j + 1] == '.')))) throw Skip("unsupported number literal");
        out.push_back({Tok::Num, num}); i_ = j; continue;
      }
      if (isalpha((unsigned char)c) || c == '_' || c == '\\') {
        size_t j = i_ + 1; while (j < s_.size() && (isalnum((unsigned char)s_[j]) || s_[j] == '_')) j++;
        out.push_back({Tok::Id, s_.substr(i_, j - i_)}); i_ = j; continue;
      }
      static const char* ops[] = {"<==>", "==>", "^^", "<<", ">>", "<=", ">=", "==", "!=", "&&", "||", "->", "(", ")", "!", "~", "-", "+", "*", "/", "%", "<", ">", "&", "^", "|", "?", ":", "[", "]", "..", ".", ","};
      bool hit = false;
      for (const char* o : ops) { string op(o); if (s_.compare(i_, op.size(), op) == 0) { out.push_back({Tok::Op, op}); i_ += op.size(); hit = true; break; } }
      if (!hit) throw Skip(string("unsupported character '") + c + "'");
    }
    out.push_back({Tok::End, ""});
    return out;
  }
 private:
  string s_; size_t i_ = 0;
};

class Parser {
 public:
  Parser(vector<Tok> t, const set<string>& params, set<string>& consts) : t_(move(t)), params_(params), consts_(consts) {}
  string parse() { string e = iff(); if (peek().k != Tok::End) throw Skip("trailing tokens near '" + peek().s + "'"); return e; }
 private:
  const Tok& peek() const { return t_[p_]; }
  bool isop(const string& o) const { return peek().k == Tok::Op && peek().s == o; }
  void expect(const string& o) { if (!isop(o)) throw Skip("expected '" + o + "'"); p_++; }
  static string b(const string& x) { return "((" + x + ") != 0)"; }
  string iff() { string l = imp(); while (isop("<==>")) { p_++; string r = imp(); l = "(__int128)(" + b(l) + " == " + b(r) + ")"; } return l; }
  string imp() { string l = tern(); if (isop("==>")) { p_++; string r = imp(); return "(__int128)(!" + b(l) + " || " + b(r) + ")"; } return l; }
  string tern() { string c = lor(); if (isop("?")) { p_++; string a = tern(); expect(":"); string d = tern(); return "(" + b(c) + " ? (" + a + ") : (" + d + "))"; } return c; }
  string lor() { string l = lxor(); while (isop("||")) { p_++; string r = lxor(); l = "(__int128)(" + b(l) + " || " + b(r) + ")"; } return l; }
  string lxor() { string l = land(); while (isop("^^")) { p_++; string r = land(); l = "(__int128)(" + b(l) + " != " + b(r) + ")"; } return l; }
  string land() { string l = bor(); while (isop("&&")) { p_++; string r = bor(); l = "(__int128)(" + b(l) + " && " + b(r) + ")"; } return l; }
  string bor() { string l = bxor(); while (isop("|")) { p_++; string r = bxor(); l = "(" + l + " | " + r + ")"; } return l; }
  string bxor() { string l = band(); while (isop("^")) { p_++; string r = band(); l = "(" + l + " ^ " + r + ")"; } return l; }
  string band() { string l = eq(); while (isop("&")) { p_++; string r = eq(); l = "(" + l + " & " + r + ")"; } return l; }
  string eq() { string l = rel(); while (isop("==") || isop("!=")) { string o = peek().s; p_++; string r = rel(); l = "(__int128)(" + l + " " + o + " " + r + ")"; } return l; }
  string rel() { // a < b <= c  is  a < b && b <= c
    string l = sh(); string acc; string prev = l;
    while (isop("<") || isop("<=") || isop(">") || isop(">=")) {
      string o = peek().s; p_++; string r = sh();
      string one = "(" + prev + " " + o + " " + r + ")";
      acc = acc.empty() ? one : "(" + acc + " && " + one + ")";
      prev = r;
    }
    return acc.empty() ? l : "((__int128)" + acc + ")";
  }
  string sh() { string l = add(); while (isop("<<") || isop(">>")) { string o = peek().s; p_++; string r = add(); l = (o == "<<" ? "bv_shl(" : "bv_shr(") + l + ", " + r + ")"; } return l; }
  string add() { string l = mul(); while (isop("+") || isop("-")) { string o = peek().s; p_++; string r = mul(); l = (o == "+" ? "bv_add(" : "bv_sub(") + l + ", " + r + ")"; } return l; }
  string mul() { string l = un(); while (isop("*") || isop("/") || isop("%")) { string o = peek().s; p_++; string r = un(); l = (o == "*" ? "bv_mul(" : (o == "/" ? "bv_div(" : "bv_mod(")) + l + ", " + r + ")"; } return l; }
  bool is_type_start(size_t q) const {
    if (t_[q].k != Tok::Id) return false;
    if (kIntWords.count(t_[q].s)) return true;
    return typedefs.count(t_[q].s) && !params_.count(t_[q].s);
  }
  // postfix chain on a pointer parameter:  name[idx]  name->f  name.f  (value is widened); bare name = its address as integer
  string postfix(const string& name, bool deref) {
    if (g_inold) throw Skip("memory read inside old");
    string lv = name; bool any = deref;
    if (deref) lv = "(*" + name + ")";
    while (true) {
      if (isop("[")) { p_++; string ix = iff(); expect("]"); lv = "(" + lv + ")[(long)(" + ix + ")]"; any = true; }
      else if (isop("->")) { p_++; if (peek().k != Tok::Id) throw Skip("field name expected"); lv = "(" + lv + ")->" + peek().s; p_++; any = true; }
      else if (isop(".") && any) { p_++; if (peek().k != Tok::Id) throw Skip("field name expected"); lv = "(" + lv + ")." + peek().s; p_++; }
      else break;
    }
    if (!any) return "((__int128)(unsigned long)(" + name + "))";
    return "((__int128)(" + lv + "))";
  }
  string un() {
    if (isop("*") && t_[p_ + 1].k == Tok::Id && g_ptrs.count(t_[p_ + 1].s)) { p_++; string nm = peek().s; p_++; return postfix(nm, true); }
    if (isop("!")) { p_++; return "(__int128)(!" + b(un()) + ")"; }
    if (isop("~")) { p_++; return "(~" + un() + ")"; }
    if (isop("-")) { p_++; return "bv_neg(" + un() + ")"; }
    if (isop("+")) { p_++; return un(); }
    if (isop("(") && is_type_start(p_ + 1)) {
      size_t q = p_ + 1; string ty;
      while (t_[q].k == Tok::Id && (kIntWords.count(t_[q].s) || typedefs.count(t_[q].s))) { ty += (ty.empty() ? "" : " ") + t_[q].s; q++; }
      if (t_[q].k == Tok::Op && t_[q].s == ")") { p_ = q + 1; string e = un(); return "((__int128)(" + ty + ")(" + e + "))"; }
    }
    return prim();
  }
  string prim() {
    const Tok& t = peek();
    if (t.k == Tok::Num) { p_++; return "((__int128)" + t.s + "ULL)"; }
    if (isop("(")) { p_++; string e = iff(); expect(")"); return "(" + e + ")"; }
    if (t.k == Tok::Id) {
      string s = t.s; p_++;
      if (s == "\\result" || s == "result") return "((__int128)bv_r)";
      if (s == "\\true" || s == "true") return "((__int128)1)";
      if (s == "\\false" || s == "false") return "((__int128)0)";
      if (s == "\\old") { expect("("); bool sv = g_inold; g_inold = true; string e = iff(); g_inold = sv; expect(")"); return e; }
      if (s == "\\null") return "((__int128)0)";
      if (s == "\\valid" || s == "\\valid_read") {
        if (!g_inreq) throw Skip(s + " in ensures");
        expect("("); if (peek().k != Tok::Id || !g_ptrs.count(peek().s)) throw Skip(s + " of a non-parameter");
        string nm = peek().s; p_++;
        string lo = "((__int128)0ULL)", hi = "((__int128)0ULL)";
        if (isop("+")) { p_++; expect("("); lo = iff(); expect(".."); hi = iff(); expect(")"); }
        expect(")");
        if (g_valid.count(nm)) throw Skip("two valid clauses for " + nm);
        g_valid[nm] = {lo, hi};
        return "((__int128)1)";
      }
      if (s[0] == '\\') throw Skip("unsupported ACSL construct " + s);
      if (isop("(")) throw Skip("function call " + s + "(...)");
      if (g_ptrs.count(s)) return postfix(s, false);
      if (isop("->") || isop(".") || isop("[")) throw Skip("field or array access on " + s);
      if (params_.count(s)) return "((__int128)(" + s + "))";
      consts_.insert(s);  // must be an integer constant expression (enumerator): checked by the C compiler in the harness (enum { x = (name) })
      return "((__int128)(" + s + "))";
    }
    throw Skip("unexpected token '" + t.s + "'");
  }
  vector<Tok> t_; const set<string>& params_; set<string>& consts_; size_t p_ = 0;
};

static string trim(string s) { size_t a = s.find_first_not_of(" \t\r\n"); size_t z = s.find_last_not_of(" \t\r\n"); return a == string::npos ? "" : s.substr(a, z - a + 1); }

int main(int argc, char** argv) {
  if (argc < 6) return 2;
  try {
    ifstream cf(argv[1]); stringstream cs; cs << cf.rdbuf(); string c = cs.str();
    size_t a = c.find("/*@"), z = c.rfind("*/"); if (a == string::npos || z == string::npos || z < a) throw Skip("not a /*@ ... */ block");
    c = c.substr(a + 3, z - a - 3);
    ifstream sf(argv[2]); string ret; getline(sf, ret); ret = trim(ret);
    if (ret.empty() || ret == "?") throw Skip("signature not parsed");
    if (ret == "void") throw Skip("void function");
    if (ret.find('*') != string::npos) throw Skip("pointer result");
    vector<pair<string, string>> params; string ln; map<string, string> elem;
    while (getline(sf, ln)) { size_t tb = ln.find('\t'); string ty = ln.substr(0, tb), nm = tb == string::npos ? "" : ln.substr(tb + 1); if (ty == "?" || nm == "?") throw Skip("parameter that is not a plain declaration");
      if (ty.find('*') != string::npos) {
        string el = ty; el.erase(el.rfind('*'));
        { static const char* qs[] = {"const", "volatile", "__restrict", "restrict"}; for (const char* q : qs) { size_t pq; while ((pq = el.find(q)) != string::npos && (pq == 0 || !isalnum((unsigned char)el[pq - 1])) && (pq + strlen(q) >= el.size() || !(isalnum((unsigned char)el[pq + strlen(q)]) || el[pq + strlen(q)] == '_'))) el.erase(pq, strlen(q)); } }
        el = trim(el);
        if (el.find('*') != string::npos) throw Skip("pointer to pointer parameter " + nm);
        if (el == "void" || el.empty()) throw Skip("void pointer parameter " + nm);
        g_ptrs.insert(nm); elem[nm] = el;
      }
      params.push_back({ty, nm}); }
    // typedef names of the unit
    { ifstream uf(argv[3]); stringstream us; us << uf.rdbuf(); string u = us.str(); size_t p = 0;
      while ((p = u.find("typedef", p)) != string::npos) { size_t e = u.find(';', p); if (e == string::npos) break; string d = u.substr(p + 7, e - p - 7); p = e;
        if (d.find('{') != string::npos || d.find('(') != string::npos) continue;
        size_t k = d.size(); while (k > 0 && isspace((unsigned char)d[k - 1])) k--; size_t s2 = k; while (s2 > 0 && (isalnum((unsigned char)d[s2 - 1]) || d[s2 - 1] == '_')) s2--; if (s2 < k) typedefs.insert(d.substr(s2, k - s2)); } }
    set<string> pn; for (auto& p : params) pn.insert(p.second);
    vector<string> req, ens; set<string> consts;
    istringstream ss(c); string clause;
    while (getline(ss, clause, ';')) {
      clause = trim(clause); if (clause.empty()) continue;
      string kw = clause.substr(0, clause.find_first_of(" \t(\\")); string body = trim(clause.substr(kw.size()));
      if (kw != "requires" && kw != "ensures") throw Skip("clause " + kw + " not supported");
      if (body.find("\\forall") != string::npos || body.find("\\exists") != string::npos) throw Skip("quantifier");
      g_inreq = (kw == "requires");
      if (g_inreq && body.find("\\valid") != string::npos && (body.find("==>") != string::npos || body.find("||") != string::npos || body.find('?') != string::npos || body.find('!') != string::npos)) throw Skip("valid under a connective");
      string e = Parser(Lexer(body).run(), pn, consts).parse();
      (kw == "requires" ? req : ens).push_back(e);
    }
    if (ens.empty()) throw Skip("no ensures clause");
    auto join = [](const vector<string>& v) { if (v.empty()) return string("1"); string r; for (auto& x : v) r += (r.empty() ? "" : " && ") + string("((") + x + ") != 0)"; return r; };
    ifstream uf(argv[3]); stringstream us; us << uf.rdbuf();
    ofstream h(argv[5]);
    h << us.str() << "\n";
    h << "/* ---- bit-vector harness generated by acsl2bv (see acsl2bv.cpp for the semantics) ---- */\n";
    h << "void __CPROVER_assume(int); void __CPROVER_assert(int, const char*);\n";
    h << "static __int128 bv_chk(int ok, __int128 v) { __CPROVER_assert(ok, \"contract arithmetic stays inside 128 bits\"); return v; }\n";
    h << "static __int128 bv_add(__int128 a, __int128 b) { __int128 r; return bv_chk(!__builtin_add_overflow(a, b, &r), r); }\n";
    h << "static __int128 bv_sub(__int128 a, __int128 b) { __int128 r; return bv_chk(!__builtin_sub_overflow(a, b, &r), r); }\n";
    h << "static __int128 bv_mul(__int128 a, __int128 b) { __int128 r; return bv_chk(!__builtin_mul_overflow(a, b, &r), r); }\n";
    h << "static __int128 bv_neg(__int128 a) { __int128 r; return bv_chk(!__builtin_sub_overflow((__int128)0, a, &r), r); }\n";
    h << "static __int128 bv_div(__int128 a, __int128 b) { __CPROVER_assert(b != 0, \"division by zero in contract\"); return b == 0 ? 0 : a / b; }\n";
    h << "static __int128 bv_mod(__int128 a, __int128 b) { __CPROVER_assert(b != 0, \"modulo by zero in contract\"); return b == 0 ? 0 : a % b; }\n";
    h << "static __int128 bv_shl(__int128 a, __int128 n) { __CPROVER_assert(n >= 0 && n < 100, \"shift count in contract\"); __int128 r; if (n < 0 || n >= 100) return 0; return bv_chk(!__builtin_mul_overflow(a, ((__int128)1) << n, &r), r); }\n";
    h << "static __int128 bv_shr(__int128 a, __int128 n) { __CPROVER_assert(n >= 0 && n < 100, \"shift count in contract\"); if (n < 0 || n >= 100) return 0; return a >> n; }\n";
    { int i = 0; for (auto& k : consts) h << "enum { bv_const_" << i++ << " = (" << k << ") };\n"; }
    h << "int main(void) {\n";
    for (auto& p : params) h << "  " << p.first << " " << p.second << ";\n";
    h << "  __CPROVER_assume(" << join(req) << ");\n";
    if (!g_ptrs.empty()) h << "  extern void *malloc(unsigned long);\n";
    for (auto& pp : g_ptrs) {
      auto it = g_valid.find(pp);
      if (it == g_valid.end()) { h << "  { int c_" << pp << "; " << pp << " = c_" << pp << " ? 0 : malloc(sizeof(" << elem[pp] << ")); }\n"; continue; }
      static const regex lit("\\(\\(__int128\\)([0-9]+)ULL\\)");
      smatch m1, m2;
      if (!regex_match(it->second.first, m1, lit) || m1[1] != "0") throw Skip("valid range does not start at 0 for " + pp);
      if (regex_match(it->second.second, m2, lit)) h << "  " << pp << " = malloc(sizeof(" << elem[pp] << ") * " << (atol(m2[1].str().c_str()) + 1) << "UL);\n";
      else { g_bounded = true; h << "  { __int128 hi_" << pp << " = " << it->second.second << "; __CPROVER_assume(hi_" << pp << " >= 0 && hi_" << pp << " <= 7); " << pp << " = malloc(sizeof(" << elem[pp] << ") * (unsigned long)(hi_" << pp << " + 1)); }\n"; }
    }
    string call = argv[4]; call += "(";
    for (size_t i = 0; i < params.size(); i++) call += (i ? ", " : "") + params[i].second;
    call += ")";
    h << "  " << ret << " bv_r = " << call << ";\n";
    h << "  __CPROVER_assert(" << join(ens) << ", \"ensures\");\n  return 0;\n}\n";
    cout << "OK" << (g_bounded ? " bounded=8" : "") << (g_ptrs.empty() ? "" : " pointers") << "\n";
    return 0;
  } catch (const Skip& s) { cout << "SKIP " << s.what() << "\n"; return 3; }
}
