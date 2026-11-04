module;

export module pbsd.userland.locate.bigram.locate.bigram;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/locate/bigram/locate.bigram.c
export namespace pbsd::userland::usr_bin::locate::bigram::locate::bigram {

[[nodiscard]] inline bool bigram_locate_bigram_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::locate::bigram::locate::bigram
