module;
#include <cstdint>

export module pbsd.bifrost.vpmu;

import pbsd.core;
import pbsd.bifrost.hypervisor;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/intel/vmx_controls.h, contrib/xen/pmu.h — VPMU modes.
export namespace pbsd::bifrost::vpmu {

inline constexpr unsigned kProcRdpmcExiting = 1u << 11;
inline constexpr unsigned kProcMsrBitmaps = 1u << 28;
inline constexpr unsigned kExitLoadPerfGlobalCtrl = 1u << 12;
inline constexpr unsigned kEntryLoadPerfGlobalCtrl = 1u << 13;

enum class Mode : unsigned char {
    Off = 0,
    Self = 1u << 0,
    Hv = 1u << 1,
    All = 1u << 2,
};

enum class Feature : unsigned char {
    None = 0,
    IntelBts = 1u << 0,
    IpcOnly = 1u << 1,
    ArchOnly = 1u << 2,
};

[[nodiscard]] constexpr Feature operator|(Feature a, Feature b) noexcept {
    return static_cast<Feature>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

struct Config {
    Mode mode{Mode::Off};
    Feature features{Feature::None};
    bool rdpmc_exiting{true};
    bool load_perf_global{false};
};

[[nodiscard]] inline Status validate_mode(Mode m) noexcept {
    const auto v = static_cast<unsigned>(m);
    if (v > static_cast<unsigned>(Mode::All)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_config(const Config& cfg) noexcept {
    if (validate_mode(cfg.mode) != Status::Ok) {
        return Status::Invalid;
    }
    if (cfg.mode == Mode::Off && cfg.load_perf_global) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned proc_controls(const Config& cfg) noexcept {
    unsigned ctl = 0;
    if (cfg.rdpmc_exiting) {
        ctl |= kProcRdpmcExiting;
    }
    if (cfg.mode != Mode::Off) {
        ctl |= kProcMsrBitmaps;
    }
    return ctl;
}

[[nodiscard]] inline hypervisor::Backend backend_for() noexcept {
    return hypervisor::Backend::Vmx;
}

} // namespace pbsd::bifrost::vpmu
