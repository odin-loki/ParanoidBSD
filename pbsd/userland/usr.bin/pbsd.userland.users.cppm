module;
#include <cstddef>

export module pbsd.userland.users;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/users/users.cc — logged-in user listing (logic-only).
export namespace pbsd::userland::usr_bin::users {

inline constexpr int kUserProcess = 7;

[[nodiscard]] inline Result<int> parse_args(int argc, char* const* argv) noexcept {
    if (argv == nullptr) {
        return result_err<int>(Status::Invalid);
    }
    if (argc > 1) {
        return result_err<int>(Status::Invalid);
    }
    return result_ok(0);
}

[[nodiscard]] inline bool is_user_session(int ut_type) noexcept {
    return ut_type == kUserProcess;
}

[[nodiscard]] inline bool valid_username(const char* name) noexcept {
    if (name == nullptr || name[0] == '\0') {
        return false;
    }
    for (const char* p = name; *p != '\0'; ++p) {
        const char c = *p;
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '_' || c == '-')) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline int compare_names(const char* a, const char* b) noexcept {
    return hosted::cstrcmp(a, b);
}

} // namespace pbsd::userland::usr_bin::users
