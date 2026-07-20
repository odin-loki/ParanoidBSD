module;

export module pbsd.userland.lock;

export import pbsd.core;

/// Port of hbsd/src/usr.bin/lock/lock.c — terminal lock timeout parsing (logic-only).
export namespace pbsd::userland::usr_bin::lock {

inline constexpr int kDefaultTimeoutMin = 15;

[[nodiscard]] inline Result<int> parse_timeout_minutes(const char* arg) noexcept {
    if (arg == nullptr || arg[0] != '-') {
        return result_err<int>(Status::Invalid);
    }
    int val = 0;
    for (const char* p = arg + 1; *p != '\0'; ++p) {
        if (*p < '0' || *p > '9') {
            return result_err<int>(Status::Invalid);
        }
        val = val * 10 + (*p - '0');
    }
    if (val <= 0) {
        return result_err<int>(Status::Invalid);
    }
    return result_ok(val);
}

[[nodiscard]] inline bool verify_password(const char* typed, const char* expected) noexcept {
    if (typed == nullptr || expected == nullptr) {
        return false;
    }
    int i = 0;
    while (typed[i] != '\0' && expected[i] != '\0') {
        if (typed[i] != expected[i]) {
            return false;
        }
        ++i;
    }
    return typed[i] == expected[i];
}

[[nodiscard]] inline Result<int> parse_args(int argc, char* const* argv) noexcept {
    if (argv == nullptr || argc < 1) {
        return result_err<int>(Status::Invalid);
    }
    return result_ok(0);
}

} // namespace pbsd::userland::usr_bin::lock
