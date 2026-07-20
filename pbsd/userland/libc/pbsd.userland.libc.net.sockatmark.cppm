module;

export module pbsd.userland.libc.net.sockatmark;

/// sockatmark from hbsd/src/lib/libc/net/sockatmark.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline bool sockatmark_oob_index(int oobmark) noexcept { return oobmark != 0; }

} // namespace pbsd::userland::libc
