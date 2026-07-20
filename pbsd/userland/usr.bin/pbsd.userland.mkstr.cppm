module;

export module pbsd.userland.mkstr;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkstr/mkstr.c
export namespace pbsd::userland::usr_bin::mkstr {

[[nodiscard]] inline bool mkstr_uppercase(char flag) noexcept { return flag == 'u'; }

} // namespace pbsd::userland::usr_bin::mkstr
