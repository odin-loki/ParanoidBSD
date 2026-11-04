module;

export module pbsd.userland.ed;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/bin/ed/main.c
export namespace pbsd::userland::bin::ed {


inline constexpr int kDefaultLineLength = 4096;

struct Options {
    bool quiet{false};
    bool suppress_prompt{false};
};


} // namespace pbsd::userland::bin::ed
