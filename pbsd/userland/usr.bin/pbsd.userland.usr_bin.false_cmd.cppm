module;

export module pbsd.userland.usr_bin.false_cmd;

export import pbsd.core;
export import pbsd.userland.false_cmd;

/// Port of hbsd/src/usr.bin/false/false.c — same semantics as bin/false(1).
export namespace pbsd::userland::usr_bin::false_cmd {

using pbsd::userland::bin::false_::run;
using pbsd::userland::bin::false_::exit_code;

} // namespace pbsd::userland::usr_bin::false_cmd
