module;

export module pbsd.userland.ypwhich;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ypwhich/ypwhich.c
export namespace pbsd::userland::usr_bin::ypwhich {

[[nodiscard]] inline Status ypwhich_map(const char* map) noexcept { if (map == nullptr || map[0] == '\0') return Status::Invalid; return Status::Ok; }

} // namespace pbsd::userland::usr_bin::ypwhich
