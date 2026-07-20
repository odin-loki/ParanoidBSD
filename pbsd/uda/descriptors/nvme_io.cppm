module;
#include <cstdint>

export module pbsd.uda.nvme_io;

export import pbsd.core;
export import pbsd.uda.schema;
export import pbsd.uda.nvme;
export import pbsd.uda.nvme_admin;

/// PROVENANCE: hbsd/src/sys/dev/nvme/{nvme.h,nvme_private.h} — IO queue pair bring-up.
export namespace pbsd::uda::nvme_io {

using namespace pbsd::uda::nvme;
using namespace pbsd::uda::nvme::admin;

inline constexpr std::uint32_t kIoEntries   = 256;
inline constexpr std::uint32_t kIoTrackers  = 128;
inline constexpr std::uint32_t kMinIoTrackers = 4;
inline constexpr std::uint32_t kMaxIoTrackers = 1024;

inline constexpr std::uint32_t kSqTail = 0x1000;
inline constexpr std::uint32_t kCqHead = 0x1004;

inline constexpr RegInsn kNvmeIoQpairInit[] = {
    {RegOp::Write32, kRegIntms, 0xFFFFFFFF, 0, 0},
    {RegOp::Write32, kRegAqa, (kAqaAsqs << 0) | (kAqaAcqs << 16), 0, 0},
    {RegOp::Write32, kRegAsq, 0x1000, 0, 0},
    {RegOp::Write32, kRegAcq, 0x2000, 0, 0},
    {RegOp::Write32, kRegCc,
     kCcEnMask | kCcCssNvm | kCcIosqes | kCcIocqes | kCcMps4096, 0, 0},
    {RegOp::CheckEq, kRegCsts, kCstsRdyMask, kCstsRdyMask, 1000},
    {RegOp::Write32, kSqTail, 0x0, 0, 0},
    {RegOp::Write32, kCqHead, 0x0, 0, 0},
    {RegOp::Write32, kRegIntmc, 0x1, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kNvmeIoQpairReset[] = {
    {RegOp::Write32, kSqTail, 0x0, 0, 0},
    {RegOp::Write32, kCqHead, 0x0, 0, 0},
    {RegOp::Write32, kRegIntms, 0x1, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor nvme_io_qpair() noexcept {
    return Descriptor{
        .name = "nvme-io-qpair",
        .provenance = "hbsd/src/sys/dev/nvme/nvme_private.h",
        .device_class = DeviceClass::Block,
        .vendor_id = 0xFFFF,
        .device_id = kPciClassStorage,
        .init_sequence = kNvmeIoQpairInit,
        .reset_sequence = kNvmeIoQpairReset,
    };
}

[[nodiscard]] constexpr Status validate_io_trackers(unsigned trackers) noexcept {
    if (trackers < kMinIoTrackers || trackers > kMaxIoTrackers) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr std::uint8_t io_opc_read() noexcept {
    return 0x02;
}
[[nodiscard]] constexpr std::uint8_t io_opc_write() noexcept {
    return 0x01;
}
[[nodiscard]] constexpr std::uint8_t io_opc_flush() noexcept {
    return 0x00;
}

[[nodiscard]] inline bool probe_qpair(SoftMmio const& mem) noexcept {
    const auto csts = mem.read32(kRegCsts);
    return (csts & kCstsRdyMask) != 0;
}

} // namespace pbsd::uda::nvme_io
