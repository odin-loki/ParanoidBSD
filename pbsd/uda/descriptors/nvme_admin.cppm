module;
#include <cstdint>

export module pbsd.uda.nvme_admin;

export import pbsd.uda.schema;
export import pbsd.uda.nvme;

/// PROVENANCE: hbsd/src/sys/dev/nvme/nvme.h — admin queue + namespace opcodes.
export namespace pbsd::uda::nvme::admin {

inline constexpr std::uint8_t kOpcDeleteIoSq   = 0x00;
inline constexpr std::uint8_t kOpcCreateIoSq   = 0x01;
inline constexpr std::uint8_t kOpcGetLogPage   = 0x02;
inline constexpr std::uint8_t kOpcDeleteIoCq   = 0x04;
inline constexpr std::uint8_t kOpcCreateIoCq   = 0x05;
inline constexpr std::uint8_t kOpcIdentify     = 0x06;
inline constexpr std::uint8_t kOpcAbort        = 0x08;
inline constexpr std::uint8_t kOpcSetFeatures  = 0x09;
inline constexpr std::uint8_t kOpcGetFeatures  = 0x0A;
inline constexpr std::uint8_t kOpcAsyncEvent   = 0x0C;

inline constexpr std::uint8_t kOpcRead         = 0x02;
inline constexpr std::uint8_t kOpcWrite        = 0x01;
inline constexpr std::uint8_t kOpcFlush        = 0x00;

inline constexpr std::uint32_t kCcCssNvm       = 0x00000000;
inline constexpr std::uint32_t kCcIosqes       = 0x00060000;
inline constexpr std::uint32_t kCcIocqes       = 0x00400000;
inline constexpr std::uint32_t kCcMps4096      = 0x00000000;

inline constexpr std::uint32_t kAqaAsqs        = 0x0000000F;
inline constexpr std::uint32_t kAqaAcqs        = 0x000F0000;

inline constexpr RegInsn kNvmeAdminInit[] = {
    {RegOp::Write32, kRegCc, 0x0, 0, 0},
    {RegOp::CheckEq, kRegCsts, 0x0, kCstsRdyMask, 1000},
    {RegOp::Write32, kRegAqa, kAqaAsqs | kAqaAcqs, 0, 0},
    {RegOp::Write32, kRegAsq, 0x1000, 0, 0},
    {RegOp::Write32, kRegAcq, 0x2000, 0, 0},
    {RegOp::Write32, kRegCc,
     kCcEnMask | kCcCssNvm | kCcIosqes | kCcIocqes | kCcMps4096, 0, 0},
    {RegOp::CheckEq, kRegCsts, kCstsRdyMask, kCstsRdyMask, 1000},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kNvmeAdminReset[] = {
    {RegOp::Write32, kRegCc, 0x0, 0, 0},
    {RegOp::Write32, kRegAqa, 0x0, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor nvme_admin_1_4() noexcept {
    return Descriptor{
        .name = "nvme-admin-1.4",
        .provenance = "hbsd/src/sys/dev/nvme/nvme.h",
        .device_class = DeviceClass::Block,
        .vendor_id = 0xFFFF,
        .device_id = kPciClassStorage,
        .init_sequence = kNvmeAdminInit,
        .reset_sequence = kNvmeAdminReset,
    };
}

[[nodiscard]] inline constexpr std::uint8_t io_opc_read() noexcept {
    return kOpcRead;
}
[[nodiscard]] inline constexpr std::uint8_t io_opc_write() noexcept {
    return kOpcWrite;
}
[[nodiscard]] inline constexpr std::uint8_t io_opc_flush() noexcept {
    return kOpcFlush;
}

} // namespace pbsd::uda::nvme::admin
