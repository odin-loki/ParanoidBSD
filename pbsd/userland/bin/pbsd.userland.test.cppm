module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.test;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/test/test.c — lexer and expression helpers (logic-only).
export namespace pbsd::userland::bin::test {

enum class TokenType : unsigned short {
    Eoi = 0,
    Operand = 1,
    Unop = 0x100,
    Binop = 0x200,
    Bunop = 0x300,
    Bbinop = 0x400,
    Paren = 0x500,
};

enum class Token : unsigned short {
    Eoi = 0,
    Operand = 1,
    FilRd = static_cast<unsigned short>(TokenType::Unop) + 1,
    FilWr,
    FilEx,
    FilExist,
    FilReg,
    FilDir,
    FilCdev,
    FilBdev,
    FilFifo,
    FilSock,
    FilSym,
    FilGz,
    FilTt,
    FilSuid,
    FilSgid,
    FilStck,
    StrEz,
    StrNz,
    FilUid,
    FilGid,
    FilNt = static_cast<unsigned short>(TokenType::Binop) + 1,
    FilOt,
    FilEq,
    StrEq,
    StrNe,
    StrLt,
    StrGt,
    IntEq,
    IntNe,
    IntGe,
    IntGt,
    IntLe,
    IntLt,
    Unot = static_cast<unsigned short>(TokenType::Bunop) + 1,
    Band = static_cast<unsigned short>(TokenType::Bbinop) + 1,
    Bor,
    Lparen = static_cast<unsigned short>(TokenType::Paren) + 1,
    Rparen,
};

struct OpEntry {
    char op_text[3];
    Token op;
};

inline constexpr OpEntry kOps1[] = {
    {"=", Token::StrEq}, {"<", Token::StrLt}, {">", Token::StrGt},
    {"!", Token::Unot}, {"(", Token::Lparen}, {")", Token::Rparen},
};

inline constexpr OpEntry kOpsM1[] = {
    {"r", Token::FilRd}, {"w", Token::FilWr}, {"x", Token::FilEx},
    {"e", Token::FilExist}, {"f", Token::FilReg}, {"d", Token::FilDir},
    {"c", Token::FilCdev}, {"b", Token::FilBdev}, {"p", Token::FilFifo},
    {"u", Token::FilSuid}, {"g", Token::FilSgid}, {"k", Token::FilStck},
    {"s", Token::FilGz}, {"t", Token::FilTt}, {"z", Token::StrEz},
    {"n", Token::StrNz}, {"h", Token::FilSym}, {"O", Token::FilUid},
    {"G", Token::FilGid}, {"L", Token::FilSym}, {"S", Token::FilSock},
    {"a", Token::Band}, {"o", Token::Bor},
};

inline constexpr OpEntry kOps2[] = {
    {"==", Token::StrEq}, {"!=", Token::StrNe},
};

inline constexpr OpEntry kOpsM2[] = {
    {"eq", Token::IntEq}, {"ne", Token::IntNe}, {"ge", Token::IntGe},
    {"gt", Token::IntGt}, {"le", Token::IntLe}, {"lt", Token::IntLt},
    {"nt", Token::FilNt}, {"ot", Token::FilOt}, {"ef", Token::FilEq},
};

[[nodiscard]] inline TokenType token_type(Token t) noexcept {
    return static_cast<TokenType>(static_cast<unsigned short>(t) & 0xff00u);
}

[[nodiscard]] inline Token find_op_1char(const OpEntry* ops, std::size_t count,
                                         const char* s) noexcept {
    if (s == nullptr || s[0] == '\0') {
        return Token::Operand;
    }
    const char c = s[0];
    for (std::size_t i = 0; i < count; ++i) {
        if (ops[i].op_text[0] == c && ops[i].op_text[1] == '\0') {
            return ops[i].op;
        }
    }
    return Token::Operand;
}

[[nodiscard]] inline Token find_op_2char(const OpEntry* ops, std::size_t count,
                                         const char* s) noexcept {
    if (s == nullptr || s[0] == '\0' || s[1] == '\0') {
        return Token::Operand;
    }
    for (std::size_t i = 0; i < count; ++i) {
        if (s[0] == ops[i].op_text[0] && s[1] == ops[i].op_text[1] &&
            ops[i].op_text[2] == '\0') {
            return ops[i].op;
        }
    }
    return Token::Operand;
}

[[nodiscard]] inline Token find_op(const char* s) noexcept {
    if (s == nullptr || s[0] == '\0') {
        return Token::Operand;
    }
    if (s[1] == '\0') {
        return find_op_1char(kOps1, sizeof(kOps1) / sizeof(kOps1[0]), s);
    }
    if (s[2] == '\0') {
        if (s[0] == '-') {
            return find_op_1char(kOpsM1, sizeof(kOpsM1) / sizeof(kOpsM1[0]), s + 1);
        }
        return find_op_2char(kOps2, sizeof(kOps2) / sizeof(kOps2[0]), s);
    }
    if (s[3] == '\0' && s[0] == '-') {
        return find_op_2char(kOpsM2, sizeof(kOpsM2) / sizeof(kOpsM2[0]), s + 1);
    }
    return Token::Operand;
}

struct LexContext {
    int argc{0};
    char** wp{nullptr};
    int parenlevel{0};
};

[[nodiscard]] inline bool is_unop_operand(const LexContext& ctx) noexcept {
    if (ctx.argc == 1) {
        return true;
    }
    if (ctx.wp == nullptr) {
        return false;
    }
    char* s = ctx.wp[1];
    if (ctx.argc == 2) {
        return ctx.parenlevel == 1 && s != nullptr && hosted::cstrcmp(s, ")") == 0;
    }
    char* t = ctx.wp[2];
    const Token num = find_op(s);
    return token_type(num) == TokenType::Binop &&
           (ctx.parenlevel == 0 || (t != nullptr && t[0] == ')' && t[1] == '\0'));
}

[[nodiscard]] inline Token lex_token(const LexContext& ctx, char* s) noexcept {
    if (s == nullptr) {
        return Token::Eoi;
    }
    const Token num = find_op(s);
    if (((token_type(num) == TokenType::Unop || token_type(num) == TokenType::Bunop) &&
         is_unop_operand(ctx)) ||
        (num == Token::Lparen && ctx.argc == 1) ||
        (num == Token::Rparen && ctx.argc == 1)) {
        return Token::Operand;
    }
    return num;
}

[[nodiscard]] inline bool is_bracket_invocation(const char* prog, const char* last) noexcept {
    if (last != nullptr && hosted::cstrcmp(last, "[") == 0) {
        return true;
    }
    if (prog == nullptr) {
        return false;
    }
    const char* p = prog;
    for (const char* q = prog; *q != '\0'; ++q) {
        if (*q == '/') {
            p = q + 1;
        }
    }
    return hosted::cstrcmp(p, "[") == 0;
}

[[nodiscard]] inline Result<int> parse_int64(const char* s) noexcept {
    if (s == nullptr || s[0] == '\0') {
        return result_err<int>(Status::Invalid);
    }
    bool neg = false;
    const char* p = s;
    if (*p == '-') {
        neg = true;
        ++p;
    } else if (*p == '+') {
        ++p;
    }
    std::int64_t val = 0;
    if (*p == '\0') {
        return result_err<int>(Status::Invalid);
    }
    while (*p != '\0') {
        if (*p < '0' || *p > '9') {
            return result_err<int>(Status::Invalid);
        }
        val = val * 10 + (*p - '0');
        ++p;
    }
    if (neg) {
        val = -val;
    }
    if (val < static_cast<std::int64_t>(-2147483647 - 1) ||
        val > static_cast<std::int64_t>(2147483647)) {
        return result_err<int>(Status::Invalid);
    }
    return result_ok(static_cast<int>(val));
}

[[nodiscard]] inline Result<int> int_compare(const char* s1, const char* s2) noexcept {
    const auto q1 = parse_int64(s1);
    const auto q2 = parse_int64(s2);
    if (!q1.has_value() || !q2.has_value()) {
        return result_err<int>(Status::Invalid);
    }
    if (q1.value > q2.value) {
        return result_ok(1);
    }
    if (q1.value < q2.value) {
        return result_ok(-1);
    }
    return result_ok(0);
}

[[nodiscard]] inline bool str_equal(const char* a, const char* b) noexcept {
    return hosted::cstrcmp(a, b) == 0;
}

[[nodiscard]] inline bool str_nonempty(const char* s) noexcept {
    return s != nullptr && s[0] != '\0';
}

[[nodiscard]] inline bool evaluate_string_relation(Token op, const char* a,
                                                   const char* b) noexcept {
    switch (op) {
    case Token::StrEq:
        return str_equal(a, b);
    case Token::StrNe:
        return !str_equal(a, b);
    case Token::StrLt:
        return hosted::cstrcmp(a, b) < 0;
    case Token::StrGt:
        return hosted::cstrcmp(a, b) > 0;
    default:
        return false;
    }
}

} // namespace pbsd::userland::bin::test
