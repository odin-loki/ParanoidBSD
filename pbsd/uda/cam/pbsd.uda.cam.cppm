module;
#include <cstdint>

export module pbsd.uda.cam;

export import pbsd.core;
export import pbsd.uda.cam.status;
export import pbsd.uda.cam.ccb;
export import pbsd.uda.cam.xpt;

/// CAM aggregate — status, CCB, and XPT transport (hbsd/src/sys/cam/).
export namespace pbsd::uda::cam {

using namespace pbsd::uda::cam::status;
using namespace pbsd::uda::cam::ccb;
using namespace pbsd::uda::cam::xpt;

struct CamSession {
    PeriphStub  periph{};
    CcbHeaderStub active{};
    bool          started{};
};

[[nodiscard]] constexpr Status start_session(CamSession& s) noexcept {
    if (s.started) {
        return Status::Busy;
    }
    if (setup_path_inquiry(s.periph.path, 0) != Status::Ok) {
        return Status::Invalid;
    }
    s.periph.softc = reinterpret_cast<void*>(1);
    if (register_periph(s.periph) != Status::Ok) {
        return Status::Protocol;
    }
    s.started = true;
    return Status::Ok;
}

[[nodiscard]] constexpr Status submit_scsi_io(CamSession& s, ScsiIoStub& io) noexcept {
    if (!s.started) {
        return Status::Invalid;
    }
    if (validate_scsi_io(io) != Status::Ok) {
        return Status::Invalid;
    }
    s.active = io.hdr;
    return submit_ccb(s.periph, s.active);
}

[[nodiscard]] constexpr Status complete_session_io(CamSession& s,
                                                   std::uint8_t cam_st) noexcept {
    if (!s.started) {
        return Status::Invalid;
    }
    return complete_ccb(s.active, cam_st);
}

[[nodiscard]] constexpr Status abort_session_io(CamSession& s) noexcept {
    if (!s.started) {
        return Status::Invalid;
    }
    return abort_ccb(s.periph, s.active);
}

[[nodiscard]] constexpr Status rescan_session(CamSession& s) noexcept {
    if (!s.started) {
        return Status::Invalid;
    }
    return rescan_path(s.periph.path);
}

[[nodiscard]] constexpr bool session_ready(const CamSession& s) noexcept {
    return s.started && periph_ready(s.periph);
}

[[nodiscard]] inline unsigned cam_module_table_size() noexcept {
    return status_table_size() + 4;
}

} // namespace pbsd::uda::cam
