module;

export module pbsd.userland.rtld.paths;

/// Default rtld search paths from hbsd/src/libexec/rtld-elf/rtld_paths.h
export namespace pbsd::userland::rtld {

inline constexpr const char* kDefaultLibPath = "/lib:/usr/lib";
inline constexpr const char* kDefaultRtldPath = "/libexec/ld-elf.so.1";
inline constexpr const char* kVarLdLibraryPath = "LD_LIBRARY_PATH";
inline constexpr const char* kVarLdPreload = "LD_PRELOAD";

} // namespace pbsd::userland::rtld
