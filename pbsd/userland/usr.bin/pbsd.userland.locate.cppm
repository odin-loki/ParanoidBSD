module;

export module pbsd.userland.locate;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/locate/locate/locate.c
export namespace pbsd::userland::usr_bin::locate {

[[nodiscard]] inline Status locate_pattern(const char* pat) noexcept { if (pat == nullptr || pat[0] == '\0') return Status::Invalid; return Status::Ok; }

} // namespace pbsd::userland::usr_bin::locate
