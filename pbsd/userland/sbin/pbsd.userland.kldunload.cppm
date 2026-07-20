module;

export module pbsd.userland.kldunload;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/sbin/kldunload/kldunload.c
export namespace pbsd::userland::sbin::kldunload {


inline constexpr const char* kOptString = "fv";

struct Options {
    bool force{false};
    bool verbose{false};
};


} // namespace pbsd::userland::sbin::kldunload
