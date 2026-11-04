module;
#include <cstddef>

export module pbsd.bifrost.snapshot;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/include/vmm_snapshot.h
export namespace pbsd::bifrost::snapshot {

enum class DeviceReq : unsigned char {
    Vioapic = 1,
    Vm,
    Vlapic,
    VmMem,
    Vhpet,
    Vmcx,
    Vatpic,
    Vatpit,
    Vpmtmr,
    Vrtc,
};

enum class Op : unsigned char {
    Save = 0,
    Restore = 1,
};

struct Buffer {
    const unsigned char* start{};
    std::size_t size{};
    const unsigned char* cur{};
    std::size_t remaining{};
};

[[nodiscard]] inline Status validate_req(DeviceReq req) noexcept {
    switch (req) {
    case DeviceReq::Vioapic:
    case DeviceReq::Vm:
    case DeviceReq::Vlapic:
    case DeviceReq::VmMem:
    case DeviceReq::Vhpet:
    case DeviceReq::Vmcx:
    case DeviceReq::Vatpic:
    case DeviceReq::Vatpit:
    case DeviceReq::Vpmtmr:
    case DeviceReq::Vrtc:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline std::size_t consumed(const Buffer& b) noexcept {
    if (b.start == nullptr || b.cur == nullptr || b.cur < b.start) {
        return 0;
    }
    return static_cast<std::size_t>(b.cur - b.start);
}

} // namespace pbsd::bifrost::snapshot
