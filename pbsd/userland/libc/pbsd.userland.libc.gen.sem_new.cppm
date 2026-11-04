module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.sem_new;

export import pbsd.core;

/// sem_new from hbsd/src/lib/libc/gen/sem_new.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status sem_new_count(unsigned count) noexcept { return count > 0 ? Status::Ok : Status::Invalid; }

} // namespace pbsd::userland::libc
