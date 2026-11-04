module;

export module pbsd.userland.ipcs.ipc;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ipcs/ipc.c
export namespace pbsd::userland::usr_bin::ipcs::ipc {

[[nodiscard]] inline bool ipc_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::ipcs::ipc
