export module pbsd.port.wave2.hbsd.src.sbin.pfctl.pfctl_ioctl;

module;
// Header bridge — replace #include of hbsd/src/sbin/pfctl/pfctl_ioctl.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

export import pbsd.userland.capsicum.helpers;
import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sbin/pfctl/pfctl_ioctl.h wave=wave2 loc=0
export namespace pbsd::port::wave2::hbsd::src::sbin::pfctl::pfctl_ioctl {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::sbin::pfctl::pfctl_ioctl
