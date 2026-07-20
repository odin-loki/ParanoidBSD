module;
#include <cstdint>

export module pbsd.stand.devinfo;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/common/devinfo.c — boot device enumeration.
export namespace pbsd::stand::devinfo {

enum class Bus : unsigned char {
    Unknown = 0,
    Pci = 1,
    Usb = 2,
    Virtio = 3,
    Ata = 4,
    Nvme = 5,
};

enum class Kind : unsigned char {
    Unknown = 0,
    Disk = 1,
    Network = 2,
    Console = 3,
};

struct Device {
    Bus  bus{Bus::Unknown};
    Kind kind{Kind::Unknown};
    unsigned unit{};
};

[[nodiscard]] inline Status validate_device(Device const& d) noexcept {
    if (d.bus == Bus::Unknown || d.kind == Kind::Unknown) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_unit(unsigned unit) noexcept {
    if (unit > 255) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::stand::devinfo
