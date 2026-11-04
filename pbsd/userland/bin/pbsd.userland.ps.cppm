module;

export module pbsd.userland.ps;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/bin/ps/ps.c
export namespace pbsd::userland::bin::ps {


inline constexpr const char* kOptString = "AaCcD:defG:gHhjJ:LlM:mN:O:o:p:rSTt:U:uvwXxZ";
inline constexpr const char* kPtsPrefix = "/dev/pts/";

struct Options {
    bool all{false};
    bool full{false};
    bool jobs{false};
    bool tty{false};
    bool wide{false};
};


} // namespace pbsd::userland::bin::ps
