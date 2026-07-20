module;

export module pbsd.userland.pkill;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/bin/pkill/pkill.c
export namespace pbsd::userland::bin::pkill {


inline constexpr const char* kOptString = "f:ix:signal:";

struct Options {
    bool full{false};
    bool ignore_case{false};
    bool exact{false};
    int signal{15};
};


} // namespace pbsd::userland::bin::pkill
