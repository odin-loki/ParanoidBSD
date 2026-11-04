module;

export module pbsd.userland.random;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/random/random.c — random line selection (logic-only).
export namespace pbsd::userland::usr_bin::random {

[[nodiscard]] inline Result<double> parse_denominator(const char* arg) noexcept {
    if (arg == nullptr || *arg == '\0') {
        return result_err<double>(Status::Invalid);
    }
    double val = 0.0;
    bool seen_dot = false;
    for (const char* p = arg; *p != '\0'; ++p) {
        if (*p == '.') {
            if (seen_dot) {
                return result_err<double>(Status::Invalid);
            }
            seen_dot = true;
            continue;
        }
        if (*p < '0' || *p > '9') {
            return result_err<double>(Status::Invalid);
        }
        val = val * 10.0 + static_cast<double>(*p - '0');
    }
    if (val <= 0.0) {
        return result_err<double>(Status::Invalid);
    }
    return result_ok(1.0 / val);
}

[[nodiscard]] inline bool select_line(double threshold, double sample) noexcept {
    return sample < threshold;
}

[[nodiscard]] inline Result<int> parse_unbuffer_flag(char ch) noexcept {
    if (ch != 'u' && ch != 'U' && ch != 'w') {
        return result_err<int>(Status::Invalid);
    }
    return result_ok(0);
}

} // namespace pbsd::userland::usr_bin::random
