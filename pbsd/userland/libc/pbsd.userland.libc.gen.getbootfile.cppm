module;

export module pbsd.userland.libc.gen.getbootfile;

/// getbootfile from hbsd/src/lib/libc/gen/getbootfile.c
export namespace pbsd::userland::libc {

inline constexpr const char* kDefaultBootfile = "/boot/kernel/kernel";

[[nodiscard]] inline const char* default_bootfile() noexcept { return kDefaultBootfile; }

} // namespace pbsd::userland::libc
