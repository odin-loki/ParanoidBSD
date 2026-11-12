// tuslice.cpp  Cut one function out of a preprocessed translation unit (gcc -E -dD output) and keep everything the C front end needs.
//
//   tuslice <tu.i> <srcfile> <name> <start> <end> <contract-file|-> <out.c> <defs.out>
//
//   - line markers and #pragma lines are dropped; #define/#undef lines (from -dD) are collected IN ORDER into <defs.out> up to the position of the target
//     (so that the contract can be macro-expanded with the definitions that were live at the function);
//   - every top-level item is kept as is (types, variables, prototypes, typedefs, static asserts), except function DEFINITIONS:
//     the target function is kept (with the contract text inserted before it), every other function definition is replaced by its prototype
//     (so a callee has no body and no specification: WP treats it as arbitrary -> PROVED-ASSUMING, listed in <out.c>.callees);
//   - gcc spellings Frama-C's parser rejects are normalised (asm __inline volatile -> asm volatile); each rewrite is counted on stderr.
// Exit: 0 found, 3 function not found / ambiguous.  stderr: one line  "ok <nfuncs> <ndropped> <callee names...>"  or the reason.
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <algorithm>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

struct Item { string text; string file; long line; bool func = false; string head, name; size_t defs_before = 0; };

static bool isid(char c) { return isalnum((unsigned char)c) || c == '_'; }

static const set<string> kw = {"__attribute__", "__attribute", "__asm__", "asm", "__asm", "__typeof__", "typeof", "__typeof", "_Alignas", "__alignof__", "sizeof", "_Alignof", "__declspec", "_Static_assert", "__extension__", "typeof_unqual", "__auto_type"};

static string funcname(const string& head) {
  // first identifier at paren depth 0 that is followed by '(' and is not a keyword whose group we skip
  size_t i = 0, n = head.size();
  while (i < n) {
    if (isid(head[i]) && !isdigit((unsigned char)head[i])) {
      size_t j = i; while (j < n && isid(head[j])) j++;
      string id = head.substr(i, j - i);
      size_t k = j; while (k < n && isspace((unsigned char)head[k])) k++;
      if (k < n && head[k] == '(') {
        if (kw.count(id)) { // skip the parenthesised group
          int d = 0; size_t m = k;
          for (; m < n; m++) { if (head[m] == '(') d++; else if (head[m] == ')') { if (--d == 0) { m++; break; } } }
          i = m; continue;
        }
        return id;
      }
      i = j; continue;
    }
    if (head[i] == '(') { // a group that is not a function name (e.g. (*fp)) : skip as a whole
      int d = 0; size_t m = i;
      for (; m < n; m++) { if (head[m] == '(') d++; else if (head[m] == ')') { if (--d == 0) { m++; break; } } }
      i = m; continue;
    }
    i++;
  }
  return "";
}

// ---- dependency analysis -------------------------------------------------------------------------------------------
// clean(): drop string/char literals, attribute/asm/typeof-like groups; returns text where top-level {...} groups are kept (for tag/enum scan) in `braced`
static string strip_groups(const string& s, const set<string>& names) {
  string r; size_t n = s.size();
  for (size_t i = 0; i < n;) {
    if (s[i] == '"') { i++; while (i < n && s[i] != '"') { if (s[i] == '\\') i++; i++; } i++; r += "\"\""; continue; }
    if (s[i] == '\'') { i++; while (i < n && s[i] != '\'') { if (s[i] == '\\') i++; i++; } i++; r += "0"; continue; }
    if (isid(s[i]) && !isdigit((unsigned char)s[i])) {
      size_t j = i; while (j < n && isid(s[j])) j++;
      string id = s.substr(i, j - i); size_t k = j; while (k < n && isspace((unsigned char)s[k])) k++;
      if (names.count(id) && k < n && s[k] == '(') { int d = 0; size_t m = k; for (; m < n; m++) { if (s[m] == '(') d++; else if (s[m] == ')') { if (--d == 0) { m++; break; } } } i = m; continue; }
      r += id; i = j; continue;
    }
    r.push_back(s[i]); i++;
  }
  return r;
}
static const set<string> groupkw = {"__attribute__", "__attribute", "__asm__", "asm", "__asm", "__typeof__", "typeof", "__typeof", "_Alignas", "__declspec", "__alignof__", "_Alignof", "sizeof", "typeof_unqual"};
static const set<string> cwords = {"struct", "union", "enum", "typedef", "extern", "static", "inline", "__inline", "__inline__", "const", "volatile", "register", "unsigned", "signed", "int", "char", "short", "long", "void", "float", "double", "_Bool", "__extension__", "restrict", "__restrict", "_Noreturn", "_Thread_local", "__thread", "__int128", "_Complex", "auto"};
static vector<string> idents(const string& s) {
  vector<string> v; size_t n = s.size();
  for (size_t i = 0; i < n;) {
    if (s[i] == '"') { i++; while (i < n && s[i] != '"') { if (s[i] == '\\') i++; i++; } i++; continue; }
    if (s[i] == '\'') { i++; while (i < n && s[i] != '\'') { if (s[i] == '\\') i++; i++; } i++; continue; }
    if (isid(s[i])) { size_t j = i; while (j < n && isid(s[j])) j++; if (!isdigit((unsigned char)s[i])) v.push_back(s.substr(i, j - i)); i = j; continue; }
    i++;
  }
  return v;
}
// names an item declares: declarators, tags with a body, enumerators
static void provides(const string& text, set<string>& out) {
  string t = strip_groups(text, groupkw);
  // tags and enumerators (any depth)
  size_t n = t.size();
  for (size_t i = 0; i < n;) {
    if (isid(t[i])) {
      size_t j = i; while (j < n && isid(t[j])) j++; string w = t.substr(i, j - i);
      if (w == "struct" || w == "union" || w == "enum") {
        size_t k = j; while (k < n && isspace((unsigned char)t[k])) k++;
        string tag; size_t k2 = k; if (k < n && isid(t[k])) { while (k2 < n && isid(t[k2])) k2++; tag = t.substr(k, k2 - k); }
        size_t k3 = k2; while (k3 < n && isspace((unsigned char)t[k3])) k3++;
        if (k3 < n && t[k3] == '{') {
          if (!tag.empty()) out.insert(tag);
          if (w == "enum") { // enumerators: first identifier of every comma segment at depth 1
            int d = 0; size_t m = k3; bool seg = true;
            for (; m < n; m++) {
              if (t[m] == '{' || t[m] == '(' || t[m] == '[') { d++; continue; }
              if (t[m] == '}' || t[m] == ')' || t[m] == ']') { if (--d == 0) break; continue; }
              if (d == 1 && t[m] == ',') { seg = true; continue; }
              if (d == 1 && seg && isid(t[m]) && !isdigit((unsigned char)t[m])) { size_t e = m; while (e < n && isid(t[e])) e++; out.insert(t.substr(m, e - m)); seg = false; m = e - 1; }
              else if (d == 1 && seg && !isspace((unsigned char)t[m])) seg = false;
            }
          }
        } else if (!tag.empty() && k3 >= n) out.insert(tag); // forward declaration  struct X
        else if (!tag.empty() && t[k3] == ';') out.insert(tag);
        i = k2 > j ? k2 : j; continue;
      }
      i = j; continue;
    }
    i++;
  }
  // declarators: remove {...} groups at depth 0, cut at ';'
  string flat; { int d = 0; for (char c : t) { if (c == '{') { if (d++ == 0) flat += "{}"; } else if (c == '}') d--; else if (d == 0) flat.push_back(c); } }
  size_t semi = flat.find(';'); if (semi != string::npos) flat = flat.substr(0, semi);
  // split at top-level commas
  vector<string> decls; { int d = 0; string cur; for (char c : flat) { if (c == '(' || c == '[') d++; else if (c == ')' || c == ']') d--; if (c == ',' && d == 0) { decls.push_back(cur); cur.clear(); } else cur.push_back(c); } decls.push_back(cur); }
  static const regex fp("\\(\\s*\\*+\\s*([A-Za-z_][A-Za-z_0-9]*)");
  for (auto d : decls) {
    size_t eq = string::npos; { int pd = 0; for (size_t i = 0; i < d.size(); i++) { if (d[i] == '(' || d[i] == '[') pd++; else if (d[i] == ')' || d[i] == ']') pd--; else if (d[i] == '=' && pd == 0) { eq = i; break; } } }
    if (eq != string::npos) d = d.substr(0, eq);
    smatch m; string name;
    size_t par = d.find('(');
    if (par != string::npos) {
      string pre = d.substr(0, par); vector<string> v = idents(pre);
      bool onlyspace = true; for (char c : pre) if (!isspace((unsigned char)c)) onlyspace = false;
      {
        int dd2 = 0; size_t e = par; for (; e < d.size(); e++) { if (d[e] == '(') dd2++; else if (d[e] == ')' && --dd2 == 0) break; }
        string inner = d.substr(par + 1, e > par ? e - par - 1 : 0); vector<string> vi = idents(inner);
        size_t nx = e + 1; while (nx < d.size() && isspace((unsigned char)d[nx])) nx++;
        string it2 = inner; while (!it2.empty() && isspace((unsigned char)it2.back())) it2.pop_back(); while (!it2.empty() && isspace((unsigned char)it2.front())) it2.erase(0,1);
        static const regex rptr("^\\*+\\s*[A-Za-z_][A-Za-z_0-9]*$"), rid("^[A-Za-z_][A-Za-z_0-9]*$");
        bool grp = nx < d.size() && (d[nx] == '(' || d[nx] == '[') && (regex_match(it2, rptr) || (regex_match(it2, rid) && d[nx] == '(' && !v.empty() && !isid(pre.empty() ? ' ' : pre.back()))) && !vi.empty();
        if (grp) name = vi.back();
        else if (regex_search(d, m, fp) && (size_t)m.position(0) == par) name = m[1];
        else if (!v.empty()) name = v.back();
      }
    } else {
      string dd = d; size_t br = dd.find('['); if (br != string::npos) dd = dd.substr(0, br); size_t col = dd.find(':'); if (col != string::npos) dd = dd.substr(0, col);
      vector<string> v = idents(dd); if (!v.empty()) name = v.back();
    }
    if (!name.empty() && !cwords.count(name)) out.insert(name);
  }
}

// ---- normalisation of gcc spellings the Frama-C parser cannot read ---------------------------------------------------
static size_t match_paren(const string& s, size_t i) { // s[i]=='(' ; returns index after the matching ')'
  int d = 0; size_t n = s.size();
  for (size_t m = i; m < n; m++) {
    if (s[m] == '"') { m++; while (m < n && s[m] != '"') { if (s[m] == '\\') m++; m++; } continue; }
    if (s[m] == '\'') { m++; while (m < n && s[m] != '\'') { if (s[m] == '\\') m++; m++; } continue; }
    if (s[m] == '(') d++; else if (s[m] == ')') { if (--d == 0) return m + 1; }
  }
  return n;
}
// __attribute__((a, b(x), c)) -> keeps only noreturn/packed; the others are ignored by Frama-C anyway and some contain GNU extensions (x ?: y) it cannot parse
static string norm_attrs(const string& s, long& cnt) {
  string r; size_t n = s.size();
  for (size_t i = 0; i < n;) {
    if (s[i] == '"') { size_t j = i + 1; while (j < n && s[j] != '"') { if (s[j] == '\\') j++; j++; } r.append(s, i, j + 1 - i); i = j + 1; continue; }
    if (isid(s[i]) && (i == 0 || !isid(s[i - 1]))) {
      size_t j = i; while (j < n && isid(s[j])) j++;
      string w = s.substr(i, j - i);
      if (w == "__attribute__" || w == "__attribute") {
        size_t k = j; while (k < n && isspace((unsigned char)s[k])) k++;
        if (k < n && s[k] == '(') {
          size_t e = match_paren(s, k); string inner = s.substr(k + 1, e - k - 2); // inner = "(a,b)"
          size_t p = inner.find('('), q = inner.rfind(')');
          if (p != string::npos && q != string::npos && q > p) {
            string list = inner.substr(p + 1, q - p - 1); string keepl;
            int d = 0; string cur; vector<string> parts;
            for (char c : list) { if (c == '(') d++; else if (c == ')') d--; if (c == ',' && d == 0) { parts.push_back(cur); cur.clear(); } else cur.push_back(c); }
            parts.push_back(cur);
            for (auto& pt : parts) { vector<string> v = idents(pt); if (v.empty()) continue; const string& a = v[0];
              if (a == "noreturn" || a == "__noreturn__" || a == "packed" || a == "__packed__") { if (!keepl.empty()) keepl += ","; keepl += a; } else cnt++; }
            if (!keepl.empty()) r += "__attribute__((" + keepl + "))";
            i = e; continue;
          }
        }
      }
      r.append(w); i = j; continue;
    }
    r.push_back(s[i]); i++;
  }
  return r;
}
// __auto_type x = e;  ->  __typeof__(e) x = e;
static string norm_autotype(const string& s, long& cnt) {
  if (s.find("__auto_type") == string::npos) return s;
  string r; size_t n = s.size(), i = 0;
  while (i < n) {
    size_t p = s.find("__auto_type", i);
    if (p == string::npos) { r.append(s, i, string::npos); break; }
    r.append(s, i, p - i);
    size_t k = p + 11; while (k < n && isspace((unsigned char)s[k])) k++;
    size_t b = k; while (k < n && isid(s[k])) k++; string nm = s.substr(b, k - b);
    while (k < n && isspace((unsigned char)s[k])) k++;
    if (nm.empty() || k >= n || s[k] != '=') { r += "__auto_type"; i = p + 11; continue; }
    size_t e = k + 1; int d = 0;
    for (; e < n; e++) { if (s[e] == '(' || s[e] == '[' || s[e] == '{') d++; else if (s[e] == ')' || s[e] == ']' || s[e] == '}') { if (d == 0) break; d--; } else if ((s[e] == ';' || s[e] == ',') && d == 0) break; }
    string ex = s.substr(k + 1, e - k - 1);
    r += "__typeof__(" + ex + ") " + nm + " =" + ex; cnt++; i = e;
  }
  return r;
}

// signature of the target for the bit-vector harness: line 1 = return type, then one line  type TAB name  per parameter ("?" for a type that is not a plain scalar declaration)
static void write_sig(const string& head, const string& name, const string& file) {
  ofstream sg(file);
  size_t n = head.size(), pos = string::npos;
  for (size_t i = 0; i + name.size() <= n; i++)
    if (head.compare(i, name.size(), name) == 0 && (i == 0 || !isid(head[i - 1])) && (i + name.size() == n || !isid(head[i + name.size()]))) {
      size_t k = i + name.size(); while (k < n && isspace((unsigned char)head[k])) k++;
      if (k < n && head[k] == '(') { pos = i; break; }
    }
  if (pos == string::npos) { sg << "?\n"; return; }
  string ret = head.substr(0, pos);
  static const set<string> drop = {"static", "inline", "__inline", "__inline__", "extern", "__extension__"};
  string r2; { size_t i = 0; while (i < ret.size()) { if (isid(ret[i])) { size_t j = i; while (j < ret.size() && isid(ret[j])) j++; string w = ret.substr(i, j - i); if (!drop.count(w)) r2 += w; else r2 += " "; i = j; } else { r2.push_back(ret[i]); i++; } } }
  while (!r2.empty() && isspace((unsigned char)r2.back())) r2.pop_back(); size_t a = r2.find_first_not_of(" \t\r\n"); r2 = a == string::npos ? "" : r2.substr(a);
  { string c; bool sp = false; for (char ch : r2) { if (isspace((unsigned char)ch)) { sp = true; continue; } if (sp && !c.empty()) c += ' '; sp = false; c.push_back(ch); } r2 = c; }
  sg << r2 << "\n";
  size_t lp = head.find('(', pos), rp = match_paren(head, lp);
  string plist = head.substr(lp + 1, rp - lp - 2);
  vector<string> parts; { int d = 0; string cur; for (char c : plist) { if (c == '(' || c == '[') d++; else if (c == ')' || c == ']') d--; if (c == ',' && d == 0) { parts.push_back(cur); cur.clear(); } else cur.push_back(c); } parts.push_back(cur); }
  for (auto& p : parts) {
    vector<string> v = idents(p); string pp = p;
    { size_t a2 = pp.find_first_not_of(" \t\r\n"); size_t z = pp.find_last_not_of(" \t\r\n"); pp = a2 == string::npos ? "" : pp.substr(a2, z - a2 + 1); }
    if (pp.empty() || pp == "void") continue;
    if (pp.find('(') != string::npos || pp.find('[') != string::npos || pp == "...") { sg << "?\t?\n"; continue; }
    if (v.size() < 2) { sg << "?\t?\n"; continue; }
    string nm = v.back(); size_t q = pp.rfind(nm);
    string ty = pp.substr(0, q); while (!ty.empty() && isspace((unsigned char)ty.back())) ty.pop_back();
    sg << ty << "\t" << nm << "\n";
  }
}

int main(int argc, char** argv) {
  if (argc < 9) { fprintf(stderr, "usage\n"); return 2; }
  string tu = argv[1], src = argv[2], name = argv[3]; long start = atol(argv[4]), end = atol(argv[5]);
  string contractf = argv[6], outc = argv[7], defsout = argv[8];
  ifstream in(tu); if (!in) { fprintf(stderr, "cannot open tu\n"); return 2; }
  string line; vector<string> defs; // define/undef lines in stream order
  vector<Item> items; Item cur; bool inItem = false; string curfile; long curline = 1;
  string buf; int depth = 0; bool bodyBrace = false; // buf = text of the current item
  long itemline = 0; string itemfile;
  int fnbraceDepth = 0;
  // character-level scan over the whole non-directive text, line by line
  long nrw = 0, nrw_asm = 0;
  auto flush_item = [&](bool isfunc) {
    // trim
    size_t a = buf.find_first_not_of(" \t\r\n"); if (a == string::npos) { buf.clear(); return; }
    Item it; it.text = buf.substr(a); it.file = itemfile; it.line = itemline; it.func = isfunc; it.defs_before = defs.size();
    items.push_back(it); buf.clear();
  };
  bool instr = false, inchr = false; char prev = 0;
  int pdepth = 0; // () and [] depth
  int bdepth = 0; // {} depth
  bool inbody = false; size_t bodyStartPos = 0; // for function body: position in buf where '{' starts
  while (getline(in, line)) {
    if (!line.empty() && line[0] == '#') {
      // line marker?  # 12 "file" flags
      if (line.size() > 2 && isdigit((unsigned char)line[2])) {
        size_t p = 2; long n = 0; while (p < line.size() && isdigit((unsigned char)line[p])) n = n * 10 + (line[p++] - '0');
        size_t q1 = line.find('"', p), q2 = line.rfind('"');
        if (q1 != string::npos && q2 > q1) curfile = line.substr(q1 + 1, q2 - q1 - 1);
        curline = n; continue;
      }
      if (line.compare(0, 8, "#define ") == 0 || line.compare(0, 7, "#undef ") == 0) { defs.push_back(line); continue; }
      continue; // #pragma, #ident, ...
    }
    // regular text line
    if (buf.find_first_not_of(" \t\r\n") == string::npos) { itemline = curline; itemfile = curfile; }
    for (size_t i = 0; i < line.size(); i++) {
      char c = line[i];
      buf.push_back(c);
      if (instr) { if (c == '\\' && i + 1 < line.size()) { buf.push_back(line[++i]); } else if (c == '"') instr = false; continue; }
      if (inchr) { if (c == '\\' && i + 1 < line.size()) { buf.push_back(line[++i]); } else if (c == '\'') inchr = false; continue; }
      if (c == '"') { instr = true; continue; }
      if (c == '\'') { inchr = true; continue; }
      if (c == '(' || c == '[') pdepth++;
      else if (c == ')' || c == ']') pdepth--;
      else if (c == '{') {
        if (bdepth == 0 && pdepth == 0 && !inbody) {
          // function body? previous non-space char of the item (before this brace) is ')'
          size_t k = buf.size() - 1; while (k > 0 && isspace((unsigned char)buf[k - 1])) k--;
          char pc = k > 0 ? buf[k - 1] : 0;
          if (pc == ')') { inbody = true; bodyStartPos = buf.size() - 1; }
        }
        bdepth++;
      } else if (c == '}') {
        bdepth--;
        if (bdepth == 0 && inbody) { flush_item(true); items.back().head = items.back().text.substr(0, bodyStartPos - (buf.empty() ? 0 : 0)); inbody = false; pdepth = 0; /* head fixed below */ }
      } else if (c == ';' && bdepth == 0 && pdepth == 0) { flush_item(false); }
    }
    buf.push_back('\n');
  }
  if (buf.find_first_not_of(" \t\r\n;") != string::npos) flush_item(false);
  long nattr = 0, nauto = 0;
  for (auto& it : items) { it.text = norm_autotype(norm_attrs(it.text, nattr), nauto); }
  // fix heads (text up to the body brace) and names
  for (auto& it : items) if (it.func) {
    // body brace: first '{' at paren depth 0 whose previous non-space is ')'
    int pd = 0; size_t pos = string::npos; bool s = false, ch = false;
    for (size_t i = 0; i < it.text.size(); i++) {
      char c = it.text[i];
      if (s) { if (c == '\\') i++; else if (c == '"') s = false; continue; }
      if (ch) { if (c == '\\') i++; else if (c == '\'') ch = false; continue; }
      if (c == '"') s = true; else if (c == '\'') ch = true;
      else if (c == '(' || c == '[') pd++; else if (c == ')' || c == ']') pd--;
      else if (c == '{' && pd == 0) { size_t k = i; while (k > 0 && isspace((unsigned char)it.text[k - 1])) k--; if (k > 0 && it.text[k - 1] == ')') { pos = i; break; } }
    }
    if (pos == string::npos) { it.func = false; continue; }
    it.head = it.text.substr(0, pos); it.name = funcname(it.head);
  }
  // find the target
  int tgt = -1, cnt = 0, tgt_loose = -1, cnt_loose = 0;
  string sbase = src; // compare by suffix (the marker file may be relative or carry ./)
  for (size_t i = 0; i < items.size(); i++) if (items[i].func && items[i].name == name) {
    cnt_loose++; tgt_loose = i;
    auto& f = items[i].file; bool samefile = f == src || (f.size() >= 2 && src.size() >= f.size() && src.compare(src.size() - f.size(), f.size(), f) == 0) || (f.compare(0, 2, "./") == 0 && src.size() > f.size() - 2 && src.compare(src.size() - (f.size() - 2), f.size() - 2, f, 2) == 0);
    if (samefile && items[i].line >= start - 6 && items[i].line <= end + 6) { if (tgt < 0) tgt = i; cnt++; }
  }
  if (tgt < 0 && cnt_loose >= 1) { // fall back: the definition may come from a macro expansion in another header; accept only a unique one
    if (cnt_loose == 1) tgt = tgt_loose; else { fprintf(stderr, "ambiguous: %d definitions of %s\n", cnt_loose, name.c_str()); return 3; }
  }
  if (tgt < 0) { fprintf(stderr, "notfound: no definition of %s in the preprocessed unit\n", name.c_str()); return 3; }
  // output
  string contract; if (contractf != "-") { ifstream cf(contractf); stringstream ss; ss << cf.rdbuf(); contract = ss.str(); }
  ofstream out(outc), callees(outc + ".callees"), df(defsout);
  write_sig(items[tgt].head, name, outc + ".sig");
  for (size_t d = 0; d < items[tgt].defs_before; d++) df << defs[d] << "\n";
  static const regex asminl("\\b(__asm__|__asm|asm)\\s+(__inline__|__inline|inline)\\b");
  static const regex vol("\\b(__asm__|__asm|asm)\\s+(__volatile__|__volatile)\\b");
  size_t nf = 0, nd = 0; set<string> seen;
  // closure: keep the target and every item that declares a name the kept items use (types, enumerators, variables, prototypes)
  vector<char> keep(items.size(), 0);
  {
    vector<set<string>> prov(items.size()); multimap<string, size_t> byname;
    for (size_t i = 0; i < items.size(); i++) {
      string t = (items[i].func && (int)i != tgt) ? items[i].head + ";" : items[i].text;
      provides(t, prov[i]); for (auto& s : prov[i]) byname.insert({s, i});
    }
    vector<string> work; set<string> done;
    auto push_uses = [&](const string& t) { for (auto& id : idents(t)) if (!done.count(id)) { done.insert(id); work.push_back(id); } };
    keep[tgt] = 1; push_uses(items[tgt].text); push_uses(contract);
    while (!work.empty()) {
      string id = work.back(); work.pop_back();
      auto r = byname.equal_range(id);
      for (auto it = r.first; it != r.second; ++it) {
        size_t i = it->second; if (keep[i]) continue; keep[i] = 1;
        push_uses((items[i].func && (int)i != tgt) ? items[i].head + ";" : items[i].text);
      }
    }
  }
  size_t nkept = 0;
  vector<string> outs; long nbf = 0; (void)nbf;
  for (size_t i = 0; i < items.size(); i++) {
    if (!keep[i]) continue; nkept++;
    string t;
    if (items[i].func && (int)i != tgt) { t = items[i].head + ";"; nd++; callees << items[i].name << "\n"; }
    else { t = items[i].text; if ((int)i == tgt) { nf++; if (!contract.empty()) outs.push_back(contract); } }
    string t2 = regex_replace(t, asminl, "$1");
    t2 = regex_replace(t2, vol, "$1 volatile");
    if (t2 != t) nrw_asm++;
    outs.push_back(t2);
  }
  // struct/union bodies: flexible array members  name[]  ->  name[0]  (gcc accepts a nested struct that ends in one, Frama-C does not);
  // bit-field widths that are not plain integers (kernel macros with __builtin_constant_p, ilog2, ...) are evaluated with gcc and replaced by the value.
  {
    long nflex = 0;
    struct BF { size_t item, from, to; string ex; };
    vector<BF> bfs;
    for (size_t k = 0; k < outs.size(); k++) {
      string& s = outs[k]; if (s.find('{') == string::npos) continue;
      vector<char> kinds; // 'S' struct/union body, 'E' enum body, 'O' other
      int pd = 0; size_t i = 0; bool qmark = false;
      vector<pair<size_t, size_t>> flex; // [from,to) of "[ ]"
      while (i < s.size()) {
        char c = s[i];
        if (c == '"') { i++; while (i < s.size() && s[i] != '"') { if (s[i] == '\\') i++; i++; } i++; continue; }
        if (c == '\'') { i++; while (i < s.size() && s[i] != '\'') { if (s[i] == '\\') i++; i++; } i++; continue; }
        if (c == '(') pd++; else if (c == ')') pd--;
        else if (c == '{') {
          // classify: look back for  struct|union|enum [tag]
          size_t j = i; while (j > 0 && isspace((unsigned char)s[j - 1])) j--;
          char kind = 'O';
          if (j > 0 && (isid(s[j - 1]))) {
            size_t e = j; while (j > 0 && isid(s[j - 1])) j--; string w1 = s.substr(j, e - j);
            if (w1 == "struct" || w1 == "union") kind = 'S'; else if (w1 == "enum") kind = 'E';
            else { size_t m = j; while (m > 0 && isspace((unsigned char)s[m - 1])) m--; size_t e2 = m; while (m > 0 && isid(s[m - 1])) m--; string w2 = s.substr(m, e2 - m);
                   if (w2 == "struct" || w2 == "union") kind = 'S'; else if (w2 == "enum") kind = 'E'; }
          }
          kinds.push_back(kind); qmark = false;
        } else if (c == '}') { if (!kinds.empty()) kinds.pop_back(); qmark = false; }
        else if (!kinds.empty() && kinds.back() == 'S' && pd == 0) {
          if (c == ';' || c == ',') qmark = false;
          else if (c == '[') { size_t m = i + 1; while (m < s.size() && isspace((unsigned char)s[m])) m++; if (m < s.size() && s[m] == ']') { flex.push_back({i, m + 1}); i = m; } else if (m < s.size() && s[m] == '0') { size_t m2 = m + 1; while (m2 < s.size() && isspace((unsigned char)s[m2])) m2++; if (m2 < s.size() && s[m2] == ']') { flex.push_back({i, m2 + 1}); i = m2; } } }
          else if (c == ':') {
            if (i + 1 < s.size() && s[i + 1] == ':') { i += 2; continue; }
            size_t e = i + 1; int d = 0;
            for (; e < s.size(); e++) { if (s[e] == '(' || s[e] == '[') d++; else if (s[e] == ')' || s[e] == ']') d--; else if ((s[e] == ';' || s[e] == ',') && d == 0) break; }
            string ex = s.substr(i + 1, e - i - 1); bool plain = true;
            for (char ch : ex) if (!(isdigit((unsigned char)ch) || isspace((unsigned char)ch))) plain = false;
            if (!plain) bfs.push_back({k, i + 1, e, ex});
            i = e; continue;
          }
        }
        i++;
      }
      for (long q = (long)flex.size() - 1; q >= 0; q--) { s.replace(flex[q].first, flex[q].second - flex[q].first, "[1]"); nflex++; }
      // flex replacements shift later offsets inside this item only for items already recorded in bfs with larger offsets: re-find them below
      if (nflex && !bfs.empty() && bfs.back().item == k) { // recompute bit-field positions of this item after the rewrite
        vector<BF> keepb; for (auto& b : bfs) if (b.item != k) keepb.push_back(b);
        for (auto& b : bfs) if (b.item == k) { size_t p = s.find(b.ex); if (p != string::npos) { b.from = p; b.to = p + b.ex.size(); keepb.push_back(b); } }
        bfs = keepb;
      }
    }
    long nbfv = 0;
    if (!bfs.empty()) {
      string prog; for (auto& o : outs) { prog += o; prog += "\n"; }
      for (size_t j = 0; j < bfs.size(); j++) prog += "enum { __pbsd_e" + to_string(j) + " = (" + bfs[j].ex + ") };\nconst long __pbsd_bf" + to_string(j) + " = __pbsd_e" + to_string(j) + ";\n";
      string tmp = outc + ".bf.c"; { ofstream t(tmp); t << prog; }
      string cmd = "gcc -S -O0 -w -x c -o - '" + tmp + "' 2>/dev/null";
      FILE* p = popen(cmd.c_str(), "r"); string asmout; char b[4096]; size_t nr; while (p && (nr = fread(b, 1, sizeof b, p)) > 0) asmout.append(b, nr); if (p) pclose(p);
      remove(tmp.c_str());
      vector<size_t> order(bfs.size()); for (size_t j = 0; j < bfs.size(); j++) order[j] = j;
      // replace back to front within each item (positions only valid in descending order)
      sort(order.begin(), order.end(), [&](size_t a, size_t b2) { return bfs[a].item != bfs[b2].item ? bfs[a].item < bfs[b2].item : bfs[a].from > bfs[b2].from; });
      for (size_t j : order) {
        string lab = "\n__pbsd_bf" + to_string(j) + ":"; size_t pos = asmout.find(lab); long val = -1;
        if (pos != string::npos) { size_t nxt = asmout.find("\n__pbsd", pos + 1); size_t q2 = asmout.find(".quad", pos), l2 = asmout.find(".long", pos);
          if (q2 != string::npos && (nxt == string::npos || q2 < nxt) && (l2 == string::npos || q2 < l2)) val = atol(asmout.c_str() + q2 + 5);
          else if (l2 != string::npos && (nxt == string::npos || l2 < nxt)) val = atol(asmout.c_str() + l2 + 5); }
        if (val < 0) { fprintf(stderr, "bitfield width not evaluated: %s\n", bfs[j].ex.substr(0, 60).c_str()); continue; }
        outs[bfs[j].item].replace(bfs[j].from, bfs[j].to - bfs[j].from, " " + to_string(val) + " "); nbfv++;
      }
    }
    nbf = nbfv; nrw_asm += 0; nattr += 0; (void)nflex;
    fprintf(stderr, "flexarrays=%ld ", nflex);
  }
  for (auto& o : outs) out << o << "\n";
  fprintf(stderr, "ok items=%zu kept=%zu protos=%zu rewrites=%ld attrs=%ld auto=%ld bitfields=%ld\n", items.size(), nkept, nd, nrw_asm, nattr, nauto, nbf);
  return 0;
}
