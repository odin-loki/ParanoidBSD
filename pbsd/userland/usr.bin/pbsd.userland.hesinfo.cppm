module;

export module pbsd.userland.hesinfo;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/hesinfo/hesinfo.c
export namespace pbsd::userland::usr_bin::hesinfo {

[[nodiscard]] inline Status hesinfo_name(const char* name) noexcept { if (name == nullptr || name[0] == '\0') return Status::Invalid; return Status::Ok; }

} // namespace pbsd::userland::usr_bin::hesinfo
