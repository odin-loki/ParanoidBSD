module;

export module pbsd.userland.genl.parser_rpc;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/genl/parser_rpc.c
export namespace pbsd::userland::usr_bin::genl::parser_rpc {

[[nodiscard]] inline bool parser_rpc_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::genl::parser_rpc
