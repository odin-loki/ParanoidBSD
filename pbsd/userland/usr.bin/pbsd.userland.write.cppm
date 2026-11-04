module;
#include <cstddef>

export module pbsd.userland.write;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/write/write.c — terminal message helpers (logic-only).
export namespace pbsd::userland::usr_bin::write {

inline constexpr int kMinArgs = 2;

[[nodiscard]] inline bool valid_login_char(char c) noexcept {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-';
}

[[nodiscard]] inline bool valid_login(const char* login) noexcept {
    if (login == nullptr || login[0] == '\0') {
        return false;
    }
    for (const char* p = login; *p != '\0'; ++p) {
        if (!valid_login_char(*p)) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline bool same_user(const char* a, const char* b) noexcept {
    if (a == nullptr || b == nullptr) {
        return false;
    }
    return hosted::cstrcmp(a, b) == 0;
}

[[nodiscard]] inline Result<int> parse_args(int argc, char* const* argv,
                                            int& optind_out) noexcept {
    if (argv == nullptr) {
        return result_err<int>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        return result_err<int>(Status::Invalid);
    }
    optind_out = i;
    if (argc - i < kMinArgs) {
        return result_err<int>(Status::Invalid);
    }
    if (!valid_login(argv[i])) {
        return result_err<int>(Status::Invalid);
    }
    return result_ok(i);
}

} // namespace pbsd::userland::usr_bin::write
