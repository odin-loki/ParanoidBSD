module;
#include <cstddef>

export module pbsd.userland.printf;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/printf/printf.c — format/escape helpers (logic-only).
export namespace pbsd::userland::usr_bin::printf {

[[nodiscard]] inline bool is_conversion(char c) noexcept {
    switch (c) {
    case 'd':
    case 'i':
    case 'o':
    case 'u':
    case 'x':
    case 'X':
    case 'f':
    case 'F':
    case 'e':
    case 'E':
    case 'g':
    case 'G':
    case 'a':
    case 'A':
    case 'c':
    case 's':
    case 'p':
    case 'n':
    case '%':
        return true;
    default:
        return false;
    }
}

[[nodiscard]] inline Result<std::size_t> escape_octal(const char* src,
                                                        char* dst,
                                                        std::size_t cap) noexcept {
    if (src == nullptr || dst == nullptr || cap == 0) {
        return result_err<std::size_t>(Status::Invalid);
    }
    unsigned val = 0;
    int digits = 0;
    for (int i = 0; i < 3 && src[i] >= '0' && src[i] <= '7'; ++i) {
        val = (val << 3) + static_cast<unsigned>(src[i] - '0');
        ++digits;
    }
    if (digits == 0) {
        return result_err<std::size_t>(Status::Invalid);
    }
    dst[0] = static_cast<char>(val & 0xff);
    return result_ok(static_cast<std::size_t>(digits));
}

[[nodiscard]] inline Result<std::size_t> parse_directive(const char* fmt,
                                                         std::size_t& width,
                                                         std::size_t& prec,
                                                         bool& have_width,
                                                         bool& have_prec) noexcept {
    if (fmt == nullptr || fmt[0] != '%') {
        return result_err<std::size_t>(Status::Invalid);
    }
    width = prec = 0;
    have_width = have_prec = false;
    std::size_t i = 1;
    if (fmt[i] == '#') {
        ++i;
    }
    while (fmt[i] == '-' || fmt[i] == '+' || fmt[i] == ' ' || fmt[i] == '0') {
        ++i;
    }
    if (fmt[i] >= '1' && fmt[i] <= '9') {
        have_width = true;
        while (fmt[i] >= '0' && fmt[i] <= '9') {
            width = width * 10 + static_cast<std::size_t>(fmt[i] - '0');
            ++i;
        }
    }
    if (fmt[i] == '.') {
        ++i;
        have_prec = true;
        while (fmt[i] >= '0' && fmt[i] <= '9') {
            prec = prec * 10 + static_cast<std::size_t>(fmt[i] - '0');
            ++i;
        }
    }
    if (!is_conversion(fmt[i])) {
        return result_err<std::size_t>(Status::Invalid);
    }
    return result_ok(i + 1);
}

} // namespace pbsd::userland::usr_bin::printf
