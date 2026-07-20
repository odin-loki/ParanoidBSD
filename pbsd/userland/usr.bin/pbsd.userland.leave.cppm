module;
#include <cstddef>

export module pbsd.userland.leave;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/leave/leave.c — departure-time parsing (logic-only).
export namespace pbsd::userland::usr_bin::leave {

inline constexpr unsigned kFiveMin = 5 * 60;
inline constexpr unsigned kOneMin = 60;

struct ParsedTime {
    int hours{};
    int minutes{};
    bool relative{false};
};

[[nodiscard]] inline bool is_digit(char c) noexcept {
    return c >= '0' && c <= '9';
}

[[nodiscard]] inline Result<ParsedTime> parse_time_spec(const char* spec) noexcept {
    if (spec == nullptr || *spec == '\0') {
        return result_err<ParsedTime>(Status::Invalid);
    }
    ParsedTime pt{};
    const char* cp = spec;
    if (*cp == '+') {
        pt.relative = true;
        ++cp;
    }
    int hours = 0;
    for (; *cp != '\0' && *cp != '\n'; ++cp) {
        if (!is_digit(*cp)) {
            return result_err<ParsedTime>(Status::Invalid);
        }
        hours = hours * 10 + (*cp - '0');
    }
    pt.minutes = hours % 100;
    pt.hours = hours / 100;
    if (pt.minutes < 0 || pt.minutes > 59) {
        return result_err<ParsedTime>(Status::Invalid);
    }
    if (!pt.relative && pt.hours > 23) {
        return result_err<ParsedTime>(Status::Invalid);
    }
    return result_ok(pt);
}

[[nodiscard]] inline unsigned seconds_relative(int hours, int minutes) noexcept {
    return static_cast<unsigned>(hours) * 3600U + static_cast<unsigned>(minutes) * 60U;
}

[[nodiscard]] inline unsigned seconds_until(int target_hour, int target_min,
                                            int now_hour, int now_min) noexcept {
    int h12 = target_hour;
    if (h12 > 11) {
        h12 -= 12;
    }
    int now12 = now_hour;
    if (now12 > 11) {
        now12 -= 12;
    }
    if (h12 < now12 || (h12 == now12 && target_min <= now_min)) {
        h12 += 12;
    }
    unsigned secs = static_cast<unsigned>(h12 - now12) * 3600U;
    if (target_min >= now_min) {
        secs += static_cast<unsigned>(target_min - now_min) * 60U;
    } else {
        secs -= static_cast<unsigned>(now_min - target_min) * 60U;
    }
    return secs;
}

[[nodiscard]] inline unsigned alarm_schedule(unsigned total_secs) noexcept {
    if (total_secs >= kFiveMin) {
        return total_secs - kFiveMin;
    }
    if (total_secs >= kOneMin) {
        return total_secs - kOneMin;
    }
    return 0;
}

} // namespace pbsd::userland::usr_bin::leave
