module;

export module pbsd.userland.timeout;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/bin/timeout/timeout.c
export namespace pbsd::userland::bin::timeout {


inline constexpr int kExitTimeout = 124;
inline constexpr int kExitInvalid = 125;
inline constexpr int kExitCmdError = 126;
inline constexpr int kExitCmdNoent = 127;
inline constexpr const char* kOptString = "k:s:pv";

struct Options {
    unsigned kill_after{0};
    unsigned signal{9};
    bool preserve_status{false};
    bool verbose{false};
};


} // namespace pbsd::userland::bin::timeout
