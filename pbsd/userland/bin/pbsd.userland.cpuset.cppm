module;

export module pbsd.userland.cpuset;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/bin/cpuset/cpuset.c
export namespace pbsd::userland::bin::cpuset {


inline constexpr const char* kOptString = "lrgj";

struct Options {
    bool list{false};
    bool read{false};
    bool get{false};
    bool jail{false};
};


} // namespace pbsd::userland::bin::cpuset
