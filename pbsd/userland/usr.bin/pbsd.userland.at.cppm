module;

export module pbsd.userland.at;

export import pbsd.core;

/// Port of hbsd/src/usr.bin/at/at.c — job queue path constants.
export namespace pbsd::userland::usr_bin::at {

inline constexpr const char* kJobDir = "/var/spool/cron/atjobs";
inline constexpr const char* kLockFile = "/var/spool/cron/atjobs/.lockfile";

namespace detail {
[[nodiscard]] inline bool ieq(const char* a, const char* b) noexcept {
    if (a == nullptr || b == nullptr) {
        return false;
    }
    while (*a != '\0' && *b != '\0') {
        char ca = *a;
        char cb = *b;
        if (ca >= 'A' && ca <= 'Z') {
            ca = static_cast<char>(ca - 'A' + 'a');
        }
        if (cb >= 'A' && cb <= 'Z') {
            cb = static_cast<char>(cb - 'A' + 'a');
        }
        if (ca != cb) {
            return false;
        }
        ++a;
        ++b;
    }
    return *a == *b;
}
} // namespace detail

[[nodiscard]] inline Result<unsigned> parse_relative_minutes(const char* spec) noexcept {
    if (spec == nullptr || *spec != '+') {
        return result_err<unsigned>(Status::Invalid);
    }
    ++spec;
    unsigned val = 0;
    for (; *spec >= '0' && *spec <= '9'; ++spec) {
        val = val * 10U + static_cast<unsigned>(*spec - '0');
    }
    if (val == 0) {
        return result_err<unsigned>(Status::Invalid);
    }
    const bool hours = detail::ieq(spec, "hours") || detail::ieq(spec, "hour");
    const bool mins = detail::ieq(spec, "minutes") || detail::ieq(spec, "minute");
    if (!hours && !mins) {
        return result_err<unsigned>(Status::Invalid);
    }
    if (hours) {
        val *= 60U;
    }
    return result_ok(val);
}

} // namespace pbsd::userland::usr_bin::at
