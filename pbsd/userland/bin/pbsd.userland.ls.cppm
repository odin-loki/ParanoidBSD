module;

export module pbsd.userland.ls;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/bin/ls/ls.c
export namespace pbsd::userland::bin::ls {


inline constexpr const char* kOptString = "ABCFGHLOPRSTUWabcdefghiklmnopqrstuvwx1@";

struct Options {
    bool long_format{false};
    bool all_entries{false};
    bool classify{false};
    bool human{false};
    bool inode{false};
    bool recursive{false};
    bool sort_time{false};
};

[[nodiscard]] inline bool accepts_flag(char c) noexcept {
    for (const char* p = kOptString; *p; ++p) {
        if (*p == c) {
            return true;
        }
    }
    return false;
}


} // namespace pbsd::userland::bin::ls
