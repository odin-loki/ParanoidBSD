module;
#include <cstdint>

export module pbsd.uda.cam.ccb;

export import pbsd.core;
export import pbsd.uda.cam.status;

/// CAM CCB function codes and queue indices (hbsd/src/sys/cam/cam_ccb.h).
export namespace pbsd::uda::cam::ccb {

inline constexpr int kXptPathInq       = 0x01;
inline constexpr int kXptDevMatch      = 0x02;
inline constexpr int kScsiIo           = 0x0f;
inline constexpr int kScsiTreq         = 0x10;
inline constexpr int kDevCtrl          = 0x11;

inline constexpr int kUnqueuedIndex    = -1;
inline constexpr int kActiveIndex      = -2;
inline constexpr int kDoneqIndex       = -3;
inline constexpr int kAsyncIndex       = -4;

inline constexpr std::uint32_t kPriorityNormal = 0x0080;
inline constexpr std::uint32_t kPriorityNone     = 0xFFFFFFFFu;

inline constexpr std::uint8_t kMaxCdbLen = 16;
inline constexpr std::uint8_t kMaxSenseLen = 64;

struct CcbHeaderStub {
    std::uint32_t status{};
    std::uint8_t  func_code{};
    std::uint8_t  path_id{};
    std::uint8_t  target_id{};
    std::uint8_t  target_lun{};
};

struct SenseDataStub {
    std::uint8_t  data[kMaxSenseLen]{};
    std::uint8_t  valid_len{};
};

struct ScsiIoStub {
    CcbHeaderStub hdr{};
    std::uint8_t  cdb[kMaxCdbLen]{};
    std::uint8_t  cdb_len{};
    std::uint32_t dxfer_len{};
    SenseDataStub sense{};
};

[[nodiscard]] constexpr std::uint8_t cam_status_mask(std::uint32_t status) noexcept {
    return static_cast<std::uint8_t>(status & 0xFFu);
}

[[nodiscard]] constexpr bool ccb_complete(const CcbHeaderStub& hdr) noexcept {
    using namespace pbsd::uda::cam::status;
    return is_complete(cam_status_mask(hdr.status));
}

[[nodiscard]] constexpr Status validate_scsi_io(const ScsiIoStub& io) noexcept {
    if (io.cdb_len == 0 || io.cdb_len > kMaxCdbLen) {
        return Status::Invalid;
    }
    if (io.hdr.func_code != kScsiIo) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status apply_completion(CcbHeaderStub& hdr,
                                                std::uint8_t cam_st) noexcept {
    hdr.status = (hdr.status & ~0xFFu) | cam_st;
    return pbsd::uda::cam::status::to_pbsd_status(cam_st);
}

} // namespace pbsd::uda::cam::ccb
