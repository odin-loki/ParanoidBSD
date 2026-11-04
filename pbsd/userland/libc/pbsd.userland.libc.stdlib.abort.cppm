module;

export module pbsd.userland.libc.stdlib.abort;

export import pbsd.core;

/// abort from hbsd/src/lib/libc/stdlib/abort.c
export namespace pbsd::userland::libc {

[[noreturn]] inline void abort_trap() noexcept { for (;;) {} }

} // namespace pbsd::userland::libc
