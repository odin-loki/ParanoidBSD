module;

export module pbsd.userland.fmt;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/fmt/fmt.c
export namespace pbsd::userland::usr_bin::fmt {


inline constexpr const char* kOptString = "cmps:t:w:";
inline constexpr int kDefaultWidth = 75;
inline constexpr int kDefaultTab = 8;

struct Options {
    bool crown{false};
    bool mail{false};
    bool split{false};
    int width{kDefaultWidth};
    int tab{kDefaultTab};
};


} // namespace pbsd::userland::usr_bin::fmt
