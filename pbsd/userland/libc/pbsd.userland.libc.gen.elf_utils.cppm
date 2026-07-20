module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.elf_utils;

export import pbsd.core;

/// elf_utils from hbsd/src/lib/libc/gen/elf_utils.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status elf_utils_fd(int fd) noexcept { return fd < 0 ? Status::Invalid : Status::Ok; }

} // namespace pbsd::userland::libc
