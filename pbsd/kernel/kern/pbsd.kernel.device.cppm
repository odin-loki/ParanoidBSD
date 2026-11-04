module;
#include <cstdint>

export module pbsd.kernel.device;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/sys/bus.h — device_t softc scaffold (UDA dual-world).
export namespace pbsd::kernel::device {

inline constexpr unsigned kMaxUnit = 65535;

struct Softc {
    const char* name{};
    unsigned    unit{};
    bool        attached{false};
};

[[nodiscard]] inline Status attach(Softc& d, const char* name,
                                   unsigned unit) noexcept {
    if (name == nullptr || name[0] == '\0' || unit > kMaxUnit) {
        return Status::Invalid;
    }
    if (d.attached) {
        return Status::Busy;
    }
    d.name = name;
    d.unit = unit;
    d.attached = true;
    return Status::Ok;
}

[[nodiscard]] inline Status detach(Softc& d) noexcept {
    if (!d.attached) {
        return Status::NotFound;
    }
    d.attached = false;
    return Status::Ok;
}

} // namespace pbsd::kernel::device
