module;

export module pbsd.userland.pwait;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/bin/pwait/pwait.c
export namespace pbsd::userland::bin::pwait {


inline constexpr const char* kOptString = "nv";

struct Options {
    bool nohang{false};
    bool verbose{false};
};


} // namespace pbsd::userland::bin::pwait
