module;
#include <cstddef>

export module pbsd.userland.lam;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/lam/lam.c — laminate columns (logic-only).
export namespace pbsd::userland::usr_bin::lam {

inline constexpr int kMaxOpenFiles = 20;
inline constexpr int kBigBufSize = 5 * 4096;

[[nodiscard]] inline Result<int> parse_args(int argc, char* const* argv,
                                            int& optind_out) noexcept {
    if (argv == nullptr) {
        return result_err<int>(Status::Invalid);
    }
    if (argc < 2) {
        return result_err<int>(Status::Invalid);
    }
    optind_out = 1;
    return result_ok(0);
}

[[nodiscard]] inline bool is_separator_spec(const char* arg) noexcept {
    return arg != nullptr && arg[0] == 't' && arg[1] == '\0';
}

[[nodiscard]] inline bool is_format_spec(const char* arg) noexcept {
    return arg != nullptr && arg[0] == 'F' && arg[1] != '\0';
}

[[nodiscard]] inline std::size_t trim_newline(char* line, std::size_t len) noexcept {
    if (line == nullptr || len == 0) {
        return 0;
    }
    if (line[len - 1] == '\n') {
        line[len - 1] = '\0';
        return len - 1;
    }
    return len;
}

[[nodiscard]] inline int compare_filespec(const char* a, const char* b) noexcept {
    return hosted::cstrcmp(a, b);
}

} // namespace pbsd::userland::usr_bin::lam
