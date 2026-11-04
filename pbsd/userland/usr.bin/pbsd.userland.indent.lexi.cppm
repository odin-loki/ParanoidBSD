module;

export module pbsd.userland.indent.lexi;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/indent/lexi.c
export namespace pbsd::userland::usr_bin::indent::lexi {

[[nodiscard]] inline bool lexi_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::indent::lexi
