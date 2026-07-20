module;
#include <cstdint>

export module pbsd.uda.cam.xpt;

export import pbsd.core;
export import pbsd.uda.cam.ccb;
export import pbsd.uda.cam.status;

/// XPT transport integration stubs (hbsd/src/sys/cam/cam_xpt.c).
export namespace pbsd::uda::cam::xpt {

inline constexpr std::uint8_t kPathInquiryLen = 56;

struct PathInquiryStub {
    std::uint8_t  path_id{};
    std::uint8_t  target_id{};
    std::uint8_t  target_lun{};
    std::uint8_t  hba_inquiry{};
    char          dev_name[16]{};
};

struct PeriphStub {
    void*         softc{};
    PathInquiryStub path{};
    bool          registered{};
};

[[nodiscard]] constexpr Status setup_path_inquiry(PathInquiryStub& pi,
                                                  std::uint8_t path_id) noexcept {
    pi.path_id = path_id;
    pi.hba_inquiry = 0x01;
    pi.dev_name[0] = 'a';
    pi.dev_name[1] = 'h';
    pi.dev_name[2] = 'c';
    pi.dev_name[3] = 'i';
    return Status::Ok;
}

[[nodiscard]] constexpr Status register_periph(PeriphStub& p) noexcept {
    if (p.registered) {
        return Status::Busy;
    }
    p.registered = true;
    return Status::Ok;
}

[[nodiscard]] constexpr Status submit_ccb(PeriphStub& p, ccb::CcbHeaderStub& hdr) noexcept {
    if (!p.registered) {
        return Status::Invalid;
    }
    if (hdr.func_code == 0) {
        return Status::Invalid;
    }
    hdr.status = pbsd::uda::cam::status::kReqInprog;
    return Status::Ok;
}

[[nodiscard]] constexpr Status complete_ccb(ccb::CcbHeaderStub& hdr,
                                            std::uint8_t cam_st) noexcept {
    return ccb::apply_completion(hdr, cam_st);
}

[[nodiscard]] constexpr bool periph_ready(const PeriphStub& p) noexcept {
    return p.registered && p.softc != nullptr;
}

[[nodiscard]] constexpr Status abort_ccb(PeriphStub& p, ccb::CcbHeaderStub& hdr) noexcept {
    if (!p.registered) {
        return Status::Invalid;
    }
    if (hdr.func_code == 0) {
        return Status::Invalid;
    }
    hdr.status = pbsd::uda::cam::status::kReqAborted;
    return Status::Ok;
}

[[nodiscard]] constexpr Status rescan_path(PathInquiryStub& pi) noexcept {
    pi.target_id = 0;
    pi.target_lun = 0;
    return setup_path_inquiry(pi, pi.path_id);
}

[[nodiscard]] constexpr Status async_notify(PeriphStub& p) noexcept {
    if (!p.registered) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::uda::cam::xpt
