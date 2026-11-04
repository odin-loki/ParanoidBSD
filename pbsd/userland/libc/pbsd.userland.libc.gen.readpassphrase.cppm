module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.readpassphrase;

export import pbsd.core;

/// readpassphrase from hbsd/src/lib/libc/gen/readpassphrase.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status readpassphrase_prompt(const char* prompt) noexcept { if (!prompt) return Status::Invalid; return Status::Ok; }

} // namespace pbsd::userland::libc
