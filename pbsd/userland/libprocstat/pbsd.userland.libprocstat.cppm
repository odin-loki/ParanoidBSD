export module pbsd.userland.libprocstat;

export import pbsd.core;

export import pbsd.userland.libprocstat.core;
export import pbsd.userland.libprocstat.common_kvm;
export import pbsd.userland.libprocstat.cd9660;
export import pbsd.userland.libprocstat.msdosfs;
export import pbsd.userland.libprocstat.smbfs;
export import pbsd.userland.libprocstat.udf;
export import pbsd.userland.libprocstat.zfs;
export import pbsd.userland.libprocstat.compat;

/// libprocstat umbrella — libprocstat.c entry points deferred.
export namespace pbsd::userland::libprocstat {

[[nodiscard]] inline StatusOnly procstat_open() noexcept {
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::libprocstat
