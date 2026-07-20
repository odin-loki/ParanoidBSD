module;

export module pbsd.userland.xinstall;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/xinstall/xinstall.c
export namespace pbsd::userland::usr_bin::xinstall {

[[nodiscard]] inline Status xinstall_src(const char* src) noexcept { if (src == nullptr || src[0] == '\0') return Status::Invalid; return Status::Ok; }

} // namespace pbsd::userland::usr_bin::xinstall
