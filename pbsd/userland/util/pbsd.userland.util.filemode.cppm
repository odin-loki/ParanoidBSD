module;
#include <cstddef>

export module pbsd.userland.util.filemode;

export import pbsd.core;
/// Octal mode parsing helpers shared by chmod/mkdir (setmode/getmode subset).
export namespace pbsd::userland::util::filemode {

inline constexpr unsigned kModeMask = 07777u;

[[nodiscard]] inline Result<unsigned> parse_octal_digit(char ch) noexcept {
    if (ch < '0' || ch > '7') {
        return result_err<unsigned>(Status::Invalid);
    }
    return result_ok(static_cast<unsigned>(ch - '0'));
}

/// Parse a chmod-style octal mode (optional leading 0).
[[nodiscard]] inline Result<unsigned> parse_octal_mode(const char* s) noexcept {
    if (s == nullptr || s[0] == '\0') {
        return result_err<unsigned>(Status::Invalid);
    }
    if (s[0] == '0' && (s[1] == '0' || s[1] == '1' || s[1] == '2' ||
                        s[1] == '3' || s[1] == '4' || s[1] == '5' ||
                        s[1] == '6' || s[1] == '7')) {
        ++s;
    }
    unsigned mode = 0;
    for (const char* p = s; *p != '\0'; ++p) {
        const auto d = parse_octal_digit(*p);
        if (!d.has_value()) {
            return result_err<unsigned>(d.status);
        }
        mode = (mode << 3) | d.value;
        if (mode > kModeMask) {
            return result_err<unsigned>(Status::Invalid);
        }
    }
    return result_ok(mode);
}

/// Apply a mode template to a base mode (getmode subset for octal templates).
[[nodiscard]] inline unsigned apply_mode_template(unsigned base,
                                                  unsigned template_mode) noexcept {
    return (base & ~07777u) | (template_mode & kModeMask);
}

inline constexpr unsigned kDefaultDirMode = 0777u;

} // namespace pbsd::userland::util::filemode
