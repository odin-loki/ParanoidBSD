module;

export module pbsd.userland.indent.parse;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/indent/parse.c
export namespace pbsd::userland::usr_bin::indent::parse {

[[nodiscard]] inline bool parse_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::indent::parse
