module;

export module pbsd.userland.file2c;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/file2c/file2c.c
export namespace pbsd::userland::usr_bin::file2c {

[[nodiscard]] inline Status file2c_name(const char* name) noexcept { if (name == nullptr || name[0] == '\0') return Status::Invalid; return Status::Ok; }

} // namespace pbsd::userland::usr_bin::file2c
