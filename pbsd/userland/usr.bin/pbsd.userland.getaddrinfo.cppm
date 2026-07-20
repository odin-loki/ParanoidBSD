module;

export module pbsd.userland.getaddrinfo;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/getaddrinfo/getaddrinfo.c
export namespace pbsd::userland::usr_bin::getaddrinfo {

[[nodiscard]] inline Status getaddrinfo_host(const char* host) noexcept { if (host == nullptr) return Status::Invalid; return Status::Ok; }

} // namespace pbsd::userland::usr_bin::getaddrinfo
