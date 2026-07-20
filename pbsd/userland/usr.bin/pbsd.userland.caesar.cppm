module;
#include <cstddef>

export module pbsd.userland.caesar;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/caesar/caesar.c — rotation cipher helpers (logic-only).
export namespace pbsd::userland::usr_bin::caesar {

inline constexpr int kLineLength = 2048;
inline constexpr int kAlphabetSize = 26;

[[nodiscard]] inline char rotate_char(unsigned char ch, int perm) noexcept {
    if (ch >= 'A' && ch <= 'Z') {
        return static_cast<char>('A' + (ch - 'A' + perm) % kAlphabetSize);
    }
    if (ch >= 'a' && ch <= 'z') {
        return static_cast<char>('a' + (ch - 'a' + perm) % kAlphabetSize);
    }
    return static_cast<char>(ch);
}

[[nodiscard]] inline int letter_index(unsigned char ch) noexcept {
    if (ch >= 'a' && ch <= 'z') {
        return ch - 'a';
    }
    if (ch >= 'A' && ch <= 'Z') {
        return ch - 'A';
    }
    return -1;
}

[[nodiscard]] inline void count_letters(const unsigned char* buf, std::size_t len,
                                        int obs[kAlphabetSize]) noexcept {
    if (buf == nullptr || obs == nullptr) {
        return;
    }
    for (int i = 0; i < kAlphabetSize; ++i) {
        obs[i] = 0;
    }
    for (std::size_t i = 0; i < len; ++i) {
        const int idx = letter_index(buf[i]);
        if (idx >= 0) {
            ++obs[idx];
        }
    }
}

[[nodiscard]] inline Result<int> parse_rotation_arg(const char* arg) noexcept {
    if (arg == nullptr || *arg == '\0') {
        return result_err<int>(Status::Invalid);
    }
    int val = 0;
    for (const char* p = arg; *p != '\0'; ++p) {
        if (*p < '0' || *p > '9') {
            return result_err<int>(Status::Invalid);
        }
        val = val * 10 + (*p - '0');
    }
    return result_ok(val % kAlphabetSize);
}

[[nodiscard]] inline int best_rotation(const int obs[kAlphabetSize],
                                       const double stdf[kAlphabetSize]) noexcept {
    int winner = 0;
    double winner_dot = 0.0;
    for (int try_rot = 0; try_rot < kAlphabetSize; ++try_rot) {
        double dot = 0.0;
        for (int i = 0; i < kAlphabetSize; ++i) {
            dot += static_cast<double>(obs[i]) * stdf[(i + try_rot) % kAlphabetSize];
        }
        if (try_rot == 0 || dot > winner_dot) {
            winner = try_rot;
            winner_dot = dot;
        }
    }
    return winner;
}

} // namespace pbsd::userland::usr_bin::caesar
