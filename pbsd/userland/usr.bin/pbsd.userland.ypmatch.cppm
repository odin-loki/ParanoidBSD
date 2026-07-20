module;

export module pbsd.userland.ypmatch;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ypmatch/ypmatch.c
export namespace pbsd::userland::usr_bin::ypmatch {

[[nodiscard]] inline Status ypmatch_key(const char* key) noexcept { if (key == nullptr || key[0] == '\0') return Status::Invalid; return Status::Ok; }

} // namespace pbsd::userland::usr_bin::ypmatch
