module;
#include <cstdint>

export module pbsd.uda.cam.status;

export import pbsd.core;

/// CAM completion status codes (hbsd/src/sys/cam/cam.h).
export namespace pbsd::uda::cam::status {

inline constexpr std::uint8_t kReqInprog       = 0x00;
inline constexpr std::uint8_t kReqCmp          = 0x01;
inline constexpr std::uint8_t kReqAborted      = 0x02;
inline constexpr std::uint8_t kUaAbort         = 0x03;
inline constexpr std::uint8_t kReqCmpErr       = 0x04;
inline constexpr std::uint8_t kBusy            = 0x05;
inline constexpr std::uint8_t kReqInvalid      = 0x06;
inline constexpr std::uint8_t kPathInvalid     = 0x07;
inline constexpr std::uint8_t kDevNotThere     = 0x08;
inline constexpr std::uint8_t kSelTimeout      = 0x0a;
inline constexpr std::uint8_t kCmdTimeout     = 0x0b;
inline constexpr std::uint8_t kScsiStatusError = 0x0c;

struct StatusEntry {
    std::uint8_t  code;
    const char*   name;
};

inline constexpr StatusEntry kStatusTable[] = {
    {kReqInprog,       "CAM_REQ_INPROG"},
    {kReqCmp,          "CAM_REQ_CMP"},
    {kReqAborted,      "CAM_REQ_ABORTED"},
    {kUaAbort,         "CAM_UA_ABORT"},
    {kReqCmpErr,       "CAM_REQ_CMP_ERR"},
    {kBusy,            "CAM_BUSY"},
    {kReqInvalid,      "CAM_REQ_INVALID"},
    {kPathInvalid,     "CAM_PATH_INVALID"},
    {kDevNotThere,     "CAM_DEV_NOT_THERE"},
    {kSelTimeout,      "CAM_SEL_TIMEOUT"},
    {kCmdTimeout,      "CAM_CMD_TIMEOUT"},
    {kScsiStatusError, "CAM_SCSI_STATUS_ERROR"},
};

[[nodiscard]] inline unsigned status_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kStatusTable) / sizeof(kStatusTable[0]));
}

[[nodiscard]] constexpr bool is_complete(std::uint8_t st) noexcept {
    return st != kReqInprog;
}

[[nodiscard]] constexpr bool is_success(std::uint8_t st) noexcept {
    return st == kReqCmp;
}

[[nodiscard]] constexpr bool is_retryable(std::uint8_t st) noexcept {
    return st == kBusy || st == kSelTimeout || st == kCmdTimeout;
}

[[nodiscard]] inline Result<const char*> status_name(std::uint8_t st) noexcept {
    for (const auto& e : kStatusTable) {
        if (e.code == st) {
            return result_ok(e.name);
        }
    }
    return result_err<const char*>(Status::NotFound);
}

[[nodiscard]] constexpr Status to_pbsd_status(std::uint8_t cam_st) noexcept {
    if (is_success(cam_st)) {
        return Status::Ok;
    }
    if (cam_st == kReqInvalid || cam_st == kPathInvalid) {
        return Status::Invalid;
    }
    if (cam_st == kDevNotThere) {
        return Status::NotFound;
    }
    if (is_retryable(cam_st)) {
        return Status::Busy;
    }
    return Status::Protocol;
}

} // namespace pbsd::uda::cam::status
