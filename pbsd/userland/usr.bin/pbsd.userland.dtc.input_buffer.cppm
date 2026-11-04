module;

export module pbsd.userland.dtc.input_buffer;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/dtc/input_buffer.c
export namespace pbsd::userland::usr_bin::dtc::input_buffer {

[[nodiscard]] inline bool input_buffer_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::dtc::input_buffer
