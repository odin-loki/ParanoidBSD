module;
#include <cstdint>

export module pbsd.uda.vmx;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/vmware/vmxnet3/if_vmxreg.h
export namespace pbsd::uda::vmx {

inline constexpr std::uint32_t kRegBar1Cmd = 0x020;

inline constexpr std::uint32_t kCmdEnable = 0xCAFE0000;
inline constexpr std::uint32_t kCmdReset  = 0xCAFE0002;

inline constexpr std::uint16_t kPciVendorVmware = 0x15AD;
inline constexpr std::uint16_t kPciDevVmxnet3   = 0x07B0;

inline constexpr RegInsn kVmxInit[] = {
    {RegOp::Write32, kRegBar1Cmd, kCmdReset, 0, 0},
    {RegOp::WaitUs, 0, 100, 0, 0},
    {RegOp::Write32, kRegBar1Cmd, kCmdEnable, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kVmxReset[] = {
    {RegOp::Write32, kRegBar1Cmd, kCmdReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor vmxnet3() noexcept {
    return Descriptor{
        .name = "vmxnet3",
        .provenance = "hbsd/src/sys/dev/vmware/vmxnet3/if_vmxreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorVmware,
        .device_id = kPciDevVmxnet3,
        .init_sequence = kVmxInit,
        .reset_sequence = kVmxReset,
    };
}

} // namespace pbsd::uda::vmx
