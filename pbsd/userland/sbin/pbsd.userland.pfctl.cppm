module;

export module pbsd.userland.pfctl;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/sbin/pfctl/pfctl.c
export namespace pbsd::userland::sbin::pfctl {


inline constexpr const char* kOptString = "a:deF:f:hi:k:mn:N:o:P:p:qR:s:tvx:z";

struct Options {
    bool force{false};
    bool verbose{false};
    bool test{false};
};


} // namespace pbsd::userland::sbin::pfctl
