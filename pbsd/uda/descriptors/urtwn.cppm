module;
#include <cstdint>

export module pbsd.uda.urtwn;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/usb/wlan/if_urtwnreg.h
export namespace pbsd::uda::urtwn {

inline constexpr std::uint8_t kRegSysFuncEn  = 0x02;
inline constexpr std::uint8_t kRegMacAddr    = 0x10;
inline constexpr std::uint8_t kSysFuncEnBbEn = 0x02;

inline constexpr RegInsn kUrtwnInit[] = {
    {RegOp::Write8, kRegSysFuncEn, kSysFuncEnBbEn, 0, 0},
    {RegOp::WaitUs, 0, 100, 0, 0},
    {RegOp::Write8, kRegMacAddr, 0x00, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kUrtwnReset[] = {
    {RegOp::Write8, kRegSysFuncEn, 0, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor urtwn_rtl8188eu() noexcept {
    return Descriptor{
        .name = "urtwn-rtl8188eu",
        .provenance = "hbsd/src/sys/dev/usb/wlan/if_urtwnreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x0BDA,
        .device_id = 0x8179,
        .init_sequence = kUrtwnInit,
        .reset_sequence = kUrtwnReset,
    };
}

} // namespace pbsd::uda::urtwn
