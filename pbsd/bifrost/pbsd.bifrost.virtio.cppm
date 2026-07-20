module;
#include <cstdint>

export module pbsd.bifrost.virtio;

export import pbsd.core;

/// Wave 8 — paravirtual I/O for BIFROST guests (SI-8 bounded MMIO).
/// PROVENANCE: hbsd/src/sys/dev/virtio/mmio/virtio_mmio.h, virtio.h
export namespace pbsd::bifrost::virtio {

enum class PvKind : unsigned char {
    Block = 0,
    Net = 1,
    Console = 2,
    Gpu = 3,
    Entropy = 4,
    Balloon = 5,
};

/// virtio-mmio status bits (virtio_mmio.h).
enum class MmioStatus : unsigned char {
    Acknowledge = 1,
    Driver      = 2,
    DriverOk    = 4,
    FeaturesOk  = 8,
    Failed      = 128,
};

/// virtio-mmio register offsets (virtio_mmio.h).
enum class MmioReg : unsigned int {
    MagicValue       = 0x000,
    Version          = 0x004,
    DeviceId         = 0x008,
    VendorId         = 0x00c,
    HostFeatures     = 0x010,
    HostFeaturesSel  = 0x014,
    GuestFeatures    = 0x020,
    GuestFeaturesSel = 0x024,
    QueueSel         = 0x030,
    QueueNumMax      = 0x034,
    QueueNum         = 0x038,
    QueueReady       = 0x044,
    QueueNotify      = 0x050,
    InterruptStatus  = 0x060,
    InterruptAck     = 0x064,
    Status           = 0x070,
    Config           = 0x100,
};

inline constexpr std::uint32_t kMmioMagicVirt   = 0x74726976;
inline constexpr std::uint32_t kMmioIntVring    = 1u << 0;
inline constexpr std::uint32_t kMmioIntConfig   = 1u << 1;
inline constexpr std::uint32_t kMmioVringAlign  = 4096;

struct PvDevice {
    PvKind kind{PvKind::Block};
    std::uint16_t vendor_id{0x1AF4};
    std::uint16_t device_id{0x1001}; // virtio-blk default
    std::uint32_t mmio_base{0};
    std::uint32_t mmio_size{0x1000};
    std::uint8_t pci_slot{0};
};

[[nodiscard]] inline PvDevice pv_block(std::uint32_t mmio_base) noexcept {
    return PvDevice{
        .kind = PvKind::Block,
        .vendor_id = 0x1AF4,
        .device_id = 0x1001,
        .mmio_base = mmio_base,
        .mmio_size = 0x1000,
        .pci_slot = 0,
    };
}

[[nodiscard]] inline PvDevice pv_net(std::uint32_t mmio_base) noexcept {
    return PvDevice{
        .kind = PvKind::Net,
        .vendor_id = 0x1AF4,
        .device_id = 0x1000,
        .mmio_base = mmio_base,
        .mmio_size = 0x1000,
        .pci_slot = 1,
    };
}

[[nodiscard]] inline PvDevice pv_console(std::uint32_t mmio_base) noexcept {
    return PvDevice{
        .kind = PvKind::Console,
        .vendor_id = 0x1AF4,
        .device_id = 0x1003,
        .mmio_base = mmio_base,
        .mmio_size = 0x1000,
        .pci_slot = 2,
    };
}

[[nodiscard]] inline PvDevice pv_gpu(std::uint32_t mmio_base) noexcept {
    return PvDevice{
        .kind = PvKind::Gpu,
        .vendor_id = 0x1AF4,
        .device_id = 0x1050,
        .mmio_base = mmio_base,
        .mmio_size = 0x1000,
        .pci_slot = 3,
    };
}

[[nodiscard]] inline Status validate_mmio_status(unsigned char status) noexcept {
    if ((status & static_cast<unsigned char>(MmioStatus::Failed)) != 0) {
        return Status::Invalid;
    }
    if ((status & static_cast<unsigned char>(MmioStatus::DriverOk)) != 0
        && (status & static_cast<unsigned char>(MmioStatus::Driver)) == 0) {
        return Status::Protocol;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_mmio_offset(std::uint32_t offset,
                                                 std::uint32_t mmio_size) noexcept {
    if (mmio_size == 0) {
        return Status::Invalid;
    }
    if (offset >= mmio_size) {
        return Status::Denied;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status attach_pv_stub(PvDevice const& dev) noexcept {
    if (dev.mmio_base == 0 || dev.mmio_size == 0) {
        return Status::Invalid;
    }
    if (dev.vendor_id != 0x1AF4) {
        return Status::Protocol;
    }
    (void)dev;
    return Status::Ok;
}

} // namespace pbsd::bifrost::virtio
