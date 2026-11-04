module;

export module pbsd.userland.look;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/look/look.c
export namespace pbsd::userland::usr_bin::look {


inline constexpr const char* kOptString = "df";
inline constexpr const char* kDefaultDict = "/usr/share/dict/words";


} // namespace pbsd::userland::usr_bin::look
