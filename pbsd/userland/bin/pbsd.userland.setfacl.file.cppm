module;

export module pbsd.userland.setfacl.file;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/setfacl/file.c
export namespace pbsd::userland::bin::setfacl::file {

[[nodiscard]] inline bool file_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::setfacl::file
