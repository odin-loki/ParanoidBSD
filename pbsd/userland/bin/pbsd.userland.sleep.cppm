module;
#include <cstddef>

export module pbsd.userland.sleep;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/sleep/sleep.c — interval parse (Capsicum-ready caller).
export namespace pbsd::userland::bin::sleep {

/// Parse one interval token: number[s|m|h|d]. Returns seconds or Invalid.
[[nodiscard]] inline Result<double> parse_interval(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return result_err<double>(Status::Invalid);
    }
    double num = 0.0;
    std::size_t i = 0;
    bool seen_digit = false;
    bool seen_dot = false;
    // Integer/fractional mantissa
    for (; arg[i] != '\0'; ++i) {
        char c = arg[i];
        if (c >= '0' && c <= '9') {
            seen_digit = true;
            num = num * 10.0 + static_cast<double>(c - '0');
            continue;
        }
        if (c == '.' && !seen_dot) {
            seen_dot = true;
            ++i;
            double place = 0.1;
            for (; arg[i] >= '0' && arg[i] <= '9'; ++i) {
                seen_digit = true;
                num += static_cast<double>(arg[i] - '0') * place;
                place *= 0.1;
            }
            break;
        }
        break;
    }
    if (!seen_digit) {
        return result_err<double>(Status::Invalid);
    }
    char unit = arg[i];
    if (unit == '\0') {
        return result_ok(num);
    }
    if (arg[i + 1] != '\0') {
        return result_err<double>(Status::Invalid);
    }
    switch (unit) {
    case 'd':
        num *= 24.0;
        [[fallthrough]];
    case 'h':
        num *= 60.0;
        [[fallthrough]];
    case 'm':
        num *= 60.0;
        [[fallthrough]];
    case 's':
        return result_ok(num);
    default:
        return result_err<double>(Status::Invalid);
    }
}

/// Sum all argv intervals (after argv[0]).
[[nodiscard]] inline Result<double> total_seconds(int argc, char* const* argv) noexcept {
    if (argv == nullptr || argc < 2) {
        return result_err<double>(Status::Invalid);
    }
    double total = 0.0;
    for (int i = 1; i < argc && argv[i] != nullptr; ++i) {
        auto part = parse_interval(argv[i]);
        if (!part.has_value()) {
            return part;
        }
        total += part.value;
    }
    return result_ok(total);
}

} // namespace pbsd::userland::bin::sleep
