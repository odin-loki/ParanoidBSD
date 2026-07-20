module;

export module pbsd.userland.addr2line;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/addr2line/addr2line.c
export namespace pbsd::userland::usr_bin::addr2line {

[[nodiscard]] inline bool addr2line_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::addr2line
