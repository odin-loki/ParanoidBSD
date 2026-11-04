module;
#include <cstdint>

export module pbsd.uda.etherswitch;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/dev/etherswitch/etherswitch.c
export namespace pbsd::uda::etherswitch {

inline constexpr unsigned kMaxPorts = 32;

enum class IoctlCmd : unsigned long {
    GetVersion = 0x80000001,
    GetPorts   = 0x80000002,
    GetPort    = 0x80000003,
};

struct PortInfo {
    unsigned index{};
    bool     link_up{};
    bool     enabled{};
};

struct SoftState {
    unsigned port_count{};
    bool     attached{};
    PortInfo ports[kMaxPorts]{};
};

[[nodiscard]] inline Status attach(SoftState& sc, unsigned ports) noexcept {
    if (sc.attached) {
        return Status::Busy;
    }
    if (ports == 0 || ports > kMaxPorts) {
        return Status::Invalid;
    }
    sc.port_count = ports;
    sc.attached = true;
    for (unsigned i = 0; i < ports; ++i) {
        sc.ports[i].index = i;
        sc.ports[i].enabled = true;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status detach(SoftState& sc) noexcept {
    if (!sc.attached) {
        return Status::Invalid;
    }
    sc.attached = false;
    sc.port_count = 0;
    return Status::Ok;
}

inline constexpr RegInsn kEtherswitchInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor etherswitch_generic() noexcept {
    return Descriptor{
        .name = "etherswitch",
        .provenance = "hbsd/src/sys/dev/etherswitch/etherswitch.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = kEtherswitchInit,
        .reset_sequence = kEtherswitchInit,
    };
}

} // namespace pbsd::uda::etherswitch
