// contract2c: translate the scalar subset of an ACSL function contract into C expressions, for step 3 (random testing) of VERIFICATION_PLAN.md.
// Input (stdin, one per line):  <id> TAB <param names, comma separated> TAB <contract text /*@ requires ...; ensures ...; */>
// Output (stdout, one per line): <id> TAB OK TAB <requires as C> TAB <ensures as C>      or      <id> TAB SKIP TAB <reason>
// Semantics: ACSL integers are mathematical; every operand is evaluated as __int128 (exact for any value that fits in 128 bits). The result variable is `s3_r`.
// Supported: integer literals (dec, hex, octal, with u/l suffixes), parameters, \result / result, \old(param) (= param: scalars are passed by value), \true/\false/
// true/false, unary ! ~ - +, * / % + - << >> < <= > >= == != & ^ | && || ^^ ==> <==> ?:, parentheses, casts to integer types.
// Anything else (pointers, fields, arrays, function calls, \valid, quantifiers, unknown names) gives SKIP with a reason: never a guess.
// Build: g++ -O2 -std=c++20 contract2c.cpp -o contract2c
#include <cctype>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

struct Skip : std::runtime_error { using std::runtime_error::runtime_error; };

struct Tok { enum K { Num, Id, Op, End } k; std::string s; };

static const std::set<std::string> kIntTypes = {"char", "short", "int", "long", "unsigned", "signed", "bool", "_Bool", "u8", "u16", "u32", "u64", "s8", "s16",
  "s32", "s64", "uint8_t", "uint16_t", "uint32_t", "uint64_t", "int8_t", "int16_t", "int32_t", "int64_t", "size_t", "ssize_t", "uint", "ulong", "uintptr_t",
  "intptr_t", "__u8", "__u16", "__u32", "__u64", "__s8", "__s16", "__s32", "__s64"};

class Lexer {
 public:
  explicit Lexer(const std::string& s) : s_(s) {}
  std::vector<Tok> run() {
    std::vector<Tok> out;
    while (true) {
      while (i_ < s_.size() && std::isspace(static_cast<unsigned char>(s_[i_]))) i_++;
      if (i_ >= s_.size()) break;
      char c = s_[i_];
      if (std::isdigit(static_cast<unsigned char>(c))) {
        size_t j = i_;
        if (c == '0' && i_ + 1 < s_.size() && (s_[i_ + 1] == 'x' || s_[i_ + 1] == 'X')) { j += 2; while (j < s_.size() && std::isxdigit(static_cast<unsigned char>(s_[j]))) j++; }
        else while (j < s_.size() && std::isdigit(static_cast<unsigned char>(s_[j]))) j++;
        std::string num = s_.substr(i_, j - i_);
        while (j < s_.size() && (s_[j] == 'u' || s_[j] == 'U' || s_[j] == 'l' || s_[j] == 'L')) j++;
        if (j < s_.size() && (std::isalpha(static_cast<unsigned char>(s_[j])) || s_[j] == '.')) throw Skip("unsupported number literal");
        out.push_back({Tok::Num, num}); i_ = j; continue;
      }
      if (std::isalpha(static_cast<unsigned char>(c)) || c == '_' || c == '\\') {
        size_t j = i_ + 1;
        while (j < s_.size() && (std::isalnum(static_cast<unsigned char>(s_[j])) || s_[j] == '_')) j++;
        out.push_back({Tok::Id, s_.substr(i_, j - i_)}); i_ = j; continue;
      }
      static const char* ops[] = {"<==>", "==>", "^^", "<<", ">>", "<=", ">=", "==", "!=", "&&", "||", "->", "(", ")", "!", "~", "-", "+", "*", "/", "%", "<",
                                  ">", "&", "^", "|", "?", ":", "[", "]", ".", ","};
      bool hit = false;
      for (const char* o : ops) { std::string op(o); if (s_.compare(i_, op.size(), op) == 0) { out.push_back({Tok::Op, op}); i_ += op.size(); hit = true; break; } }
      if (!hit) throw Skip(std::string("unsupported character '") + c + "'");
    }
    out.push_back({Tok::End, ""});
    return out;
  }
 private:
  std::string s_; size_t i_ = 0;
};

// Recursive descent with ACSL precedence (lowest first): <==>, ==> (right), ?:, ||, ^^, &&, |, ^, &, == !=, < <= > >= (no chains), << >>, + -, * / %, unary, primary.
class Parser {
 public:
  Parser(std::vector<Tok> t, const std::set<std::string>& params) : t_(std::move(t)), params_(params) {}
  std::string parse() { std::string e = iff(); if (peek().k != Tok::End) throw Skip("trailing tokens near '" + peek().s + "'"); return e; }
 private:
  const Tok& peek() const { return t_[p_]; }
  bool isop(const std::string& o) const { return peek().k == Tok::Op && peek().s == o; }
  void expect(const std::string& o) { if (!isop(o)) throw Skip("expected '" + o + "'"); p_++; }
  static std::string b(const std::string& x) { return "((" + x + ") != 0)"; }   // as a boolean 0/1
  std::string iff() { std::string l = imp(); while (isop("<==>")) { p_++; std::string r = imp(); l = "(__int128)(" + b(l) + " == " + b(r) + ")"; } return l; }
  std::string imp() { std::string l = tern(); if (isop("==>")) { p_++; std::string r = imp(); return "(__int128)(!" + b(l) + " || " + b(r) + ")"; } return l; }
  std::string tern() { std::string c = lor(); if (isop("?")) { p_++; std::string a = tern(); expect(":"); std::string d = tern(); return "(" + b(c) + " ? (" + a + ") : (" + d + "))"; } return c; }
  std::string lor() { std::string l = lxor(); while (isop("||")) { p_++; std::string r = lxor(); l = "(__int128)(" + b(l) + " || " + b(r) + ")"; } return l; }
  std::string lxor() { std::string l = land(); while (isop("^^")) { p_++; std::string r = land(); l = "(__int128)(" + b(l) + " != " + b(r) + ")"; } return l; }
  std::string land() { std::string l = bor(); while (isop("&&")) { p_++; std::string r = bor(); l = "(__int128)(" + b(l) + " && " + b(r) + ")"; } return l; }
  std::string bor() { std::string l = bxor(); while (isop("|")) { p_++; std::string r = bxor(); l = "(" + l + " | " + r + ")"; } return l; }
  std::string bxor() { std::string l = band(); while (isop("^")) { p_++; std::string r = band(); l = "(" + l + " ^ " + r + ")"; } return l; }
  std::string band() { std::string l = eq(); while (isop("&")) { p_++; std::string r = eq(); l = "(" + l + " & " + r + ")"; } return l; }
  std::string eq() { std::string l = rel(); while (isop("==") || isop("!=")) { std::string o = peek().s; p_++; std::string r = rel(); l = "(__int128)(" + l + " " + o + " " + r + ")"; } return l; }
  std::string rel() {
    std::string l = sh(); int n = 0;
    while (isop("<") || isop("<=") || isop(">") || isop(">=")) {
      if (++n > 1) throw Skip("relation chain (a < b < c) not supported");
      std::string o = peek().s; p_++; std::string r = sh(); l = "(__int128)(" + l + " " + o + " " + r + ")";
    }
    return l;
  }
  std::string sh() { std::string l = add(); while (isop("<<") || isop(">>")) { std::string o = peek().s; p_++; std::string r = add(); l = (o == "<<" ? "s3_shl(" : "s3_shr(") + l + ", " + r + ")"; } return l; }
  std::string add() { std::string l = mul(); while (isop("+") || isop("-")) { std::string o = peek().s; p_++; std::string r = mul(); l = "(" + l + " " + o + " " + r + ")"; } return l; }
  std::string mul() { std::string l = un(); while (isop("*") || isop("/") || isop("%")) { std::string o = peek().s; p_++; std::string r = un(); l = (o == "*" ? "(" + l + " * " + r + ")" : (o == "/" ? "s3_div(" : "s3_mod(") + l + ", " + r + ")"); } return l; }
  std::string un() {
    if (isop("!")) { p_++; return "(__int128)(!" + b(un()) + ")"; }
    if (isop("~")) { p_++; return "(~" + un() + ")"; }
    if (isop("-")) { p_++; return "(-" + un() + ")"; }
    if (isop("+")) { p_++; return un(); }
    // cast: ( type words ) unary
    if (isop("(") && t_[p_ + 1].k == Tok::Id && kIntTypes.count(t_[p_ + 1].s)) {
      size_t q = p_ + 1; std::string ty;
      while (t_[q].k == Tok::Id && kIntTypes.count(t_[q].s)) { ty += (ty.empty() ? "" : " ") + t_[q].s; q++; }
      if (t_[q].k == Tok::Op && t_[q].s == ")") { p_ = q + 1; std::string e = un(); return "(__int128)(" + ty + ")(" + e + ")"; }
    }
    return prim();
  }
  std::string prim() {
    const Tok& t = peek();
    if (t.k == Tok::Num) { p_++; return "((__int128)" + t.s + "ULL)"; }
    if (isop("(")) { p_++; std::string e = iff(); expect(")"); return "(" + e + ")"; }
    if (t.k == Tok::Id) {
      std::string s = t.s; p_++;
      if (s == "\\result" || s == "result") return "((__int128)s3_r)";
      if (s == "\\true" || s == "true") return "((__int128)1)";
      if (s == "\\false" || s == "false") return "((__int128)0)";
      if (s == "\\old" || s == "\\at") { expect("("); std::string e = iff(); if (s == "\\at") throw Skip("\\at not supported"); expect(")"); return e; }
      if (s[0] == '\\') throw Skip("unsupported ACSL construct " + s);
      if (isop("(")) throw Skip("function call " + s + "(...)");
      if (isop("->") || isop(".") || isop("[")) throw Skip("field or array access on " + s);
      if (!params_.count(s)) throw Skip("name " + s + " is not a parameter");
      return "((__int128)(" + s + "))";
    }
    throw Skip("unexpected token '" + t.s + "'");
  }
  std::vector<Tok> t_; const std::set<std::string>& params_; size_t p_ = 0;
};

static std::string trim(std::string s) { size_t a = s.find_first_not_of(" \t"); size_t z = s.find_last_not_of(" \t"); return a == std::string::npos ? "" : s.substr(a, z - a + 1); }

int main() {
  std::string line;
  while (std::getline(std::cin, line)) {
    std::istringstream ls(line); std::string id, plist, contract;
    std::getline(ls, id, '\t'); std::getline(ls, plist, '\t'); std::getline(ls, contract);
    try {
      std::set<std::string> params; { std::istringstream ps(plist); std::string p; while (std::getline(ps, p, ',')) { p = trim(p); if (!p.empty()) params.insert(p); } }
      std::string c = contract;
      size_t a = c.find("/*@"); size_t z = c.rfind("*/");
      if (a == std::string::npos || z == std::string::npos || z < a) throw Skip("not a /*@ ... */ block");
      c = c.substr(a + 3, z - a - 3);
      std::vector<std::string> req, ens;
      std::istringstream cs(c); std::string clause;
      while (std::getline(cs, clause, ';')) {
        clause = trim(clause); if (clause.empty()) continue;
        std::string kw = clause.substr(0, clause.find_first_of(" \t(\\"));
        std::string body = trim(clause.substr(kw.size()));
        if (kw != "requires" && kw != "ensures") throw Skip("clause " + kw + " not supported");
        if (body.find("\\forall") != std::string::npos || body.find("\\exists") != std::string::npos) throw Skip("quantifier");
        std::string e = Parser(Lexer(body).run(), params).parse();
        (kw == "requires" ? req : ens).push_back(e);
      }
      if (ens.empty()) throw Skip("no ensures clause");
      auto join = [](const std::vector<std::string>& v) { if (v.empty()) return std::string("1"); std::string r; for (auto& x : v) r += (r.empty() ? "" : " && ") + std::string("((") + x + ") != 0)"; return r; };
      std::cout << id << "\tOK\t" << join(req) << "\t" << join(ens) << "\n";
    } catch (const Skip& s) {
      std::cout << id << "\tSKIP\t" << s.what() << "\n";
    }
  }
}
