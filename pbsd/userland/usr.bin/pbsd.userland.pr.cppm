module;

export module pbsd.userland.pr;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/pr/{pr.c,pr.h} — pagination defaults.
export namespace pbsd::userland::usr_bin::pr {

inline constexpr int kColCount = 1;
inline constexpr char kInChar = '\t';
inline constexpr int kInGap = 8;
inline constexpr char kOutChar = '\t';
inline constexpr int kOutGap = 8;
inline constexpr int kLines = 66;
inline constexpr int kNumWidth = 5;
inline constexpr char kNumChar = '\t';
inline constexpr char kSepChar = '\t';
inline constexpr int kPageWidth = 72;
inline constexpr int kSinglePageWidth = 512;
inline constexpr int kHeadLen = 5;
inline constexpr int kTailLen = 5;
inline constexpr int kLineBuf = 8192;
inline constexpr const char* kOptString = "#adFfmrte?h:i?L:l:n?o:ps?w:";

struct Options {
    int page_start{1};
    int columns{kColCount};
    int lines{kLines};
    int page_width{kPageWidth};
    bool double_space{false};
    bool form_feed{false};
    bool merge{false};
    bool no_header{false};
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

} // namespace pbsd::userland::usr_bin::pr
