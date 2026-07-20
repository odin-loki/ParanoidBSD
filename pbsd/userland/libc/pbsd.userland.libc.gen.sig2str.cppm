module;
#include <cstddef>

export module pbsd.userland.libc.gen.sig2str;

export import pbsd.core;

/// sig2str/str2sig from hbsd/src/lib/libc/gen/sig2str.c (subset).
export namespace pbsd::userland::libc {

inline constexpr int kSig2StrMax = 32;
inline constexpr int kSigRtMin = 128;
inline constexpr int kSigRtMax = 255;

[[nodiscard]] inline Result<int> sig2str(int signum, char* str, std::size_t len) noexcept {
    if (str == nullptr || len == 0) {
        return result_err<int>(Status::Invalid);
    }
    if (signum <= 0 || signum > kSigRtMax) {
        return result_err<int>(Status::Invalid);
    }
    static constexpr const char* kNames[] = {
        "", "HUP", "INT", "QUIT", "ILL", "TRAP", "ABRT", "EMT", "FPE", "KILL", "BUS",
    };
    if (signum < static_cast<int>(sizeof(kNames) / sizeof(kNames[0]))) {
        std::size_t i = 0;
        for (const char* p = kNames[signum]; *p != '\0' && i + 1 < len; ++p) {
            str[i++] = *p;
        }
        str[i] = '\0';
        return result_ok(0);
    }
    if (signum == kSigRtMin) {
        str[0] = 'R';
        str[1] = 'T';
        str[2] = 'M';
        str[3] = 'I';
        str[4] = 'N';
        str[5] = '\0';
        return result_ok(0);
    }
    return result_err<int>(Status::NotFound);
}

[[nodiscard]] inline Result<int> str2sig(const char* str, int& signum_out) noexcept {
    if (str == nullptr) {
        return result_err<int>(Status::Invalid);
    }
    const char* p = str;
    if (p[0] == 'S' && p[1] == 'I' && p[2] == 'G') {
        p += 3;
    }
    static constexpr const char* kNames[] = {
        "HUP", "INT", "QUIT", "ILL", "TRAP", "ABRT", "EMT", "FPE", "KILL", "BUS",
    };
    for (int i = 0; i < static_cast<int>(sizeof(kNames) / sizeof(kNames[0])); ++i) {
        const char* name = kNames[i];
        std::size_t j = 0;
        while (name[j] != '\0' && p[j] == name[j]) {
            ++j;
        }
        if (name[j] == '\0' && p[j] == '\0') {
            signum_out = i + 1;
            return result_ok(0);
        }
    }
    return result_err<int>(Status::NotFound);
}

} // namespace pbsd::userland::libc
