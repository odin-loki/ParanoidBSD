module;

export module pbsd.userland.mknod;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/sbin/mknod/mknod.c
export namespace pbsd::userland::sbin::mknod {


inline constexpr const char* kOptString = "";

struct Options {
    const char* mode{nullptr};
    const char* dev{nullptr};
};


} // namespace pbsd::userland::sbin::mknod
