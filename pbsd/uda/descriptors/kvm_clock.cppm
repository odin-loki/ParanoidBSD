module;
#include <cstdint>

export module pbsd.uda.kvm_clock;

export import pbsd.uda.schema;
export import pbsd.uda.virtio.common;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/dev/kvm_clock/kvm_clock.c
export namespace pbsd::uda::kvm_clock {

inline constexpr const char kDevName[] = "kvmclock";
inline constexpr std::uint32_t kMsrSystemTime = 0x0000000100000004u;
inline constexpr std::uint32_t kMsrEnableBit    = 0x1;

struct SoftState {
    bool     enabled{};
    bool     attached{};
    std::uint64_t wall_sec{};
    std::uint32_t wall_nsec{};
};

[[nodiscard]] inline Status attach(SoftState& sc) noexcept {
    if (sc.attached) {
        return Status::Busy;
    }
    sc.enabled = false;
    sc.attached = true;
    return Status::Ok;
}

[[nodiscard]] inline Status enable(SoftState& sc) noexcept {
    if (!sc.attached) {
        return Status::Invalid;
    }
    sc.enabled = true;
    return Status::Ok;
}

[[nodiscard]] inline bool probe_mmio(SoftMmio& mem) noexcept {
    const auto val = mem.read32(0);
    return val != 0 && val != 0xFFFFFFFFu;
}

inline constexpr RegInsn kKvmClockInit[] = {
    {RegOp::Write32, 0, kMsrEnableBit, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor kvm_clock_pv() noexcept {
    return Descriptor{
        .name = "kvm_clock",
        .provenance = "hbsd/src/sys/dev/kvm_clock/kvm_clock.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = kKvmClockInit,
        .reset_sequence = kKvmClockInit,
    };
}

} // namespace pbsd::uda::kvm_clock
