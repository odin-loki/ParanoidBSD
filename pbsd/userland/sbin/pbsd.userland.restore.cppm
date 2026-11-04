module;

export module pbsd.userland.restore;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/sbin/restore/main.c
export namespace pbsd::userland::sbin::restore {


inline constexpr const char* kOptString = "bf:hi:m:rs:t:vx";

struct Options {
    bool interactive{false};
    bool verbose{false};
    bool extract{false};
};


} // namespace pbsd::userland::sbin::restore
