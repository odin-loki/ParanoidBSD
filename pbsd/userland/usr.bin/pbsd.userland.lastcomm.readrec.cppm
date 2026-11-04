module;

export module pbsd.userland.lastcomm.readrec;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/lastcomm/readrec.c
export namespace pbsd::userland::usr_bin::lastcomm::readrec {

[[nodiscard]] inline bool readrec_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::lastcomm::readrec
