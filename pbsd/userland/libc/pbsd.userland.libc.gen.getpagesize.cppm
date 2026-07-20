module;

export module pbsd.userland.libc.gen.getpagesize;

/// getpagesize from hbsd/src/lib/libc/gen/getpagesize.c
export namespace pbsd::userland::libc {

inline constexpr int kDefaultPageSize = 4096;

[[nodiscard]] inline int getpagesize() noexcept { return kDefaultPageSize; }

} // namespace pbsd::userland::libc
