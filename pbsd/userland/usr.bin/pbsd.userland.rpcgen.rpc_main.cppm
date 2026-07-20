module;

export module pbsd.userland.rpcgen.rpc_main;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/rpcgen/rpc_main.c
export namespace pbsd::userland::usr_bin::rpcgen::rpc_main {

[[nodiscard]] inline bool rpc_main_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::rpcgen::rpc_main
