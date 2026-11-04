module;
#include <cstddef>

export module pbsd.userland.kill;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/kill/kill.c — signal/name parsing and pid validation.
export namespace pbsd::userland::bin::kill {

struct SignalEntry {
    const char* name;
    int number;
};

inline constexpr SignalEntry kSignals[] = {
    {"HUP", 1},  {"INT", 2},  {"QUIT", 3},  {"ILL", 4},  {"TRAP", 5},
    {"ABRT", 6}, {"EMT", 7},  {"FPE", 8},   {"KILL", 9}, {"BUS", 10},
    {"SEGV", 11},{"SYS", 12}, {"PIPE", 13}, {"ALRM", 14},{"TERM", 15},
    {"URG", 16}, {"STOP", 17},{"TSTP", 18},{"CONT", 19},{"CHLD", 20},
    {"TTIN", 21},{"TTOU", 22},{"IO", 23},  {"XCPU", 24},{"XFSZ", 25},
    {"VTALRM", 26},{"PROF", 27},{"WINCH", 28},{"INFO", 29},{"USR1", 30},
    {"USR2", 31},
};

enum class ListMode : unsigned char {
    None,
    AllNames,
    NameForNumber,
};

struct Options {
    ListMode list{ListMode::None};
    int signal{15}; // SIGTERM
    int list_number{-1};
};

[[nodiscard]] inline bool names_equal(const char* a, const char* b) noexcept {
    return hosted::cstrcmp(a, b) == 0;
}

[[nodiscard]] inline Result<int> lookup_signal_name(const char* name) noexcept {
    if (name == nullptr || name[0] == '\0') {
        return result_err<int>(Status::Invalid);
    }
    for (const auto& e : kSignals) {
        if (names_equal(e.name, name)) {
            return result_ok(e.number);
        }
    }
    return result_err<int>(Status::NotFound);
}

[[nodiscard]] inline Result<int> lookup_signal_number(int num) noexcept {
    if (num < 0 || num > 31) {
        return result_err<int>(Status::NotFound);
    }
    for (const auto& e : kSignals) {
        if (e.number == num) {
            return result_ok(num);
        }
    }
    return result_ok(num);
}

[[nodiscard]] inline Result<int> parse_signal_token(const char* tok) noexcept {
    if (tok == nullptr || tok[0] == '\0') {
        return result_err<int>(Status::Invalid);
    }
    if (tok[0] == '0' && tok[1] == '\0') {
        return result_ok(0);
    }
    if (tok[0] >= '0' && tok[0] <= '9') {
        long val = 0;
        for (const char* p = tok; *p != '\0'; ++p) {
            if (*p < '0' || *p > '9') {
                return result_err<int>(Status::Invalid);
            }
            val = val * 10 + (*p - '0');
        }
        if (val >= 128) {
            val -= 128;
        }
        return lookup_signal_number(static_cast<int>(val));
    }
    if (tok[0] == '-' && tok[1] != '\0') {
        return parse_signal_token(tok + 1);
    }
    return lookup_signal_name(tok);
}

[[nodiscard]] inline Result<long> parse_pid(const char* s) noexcept {
    if (s == nullptr || s[0] == '\0') {
        return result_err<long>(Status::Invalid);
    }
    long val = 0;
    for (const char* p = s; *p != '\0'; ++p) {
        if (*p < '0' || *p > '9') {
            return result_err<long>(Status::Invalid);
        }
        val = val * 10 + (*p - '0');
        if (val > 2147483647L) {
            return result_err<long>(Status::Invalid);
        }
    }
    return result_ok(val);
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv) noexcept {
    Options opt{};
    if (argv == nullptr || argc < 2) {
        return result_err<Options>(Status::Invalid);
    }

    int i = 1;
    if (argv[i] != nullptr && names_equal(argv[i], "-l")) {
        opt.list = ListMode::AllNames;
        ++i;
        if (i < argc && argv[i] != nullptr) {
            const auto num = parse_pid(argv[i]);
            if (!num.has_value()) {
                return result_err<Options>(Status::Invalid);
            }
            opt.list = ListMode::NameForNumber;
            opt.list_number = static_cast<int>(num.value);
            ++i;
        }
        if (i != argc) {
            return result_err<Options>(Status::Invalid);
        }
        return result_ok(opt);
    }

    if (argv[i] != nullptr && names_equal(argv[i], "-s")) {
        ++i;
        if (i >= argc || argv[i] == nullptr) {
            return result_err<Options>(Status::Invalid);
        }
        const auto sig = parse_signal_token(argv[i]);
        if (!sig.has_value()) {
            return result_err<Options>(sig.status);
        }
        opt.signal = sig.value;
        ++i;
    } else if (argv[i] != nullptr && argv[i][0] == '-' && argv[i][1] != '\0' &&
               argv[i][1] != '-') {
        const auto sig = parse_signal_token(argv[i] + 1);
        if (!sig.has_value()) {
            return result_err<Options>(sig.status);
        }
        opt.signal = sig.value;
        ++i;
    }

    if (i < argc && argv[i] != nullptr && names_equal(argv[i], "--")) {
        ++i;
    }
    if (i >= argc) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

[[nodiscard]] inline unsigned signal_name_count() noexcept {
    return static_cast<unsigned>(sizeof(kSignals) / sizeof(kSignals[0]));
}

[[nodiscard]] inline const char* signal_name_at(unsigned index) noexcept {
    if (index >= signal_name_count()) {
        return nullptr;
    }
    return kSignals[index].name;
}

} // namespace pbsd::userland::bin::kill
