module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.regex.engine;

export import pbsd.core;

/// engine from hbsd/src/lib/libc/regex/engine.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status engine_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
