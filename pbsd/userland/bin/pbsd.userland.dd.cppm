module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.dd;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/dd/args.c — JCL operand parsing (logic-only).
export namespace pbsd::userland::bin::dd {

using DdFlags = std::uint64_t;

inline constexpr DdFlags C_ASCII = 0x0000000000000001ull;
inline constexpr DdFlags C_BLOCK = 0x0000000000000002ull;
inline constexpr DdFlags C_BS = 0x0000000000000004ull;
inline constexpr DdFlags C_CBS = 0x0000000000000008ull;
inline constexpr DdFlags C_COUNT = 0x0000000000000010ull;
inline constexpr DdFlags C_SYNC = 0x0000000000800000ull;
inline constexpr DdFlags C_IFULLBLOCK = 0x0000000400000000ull;
inline constexpr DdFlags C_IDIRECT = 0x0000000800000000ull;
inline constexpr DdFlags C_ODIRECT = 0x0000001000000000ull;
inline constexpr DdFlags C_NOINFO = 0x0000000020000000ull;
inline constexpr DdFlags C_NOXFER = 0x0000000010000000ull;
inline constexpr DdFlags C_PROGRESS = 0x0000000040000000ull;

enum class Operand : unsigned char {
    Bs, Cbs, Conv, Count, Files, Fillchar, Ibs, If, Iflag, Obs, Of, Oflag,
    Iseek, Oseek, Seek, Skip, Speed, Status,
};

struct OperandSpec {
    const char* name;
    Operand op;
    DdFlags set;
    DdFlags noset;
};

inline constexpr OperandSpec kOperands[] = {
    {"bs", Operand::Bs, C_BS, C_BS},
    {"cbs", Operand::Cbs, C_CBS, C_CBS},
    {"conv", Operand::Conv, 0, 0},
    {"count", Operand::Count, C_COUNT, C_COUNT},
    {"files", Operand::Files, 0, 0},
    {"fillchar", Operand::Fillchar, 0, 0},
    {"ibs", Operand::Ibs, 0, C_BS},
    {"if", Operand::If, 0, 0},
    {"iflag", Operand::Iflag, 0, 0},
    {"obs", Operand::Obs, 0, C_BS},
    {"of", Operand::Of, 0, 0},
    {"oflag", Operand::Oflag, 0, 0},
    {"iseek", Operand::Iseek, 0, 0},
    {"oseek", Operand::Oseek, 0, 0},
    {"seek", Operand::Seek, 0, 0},
    {"skip", Operand::Skip, 0, 0},
    {"speed", Operand::Speed, 0, 0},
    {"status", Operand::Status, 0, 0},
};

struct ConvSpec {
    const char* name;
    DdFlags set;
    DdFlags noset;
};

inline constexpr ConvSpec kConversions[] = {
    {"ascii", C_ASCII, 0},
    {"block", C_BLOCK, 0},
    {"sync", C_SYNC, C_IFULLBLOCK},
    {"swab", 0, 0},
    {"unblock", 0, C_BLOCK},
};

struct ParsedOperand {
    Operand op;
    const char* value;
};

struct JclState {
    DdFlags ddflags{0};
    std::size_t ibs{512};
    std::size_t obs{512};
    std::size_t cbs{0};
    std::uint64_t count{0};
    bool count_unlimited{false};
};

[[nodiscard]] inline int compare_cstr(const char* a, const char* b) noexcept {
    return hosted::cstrcmp(a, b);
}

[[nodiscard]] inline Result<const OperandSpec*> lookup_operand(const char* name) noexcept {
    if (name == nullptr) {
        return result_err<const OperandSpec*>(Status::Invalid);
    }
    for (const auto& e : kOperands) {
        if (compare_cstr(e.name, name) == 0) {
            return result_ok(&e);
        }
    }
    return result_err<const OperandSpec*>(Status::NotFound);
}

[[nodiscard]] inline Result<const ConvSpec*> lookup_conv(const char* name) noexcept {
    if (name == nullptr) {
        return result_err<const ConvSpec*>(Status::Invalid);
    }
    for (const auto& e : kConversions) {
        if (compare_cstr(e.name, name) == 0) {
            return result_ok(&e);
        }
    }
    return result_err<const ConvSpec*>(Status::NotFound);
}

[[nodiscard]] inline Result<std::uint64_t> postfix_multiplier(char expr) noexcept {
    switch (expr) {
    case 'B':
    case 'b':
        return result_ok<std::uint64_t>(512);
    case 'K':
    case 'k':
        return result_ok<std::uint64_t>(1ull << 10);
    case 'M':
    case 'm':
        return result_ok<std::uint64_t>(1ull << 20);
    case 'G':
    case 'g':
        return result_ok<std::uint64_t>(1ull << 30);
    case 'W':
    case 'w':
        return result_ok<std::uint64_t>(static_cast<std::uint64_t>(sizeof(int)));
    default:
        return result_ok<std::uint64_t>(0);
    }
}

[[nodiscard]] inline Result<std::uint64_t> parse_numeric(const char* val,
                                                         bool allow_product) noexcept {
    if (val == nullptr || val[0] == '\0') {
        return result_err<std::uint64_t>(Status::Invalid);
    }
    std::uint64_t num = 0;
    for (const char* p = val; *p >= '0' && *p <= '9'; ++p) {
        num = num * 10 + static_cast<std::uint64_t>(*p - '0');
    }
    const char* expr = val;
    while (*expr >= '0' && *expr <= '9') {
        ++expr;
    }
    const auto mult = postfix_multiplier(*expr);
    if (mult.has_value() && mult.value != 0) {
        num *= mult.value;
        ++expr;
    }
    if (*expr == '\0') {
        return result_ok(num);
    }
    if (!allow_product || (*expr != 'x' && *expr != 'X' && *expr != '*')) {
        return result_err<std::uint64_t>(Status::Invalid);
    }
    const auto tail = parse_numeric(expr + 1, true);
    if (!tail.has_value()) {
        return result_err<std::uint64_t>(tail.status);
    }
    return result_ok(num * tail.value);
}

[[nodiscard]] inline Result<ParsedOperand> split_operand(char* oper) noexcept {
    if (oper == nullptr) {
        return result_err<ParsedOperand>(Status::Invalid);
    }
    char* eq = nullptr;
    for (char* p = oper; *p != '\0'; ++p) {
        if (*p == '=') {
            eq = p;
            break;
        }
    }
    if (eq == nullptr || eq[1] == '\0') {
        return result_err<ParsedOperand>(Status::Invalid);
    }
    *eq = '\0';
    const auto spec = lookup_operand(oper);
    if (!spec.has_value()) {
        return result_err<ParsedOperand>(spec.status);
    }
    ParsedOperand out{};
    out.op = spec.value->op;
    out.value = eq + 1;
    return result_ok(out);
}

[[nodiscard]] inline Result<JclState> apply_operand(JclState st, const OperandSpec* spec,
                                                    const char* value) noexcept {
    if (spec == nullptr || value == nullptr) {
        return result_err<JclState>(Status::Invalid);
    }
    if ((st.ddflags & spec->noset) != 0) {
        return result_err<JclState>(Status::Invalid);
    }
    st.ddflags |= spec->set;

    switch (spec->op) {
    case Operand::Bs: {
        const auto n = parse_numeric(value, false);
        if (!n.has_value() || n.value < 1) {
            return result_err<JclState>(Status::Invalid);
        }
        st.ibs = static_cast<std::size_t>(n.value);
        st.obs = static_cast<std::size_t>(n.value);
        break;
    }
    case Operand::Ibs: {
        const auto n = parse_numeric(value, false);
        if (!n.has_value() || n.value < 1) {
            return result_err<JclState>(Status::Invalid);
        }
        st.ibs = static_cast<std::size_t>(n.value);
        break;
    }
    case Operand::Obs: {
        const auto n = parse_numeric(value, false);
        if (!n.has_value() || n.value < 1) {
            return result_err<JclState>(Status::Invalid);
        }
        st.obs = static_cast<std::size_t>(n.value);
        break;
    }
    case Operand::Cbs: {
        const auto n = parse_numeric(value, false);
        if (!n.has_value() || n.value < 1) {
            return result_err<JclState>(Status::Invalid);
        }
        st.cbs = static_cast<std::size_t>(n.value);
        break;
    }
    case Operand::Count: {
        const auto n = parse_numeric(value, false);
        if (!n.has_value()) {
            return result_err<JclState>(Status::Invalid);
        }
        if (n.value == 0) {
            st.count_unlimited = true;
        } else {
            st.count = n.value;
        }
        break;
    }
    case Operand::Status:
        if (compare_cstr(value, "none") == 0) {
            st.ddflags |= C_NOINFO;
        } else if (compare_cstr(value, "noxfer") == 0) {
            st.ddflags |= C_NOXFER;
        } else if (compare_cstr(value, "progress") == 0) {
            st.ddflags |= C_PROGRESS;
        } else {
            return result_err<JclState>(Status::Invalid);
        }
        break;
    case Operand::Conv: {
        char buf[64];
        const char* p = value;
        while (*p != '\0') {
            std::size_t i = 0;
            while (p[i] != '\0' && p[i] != ',') {
                ++i;
            }
            if (i >= sizeof(buf)) {
                return result_err<JclState>(Status::Invalid);
            }
            for (std::size_t j = 0; j < i; ++j) {
                buf[j] = p[j];
            }
            buf[i] = '\0';
            const auto conv = lookup_conv(buf);
            if (!conv.has_value()) {
                return result_err<JclState>(conv.status);
            }
            if ((st.ddflags & conv.value->noset) != 0) {
                return result_err<JclState>(Status::Invalid);
            }
            st.ddflags |= conv.value->set;
            p += i;
            if (*p == ',') {
                ++p;
            }
        }
        break;
    }
    default:
        break;
    }
    return result_ok(st);
}

[[nodiscard]] inline Result<JclState> parse_jcl_operands(char* const* argv) noexcept {
    JclState st{};
    if (argv == nullptr) {
        return result_ok(st);
    }
    for (int i = 0; argv[i] != nullptr; ++i) {
        char oper_buf[256];
        const char* src = argv[i];
        std::size_t len = hosted::cstrlen(src);
        if (len >= sizeof(oper_buf)) {
            return result_err<JclState>(Status::Invalid);
        }
        for (std::size_t j = 0; j <= len; ++j) {
            oper_buf[j] = src[j];
        }
        const auto split = split_operand(oper_buf);
        if (!split.has_value()) {
            return result_err<JclState>(split.status);
        }
        const auto spec = lookup_operand(oper_buf);
        if (!spec.has_value()) {
            return result_err<JclState>(spec.status);
        }
        const auto applied = apply_operand(st, spec.value, split.value.value);
        if (!applied.has_value()) {
            return result_err<JclState>(applied.status);
        }
        st = applied.value;
    }
    return result_ok(st);
}

} // namespace pbsd::userland::bin::dd
