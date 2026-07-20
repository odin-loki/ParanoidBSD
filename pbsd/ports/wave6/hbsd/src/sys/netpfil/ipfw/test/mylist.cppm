export module pbsd.port.wave6.hbsd.src.sys.netpfil.ipfw.test.mylist;

module;
// Header bridge — replace #include of hbsd/src/sys/netpfil/ipfw/test/mylist.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/ipfw/test/mylist.h wave=wave6 loc=49
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::test::mylist {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::ipfw::test::mylist
