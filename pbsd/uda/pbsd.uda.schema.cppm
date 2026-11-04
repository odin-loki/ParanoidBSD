module;
#include <cstdint>
#include <span>
#include <string_view>

export module pbsd.uda.schema;

import pbsd.core;

export namespace pbsd::uda {

enum class RegOp : std::uint8_t {
    Write8  = 0x01,
    Write16 = 0x02,
    Write32 = 0x03,
    Read8   = 0x11,
    Read32  = 0x13,
    WaitUs  = 0x20,
    CheckEq = 0x30,
    Done    = 0xFF,
};

struct RegInsn {
    RegOp         op{};
    std::uint32_t offset{};
    std::uint32_t value{};
    std::uint32_t mask{};
    std::uint32_t timeout{};
};

enum class DeviceClass : std::uint8_t {
    Block   = 0x01,
    Network = 0x02,
    Display = 0x03,
    Input   = 0x04,
    Sensor  = 0x05,
};

struct Descriptor {
    std::string_view           name{};
    std::string_view           provenance{};
    DeviceClass                device_class{};
    std::uint16_t              vendor_id{};
    std::uint16_t              device_id{};
    std::span<const RegInsn>   init_sequence{};
    std::span<const RegInsn>   reset_sequence{};
};

struct MmioCapability {
    std::uintptr_t base{};
    std::size_t    length{};

    [[nodiscard]] constexpr bool contains(std::uint32_t offset, std::size_t size) const noexcept {
        const auto off = static_cast<std::size_t>(offset);
        return off + size <= length;
    }

    [[nodiscard]] constexpr bool valid() const noexcept { return length > 0; }
};

struct IrqCapability {
    unsigned line{};

    [[nodiscard]] constexpr bool valid() const noexcept { return line != 0; }
};

struct DeviceGrant {
    MmioCapability mmio{};
    IrqCapability  irq{};

    [[nodiscard]] constexpr bool mmio_valid() const noexcept { return mmio.valid(); }
};

[[nodiscard]] constexpr Status validate_descriptor(const Descriptor& d) noexcept {
    if (d.name.empty() || d.provenance.empty()) {
        return Status::Invalid;
    }
    if (d.init_sequence.empty()) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::uda
