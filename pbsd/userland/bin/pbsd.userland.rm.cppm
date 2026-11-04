module;

export module pbsd.userland.rm;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/bin/rm/rm.c
export namespace pbsd::userland::bin::rm {


inline constexpr const char* kOptString = "dfiPRrvWx";

struct Options {
    bool recursive{false};
    bool force{false};
    bool interactive{false};
    bool verbose{false};
    bool preserve_root{false};
};

[[nodiscard]] inline bool accepts_flag(char c) noexcept {
    for (const char* p = kOptString; *p; ++p) {
        if (*p == c) {
            return true;
        }
    }
    return false;
}


} // namespace pbsd::userland::bin::rm
