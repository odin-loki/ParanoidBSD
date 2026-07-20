module;

export module pbsd.userland.ldconfig;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/sbin/ldconfig/ldconfig.c
export namespace pbsd::userland::sbin::ldconfig {


inline constexpr const char* kOptString = "32i:m:rsUu";

struct Options {
    bool merge{false};
    bool secure{false};
    bool unique{false};
};


} // namespace pbsd::userland::sbin::ldconfig
