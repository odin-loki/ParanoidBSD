module;

export module pbsd.userland.vacation;

export import pbsd.core;

/// Port of hbsd/src/usr.bin/vacation/vacation.c — autoreply message limits.
export namespace pbsd::userland::usr_bin::vacation {

inline constexpr int kMaxSubject = 256;
inline constexpr int kMaxFrom = 256;

[[nodiscard]] inline bool login_char_ok(char c) noexcept {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.';
}

[[nodiscard]] inline Result<int> validate_login(const char* login) noexcept {
    if (login == nullptr || *login == '\0') {
        return result_err<int>(Status::Invalid);
    }
    for (const char* p = login; *p != '\0'; ++p) {
        if (!login_char_ok(*p)) {
            return result_err<int>(Status::Invalid);
        }
    }
    return result_ok(0);
}

[[nodiscard]] inline bool subject_present(const char* subject) noexcept {
    return subject != nullptr && *subject != '\0';
}

} // namespace pbsd::userland::usr_bin::vacation
