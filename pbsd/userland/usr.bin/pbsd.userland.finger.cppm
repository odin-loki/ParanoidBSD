module;

export module pbsd.userland.finger;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/finger/finger.c — output format flags.
export namespace pbsd::userland::usr_bin::finger {

struct Options {
    bool long_format{false};
    bool no_plan{false};
    bool suppress{false};
    bool mail_status{false};
    bool host_field{true};
};

[[nodiscard]] inline Result<Options> parse_short_option(char opt) noexcept {
    Options o{};
    switch (opt) {
    case 'l':
        o.long_format = true;
        return result_ok(o);
    case 's':
        o.suppress = true;
        return result_ok(o);
    case 'm':
        o.mail_status = true;
        return result_ok(o);
    case 'h':
        o.host_field = true;
        return result_ok(o);
    case 'o':
        o.host_field = false;
        return result_ok(o);
    case 'p':
        o.no_plan = true;
        return result_ok(o);
    default:
        return result_err<Options>(Status::Invalid);
    }
}

[[nodiscard]] inline bool looks_like_phone_extension(const char* s) noexcept {
    if (s == nullptr) {
        return false;
    }
    int digits = 0;
    for (const char* p = s; *p != '\0'; ++p) {
        if (*p >= '0' && *p <= '9') {
            ++digits;
        } else if (*p != ' ' && *p != '-') {
            return false;
        }
    }
    return digits == 4;
}

} // namespace pbsd::userland::usr_bin::finger
