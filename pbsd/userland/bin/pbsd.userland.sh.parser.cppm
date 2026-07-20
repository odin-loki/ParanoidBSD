module;

export module pbsd.userland.sh.parser;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/sh/parser.c
export namespace pbsd::userland::bin::sh::parser {

[[nodiscard]] inline bool parser_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::sh::parser
