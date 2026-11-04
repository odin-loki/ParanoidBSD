module;

export module pbsd.userland.kldstat;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/sbin/kldstat/kldstat.c
export namespace pbsd::userland::sbin::kldstat {


inline constexpr const char* kOptString = "qiv";

struct Options {
    bool quiet{false};
    bool id{false};
    bool verbose{false};
};


} // namespace pbsd::userland::sbin::kldstat
