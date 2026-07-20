module;

export module pbsd.userland.ssh_copy_id;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ssh-copy-id/ssh-copy-id.c
export namespace pbsd::userland::usr_bin::ssh_copy_id {

[[nodiscard]] inline bool ssh_copy_id_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::ssh_copy_id
