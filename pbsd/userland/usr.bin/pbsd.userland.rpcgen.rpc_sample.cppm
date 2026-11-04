module;

export module pbsd.userland.rpcgen.rpc_sample;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/rpcgen/rpc_sample.c
export namespace pbsd::userland::usr_bin::rpcgen::rpc_sample {

[[nodiscard]] inline bool rpc_sample_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::rpcgen::rpc_sample
