module;
#include <cstdint>

export module pbsd.bifrost;

export import pbsd.handles;
export import pbsd.bifrost.virtio;
export import pbsd.bifrost.hypervisor;
export import pbsd.bifrost.nested;
export import pbsd.bifrost.ept;
export import pbsd.bifrost.vmcb;
export import pbsd.bifrost.vmcs;
export import pbsd.bifrost.npt;
export import pbsd.bifrost.ioapic;
export import pbsd.bifrost.lapic;
export import pbsd.bifrost.vpid;
export import pbsd.bifrost.exit;
export import pbsd.bifrost.tlb;
export import pbsd.bifrost.msr_bitmap;
export import pbsd.bifrost.vpmu;
export import pbsd.bifrost.vmx_ctrl;
export import pbsd.bifrost.vioapic;
export import pbsd.bifrost.vlapic;
export import pbsd.bifrost.ioport;
export import pbsd.bifrost.xsave;
export import pbsd.bifrost.vhpet;
export import pbsd.bifrost.hypercall;
export import pbsd.bifrost.vmx;
export import pbsd.bifrost.svm;
export import pbsd.bifrost.inst_emul;
export import pbsd.bifrost.snapshot;
export import pbsd.bifrost.iommu;
export import pbsd.bifrost.pit;
export import pbsd.bifrost.pci_pass;
export import pbsd.bifrost.vga;
export import pbsd.bifrost.rtc;
export import pbsd.bifrost.intercept;
export import pbsd.bifrost.memslot;
export import pbsd.bifrost.irqchip;
export import pbsd.bifrost.migration;
export import pbsd.bifrost.timer;
export import pbsd.bifrost.apic_timer;
export import pbsd.bifrost.gpa_pool;
export import pbsd.bifrost.msr_filter;
export import pbsd.bifrost.vmexit_reason;
export import pbsd.bifrost.guest_mem;
export import pbsd.bifrost.cpuid;
export import pbsd.bifrost.mtrr;
export import pbsd.bifrost.cr_shadow;
export import pbsd.bifrost.dr_shadow;
export import pbsd.bifrost.vnet;
export import pbsd.bifrost.vblk;
export import pbsd.bifrost.vconsole;
export import pbsd.bifrost.vinput;
export import pbsd.bifrost.vserial;
export import pbsd.bifrost.vballoon;
export import pbsd.bifrost.msi;
export import pbsd.bifrost.msix;
import pbsd.core;

/// Wave 8 — Type-1 VM nucleus (SI-8).
///
/// SI-8 isolation notes:
/// - Every guest vCPU, RAM page, and MMIO window is reachable only through the VmHandle
///   capability tree granted at create_vm time (SI-1: child rights ⊆ parent).
/// - Paravirtual devices (virtio PV) are host-side UDA engines; the guest sees
///   virtio MMIO only inside the grant — no ambient host kernel mappings (SI-2, SI-7).
/// - destroy_vm revokes the VmHandle lineage subtree atomically (SI-6).
/// - Guest RAM is host-owned; the guest never receives a handle to host kernel objects.
export namespace pbsd::bifrost {

enum class VmState : unsigned char {
    Created = 0,
    Running = 1,
    Stopped = 2,
    Destroyed = 3,
};

struct VmConfig {
    std::uint32_t vcpus{1};
    std::uint64_t ram_bytes{512ULL * 1024 * 1024};
    bool enable_pv{true};
    hypervisor::Backend backend{hypervisor::Backend::VirtioMmio};
};

class Hypervisor {
public:
    [[nodiscard]] Result<VmHandle> create_vm(LineageTree& tree,
                                             CapabilityRights rights,
                                             VmConfig cfg = {}) noexcept {
        if (rights == CapabilityRights::None) {
            return {Status::Denied, VmHandle{}};
        }
        if (check_grant(CapabilityRights::All, rights) != Status::Ok) {
            return {Status::Denied, VmHandle{}};
        }
        if (cfg.vcpus == 0 || cfg.ram_bytes == 0) {
            return {Status::Invalid, VmHandle{}};
        }

        auto id = tree.create_root();
        if (id == kInvalidLineage) {
            return {Status::NoMemory, VmHandle{}};
        }

        placeholder_.state = VmState::Created;
        placeholder_.cfg = cfg;
        placeholder_.pv_count = 0;
        placeholder_.lineage = id;

        if (cfg.enable_pv) {
            std::uint32_t base = 0xF000'0000u;
            virtio::PvDevice devs[] = {
                virtio::pv_block(base),
                virtio::pv_net(base + 0x1000u),
                virtio::pv_console(base + 0x2000u),
            };
            for (auto const& d : devs) {
                if (virtio::attach_pv_stub(d) != Status::Ok) {
                    tree.revoke(id);
                    return {Status::Protocol, VmHandle{}};
                }
                ++placeholder_.pv_count;
            }
        }

        return {Status::Ok, VmHandle{&placeholder_, rights, id}};
    }

    [[nodiscard]] Status start_vm(VmHandle const& vm, LineageTree const& tree) noexcept {
        if (!vm.valid() || placeholder_.state == VmState::Destroyed) {
            return Status::Invalid;
        }
        if (!vm.is_alive(tree)) {
            return Status::Denied;
        }
        if (placeholder_.state == VmState::Running) {
            return Status::Busy;
        }
        placeholder_.state = VmState::Running;
        (void)vm;
        return Status::Ok;
    }

    [[nodiscard]] Status stop_vm(VmHandle const& vm, LineageTree const& tree) noexcept {
        if (!vm.valid() || placeholder_.state == VmState::Destroyed) {
            return Status::Invalid;
        }
        if (!vm.is_alive(tree)) {
            return Status::Denied;
        }
        placeholder_.state = VmState::Stopped;
        (void)vm;
        return Status::Ok;
    }

    [[nodiscard]] Status destroy_vm(VmHandle&& vm, LineageTree& tree) noexcept {
        if (!vm.valid()) {
            return Status::Invalid;
        }
        auto lid = vm.lineage();
        placeholder_.state = VmState::Destroyed;
        placeholder_.pv_count = 0;
        tree.revoke_subtree(lid);
        vm.reset();
        return Status::Ok;
    }

    [[nodiscard]] Status handle_exit(VmHandle const& vm, LineageTree const& tree,
                                     hypervisor::Backend b,
                                     unsigned exit_code) noexcept {
        if (!vm.valid() || !vm.is_alive(tree)) {
            return Status::Denied;
        }
        hypervisor::Amd64VmExit mapped{};
        hypervisor::ExitAction action{};
        if (hypervisor::dispatch_exit(b, exit_code, &mapped, &action) != Status::Ok) {
            return Status::Protocol;
        }
        if (action == hypervisor::ExitAction::Teardown) {
            return Status::Invalid;
        }
        (void)mapped;
        return Status::Ok;
    }

    [[nodiscard]] Status guest_mmio(VmHandle const& vm, LineageTree const& tree,
                                    std::uint32_t offset,
                                    std::uint32_t mmio_size) noexcept {
        if (!vm.valid() || !vm.has_right(CapabilityRights::Map)) {
            return Status::Denied;
        }
        if (!vm.is_alive(tree)) {
            return Status::Denied;
        }
        return virtio::validate_mmio_offset(offset, mmio_size);
    }

    [[nodiscard]] VmState state() const noexcept { return placeholder_.state; }

private:
    struct VmObjectEx : VmObject {
        VmState state{VmState::Created};
        VmConfig cfg{};
        unsigned pv_count{0};
        LineageId lineage{kInvalidLineage};
    };

    VmObjectEx placeholder_{};
};

} // namespace pbsd::bifrost
