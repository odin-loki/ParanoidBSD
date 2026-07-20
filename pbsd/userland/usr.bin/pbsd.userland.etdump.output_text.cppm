module;

export module pbsd.userland.etdump.output_text;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/etdump/output_text.c
export namespace pbsd::userland::usr_bin::etdump::output_text {

[[nodiscard]] inline bool output_text_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::etdump::output_text
