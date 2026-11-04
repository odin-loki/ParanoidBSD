module;
#include <cstddef>

export module pbsd.userland.renice;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/renice/renice.c — priority scope helpers (logic-only).
export namespace pbsd::userland::usr_bin::renice {

enum class Scope : int {
    Process = 0,
    Pgrp = 1,
    User = 2,
};

inline constexpr int kPrioMin = -20;
inline constexpr int kPrioMax = 20;

struct Options {
    Scope scope{Scope::Process};
    int priority{0};
    bool incremental{false};
    bool have_priority{false};
};

[[nodiscard]] inline Result<int> parse_priority(const char* s) noexcept {
    if (s == nullptr || *s == '\0') {
        return result_err<int>(Status::Invalid);
    }
    const char* p = s;
    bool negative = false;
    if (*p == '+' || *p == '-') {
        negative = (*p == '-');
        ++p;
    }
    if (*p == '\0') {
        return result_err<int>(Status::Invalid);
    }
    int val = 0;
    for (; *p >= '0' && *p <= '9'; ++p) {
        val = val * 10 + (*p - '0');
    }
    if (*p != '\0') {
        return result_err<int>(Status::Invalid);
    }
    if (negative) {
        val = -val;
    }
    return result_ok(val);
}

[[nodiscard]] inline int clamp_priority(int prio) noexcept {
    if (prio > kPrioMax) {
        return kPrioMax;
    }
    if (prio < kPrioMin) {
        return kPrioMin;
    }
    return prio;
}

[[nodiscard]] inline int apply_increment(int old_prio, int delta) noexcept {
    return clamp_priority(old_prio + delta);
}

[[nodiscard]] inline Result<Scope> parse_scope_flag(const char* arg) noexcept {
    if (arg == nullptr) {
        return result_err<Scope>(Status::Invalid);
    }
    if (hosted::cstrcmp(arg, "-g") == 0) {
        return result_ok(Scope::Pgrp);
    }
    if (hosted::cstrcmp(arg, "-u") == 0) {
        return result_ok(Scope::User);
    }
    if (hosted::cstrcmp(arg, "-p") == 0) {
        return result_ok(Scope::Process);
    }
    return result_err<Scope>(Status::Invalid);
}

[[nodiscard]] inline bool is_delimiter(const char* arg) noexcept {
    return arg != nullptr && hosted::cstrcmp(arg, "--") == 0;
}

[[nodiscard]] inline bool is_increment_flag(const char* arg) noexcept {
    return arg != nullptr && hosted::cstrcmp(arg, "-n") == 0;
}

} // namespace pbsd::userland::usr_bin::renice
