export module pbsd.port.wave6.hbsd.src.sys.netlink.netlink_message_parser;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netlink/netlink_message_parser.c
// void netlink_message_parser_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netlink/netlink_message_parser.c wave=wave6 loc=637
export namespace pbsd::port::wave6::hbsd::src::sys::netlink::netlink_message_parser {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netlink::netlink_message_parser
