module;
#include <cstddef>

export module pbsd.userland.libc.gen.fmtcheck;

export import pbsd.core;

/// fmtcheck from hbsd/src/lib/libc/gen/fmtcheck.c (format specifier scan).
export namespace pbsd::userland::libc {

enum class FmtKind : unsigned char { Start, Width, Precision, Done, Unknown };

[[nodiscard]] inline FmtKind scan_conversion(const char* fmt, std::size_t& idx) noexcept {
    if (fmt == nullptr) {
        return FmtKind::Unknown;
    }
    if (fmt[idx] == '\0') {
        return FmtKind::Done;
    }
    if (fmt[idx] != '%') {
        ++idx;
        return FmtKind::Start;
    }
    ++idx;
    if (fmt[idx] == '\0') {
        return FmtKind::Unknown;
    }
    if (fmt[idx] == '*') {
        ++idx;
    }
    while (fmt[idx] >= '0' && fmt[idx] <= '9') {
        ++idx;
    }
    if (fmt[idx] == '.') {
        ++idx;
        while (fmt[idx] >= '0' && fmt[idx] <= '9') {
            ++idx;
        }
        return FmtKind::Precision;
    }
    if (fmt[idx] == 'l' || fmt[idx] == 'h' || fmt[idx] == 'z') {
        ++idx;
    }
    if (fmt[idx] != '\0') {
        ++idx;
    }
    return FmtKind::Done;
}

[[nodiscard]] inline Result<int> formats_compatible(const char* fmt1,
                                                    const char* fmt2) noexcept {
    if (fmt1 == nullptr || fmt2 == nullptr) {
        return result_err<int>(Status::Invalid);
    }
    std::size_t i1 = 0;
    std::size_t i2 = 0;
    while (fmt1[i1] != '\0' || fmt2[i2] != '\0') {
        if (fmt1[i1] == '\0' || fmt2[i2] == '\0') {
            return result_ok(1);
        }
        if (fmt1[i1] != fmt2[i2] && fmt1[i1] == '%' && fmt2[i2] == '%') {
            scan_conversion(fmt1, i1);
            scan_conversion(fmt2, i2);
            continue;
        }
        if (fmt1[i1] != fmt2[i2]) {
            return result_ok(1);
        }
        ++i1;
        ++i2;
    }
    return result_ok(0);
}

} // namespace pbsd::userland::libc
