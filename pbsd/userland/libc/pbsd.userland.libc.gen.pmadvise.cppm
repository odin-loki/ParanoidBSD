module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.pmadvise;

export import pbsd.core;

/// pmadvise from hbsd/src/lib/libc/gen/pmadvise.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status pmadvise_addr(void* addr, std::size_t len) noexcept { (void)addr; (void)len; return Status::Ok; }

} // namespace pbsd::userland::libc
