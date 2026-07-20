module;

export module pbsd.userland.tftp;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tftp/tftp.c
export namespace pbsd::userland::usr_bin::tftp {

[[nodiscard]] inline bool tftp_verbose(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::tftp
