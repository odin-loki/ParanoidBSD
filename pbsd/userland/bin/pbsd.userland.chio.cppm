module;

export module pbsd.userland.chio;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/bin/chio/chio.c
export namespace pbsd::userland::bin::chio {


inline constexpr const char* kOptString = "f:";

struct Options {
    const char* changer{nullptr};
};


} // namespace pbsd::userland::bin::chio
