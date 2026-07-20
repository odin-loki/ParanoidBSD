module;

export module pbsd.userland.ul;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/ul/ul.c — underline/overstrike filter.
export namespace pbsd::userland::usr_bin::ul {

inline constexpr char kIesc = '\033';
inline constexpr char kSo = '\016';
inline constexpr char kSi = '\017';
inline constexpr char kHfwd = '9';
inline constexpr char kHrev = '8';
inline constexpr char kFrev = '7';
inline constexpr int kMaxBuf = 512;
inline constexpr const char* kOptString = "it:T:";
inline constexpr const char* kDefaultTerm = "lpr";

enum CharMode : unsigned {
    kNormal = 000,
    kAltSet = 001,
    kSupersc = 002,
    kSubsc = 004,
    kUnderl = 010,
    kBold = 020,
};

struct Options {
    bool italic{false};
    const char* termtype{kDefaultTerm};
};

[[nodiscard]] inline bool accepts_flag(char c) noexcept {
    const char* allowed = kOptString;
    for (const char* p = allowed; *p; ++p) {
        if (*p == c) {
            return true;
        }
    }
    return false;
}

} // namespace pbsd::userland::usr_bin::ul
