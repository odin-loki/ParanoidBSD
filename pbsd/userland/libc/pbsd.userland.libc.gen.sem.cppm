module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.sem;

export import pbsd.core;

/// sem from hbsd/src/lib/libc/gen/sem.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status sem_key(int key) noexcept { (void)key; return Status::Ok; }

} // namespace pbsd::userland::libc
