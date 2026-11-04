module;
#include <cstdint>

export module pbsd.kernel.pax_mac;

import pbsd.core;
import pbsd.kernel.vm;

/// PaX / MAC typed glue — algorithms from `hbsd_pax_noexec.c`, `pax.h`.
export namespace pbsd::kernel::security {

// pax.h — feature state (4-state sysctl values)
enum class PaxFeatureState : int {
    Disabled      = 0,
    OptIn         = 1,
    OptOut        = 2,
    ForceEnabled  = 3,
};

enum class PaxSimpleState : int {
    Disabled = 0,
    Enabled  = 1,
};

// pax.h — process note flags
inline constexpr std::uint32_t kPaxNotePageexec   = 0x00000001u;
inline constexpr std::uint32_t kPaxNoteNopageexec = 0x00000002u;
inline constexpr std::uint32_t kPaxNoteMprotect   = 0x00000004u;
inline constexpr std::uint32_t kPaxNoteNomprotect = 0x00000008u;
inline constexpr std::uint32_t kPaxNoteAslr       = 0x00000040u;
inline constexpr std::uint32_t kPaxNoteNoaslr     = 0x00000080u;

enum class PaxFeature : unsigned int {
    None     = 0,
    PageExec = 1u << 0,
    MProtect = 1u << 1,
    ASLR     = 1u << 2,
    SegvMap  = 1u << 3,
};

[[nodiscard]] constexpr PaxFeature operator|(PaxFeature a, PaxFeature b) noexcept {
    return static_cast<PaxFeature>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

[[nodiscard]] constexpr bool has_pax(PaxFeature set, PaxFeature bit) noexcept {
    return (static_cast<unsigned>(set) & static_cast<unsigned>(bit)) != 0;
}

[[nodiscard]] constexpr bool has_note(std::uint32_t flags, std::uint32_t note) noexcept {
    return (flags & note) == note;
}

struct PaxProcessFlags {
    std::uint32_t notes{0};
};

struct PaxPrisonPolicy {
    PaxFeatureState pageexec_status{PaxFeatureState::OptOut};
    PaxFeatureState mprotect_status{PaxFeatureState::OptOut};
};

/// `pax_pageexec_active` — note flags + prison status.
[[nodiscard]] inline bool pageexec_active(const PaxProcessFlags& proc,
                                          const PaxPrisonPolicy& prison) noexcept {
    if (has_note(proc.notes, kPaxNotePageexec)) {
        return true;
    }
    if (has_note(proc.notes, kPaxNoteNopageexec)) {
        return false;
    }
    switch (prison.pageexec_status) {
    case PaxFeatureState::Disabled:
        return false;
    case PaxFeatureState::ForceEnabled:
        return true;
    case PaxFeatureState::OptIn:
    case PaxFeatureState::OptOut:
    default:
        return true;
    }
}

/// `pax_mprotect_active`
[[nodiscard]] inline bool mprotect_active(const PaxProcessFlags& proc,
                                          const PaxPrisonPolicy& prison) noexcept {
    if (has_note(proc.notes, kPaxNoteMprotect)) {
        return true;
    }
    if (has_note(proc.notes, kPaxNoteNomprotect)) {
        return false;
    }
    switch (prison.mprotect_status) {
    case PaxFeatureState::Disabled:
        return false;
    case PaxFeatureState::ForceEnabled:
        return true;
    case PaxFeatureState::OptIn:
    case PaxFeatureState::OptOut:
    default:
        return true;
    }
}

/// `pax_pageexec` — hbsd_pax_noexec.c:269-273
[[nodiscard]] inline unsigned char pageexec_apply_prot(unsigned char prot) noexcept {
    if ((prot & (vm::kVmProtWrite | vm::kVmProtExecute)) != vm::kVmProtExecute) {
        return static_cast<unsigned char>(prot & ~vm::kVmProtExecute);
    }
    return static_cast<unsigned char>(prot & ~vm::kVmProtWrite);
}

[[nodiscard]] inline vm::MapProtection pageexec_apply(vm::MapProtection prot) noexcept {
    return vm::from_vm_prot(pageexec_apply_prot(vm::to_vm_prot(prot)));
}

/// `pax_mprotect` — adjust maxprot ceiling (hbsd_pax_noexec.c:358-371).
[[nodiscard]] inline unsigned char mprotect_apply(unsigned char prot,
                                                  unsigned char maxprot) noexcept {
    if ((maxprot & (vm::kVmProtWrite | vm::kVmProtExecute)) != vm::kVmProtExecute &&
        (prot & vm::kVmProtExecute) != vm::kVmProtExecute) {
        return static_cast<unsigned char>(maxprot & ~vm::kVmProtExecute);
    }
    return static_cast<unsigned char>(maxprot & ~vm::kVmProtWrite);
}

/// `pax_mprotect_apply_max` — alias for ceiling-only adjustment.
[[nodiscard]] inline unsigned char mprotect_apply_max(unsigned char prot,
                                                      unsigned char maxprot) noexcept {
    return mprotect_apply(prot, maxprot);
}

/// `pax_mprotect_enforce` — block introducing new executable mappings.
[[nodiscard]] inline Status mprotect_enforce(unsigned char old_prot,
                                             unsigned char new_prot,
                                             bool active) noexcept {
    if (!active) {
        return Status::Ok;
    }
    if ((new_prot & vm::kVmProtExecute) == vm::kVmProtExecute &&
        (old_prot & vm::kVmProtExecute) != vm::kVmProtExecute) {
        return Status::Denied;
    }
    return Status::Ok;
}

/// `pax_mprotect_setup_flags` — derive note flags from prison 4-state policy.
[[nodiscard]] inline std::uint32_t mprotect_setup_flags(std::uint32_t flags,
                                                        std::uint32_t mode,
                                                        PaxFeatureState status) noexcept {
    switch (status) {
    case PaxFeatureState::Disabled:
        flags &= ~kPaxNoteMprotect;
        flags |= kPaxNoteNomprotect;
        return flags;
    case PaxFeatureState::ForceEnabled:
        flags |= (kPaxNoteMprotect | kPaxNotePageexec);
        flags &= ~(kPaxNoteNomprotect | kPaxNoteNopageexec);
        return flags;
    case PaxFeatureState::OptIn:
        if ((mode & kPaxNoteMprotect) != 0) {
            flags |= (kPaxNoteMprotect | kPaxNotePageexec);
            flags &= ~(kPaxNoteNomprotect | kPaxNoteNopageexec);
        } else {
            flags &= ~kPaxNoteMprotect;
            flags |= kPaxNoteNomprotect;
        }
        return flags;
    case PaxFeatureState::OptOut:
        if ((mode & kPaxNoteNomprotect) != 0) {
            flags &= ~kPaxNoteMprotect;
            flags |= kPaxNoteNomprotect;
        } else {
            flags |= (kPaxNoteMprotect | kPaxNotePageexec);
            flags &= ~(kPaxNoteNomprotect | kPaxNoteNopageexec);
        }
        return flags;
    }
    flags |= (kPaxNoteMprotect | kPaxNotePageexec);
    flags &= ~(kPaxNoteNomprotect | kPaxNoteNopageexec);
    return flags;
}

enum class MacLabelKind : unsigned char {
    None     = 0,
    Process  = 1,
    Vnode    = 2,
    Socket   = 3,
    Ipc      = 4,
};

struct MacLabel {
    MacLabelKind kind{MacLabelKind::None};
    unsigned int slot{0};
};

class PaxPolicy {
public:
    explicit PaxPolicy(PaxPrisonPolicy prison = {}) noexcept : prison_(prison) {}

    [[nodiscard]] constexpr PaxFeature enabled() const noexcept { return enabled_; }

    [[nodiscard]] Status enforce_map(vm::MapProtection prot,
                                     const PaxProcessFlags& proc) const noexcept {
        if (mprotect_active(proc, prison_)) {
            return vm::validate_wx(prot);
        }
        if (has_pax(enabled_, PaxFeature::MProtect)) {
            return vm::validate_wx(prot);
        }
        return Status::Ok;
    }

    [[nodiscard]] Status mprotect_change(unsigned char old_prot,
                                         unsigned char new_prot,
                                         const PaxProcessFlags& proc) const noexcept {
        return mprotect_enforce(old_prot, new_prot, mprotect_active(proc, prison_));
    }

    [[nodiscard]] Status require_pageexec() const noexcept {
        return has_pax(enabled_, PaxFeature::PageExec) ? Status::Ok : Status::Denied;
    }

private:
    PaxFeature      enabled_{PaxFeature::PageExec | PaxFeature::MProtect | PaxFeature::ASLR};
    PaxPrisonPolicy prison_;
};

class MacGlue {
public:
    [[nodiscard]] Status check_access(MacLabel subject, MacLabel object,
                                      CapabilityRights want) noexcept {
        if (subject.kind == MacLabelKind::None || object.kind == MacLabelKind::None) {
            return Status::Denied;
        }
        (void)want;
        return Status::Ok;
    }

    [[nodiscard]] Status relabel(MacLabel& label, MacLabelKind kind,
                                 unsigned slot) noexcept {
        label.kind = kind;
        label.slot = slot;
        return Status::Ok;
    }
};

} // namespace pbsd::kernel::security
