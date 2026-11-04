// PBSD SI harness — Wave 1+ security invariants
import pbsd.handles;
import pbsd.uda.engine;
import pbsd.uda.virtio_blk;
import pbsd.uda.virtio_net;
import pbsd.uda.nvme;
import pbsd.uda.virtio_scsi;
import pbsd.uda.ahci;
import pbsd.uda.e1000;
import pbsd.uda.xhci;
import pbsd.uda.virtio_console;
import pbsd.uda.usbhid;
import pbsd.uda.virtio_gpu;
import pbsd.uda.virtio_random;
import pbsd.uda.virtio_balloon;
import pbsd.uda.nvme_admin;
import pbsd.uda.ixgbe;
import pbsd.uda.uhci;
import pbsd.uda.ehci;
import pbsd.uda.ns8250;
import pbsd.uda.gpio;
import pbsd.uda.acpi_button;
import pbsd.uda.igb;
import pbsd.uda.em;
import pbsd.uda.em_82574;
import pbsd.uda.re;
import pbsd.uda.igc;
import pbsd.uda.hda;
import pbsd.uda.smbus;
import pbsd.uda.ichwd;
import pbsd.uda.acpi_thermal;
import pbsd.uda.vtnet_mq;
import pbsd.uda.ahci_cam;
import pbsd.uda.nvme_io;
import pbsd.uda.igb_vf;
import pbsd.uda.uart_pci;
import pbsd.uda.acpi_video;
import pbsd.uda.cam.status;
import pbsd.uda.cam.ccb;
import pbsd.uda.cam.xpt;
import pbsd.uda.cam;
import pbsd.uda.virtio_scsi_cam;
import pbsd.uda.sdhci;
import pbsd.uda.mpt_cam;
import pbsd.uda.bge;
import pbsd.uda.ata_pci;
import pbsd.uda.msk;
import pbsd.uda.ixl;
import pbsd.uda.umass;
import pbsd.uda.da;
import pbsd.uda.ale;
import pbsd.uda.axe;
import pbsd.uda.fxp;
import pbsd.uda.vmx;
import pbsd.kernel.callout;
import pbsd.kernel.tty;
import pbsd.kernel.cons;
import pbsd.kernel.uipc;
import pbsd.kernel.sysvshm;
import pbsd.kernel.intr;
import pbsd.kernel.busdma;
import pbsd.kernel.sx;
import pbsd.kernel.rwlock;
import pbsd.kernel.condvar;
import pbsd.kernel.taskqueue;
import pbsd.kernel.uio;
import pbsd.kernel.lockf;
import pbsd.kernel.vm;
import pbsd.kernel.alloc;
import pbsd.kernel.sched;
import pbsd.kernel.capsicum;
import pbsd.kernel.prot;
import pbsd.kernel.pax_mac;
import pbsd.kernel.pax_aslr;
import pbsd.kernel.resource;
import pbsd.kernel.sysctl;
import pbsd.kernel.signal;
import pbsd.kernel.errno;
import pbsd.kernel.vm_page;
import pbsd.kernel.vm_map;
import pbsd.kernel.vm_object;
import pbsd.kernel.vm_fault;
import pbsd.kernel.uma;
import pbsd.kernel.vm_pager;
import pbsd.kernel.vm_phys;
import pbsd.kernel.param;
import pbsd.kernel.security_bsd;
import pbsd.kernel.syscall;
import pbsd.kernel.ucred;
import pbsd.kernel.jail;
import pbsd.kernel.timekeeping;
import pbsd.kernel.mutex;
import pbsd.kernel.turnstile;
import pbsd.kernel.ktrace;
import pbsd.kernel.syscall_args;
import pbsd.kernel.kevent;
import pbsd.kernel.poll;
import pbsd.kernel.socket_syscall;
import pbsd.kernel.vnode;
import pbsd.kernel.pipe;
import pbsd.kernel.fifo;
import pbsd.kernel.namei;
import pbsd.kernel.route;
import pbsd.kernel.ifnet;
import pbsd.kernel.bpf;
import pbsd.kernel.netgraph;
import pbsd.kernel.audit;
import pbsd.kernel.sockbuf;
import pbsd.kernel.domain;
import pbsd.kernel.protosw;
import pbsd.kernel.sockio;
import pbsd.kernel.rtentry;
import pbsd.kernel.filedesc;
import pbsd.kernel.selinfo;
import pbsd.kernel.select;
import pbsd.kernel.aio;
import pbsd.kernel.mbuf;
import pbsd.kernel.pcb;
import pbsd.kernel.inpcb;
import pbsd.kernel.socket_ops;
import pbsd.kernel.carp;
import pbsd.kernel.gif;
import pbsd.kernel.gre;
import pbsd.kernel.bridge;
import pbsd.kernel.vlan;
import pbsd.kernel.lagg;
import pbsd.kernel.if_media;
import pbsd.kernel.if_clone;
import pbsd.kernel.pfil;
import pbsd.kernel.random;
import pbsd.kernel.entropy;
import pbsd.kernel.crypto;
import pbsd.kernel.geom;
import pbsd.kernel.cam;
import pbsd.bifrost;
import pbsd.net;
import pbsd.fs;
import pbsd.geom;
import pbsd.zfs;
import pbsd.stand;
import pbsd.arch.amd64;
import pbsd.arch.arm64;
import pbsd.compositor;
import pbsd.pkg;
import pbsd.userland.echo;
import pbsd.userland.hostname;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.volume;
import pbsd.kde.kwin.effects.fade;
import pbsd.theme.plasma.aero.panel;
import pbsd.theme.plasma.aero.icons;

#include <cstdio>
#include <cstring>
#include <cstdint>

extern "C" {
#include "pbsd_kernel_dual_link.h"
}

namespace {

struct DummyObj {
    static int live;
    static void release(DummyObj* p) noexcept {
        if (p) {
            --live;
        }
    }
};
int DummyObj::live = 0;

int failures = 0;

void expect(bool cond, const char* msg) {
    if (!cond) {
        std::printf("FAIL: %s\n", msg);
        ++failures;
    }
}

void test_si1_rights_algebra() {
    using namespace pbsd;

    // Subset proofs (compile-time + runtime)
    expect(rights_subset(CapabilityRights::Read, CapabilityRights::All), "SI-1 read ⊆ all");
    expect(rights_subset(CapabilityRights::Read | CapabilityRights::Write,
                         CapabilityRights::All),
           "SI-1 rw ⊆ all");
    expect(!rights_subset(CapabilityRights::All, CapabilityRights::Read), "SI-1 all ⊈ read");
    expect(!rights_subset(CapabilityRights::Write, CapabilityRights::Read), "SI-1 write ⊈ read");

    // Grant / duplicate enforcement
    expect(check_grant(CapabilityRights::Read | CapabilityRights::Grant,
                       CapabilityRights::Read) == Status::Ok,
           "SI-1 grant ok");
    expect(check_grant(CapabilityRights::Read, CapabilityRights::Write) == Status::Denied,
           "SI-1 grant denied widen");
    expect(check_duplicate(CapabilityRights::Read | CapabilityRights::Duplicate,
                           CapabilityRights::Read) == Status::Ok,
           "SI-1 duplicate ok");
    expect(check_duplicate(CapabilityRights::Read, CapabilityRights::Read) == Status::Denied,
           "SI-1 duplicate without flag");

    // Algebra
    const auto u = rights_union(CapabilityRights::Read, CapabilityRights::Write);
    expect(has_right(u, CapabilityRights::Read) && has_right(u, CapabilityRights::Write),
           "SI-1 union");
    const auto n = narrow_rights(CapabilityRights::All, CapabilityRights::Read);
    expect(rights_equal(n, CapabilityRights::Read), "SI-1 narrow");
    expect(rights_empty(rights_diff(CapabilityRights::Read, CapabilityRights::Read)),
           "SI-1 diff empty");
}

void test_si6_lineage_revocation() {
    using namespace pbsd;
    using namespace pbsd::kernel::capsicum;

    LineageTree tree;
    const auto root = tree.create_root();
    const auto child = tree.create_child(root);
    const auto grand = tree.create_child(child);
    const auto sibling = tree.create_child(root);

    expect(tree.valid(root), "SI-6 root valid");
    expect(tree.valid(child), "SI-6 child valid");
    expect(tree.valid(grand), "SI-6 grand valid");
    expect(tree.is_ancestor_of(root, grand), "SI-6 root ancestor of grand");
    expect(tree.parent(grand) == child, "SI-6 parent link");
    expect(!tree.is_revoked(root), "SI-6 root not revoked");

    tree.revoke_subtree(child);
    expect(tree.is_revoked(child), "SI-6 child revoked");
    expect(tree.is_revoked(grand), "SI-6 grand revoked");
    expect(!tree.valid(child), "SI-6 child invalid");
    expect(!tree.valid(grand), "SI-6 grand invalid");
    expect(tree.valid(root), "SI-6 root still valid");
    expect(tree.valid(sibling), "SI-6 sibling unaffected");

    // Grant blocked on revoked lineage
    DummyObj obj{};
    DummyObj::live = 1;
    UniqueHandle<DummyObj> h{&obj, CapabilityRights::Read | CapabilityRights::Grant, child};
    auto g = h.grant(CapabilityRights::Read, tree);
    expect(g.status == Status::NoMemory, "SI-6 grant on revoked lineage fails");
    expect(!h.is_alive(tree), "SI-6 handle not alive after subtree revoke");
    h.reset();

    // Dual-link revoke + cap_rights_limit narrowing (kern_cap_rights_limit)
    const auto table = CapRightsTable::make();
    expect(cap_rights_limit(table.mmap_rwx, table.mmap_r) == Status::Ok,
           "SI-6 cap_rights_limit narrow ok");
    expect(cap_rights_limit(table.mmap_r, table.mmap_rwx) == Status::Denied,
           "SI-6 cap_rights_limit widen denied");
    expect(pbsd_kernel_revoke_lineage(static_cast<unsigned long long>(child)) ==
               PBSD_STATUS_OK,
           "SI-6 dual-link revoke_lineage");

    using namespace pbsd::kernel::resource;
    expect(cap_mode_priority_allowed(kPrioProcess, 0, 42, true) == Status::Ok,
           "SI-6 capmode priority self ok");
    expect(cap_mode_priority_allowed(kPrioPgrp, 0, 42, true) == Status::Denied,
           "SI-6 capmode priority pgrp denied");

    using namespace pbsd::kernel::security::aslr;
    const auto deltas = AslrDeltas::from_random(0xDEADBEEFull);
    expect(apply_mmap_delta(0x1000, 0, pbsd::kernel::vm::kMapPrivate, deltas.mmap, true) > 0x1000,
           "SI-6 aslr mmap delta applied");
}

void test_handles_and_vm() {
    using namespace pbsd;

    LineageTree tree;
    const auto root = tree.create_root();

    DummyObj obj{};
    DummyObj::live = 1;
    {
        UniqueHandle<DummyObj> h{&obj,
                                 CapabilityRights::Read | CapabilityRights::Write
                                     | CapabilityRights::Grant | CapabilityRights::Duplicate,
                                 root};
        auto b = h.borrow(CapabilityRights::Read);
        expect(b.valid() && b.has_right(CapabilityRights::Read), "borrow read");
        expect(!b.has_right(CapabilityRights::Write), "borrow stripped write");

        auto g = h.grant(CapabilityRights::Read, tree);
        expect(g.status == Status::Ok && g.value.valid(), "grant borrow handle");
        expect(g.value.is_alive(tree), "granted handle alive");

        auto d = h.duplicate(CapabilityRights::Read, tree);
        expect(d.status == Status::Ok && d.value.valid(), "duplicate borrow handle");

        unsigned ref = 0;
        auto s = share(util::move(h), ref);
        expect(s.status == Status::Ok && s.value.valid(), "share from unique");
        expect(ref == 1, "shared refcount");
        expect(s.value.has_right(CapabilityRights::Read), "shared retains read");
    }
    expect(DummyObj::live == 0, "unique/shared released");

    VmObject vm_obj{};
    VmHandle vm{&vm_obj, CapabilityRights::Map, root};
    expect(vm.valid() && vm.has_right(CapabilityRights::Map), "VmHandle valid");
    vm.reset();
}

void setup_virtio_mmio(pbsd::uda::SoftMmio& mem, std::uint32_t device_id) {
    using namespace pbsd::uda::virtio;
    mem.write32(kMmioMagicValue, kMmioMagicVirt);
    mem.write32(kMmioDeviceId, device_id);
    mem.write32(kMmioStatus, kFeaturesOk);
}

void test_si7_uda_capability() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::virtio;
    using namespace pbsd::uda::virtio::blk;

    SoftMmio mem{};
    setup_virtio_mmio(mem, kIdBlock);

    const auto desc = virtio_blk_1_0();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 virtio-blk descriptor valid");
    expect(virtio::blk::matches_pci(0x1AF4, 0x1001), "SI-7 virtio-blk pci id");
    expect(probe_mmio(mem), "SI-7 virtio-blk mmio probe");

    BlockEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe virtio-blk");
    expect(eng.start() == Status::Ok, "SI-7 start virtio-blk");
    expect(eng.submit_read(0, 8) == Status::Ok, "SI-7 block read submit");
    expect(eng.submit_write(64, 1) == Status::Ok, "SI-7 block write submit");
    expect(eng.pending_count() == 2, "SI-7 block queue depth");
    BlockRequest req{};
    expect(eng.complete_one(&req) == Status::Ok, "SI-7 block complete");
    expect(req.op == BlockOp::Read, "SI-7 block fifo order");
    expect(eng.stop() == Status::Ok, "SI-7 stop virtio-blk");

    constexpr RegInsn ok_seq[] = {
        {RegOp::Write32, kMmioStatus, kStatusAck, 0, 0},
        {RegOp::Done, 0, 0, 0, 0},
    };
    DeviceGrant grant{{0, SoftMmio::kSize}, {1}};
    BytecodeInterpreter in_bounds{grant, mem};
    expect(in_bounds.run(ok_seq) == Status::Ok, "SI-7 in-bounds write");

    constexpr RegInsn bad[] = {
        {RegOp::Write32, 0x10000, 1, 0, 0},
        {RegOp::Done, 0, 0, 0, 0},
    };
    MmioCapability tiny{0, 16};
    BytecodeInterpreter oob{tiny, mem};
    expect(oob.run(bad) == Status::Denied, "SI-7 deny oob write");

    constexpr RegInsn beyond_grant[] = {
        {RegOp::Write32, 0x10, 0x42, 0, 0},
        {RegOp::Done, 0, 0, 0, 0},
    };
    DeviceGrant narrow{{0, 8}, {}};
    BytecodeInterpreter narrow_interp{narrow, mem};
    expect(narrow_interp.run(beyond_grant) == Status::Denied, "SI-7 deny beyond grant");
}

void test_si7_virtio_net() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::virtio;
    using namespace pbsd::uda::virtio::net;

    SoftMmio mem{};
    setup_virtio_mmio(mem, kIdNetwork);

    const auto desc = virtio_net_1_0();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 virtio-net descriptor valid");
    expect(virtio::net::matches_pci(0x1AF4, 0x1000), "SI-7 virtio-net pci id");
    expect(probe_mmio(mem), "SI-7 virtio-net mmio probe");

    NetEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe virtio-net");
    expect(eng.start() == Status::Ok, "SI-7 start virtio-net");
    expect(eng.submit_tx({.length = 64}) == Status::Ok, "SI-7 net tx submit");
    expect(eng.tx_count() == 1, "SI-7 net tx count");
    expect(eng.stop() == Status::Ok, "SI-7 stop virtio-net");
}

void test_si7_nvme() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::nvme;

    SoftMmio mem{};
    mem.write32(kRegCap, 0x003FF);

    const auto desc = nvme_controller_1_4();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 nvme descriptor valid");
    expect(probe_mmio(mem), "SI-7 nvme mmio probe");

    BlockEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe nvme");
    expect(eng.start() == Status::Ok, "SI-7 start nvme");
    expect(eng.stop() == Status::Ok, "SI-7 stop nvme");
}

void test_si7_virtio_scsi() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::virtio;
    using namespace pbsd::uda::virtio::scsi;

    SoftMmio mem{};
    setup_virtio_mmio(mem, kIdScsi);

    const auto desc = virtio_scsi_1_0();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 virtio-scsi descriptor valid");
    expect(virtio::scsi::matches_pci(0x1AF4, 0x1004), "SI-7 virtio-scsi pci id");
    expect(probe_mmio(mem), "SI-7 virtio-scsi mmio probe");

    BlockEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe virtio-scsi");
    expect(eng.start() == Status::Ok, "SI-7 start virtio-scsi");
}

void test_si7_ahci() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::ahci;

    SoftMmio mem{};
    mem.write32(kRegCap, 0x00000005);
    mem.write32(kRegGhc, 0x0);

    const auto desc = ahci_sata_1_3();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 ahci descriptor valid");
    expect(probe_mmio(mem), "SI-7 ahci mmio probe");

    BlockEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe ahci");
    expect(eng.start() == Status::Ok, "SI-7 start ahci");
}

void test_si7_e1000() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::e1000;

    SoftMmio mem{};

    const auto desc = e1000_em_82540();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 e1000 descriptor valid");
    expect(matches_pci(0x8086, 0x100E), "SI-7 e1000 pci id");

    NetEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe e1000");
    expect(eng.start() == Status::Ok, "SI-7 start e1000");
}

void test_si7_xhci() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::xhci;

    SoftMmio mem{};
    mem.write32(kRegUsbsts, 0x0);
    mem.write32(kRegUsbcmd, 0x0);

    const auto desc = xhci_usb3_1_0();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 xhci descriptor valid");
    expect(probe_mmio(mem), "SI-7 xhci mmio probe");

    InputEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe xhci");
    expect(eng.start() == Status::Ok, "SI-7 start xhci");
}

void test_si7_virtio_console() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::virtio;
    using namespace pbsd::uda::virtio::console;

    SoftMmio mem{};
    setup_virtio_mmio(mem, kIdConsole);

    const auto desc = virtio_console_1_0();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 virtio-console descriptor valid");
    expect(virtio::console::matches_pci(0x1AF4, 0x1003), "SI-7 virtio-console pci id");

    InputEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe virtio-console");
    expect(eng.start() == Status::Ok, "SI-7 start virtio-console");
}

void test_si7_usbhid() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::usbhid;

    SoftMmio mem{};
    const auto desc = usb_hid_boot();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 usbhid descriptor valid");

    InputEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe usbhid");
    expect(eng.start() == Status::Ok, "SI-7 start usbhid");
    HidReport rep{};
    rep.length = 2;
    rep.bytes[0] = 0x01;
    expect(eng.submit_report(rep) == Status::Ok, "SI-7 hid report submit");
}

void test_si7_virtio_gpu() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::virtio;
    using namespace pbsd::uda::virtio::gpu;

    SoftMmio mem{};
    setup_virtio_mmio(mem, kIdGpu);

    const auto desc = virtio_gpu_1_0();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 virtio-gpu descriptor valid");
    expect(gpu::matches_pci(0x1AF4, 0x1050), "SI-7 virtio-gpu pci id");
    expect(probe_mmio(mem), "SI-7 virtio-gpu mmio probe");

    DisplayEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe virtio-gpu");
    expect(eng.start() == Status::Ok, "SI-7 start virtio-gpu");
    expect(eng.submit_scanout(0, 1, 1024, 768) == Status::Ok, "SI-7 gpu scanout");
    expect(eng.pending_count() == 1, "SI-7 gpu queue depth");
    DisplayRequest req{};
    expect(eng.complete_one(&req) == Status::Ok, "SI-7 gpu complete");
    expect(req.cmd == DisplayCmd::SetScanout, "SI-7 gpu scanout cmd");
}

void test_si7_virtio_random() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::virtio;
    using namespace pbsd::uda::virtio::random;

    SoftMmio mem{};
    setup_virtio_mmio(mem, kIdEntropy);

    const auto desc = virtio_random_1_0();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 virtio-random descriptor valid");
    expect(probe_mmio(mem), "SI-7 virtio-random mmio probe");

    SensorEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe virtio-random");
    expect(eng.start() == Status::Ok, "SI-7 start virtio-random");
    expect(eng.submit_entropy({.bytes = 32}) == Status::Ok, "SI-7 entropy submit");
    EntropyRequest req{};
    expect(eng.complete_entropy(&req) == Status::Ok, "SI-7 entropy complete");
    expect(req.bytes == 32, "SI-7 entropy bytes");
}

void test_si7_virtio_balloon() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::virtio;
    using namespace pbsd::uda::virtio::balloon;

    SoftMmio mem{};
    setup_virtio_mmio(mem, kIdBalloon);

    const auto desc = virtio_balloon_1_0();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 virtio-balloon descriptor valid");

    SensorEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe virtio-balloon");
    expect(eng.start() == Status::Ok, "SI-7 start virtio-balloon");
    expect(eng.submit_balloon_stat({.tag = 4, .val = 4096}) == Status::Ok,
           "SI-7 balloon stat submit");
}

void test_si7_nvme_admin() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::nvme;
    using namespace pbsd::uda::nvme::admin;

    SoftMmio mem{};
    mem.write32(kRegCap, 0x003FF);

    const auto desc = nvme_admin_1_4();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 nvme-admin descriptor valid");

    BlockEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe nvme-admin");
    expect(eng.start() == Status::Ok, "SI-7 start nvme-admin");
    expect(eng.submit_nvme_io({BlockOp::Read, 0, 0, 8}, admin::io_opc_read()) == Status::Ok,
           "SI-7 nvme io read path");
    std::uint8_t opc{};
    BlockRequest req{};
    expect(eng.complete_nvme_io(&req, &opc) == Status::Ok, "SI-7 nvme io complete");
    expect(opc == admin::io_opc_read(), "SI-7 nvme read opcode");
}

void test_si7_ixgbe() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::ixgbe;

    SoftMmio mem{};
    const auto desc = ixgbe_82599_sfp();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 ixgbe descriptor valid");
    expect(matches_pci(0x8086, 0x10FB), "SI-7 ixgbe pci id");

    NetEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe ixgbe");
    expect(eng.start() == Status::Ok, "SI-7 start ixgbe");
    expect(eng.submit_tx({.length = 1500}) == Status::Ok, "SI-7 ixgbe tx");
}

void test_si7_uhci() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::uhci;

    SoftMmio mem{};
    mem.write32(kRegSts, kStsHch);

    const auto desc = uhci_usb1_1();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 uhci descriptor valid");
    expect(probe_mmio(mem), "SI-7 uhci mmio probe");

    InputEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe uhci");
    expect(eng.start() == Status::Ok, "SI-7 start uhci");
}

void test_si7_ehci() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::ehci;

    SoftMmio mem{};
    mem.write32(kRegUsbsts, kStsHch);

    const auto desc = ehci_usb2_0();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 ehci descriptor valid");
    expect(pbsd::uda::ehci::probe_mmio(mem), "SI-7 ehci mmio probe");

    InputEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe ehci");
    expect(eng.start() == Status::Ok, "SI-7 start ehci");
}

void test_si7_ns8250() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::ns8250;

    SoftMmio mem{};
    const auto desc = ns8250_com1();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 ns8250 descriptor valid");

    InputEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe ns8250");
    expect(eng.start() == Status::Ok, "SI-7 start ns8250");
    expect(eng.submit_char('A') == Status::Ok, "SI-7 uart tx submit");
    expect(eng.inject_char('B') == Status::Ok, "SI-7 uart rx inject");
    std::uint8_t c{};
    expect(eng.poll_char(&c) == Status::Ok, "SI-7 uart rx poll");
    expect(c == 'B', "SI-7 uart rx byte");
}

void test_si7_gpio() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::gpio;

    SoftMmio mem{};
    mem.write32(kRegPadCfg0, 0x0);

    const auto desc = chv_gpio_1_0();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 gpio descriptor valid");
    expect(probe_mmio(mem), "SI-7 gpio mmio probe");

    SensorEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe gpio");
    expect(eng.start() == Status::Ok, "SI-7 start gpio");
    expect(eng.inject_event({SensorKind::Gpio, 0, 1}) == Status::Ok, "SI-7 gpio event");
}

void test_si7_acpi_button() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::acpi_button;

    SoftMmio mem{};
    const auto desc = acpi_power_button();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 acpi-button descriptor valid");

    SensorEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe acpi-button");
    expect(eng.start() == Status::Ok, "SI-7 start acpi-button");
    expect(eng.inject_event({SensorKind::Button, kNotifySleep, 0}) == Status::Ok,
           "SI-7 acpi button event");
    SensorEvent ev{};
    expect(eng.poll_event(&ev) == Status::Ok, "SI-7 acpi button poll");
    expect(ev.notify_code == kNotifySleep, "SI-7 acpi notify code");
}

void test_si7_igb() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::igb;

    SoftMmio mem{};
    const auto desc = igb_82575_eb_copper();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 igb descriptor valid");
    expect(matches_pci(0x8086, 0x10A7), "SI-7 igb pci id");

    NetEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe igb");
    expect(eng.start() == Status::Ok, "SI-7 start igb");
    expect(eng.set_link_up(true) == Status::Ok, "SI-7 igb link up");
    expect(eng.submit_tso({.length = 9000}, 1460) == Status::Ok, "SI-7 igb tso");
}

void test_si7_em() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::em;

    SoftMmio mem{};
    const auto desc = em_82571_eb_copper();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 em descriptor valid");
    expect(matches_pci(0x8086, 0x105E), "SI-7 em pci id");

    NetEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe em");
    expect(eng.start() == Status::Ok, "SI-7 start em");
    expect(eng.submit_tx({.length = 512}) == Status::Ok, "SI-7 em tx");
}

void test_si7_em_82574() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::em_82574;

    SoftMmio mem{};
    const auto desc = em_82574l();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 em-82574 descriptor valid");

    NetEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe em-82574");
    expect(eng.start() == Status::Ok, "SI-7 start em-82574");
}

void test_si7_re() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::re;

    SoftMmio mem{};
    const auto desc = re_rtl8168();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 re descriptor valid");
    expect(matches_pci(0x10EC, 0x8168), "SI-7 re pci id");

    NetEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe re");
    expect(eng.start() == Status::Ok, "SI-7 start re");
    expect(eng.note_rx_csum(true) == Status::Ok, "SI-7 re rx csum ok");
    expect(eng.rx_csum_ok() == 1, "SI-7 re csum counter");
}

void test_si7_igc() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::igc;

    SoftMmio mem{};
    const auto desc = igc_i225_lm();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 igc descriptor valid");
    expect(matches_pci(0x8086, 0x15F2), "SI-7 igc pci id");

    NetEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe igc");
    expect(eng.start() == Status::Ok, "SI-7 start igc");
}

void test_si7_hda() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::hda;

    SoftMmio mem{};
    mem.write32(0x00, 0x00000100);

    const auto desc = hda_intel_82801f();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 hda descriptor valid");
    expect(probe_mmio(mem), "SI-7 hda mmio probe");

    InputEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe hda");
    expect(eng.start() == Status::Ok, "SI-7 start hda");
    expect(eng.submit_hda_stream({1, 4096, 0x00040000}) == Status::Ok,
           "SI-7 hda stream submit");
}

void test_si7_smbus() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::smbus;

    SoftMmio mem{};
    const auto desc = ichsmb_82801aa();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 smbus descriptor valid");
    expect(probe_mmio(mem), "SI-7 smbus mmio probe");

    SensorEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe smbus");
    expect(eng.start() == Status::Ok, "SI-7 start smbus");
    expect(eng.submit_smbus(0x50, 0x00) == Status::Ok, "SI-7 smbus txn");
}

void test_si7_ichwd() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::ichwd;

    SoftMmio mem{};
    const auto desc = ichwd_ich7();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 ichwd descriptor valid");

    SensorEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe ichwd");
    expect(eng.start() == Status::Ok, "SI-7 start ichwd");
    expect(eng.pet_watchdog() == Status::Ok, "SI-7 ichwd pet");
    expect(eng.watchdog_pets() == 1, "SI-7 ichwd pet count");
}

void test_si7_acpi_thermal() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::acpi_thermal;

    SoftMmio mem{};
    const auto desc = acpi_thermal_zone();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 acpi-thermal descriptor valid");

    SensorEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe acpi-thermal");
    expect(eng.start() == Status::Ok, "SI-7 start acpi-thermal");
    expect(eng.submit_thermal(3000, kNotifyTemperature) == Status::Ok,
           "SI-7 acpi thermal sample");
    SensorEvent ev{};
    expect(eng.poll_event(&ev) == Status::Ok, "SI-7 acpi thermal poll");
    expect(ev.kind == SensorKind::Thermal, "SI-7 thermal kind");
}

void test_si7_vtnet_mq() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::virtio;
    using namespace pbsd::uda::vtnet_mq;

    SoftMmio mem{};
    mem.write32(kMmioMagicValue, kMmioMagicVirt);
    mem.write32(kMmioDeviceId, kIdNetwork);
    mem.write32(kMmioStatus, kFeaturesOk);

    const auto desc = vtnet_mq_modern();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 vtnet-mq descriptor valid");
    expect(probe_mmio(mem), "SI-7 vtnet-mq mmio probe");

    NetEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe vtnet-mq");
    expect(eng.start() == Status::Ok, "SI-7 start vtnet-mq");
    expect(eng.submit_tx({.length = 1500}) == Status::Ok, "SI-7 vtnet-mq tx");
}

void test_si7_ahci_cam() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::ahci_cam;

    SoftMmio mem{};
    const auto desc = ahci_cam_port0();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 ahci-cam descriptor valid");
    expect(port_mmio_base(0) == 0x100, "SI-7 ahci-cam port base");

    BlockEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe ahci-cam");
    expect(eng.start() == Status::Ok, "SI-7 start ahci-cam");
}

void test_si7_nvme_io() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::nvme_io;

    SoftMmio mem{};
    const auto desc = nvme_io_qpair();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 nvme-io descriptor valid");
    expect(validate_io_trackers(kIoTrackers) == Status::Ok, "SI-7 nvme-io trackers");
    expect(nvme_io::io_opc_read() == 0x02, "SI-7 nvme-io read opc");

    BlockEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe nvme-io");
    expect(eng.start() == Status::Ok, "SI-7 start nvme-io");
}

void test_si7_igb_vf() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::igb_vf;

    SoftMmio mem{};
    const auto desc = igb_82576_vf();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 igb-vf descriptor valid");
    expect(matches_pci(0x8086, 0x10CA), "SI-7 igb-vf pci id");

    NetEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe igb-vf");
    expect(eng.start() == Status::Ok, "SI-7 start igb-vf");
}

void test_si7_uart_pci() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::uart_pci;

    SoftMmio mem{};
    mem.write32(5, 0x60);
    const auto desc = uart_pci_intel_sol();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 uart-pci descriptor valid");
    expect(matches_pci(0x8086, 0x108F), "SI-7 uart-pci sol id");

    InputEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe uart-pci");
    expect(eng.start() == Status::Ok, "SI-7 start uart-pci");
}

void test_si7_acpi_video() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::acpi_video;

    SoftMmio mem{};
    const auto desc = acpi_video_backlight();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 acpi-video descriptor valid");
    expect(is_brightness_notify(kNotifyIncBrn), "SI-7 acpi-video brn notify");
    expect(clamp_brightness(150) == kBrnMax, "SI-7 acpi-video clamp");

    DisplayEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe acpi-video");
    expect(eng.start() == Status::Ok, "SI-7 start acpi-video");
}

void test_si7_cam_integration() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::cam;
    using namespace pbsd::uda::cam::status;
    using namespace pbsd::uda::cam::ccb;
    using namespace pbsd::uda::cam::xpt;
    using namespace pbsd::uda::virtio_scsi_cam;

    expect(status_table_size() >= 10, "SI-7 cam status table");
    expect(is_success(kReqCmp), "SI-7 cam req cmp");
    expect(to_pbsd_status(kBusy) == Status::Busy, "SI-7 cam busy map");
    expect(cam_module_table_size() >= 14, "SI-7 cam aggregate table");

    ScsiIoStub io{};
    io.hdr.func_code = kScsiIo;
    io.cdb_len = 6;
    expect(validate_scsi_io(io) == Status::Ok, "SI-7 cam scsi io");

    CamSession session{};
    expect(start_session(session) == Status::Ok, "SI-7 cam session start");
    expect(session_ready(session), "SI-7 cam session ready");
    expect(submit_scsi_io(session, io) == Status::Ok, "SI-7 cam session submit");
    expect(complete_session_io(session, kReqCmp) == Status::Ok, "SI-7 cam session complete");

    PeriphStub periph{};
    periph.softc = reinterpret_cast<void*>(1);
    CcbHeaderStub hdr{.func_code = kScsiIo};
    expect(register_periph(periph) == Status::Ok, "SI-7 cam register");
    expect(submit_ccb(periph, hdr) == Status::Ok, "SI-7 cam submit");
    expect(complete_ccb(hdr, kReqCmp) == Status::Ok, "SI-7 cam complete");

    SoftMmio mem{};
    const auto desc = pbsd::uda::virtio_scsi_cam::virtio_scsi_cam();
    expect(validate_descriptor(desc) == Status::Ok, "SI-7 virtio-scsi-cam valid");
    expect(matches_virtio_scsi(0x1048), "SI-7 virtio-scsi id");

    BlockEngine eng{mem};
    expect(eng.probe(desc) == Status::Ok, "SI-7 probe virtio-scsi-cam");
    expect(eng.start() == Status::Ok, "SI-7 start virtio-scsi-cam");
    expect(eng.submit_cam_io({BlockOp::Read, 0, 0, 1}, kReqCmp) == Status::Ok,
           "SI-7 cam block submit");

    expect(abort_session_io(session) == Status::Ok, "SI-7 cam session abort");
    expect(rescan_session(session) == Status::Ok, "SI-7 cam session rescan");
    expect(abort_ccb(periph, hdr) == Status::Ok, "SI-7 cam abort ccb");
}

void test_si7_new_descriptors() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::sdhci;
    using namespace pbsd::uda::mpt_cam;
    using namespace pbsd::uda::bge;
    using namespace pbsd::uda::ata_pci;
    using namespace pbsd::uda::msk;

    SoftMmio mem{};

    const auto sd = sdhci_pci_host();
    expect(validate_descriptor(sd) == Status::Ok, "SI-7 sdhci descriptor");
    BlockEngine seng{mem};
    expect(seng.probe(sd) == Status::Ok, "SI-7 sdhci probe");
    mem.write32(kRegPresentState, kPresentCardStable);
    expect(probe_present(mem), "SI-7 sdhci present");

    const auto md = mpt_lsi_cam();
    expect(validate_descriptor(md) == Status::Ok, "SI-7 mpt-cam descriptor");
    expect(pbsd::uda::mpt_cam::matches_pci(0x1000, 0x0030), "SI-7 mpt pci id");
    mem.write32(kOffsetDoorbell, kDbStateReady);
    expect(probe_doorbell(mem), "SI-7 mpt doorbell");

    const auto bd = bge_5700_copper();
    expect(validate_descriptor(bd) == Status::Ok, "SI-7 bge descriptor");
    NetEngine neng{mem};
    expect(neng.probe(bd) == Status::Ok, "SI-7 bge probe");
    mem.write32(kRegMacMode, kMacModeEnable);
    expect(pbsd::uda::bge::probe_mmio(mem), "SI-7 bge mmio");

    const auto ad = ata_pci_channel0();
    expect(validate_descriptor(ad) == Status::Ok, "SI-7 ata-pci descriptor");
    BlockEngine aeng{mem};
    expect(aeng.probe(ad) == Status::Ok, "SI-7 ata-pci probe");
    mem.write32(kRegStatus, kStatusDrq);
    expect(probe_status(mem), "SI-7 ata-pci status");

    const auto yd = msk_yukon2_8021cu();
    expect(validate_descriptor(yd) == Status::Ok, "SI-7 msk descriptor");
    expect(neng.probe(yd) == Status::Ok, "SI-7 msk probe");
    mem.write32(kRegCtst, kCtstRun);
    expect(pbsd::uda::msk::probe_mmio(mem), "SI-7 msk mmio");
}

void test_si7_burst9_descriptors() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::ixl;
    using namespace pbsd::uda::umass;
    using namespace pbsd::uda::da;
    using namespace pbsd::uda::ale;
    using namespace pbsd::uda::axe;
    using namespace pbsd::uda::fxp;
    using namespace pbsd::uda::vmx;

    const auto xl = ixl_x710_sfp();
    expect(validate_descriptor(xl) == Status::Ok, "SI-7 ixl descriptor");
    expect(xl.vendor_id == 0x8086, "SI-7 ixl vendor");

    const auto um = umass_scsi();
    expect(validate_descriptor(um) == Status::Ok, "SI-7 umass descriptor");

    const auto dd = da_direct();
    expect(validate_descriptor(dd) == Status::Ok, "SI-7 da descriptor");
    expect(valid_sector(kSector512), "SI-7 da sector512");

    const auto al = ale_ar81xx();
    expect(validate_descriptor(al) == Status::Ok, "SI-7 ale descriptor");
    SoftMmio mem{};
    mem.write32(kRegMacCfg, kMacCfgTxEnb | kMacCfgRxEnb);
    expect(probe_mmio(mem), "SI-7 ale mmio probe");

    const auto ax = axe_ax88772();
    expect(validate_descriptor(ax) == Status::Ok, "SI-7 axe descriptor");

    const auto fx = fxp_82559();
    expect(validate_descriptor(fx) == Status::Ok, "SI-7 fxp descriptor");

    const auto vm = vmxnet3();
    expect(validate_descriptor(vm) == Status::Ok, "SI-7 vmx descriptor");
    expect(vm.vendor_id == 0x15AD, "SI-7 vmx vendor");
}

void test_wave5_kernel_ipc() {
    using namespace pbsd;
    using namespace pbsd::kernel::callout;
    using namespace pbsd::kernel::tty;
    using namespace pbsd::kernel::cons;
    using namespace pbsd::kernel::uipc;
    using namespace pbsd::kernel::sysvshm;
    using namespace pbsd::kernel::intr;
    using namespace pbsd::kernel::busdma;
    using namespace pbsd::kernel::sx;
    using namespace pbsd::kernel::rwlock;
    using namespace pbsd::kernel::condvar;
    using namespace pbsd::kernel::taskqueue;
    using namespace pbsd::kernel::uio;
    using namespace pbsd::kernel::lockf;

    expect(validate_ticks(100) == Status::Ok, "wave5 callout ticks");
    expect(can_schedule(CalloutState::Idle), "wave5 callout idle");
    expect(is_expired(200, 100), "wave5 callout expired");

    TermiosStub term{};
    term.lflag = kLflagIcanon | kLflagEcho;
    expect(canonical(term), "wave5 tty canonical");
    expect(validate_termios(term) == Status::Ok, "wave5 tty valid");

    ConsoleState cs{};
    cs.flags = enable(kConsFlagsMute);
    expect(is_muted(cs.flags), "wave5 cons muted");
    expect(validate_poll_hz(kConsPollhz) == Status::Ok, "wave5 cons poll");

    expect(validate_sun_path_len(14) == Status::Ok, "wave5 uipc path");

    expect(validate_shm_size(4096) == Status::Ok, "wave5 sysv shm size");

    expect(validate_irq(42) == Status::Ok, "wave5 intr irq");
    expect(is_msi(IntrType::MsiX), "wave5 intr msix");

    BusDmaTag tag{.max_size = 65536, .max_segs = 16};
    expect(validate_tag(tag) == Status::Ok, "wave5 busdma tag");

    expect(is_shared(kLockShared), "wave5 sx shared");
    expect(validate_sx_word(0) == Status::Ok, "wave5 sx word");

    expect(readers(kUnlocked) == 0, "wave5 rwlock unlocked");
    expect(validate_rwlock_word(kUnlocked) == Status::Ok, "wave5 rwlock word");

    CondvarStub cv{};
    expect(wait_enqueue(cv) == Status::Ok, "wave5 cv wait");
    expect(signal(cv) == Status::Ok, "wave5 cv signal");

    TaskqueueStub tq{};
    expect(enqueue(tq) == Status::Ok, "wave5 taskqueue enqueue");

    UioStub uio{.iovcnt = 4, .resid = 512};
    expect(validate_iovcnt(uio.iovcnt) == Status::Ok, "wave5 uio iovcnt");
    expect(is_read(uio.rw), "wave5 uio read");

    LockfEntry lf{.type = kFWrLck, .start = 0, .end = 1024};
    expect(validate_entry(lf) == Status::Ok, "wave5 lockf entry");
    expect(is_write_lock(lf.type), "wave5 lockf write");
}

void test_si7_engine_deepening() {
    using namespace pbsd;
    using namespace pbsd::uda;
    using namespace pbsd::uda::virtio::net;
    using namespace pbsd::uda::virtio::gpu;
    using namespace pbsd::uda::virtio::random;
    using namespace pbsd::uda::ns8250;

    SoftMmio mem{};
    NetEngine net{mem};
    const auto nd = virtio_net_1_0();
    expect(net.probe(nd) == Status::Ok, "SI-7 net probe");
    expect(net.start() == Status::Ok, "SI-7 net start");
    expect(net.join_multicast(0x01) == Status::Ok, "SI-7 net mcast");
    expect(net.bpf_tap(64) == Status::Ok, "SI-7 net bpf tap");

    DisplayEngine disp{mem};
    const auto gd = virtio_gpu_1_0();
    expect(disp.probe(gd) == Status::Ok, "SI-7 display probe");
    expect(disp.start() == Status::Ok, "SI-7 display start");
    expect(disp.submit_create2d(1, 640, 480) == Status::Ok, "SI-7 display create2d");

    InputEngine inp{mem};
    const auto ud = ns8250_com1();
    expect(inp.probe(ud) == Status::Ok, "SI-7 input probe");
    expect(inp.start() == Status::Ok, "SI-7 input start");
    HidReport reps[2] = {{.bytes = {1, 2}, .length = 2}, {.bytes = {3}, .length = 1}};
    expect(inp.submit_report_batch(reps, 2) == Status::Ok, "SI-7 input batch");

    SensorEngine sen{mem};
    const auto sd = virtio_random_1_0();
    expect(sen.probe(sd) == Status::Ok, "SI-7 sensor probe");
    expect(sen.start() == Status::Ok, "SI-7 sensor start");
    expect(sen.submit_gpio_edge(4, true) == Status::Ok, "SI-7 sensor gpio edge");

    mem.write32(pbsd::uda::em::kRegStatus, 0x2);
    expect(pbsd::uda::em::probe_mmio(mem), "SI-7 em mmio probe");
    mem.write32(pbsd::uda::re::kRegCfg1, 0x20);
    expect(pbsd::uda::re::probe_mmio(mem), "SI-7 re mmio probe");
    mem.write32(pbsd::uda::igc::kRegStatus, 0x2);
    expect(pbsd::uda::igc::probe_mmio(mem), "SI-7 igc mmio probe");
    mem.write32(pbsd::uda::e1000::kRegStatus, 0x2);
    expect(pbsd::uda::e1000::probe_mmio(mem), "SI-7 e1000 mmio probe");
    mem.write32(pbsd::uda::igb::kRegStatus, 0x2);
    expect(pbsd::uda::igb::probe_mmio(mem), "SI-7 igb mmio probe");
    mem.write32(pbsd::uda::ixgbe::kRegStatus, 0x2);
    expect(pbsd::uda::ixgbe::probe_mmio(mem), "SI-7 ixgbe mmio probe");
    mem.write32(pbsd::uda::ahci_cam::kPort0Base + pbsd::uda::ahci_cam::kPSsts, 0x3);
    expect(pbsd::uda::ahci_cam::probe_port(mem, 0), "SI-7 ahci-cam port probe");
    mem.write32(pbsd::uda::nvme_io::kRegCsts, pbsd::uda::nvme_io::kCstsRdyMask);
    expect(pbsd::uda::nvme_io::probe_qpair(mem), "SI-7 nvme-io qpair probe");

    BlockEngine blk{mem};
    expect(blk.probe(pbsd::uda::mpt_cam::mpt_lsi_cam()) == Status::Ok, "SI-7 mpt block probe");
    mem.write32(pbsd::uda::mpt_cam::kOffsetDoorbell, 0);
    mem.write32(pbsd::uda::mpt_cam::kOffsetIntrStatus, 0);
    expect(blk.start() == Status::Ok, "SI-7 mpt block start");
    expect(blk.submit_mpt_io({BlockOp::Read, 0, 0, 1}, 0x20000000) == Status::Ok,
           "SI-7 mpt io submit");

    NetRxDesc rxbatch[2] = {{.length = 64}, {.length = 128}};
    expect(net.submit_rx_batch(rxbatch, 2) == Status::Ok, "SI-7 net rx batch");
    NetTxDesc txdone{};
    expect(net.submit_tx({.length = 512}) == Status::Ok, "SI-7 net tx for complete");
    expect(net.complete_tx(&txdone) == Status::Ok, "SI-7 net tx complete");
}

void test_si2_no_ambient_echo() {
    using namespace pbsd::userland::bin::echo;

    char a1[] = "-n";
    char a2[] = "hi";
    char* argv[] = {a1, a2, nullptr};
    auto opt = parse_options(argv);
    expect(opt.has_value() && opt.value.no_newline, "SI-2 echo -n");
    auto* rest = args_after_options(argv, opt.value);
    expect(rest != nullptr && rest[0] != nullptr && std::strcmp(rest[0], "hi") == 0,
           "SI-2 echo args after -n");

    // Explicit fd required — negative stdout is Invalid (no ambient authority)
    auto bad = write_args(-1, rest, opt.value);
    expect(bad.status == pbsd::Status::Invalid, "SI-2 deny bad fd");
}

void test_si3_wx() {
    using namespace pbsd;
    using namespace pbsd::kernel::vm;
    using namespace pbsd::kernel::capsicum;
    using namespace pbsd::kernel::security;

    expect(validate_wx(MapProtection::Write | MapProtection::Execute) == Status::Denied,
           "SI-3 deny W^X");
    expect(validate_wx(MapProtection::Read | MapProtection::Execute) == Status::Ok,
           "SI-3 allow RX");
    expect(validate_wx(MapProtection::Read | MapProtection::Write) == Status::Ok,
           "SI-3 allow RW");

    const auto table = CapRightsTable::make();
    expect(cap_rights_to_vmprot(table.mmap_rx) == (kVmProtRead | kVmProtExecute),
           "SI-3 cap mmap_rx vmprot");
    expect(cap_rights_to_vmprot(table.mmap_wx) == (kVmProtWrite | kVmProtExecute),
           "SI-3 cap mmap_wx vmprot");
    expect(validate_wx_vm(cap_rights_to_vmprot(table.mmap_wx)) == Status::Denied,
           "SI-3 cap wx denied");

    expect(mprotect_apply(kVmProtRead, kVmProtRead | kVmProtWrite | kVmProtExecute) ==
               (kVmProtRead | kVmProtWrite),
           "SI-3 pax mprotect strips X");
    expect(pageexec_apply_prot(kVmProtRead | kVmProtWrite | kVmProtExecute) ==
               (kVmProtRead | kVmProtWrite),
           "SI-3 pax pageexec strips X");

    MmapRequest bad_stack{.prot = kProtRead, .flags = kMapStack, .fd = -1, .length = 4096};
    expect(validate_mmap_flags(bad_stack) == Status::Invalid, "SI-3 MAP_STACK needs RW");
    MmapRequest ok_anon{.prot = kProtRead | kProtWrite,
                        .flags = kMapPrivate | kMapAnon,
                        .fd = -1,
                        .length = 4096};
    expect(validate_mmap_flags(ok_anon) == Status::Ok, "SI-3 MAP_ANON ok");

    LineageTree tree;
    auto root = tree.create_root();
    VmObject obj{};
    VmHandle h{&obj, CapabilityRights::Map, root};
    VmMapper mapper;
    auto denied = mapper.map(h, MapProtection::Write | MapProtection::Execute, 0x1000, 4096);
    expect(denied.status == Status::Denied, "SI-3 mapper rejects WX");
    auto okm = mapper.map(h, MapProtection::Read | MapProtection::Write, 0x1000, 4096);
    expect(okm.status == Status::Ok, "SI-3 mapper allows RW");
    h.reset();
}

void test_si4_zeroed_alloc() {
    using namespace pbsd::kernel::alloc;

    auto* p = static_cast<unsigned char*>(kmem_zalloc(32));
    expect(p != nullptr, "SI-4 alloc");
    bool all_zero = true;
    for (int i = 0; i < 32; ++i) {
        if (p[i] != 0) {
            all_zero = false;
        }
    }
    expect(all_zero, "SI-4 memory zeroed");
    kmem_free(p);

    struct Pod { int a; int b; };
    Pod* pod = ZeroedAlloc<Pod>::allocate();
    expect(pod != nullptr && pod->a == 0 && pod->b == 0, "SI-4 ZeroedAlloc POD");
    ZeroedAlloc<Pod>::deallocate(pod);
}

void test_si5_no_blocking_in_isr() {
    using namespace pbsd;
    using namespace pbsd::kernel::sched;

    LineageTree tree;
    const auto root = tree.create_root();
    ThreadObject tobj{};
    ThreadHandle th{&tobj, CapabilityRights::Read, root};
    DefaultSchedHook hook;
    SchedParams params{};

    expect(hook.on_enqueue(th, params) == Status::Ok, "SI-5 enqueue in thread ctx");

    {
        IsrScopeGuard guard{};
        expect(require_thread_context() == Status::Denied, "SI-5 ISR context flagged");
        expect(hook.on_enqueue(th, params) == Status::Denied, "SI-5 enqueue denied in ISR");
        expect(hook.on_yield(th) == Status::Denied, "SI-5 yield denied in ISR");
    }

    expect(require_thread_context() == Status::Ok, "SI-5 thread context restored");
    expect(hook.on_yield(th) == Status::Ok, "SI-5 yield ok after ISR");
}

void test_si8_vmhandle_isolation() {
    using namespace pbsd;
    using namespace pbsd::bifrost;
    using namespace pbsd::bifrost::hypervisor;
    using namespace pbsd::bifrost::virtio;

    LineageTree tree;
    Hypervisor hv;

    auto denied = hv.create_vm(tree, CapabilityRights::None);
    expect(denied.status == Status::Denied, "SI-8 deny create without rights");

    auto vm = hv.create_vm(tree, CapabilityRights::All);
    expect(vm.status == Status::Ok && vm.value.valid(), "SI-8 create_vm with All");
    expect(vm.value.has_right(CapabilityRights::Map), "SI-8 VmHandle carries Map");

    expect(hv.guest_mmio(vm.value, tree, 0x08, 0x1000) == Status::Ok, "SI-8 in-bounds mmio");
    expect(hv.guest_mmio(vm.value, tree, 0x2000, 0x1000) == Status::Denied,
           "SI-8 oob mmio denied");

    expect(hv.handle_exit(vm.value, tree, Backend::Vmx,
                          static_cast<unsigned>(VmxExit::Cpuid)) == Status::Ok,
           "SI-8 vmx exit mapped");
    expect(hv.handle_exit(vm.value, tree, Backend::Svm,
                          static_cast<unsigned>(VmcbExit::Io)) == Status::Ok,
           "SI-8 svm exit mapped");

    expect(hv.start_vm(vm.value, tree) == Status::Ok, "SI-8 start");
    expect(hv.stop_vm(vm.value, tree) == Status::Ok, "SI-8 stop");

    Amd64VmExit mapped{};
    ExitAction action{};
    expect(dispatch_exit(Backend::Vmx, static_cast<unsigned>(VmxExit::Rdmsr),
                         &mapped, &action) == Status::Ok,
           "SI-8 vmx rdmsr dispatch");
    expect(mapped == Amd64VmExit::Rdmsr, "SI-8 vmx rdmsr mapped");
    expect(hv.handle_exit(vm.value, tree, Backend::Vmx,
                          static_cast<unsigned>(VmxExit::Hlt)) == Status::Ok,
           "SI-8 handle hlt exit");
    expect(exit_table_size() >= 12, "SI-8 exit table expanded");

    auto lid = vm.value.lineage();
    expect(hv.destroy_vm(util::move(vm.value), tree) == Status::Ok, "SI-8 destroy");
    expect(tree.is_revoked(lid), "SI-8 lineage revoked on destroy");
    expect(!tree.valid(lid), "SI-8 lineage invalid after destroy");
    expect(hv.state() == VmState::Destroyed, "SI-8 vm destroyed state");

    expect(validate_mmio_offset(0, 0) == Status::Invalid, "SI-8 zero mmio rejected");
    expect(attach_pv_stub(pv_block(0)) == Status::Invalid, "SI-8 zero base pv rejected");
}

void test_wave4_kernel_tables() {
    using namespace pbsd;
    using namespace pbsd::kernel::capsicum;
    using namespace pbsd::kernel::resource;
    using namespace pbsd::kernel::sysctl;
    using namespace pbsd::kernel::signal;
    using namespace pbsd::kernel::errno_ns;
    using namespace pbsd::kernel::vm_page;
    using namespace pbsd::kernel::vm_map;
    using namespace pbsd::kernel::vm_object;
    using namespace pbsd::kernel::vm_fault;
    using namespace pbsd::kernel::uma;
    using namespace pbsd::kernel::vm_pager;
    using namespace pbsd::kernel::vm_phys;
    using namespace pbsd::kernel::param;
    using namespace pbsd::kernel::security_bsd;
    using namespace pbsd::kernel::syscall;
    using namespace pbsd::kernel::jail;
    using namespace pbsd::kernel::timekeeping;
    using namespace pbsd::kernel::mutex;
    using namespace pbsd::kernel::turnstile;
    using namespace pbsd::kernel::ktrace;
    using namespace pbsd::kernel::vm;
    using namespace pbsd::kernel::syscall_args;
    using namespace pbsd::kernel::kevent;
    using namespace pbsd::kernel::poll;
    using namespace pbsd::kernel::socket_syscall;
    using namespace pbsd::kernel::vnode;
    using namespace pbsd::kernel::pipe;
    using namespace pbsd::kernel::fifo;
    using namespace pbsd::kernel::namei;
    using namespace pbsd::kernel::filedesc;
    using namespace pbsd::kernel::selinfo;

    const auto cap_table = CapRightsTable::make();
    expect(CapRightsTable::table_entry_count() >= 18, "wave4 cap rights table");
    expect(cap_rights_is_valid(cap_table.mmap_r), "wave4 cap mmap_r valid");

    const auto hardened = pbsd::kernel::prot::hardened_visibility_policy();
    pbsd::kernel::prot::Ucred viewer{.ruid = 0, .jail_id = 1};
    pbsd::kernel::prot::Ucred other{.ruid = 1, .jail_id = 1};
    expect(pbsd::kernel::prot::can_see_other_uids(viewer, other, hardened) == Status::Denied,
           "wave4 hardened see_other_uids");

    expect(validate_rlimit_id(kRlimitNofile) == Status::Ok, "wave4 rlimit id");
    expect(rlimit_table_size() >= 16, "wave4 rlimit table");
    expect(rlimit_name(kRlimitStack).has_value(), "wave4 rlimit name");

    expect(top_level_table_size() >= 9, "wave4 sysctl top table");
    expect(top_level_number("kern").has_value(), "wave4 sysctl kern oid");
    expect(kern_table_size() >= 8, "wave4 sysctl kern table");
    expect(vm_table_size() >= 5, "wave4 sysctl vm table");
    expect(kern_number("maxproc").has_value(), "wave4 sysctl kern maxproc");
    expect(cap_mode_access(kCtlFlagCapRd, false, true) == Status::Ok,
           "wave4 sysctl cap read");

    expect(syscall_table_size() >= 10, "wave4 syscall table");
    expect(syscall_number("jail_get").has_value(), "wave4 syscall jail_get");
    expect(is_jail_syscall(kSysJailSet), "wave4 jail syscall class");
    expect(is_time_syscall(kSysClockGettime), "wave4 time syscall class");

    pbsd::kernel::ucred::Ucred cred{};
    cred.rgid = 10;
    cred.groups[0] = 20;
    cred.ngroups = 1;
    expect(pbsd::kernel::ucred::groupmember(20, cred), "wave4 ucred groupmember");
    expect(pbsd::kernel::ucred::validate_ngroups(16) == Status::Ok, "wave4 ucred ngroups");

    expect(validate_jail_id(1) == Status::Ok, "wave4 jail id");
    expect(flag_create(kJailCreate), "wave4 jail create flag");
    expect(pbsd::kernel::jail::flag_table_size() >= 4, "wave4 jail flag table");

    expect(validate_clock_id(kClockMonotonic) == Status::Ok, "wave4 clock id");
    expect(clock_table_size() >= 6, "wave4 clock table");
    expect(normalize_year(69) == 2069, "wave4 clock year norm");

    MutexClass mtx{};
    expect(validate_init_flags(kMtxDef) == Status::Ok, "wave4 mtx init flags");
    expect(try_lock(mtx, 1) == Status::Ok, "wave4 mtx trylock");
    expect(unlock(mtx, 1) == Status::Ok, "wave4 mtx unlock");

    TurnstileStub ts{};
    expect(chain_lock(ts, 1) == Status::Ok, "wave4 turnstile chain lock");
    expect(wait(ts, kTsExclusiveQueue) == Status::Ok, "wave4 turnstile wait");
    expect(signal(ts, kTsExclusiveQueue), "wave4 turnstile signal");

    expect(ktrcheck(enable_facility(0, kKtrSyscall), kKtrSyscall), "wave4 ktrace enable");
    expect(pbsd::kernel::ktrace::type_table_size() >= 8, "wave4 ktrace type table");
    expect(ktrop(kKtropSet) == 0, "wave4 ktrace ktrop");

    const auto sig = str2sig("SIGTERM");
    expect(sig.has_value() && sig.value == kSigTerm, "wave4 str2sig");
    expect(sig_table_size() >= 31, "wave4 signal table");

    const auto ecap = from_errno_int(94);
    expect(ecap.has_value() && is_capability_error(ecap.value), "wave4 ECAPMODE");
    expect(errno_table_size() >= 10, "wave4 errno table");
    expect(to_status(Errno::CapMode) == Status::Denied, "wave4 errno status");

    expect(is_active_queue(kPqActive), "wave4 vm_page pq active");
    expect(evaluate_free_target(PageCounts{.inactive = 100}, 50, 10) == Status::Ok,
           "wave4 vm_page free target");

    MapEntry entry{.ef = kMapEntryGuard, .cur_prot = kVmProtNone};
    expect(protect_entry(entry, kProtRead) == Status::Denied, "wave4 guard protect");
    expect(validate_stack_prot(kProtRead | kProtWrite) == Status::Ok, "wave4 stack prot");

    expect(is_anonymous(kObjAnon), "wave4 vm_object anon");
    expect(validate_truncate_size(4096) == Status::Ok, "wave4 truncate align");
    expect(validate_fault_prot(kVmProtRead | kVmProtExecute) == Status::Ok,
           "wave4 fault rx");

    expect(zone_zeroes(kZoneZinit), "wave4 uma zinit");
    expect(align_item_size(5) == 8, "wave4 uma align");
    expect(pager_table_size() >= 6, "wave4 pager table");
    expect(pager_ok(kPagerOk), "wave4 pager ok");

    PhysSeg seg{.start = 0x1000, .end = 0x2000};
    expect(validate_phys_range(seg) == Status::Ok, "wave4 phys seg");
    expect(free_order_for_pages(4) >= 2, "wave4 phys order");

    const auto tun = init_param2(64);
    expect(tun.maxproc == nproc_from_maxusers(64), "wave4 param maxproc");
    expect(tick_usec(kDefaultHz) == 1000, "wave4 param tick");

    SecurityBsdSysctl sb = hardened_defaults();
    expect(!sb.see_other_uids, "wave4 security_bsd hardened");
    auto pol = to_visibility_policy(sb);
    expect(!pol.see_other_gids, "wave4 security_bsd policy");
    expect(security_bsd_table_size() >= 5, "wave4 security_bsd table");

    expect(pbsd_kernel_sysctl_top_number("vm") == kCtlVm, "wave4 dual-link sysctl");
    expect(pbsd_kernel_str2sig("KILL") == kSigKill, "wave4 dual-link str2sig");
    expect(pbsd_kernel_validate_rlimit_id(kRlimitCore) == static_cast<int>(Status::Ok),
           "wave4 dual-link rlimit");
    expect(pbsd_kernel_pager_status_ok(kPagerOk) == PBSD_STATUS_OK,
           "wave4 dual-link pager");
    expect(pbsd_kernel_cap_rights_is_valid(cap_table.read.cr_rights[0],
                                           cap_table.read.cr_rights[1])
               == PBSD_STATUS_OK,
           "wave4 dual-link cap valid");

    expect(arg_table_size() >= 10, "wave4 syscall args table");
    expect(field_count("read").has_value(), "wave4 read args");
    expect(validate_read({3, reinterpret_cast<void*>(1), 64}), "wave4 read args valid");

    expect(filter_table_size() >= 10, "wave4 kevent filter table");
    expect(is_io_filter(kEvFiltRead), "wave4 kevent io filter");
    expect(action_is_add(kEvAdd), "wave4 kevent add");

    expect(static_cast<short>(pbsd::kernel::poll::Event::WrBand) != 0,
           "wave4 poll event bits");
    expect(pbsd::kernel::poll::has(pbsd::kernel::poll::Event::In | pbsd::kernel::poll::Event::Out,
                                   pbsd::kernel::poll::Event::In),
           "wave4 poll readable");
    expect(pbsd::kernel::poll::validate_fd({0, pbsd::kernel::poll::Event::In,
                                            pbsd::kernel::poll::Event::None}) == Status::Ok,
           "wave4 pollfd");

    expect(socket_syscall_table_size() >= 10, "wave4 socket syscall table");
    expect(is_socket_syscall(kSysSocket), "wave4 socket class");
    expect(validate_socket_triple(kAfInet, kSockStream, 0) == Status::Ok,
           "wave4 socket triple");

    expect(vtype_table_size() >= 8, "wave4 vnode type table");
    expect(vtype_is_special(Vtype::VFifo), "wave4 vnode fifo");
    expect(vnode_pollable(Vtype::VReg), "wave4 vnode pollable");

    expect(validate_pipe2_flags(0) == Status::Ok, "wave4 pipe2 flags");
    expect(is_pipe_syscall(kSysPipe2), "wave4 pipe syscall");
    expect(clamp_pipe_buf(100) == kMinPipeBuf, "wave4 pipe buf clamp");

    expect(is_fifo_mode(kSIfifo | 0644), "wave4 fifo mode");
    expect(validate_mkfifo({"/tmp/x", kSIfifo | 0600}) == Status::Ok, "wave4 mkfifo");

    expect(static_cast<unsigned>(Flags::LockParent) != 0, "wave4 namei mod table");
    expect(has(Flags::LockParent, Flags::LockParent), "wave4 namei parent");
    expect(validate(ComponentName{.flags = Flags::Follow}) == Status::Ok, "wave4 namei flags");

    expect(dtype_table_size() >= 4, "wave4 filedesc dtype table");
    expect(auto_close_on_exec(kUfExclose), "wave4 uf exclose");
    expect(validate_fd(0, 64) == Status::Ok, "wave4 fd validate");

    SelinfoStub si{};
    expect(sel_record(si) == Status::Ok, "wave4 selinfo record");
    expect(sel_wakeup(si) == Status::Ok, "wave4 selinfo wakeup");
    expect(poll_events_from_kevent(kEvFiltRead, 0), "wave4 selinfo kevent bridge");
}

void test_wave4_io_socket() {
    using namespace pbsd;
    using namespace pbsd::kernel::select;
    using namespace pbsd::kernel::socket_ops;
    using namespace pbsd::kernel::aio;
    using namespace pbsd::kernel::mbuf;
    using namespace pbsd::kernel::pcb;
    using namespace pbsd::kernel::inpcb;

    FdSet rset{};
    fd_set(3, rset);
    expect(fd_isset(3, rset), "wave4 select fd set");
    expect(validate_nfds(8) == Status::Ok, "wave4 select nfds");
    expect(is_select_syscall(kSysSelect), "wave4 select syscall");

    SocketStub so{.so_options = kSoReuseaddr,
                  .so_state = static_cast<short>(kSsIsconnected),
                  .so_type = 1};
    expect(validate_socket(so) == Status::Ok, "wave4 socket ops");
    expect(so_connected(so.so_state), "wave4 socket connected");
    expect(option_table_size() >= 6, "wave4 socket option table");

    expect(is_aio_syscall(kSysAioRead), "wave4 aio syscall");
    expect(validate_listio_count(4) == Status::Ok, "wave4 aio listio count");
    expect(cancel_complete(kAioCanceled), "wave4 aio cancel");

    expect(has_flag(Flags::Pkthdr, Flags::Pkthdr), "wave4 mbuf pkthdr");
    expect(validate_pkthdr(Flags::Pkthdr) == Status::Ok, "wave4 mbuf validate");

    expect(state_table_size() >= 11, "wave4 pcb state table");
    expect(validate_transition(TcpState::Closed, TcpState::Listen) == Status::Ok,
           "wave4 pcb transition");

    InpcbStub inp{.inp_vflags = kInpIpv4, .inp_flags = kInpBindany};
    expect(validate_inpcb(inp) == Status::Ok, "wave4 inpcb");
    expect(pbsd::kernel::inpcb::flag_table_size() >= 8, "wave4 inpcb flag table");
}

void test_wave4_net_stack() {
    using namespace pbsd;
    using namespace pbsd::kernel::route;
    using namespace pbsd::kernel::ifnet;
    using namespace pbsd::kernel::bpf;
    using namespace pbsd::kernel::netgraph;
    using namespace pbsd::kernel::audit;
    using namespace pbsd::kernel::sockbuf;
    using namespace pbsd::kernel::domain;
    using namespace pbsd::kernel::protosw;
    using namespace pbsd::kernel::sockio;
    using namespace pbsd::kernel::rtentry;

    expect(pbsd::kernel::route::flag_table_size() >= 10, "wave4 kernel route table");
    expect(pbsd::kernel::route::validate_flags(static_cast<unsigned>(pbsd::kernel::route::Flag::Up))
               == Status::Ok,
           "wave4 kernel route flags");
    expect(pbsd::kernel::route::is_usable(static_cast<unsigned>(pbsd::kernel::route::Flag::Up)),
           "wave4 kernel route up");

    pbsd::kernel::ifnet::IfnetStub ifp{
        .if_flags = static_cast<unsigned>(pbsd::kernel::ifnet::Flag::Up),
        .if_link_state = pbsd::kernel::ifnet::LinkState::Up};
    expect(pbsd::kernel::ifnet::validate_ifnet(ifp) == Status::Ok, "wave4 kernel ifnet");
    expect(pbsd::kernel::ifnet::validate_link_transition(pbsd::kernel::ifnet::LinkState::Down,
                                                       pbsd::kernel::ifnet::LinkState::Up)
               == Status::Ok,
           "wave4 kernel link transition");

    Insn prog[] = {{0x06, 0, 0, 0}};
    expect(validate_program(prog, 1) == Status::Ok, "wave4 kernel bpf ret");

    NodeStub node{};
    expect(validate_node(node) == Status::Ok, "wave4 kernel netgraph node");
    expect(abi_compatible(kAbiVersion), "wave4 kernel netgraph abi");

    AuditState ast{.trail_enabled = true, .syscalls_enabled = true};
    expect(should_audit_syscall(ast, kAueOpen), "wave4 kernel audit syscall");
    expect(validate_trigger(kTriggerRotateKernel) == Status::Ok, "wave4 kernel audit trig");

    SockbufStub sb{.sb_cc = 0, .sb_hiwat = kDefaultRecvSpace, .sb_lowat = 1,
                   .so_state = kSsIsconnected};
    expect(validate_sockbuf(sb) == Status::Ok, "wave4 kernel sockbuf");
    expect(so_connected(sb.so_state), "wave4 kernel sock connected");

    expect(domain_table_size() >= 4, "wave4 kernel domain table");
    expect(validate_family(kAfInet) == Status::Ok, "wave4 kernel af inet");

    ProtoswStub pr{.pr_type = 1, .pr_flags = kPrAtomic | kPrConnrequired, .pr_protocol = 6};
    expect(validate_protosw(pr) == Status::Ok, "wave4 kernel protosw");

    expect(ioctl_table_size() >= 4, "wave4 kernel sockio table");
    expect(is_ifnet_ioctl(kSiocsifflags), "wave4 kernel ifnet ioctl");

    RtentryStub rt{.rt_flags = kRtfUp | kRtfHost, .rt_mtu = 1500};
    expect(validate_rtentry(rt) == Status::Ok, "wave4 kernel rtentry");
}

void test_wave6_net_fs_geom_zfs() {
    using namespace pbsd;
    using namespace pbsd::net;
    using namespace pbsd::net::sockopt;
    using namespace pbsd::net::netinet;
    using namespace pbsd::net::tcp;
    using namespace pbsd::net::udp;
    using namespace pbsd::net::ip;
    using namespace pbsd::net::icmp;
    using namespace pbsd::net::ifnet;
    using namespace pbsd::net::route;
    using namespace pbsd::net::in_pcb;
    using namespace pbsd::net::arp;
    using namespace pbsd::net::ether;
    using namespace pbsd::net::mbuf;
    using namespace pbsd::net::bpf;
    using namespace pbsd::net::igmp;
    using namespace pbsd::net::bridge;
    using namespace pbsd::net::pf;
    using namespace pbsd::net::altq;
    using namespace pbsd::net::in6;
    using namespace pbsd::net::gre;
    using namespace pbsd::net::epair;
    using namespace pbsd::net::tun;
    using namespace pbsd::net::tap;
    using namespace pbsd::net::ndp;
    using namespace pbsd::net::lagg;
    using namespace pbsd::net::gif;
    using namespace pbsd::net::loopback;
    using namespace pbsd::net::ipfw;
    using namespace pbsd::net::carp;
    using namespace pbsd::net::ip6;
    using namespace pbsd::net::udp6;
    using namespace pbsd::net::tcp6;
    using namespace pbsd::net::sctp;
    using namespace pbsd::net::netmap;
    using namespace pbsd::net::netgraph;
    using namespace pbsd::net::vlan;
    using namespace pbsd::net::ipsec;
    using namespace pbsd::net::vxlan;
    using namespace pbsd::net::stf;
    using namespace pbsd::net::wg;
    using namespace pbsd::net::pfsync;
    using namespace pbsd::fs;
    using namespace pbsd::fs::vop;
    using namespace pbsd::fs::mount;
    using namespace pbsd::fs::namei;
    using namespace pbsd::fs::stat;
    using namespace pbsd::fs::fcntl;
    using namespace pbsd::fs::dirent;
    using namespace pbsd::fs::attr;
    using namespace pbsd::fs::lockf;
    using namespace pbsd::fs::quota;
    using namespace pbsd::fs::ufs;
    using namespace pbsd::fs::tmpfs;
    using namespace pbsd::fs::pipefs;
    using namespace pbsd::fs::extattr;
    using namespace pbsd::fs::nfs;
    using namespace pbsd::fs::nullfs;
    using namespace pbsd::fs::procfs;
    using namespace pbsd::fs::devfs;
    using namespace pbsd::fs::fdescfs;
    using namespace pbsd::fs::linprocfs;
    using namespace pbsd::fs::fusefs;
    using namespace pbsd::fs::msdosfs;
    using namespace pbsd::fs::cd9660;
    using namespace pbsd::fs::unionfs;
    using namespace pbsd::fs::isofs;
    using namespace pbsd::fs::autofs;
    using namespace pbsd::fs::ext2fs;
    using namespace pbsd::fs::smbfs;
    using namespace pbsd::geom;
    using namespace pbsd::geom::ctl;
    using namespace pbsd::geom::disk;
    using namespace pbsd::geom::slice;
    using namespace pbsd::geom::flashmap;
    using namespace pbsd::geom::part;
    using namespace pbsd::geom::mirror;
    using namespace pbsd::geom::label;
    using namespace pbsd::geom::taste;
    using namespace pbsd::geom::concat;
    using namespace pbsd::geom::stripe;
    using namespace pbsd::geom::raid3;
    using namespace pbsd::geom::eli;
    using namespace pbsd::geom::journal;
    using namespace pbsd::geom::gate;
    using namespace pbsd::geom::multipath;
    using namespace pbsd::geom::cache;
    using namespace pbsd::geom::shsec;
    using namespace pbsd::geom::virstor;
    using namespace pbsd::zfs;
    using namespace pbsd::zfs::ioctl;
    using namespace pbsd::zfs::features;
    using namespace pbsd::zfs::props;
    using namespace pbsd::zfs::zio;
    using namespace pbsd::zfs::spa;
    using namespace pbsd::zfs::zio_checksum;
    using namespace pbsd::zfs::dmu;
    using namespace pbsd::zfs::vdev;
    using namespace pbsd::zfs::arc;
    using namespace pbsd::zfs::l2arc;
    using namespace pbsd::zfs::zap;
    using namespace pbsd::zfs::zil;
    using namespace pbsd::zfs::zio_flag;
    using namespace pbsd::zfs::compress;

    LineageTree tree;
    auto root = tree.create_root();

    expect(static_cast<int>(IpProto::Tcp) == 6, "wave6 ipproto tcp");
    expect(static_cast<int>(IpProto::Gre) == 47, "wave6 ipproto gre");
    expect(is_transport_proto(IpProto::Tcp), "wave6 tcp transport");
    expect(ip_proto_table_size() >= 10, "wave6 ip proto table");
    expect(is_multicast_option(IpOption::AddMembership), "wave6 ip multicast opt");
    expect(ip_option_table_size() >= 5, "wave6 ip option table");
    expect(tcp::option_table_size() >= 15, "wave6 tcp option table");
    expect(tcp::flags_has(static_cast<unsigned short>(tcp::Flag::Syn)
                          | static_cast<unsigned short>(tcp::Flag::Ack),
                          tcp::Flag::Syn),
           "wave6 tcp flags");
    expect(udp::option_table_size() >= 1, "wave6 udp option table");
    expect(valid_encap_type(static_cast<int>(EncapType::EspInUdp)), "wave6 udp encap");
    expect(validate_header(IpHeader{.version_hl = 0x45, .total_len = 20, .ttl = 64})
               == Status::Ok,
           "wave6 ip header");
    expect(tos_table_size() >= 4, "wave6 ip tos table");
    expect(icmp::type_table_size() >= 10, "wave6 icmp type table");
    expect(icmp::is_info_type(icmp::Type::Echo), "wave6 icmp info");
    expect(icmp::is_error_type(icmp::Type::Unreach), "wave6 icmp error");
    expect(validate_header(icmp::Header{.type = static_cast<unsigned char>(icmp::Type::Echo),
                                       .code = 0})
               == Status::Ok,
           "wave6 icmp header");
    expect(if_flag_table_size() >= 8, "wave6 ifnet flag table");
    expect(ifnet::is_up(static_cast<unsigned>(ifnet::Flag::Up)), "wave6 if up");
    expect(ifnet::check_flag_change(CapabilityRights::Read | CapabilityRights::Write,
                                    ifnet::Flag::Up, true)
               == Status::Ok,
           "wave6 if flag cap");
    expect(tcp::state_table_size() >= 10, "wave6 tcp state table");
    expect(tcp::have_established(tcp::State::Established), "wave6 tcp established");
    expect(tcp::validate_state_transition(tcp::State::Listen, tcp::State::SynReceived) == Status::Ok,
           "wave6 tcp fsm");
    expect(route::flag_table_size() >= 8, "wave6 route flag table");
    expect(route::validate_flags(static_cast<unsigned>(route::Flag::Up)) == Status::Ok,
           "wave6 route flags");
    expect(in_pcb::flag_table_size() >= 8, "wave6 in_pcb flag table");
    expect(in_pcb::validate_flags(0) == Status::Ok, "wave6 in_pcb flags");
    expect(arp::op_table_size() >= 4, "wave6 arp op table");
    expect(arp::validate_header(arp::Header{.hw_len = 6, .proto_len = 4,
                                            .hw_type = static_cast<unsigned short>(
                                                arp::Hardware::Ether)})
               == Status::Ok,
           "wave6 arp header");
    expect(ether::validate_type(ether::kTypeIp) == Status::Ok, "wave6 ether type");
    expect(validate_vtype(Vtype::VRegular) == Status::Ok, "wave6 vtype reg");
    expect(vtype_is_dev(Vtype::VChar), "wave6 vtype dev");
    expect(vv_has(static_cast<unsigned short>(VvFlag::Root), VvFlag::Root), "wave6 vv root");
    expect(namei::op_table_size() >= 6, "wave6 namei op table");
    expect(namei::check_op(CapabilityRights::Read | CapabilityRights::Execute,
                           namei::OpFlag::Lookup) == Status::Ok,
           "wave6 namei lookup");
    expect(stat::validate_mode(static_cast<unsigned>(stat::FileType::Regular)) == Status::Ok,
           "wave6 stat mode");
    expect(fcntl::check_open(CapabilityRights::Read, fcntl::OpenFlag::Rdonly) == Status::Ok,
           "wave6 fcntl rdonly");
    expect(dirent::validate_entry(dirent::Entry{.namelen = 8}) == Status::Ok,
           "wave6 dirent entry");
    expect(mount_flag_table_size() >= 10, "wave6 mount flag table");
    expect(mount::check_mount_flag(CapabilityRights::Read | CapabilityRights::Write,
                                   mount::Flag::Noexec, true)
               == Status::Ok,
           "wave6 mount noexec");
    expect(validate_geom_state(GeomState::Active) == Status::Ok, "wave6 geom state");
    expect(disk::validate_rotation_rate(static_cast<unsigned>(disk::RotationRate::NonRotating))
               == Status::Ok,
           "wave6 geom disk rr");
    expect(slice::validate_slice(slice::Slice{.offset = 0, .length = 512, .sector_size = 512})
               == Status::Ok,
           "wave6 geom slice");
    expect(flashmap::validate_region(flashmap::Region{.offset = 0, .length = 4096,
                                                      .erase_size = 4096})
               == Status::Ok,
           "wave6 geom flashmap");

    Stack net;
    expect(net.init() == Status::Ok, "wave6 net init");
    auto sock = net.socket(SocketType::Stream, CapabilityRights::Read | CapabilityRights::Write, root);
    expect(sock.status == Status::Ok, "wave6 socket");
    expect(net.setsockopt(sock.value, Level::Socket, static_cast<int>(SoName::SndBuf),
                          OptDir::Set) == Status::Ok,
           "wave6 setsockopt cap check");
    expect(net.setsockopt(sock.value, Level::Tcp, static_cast<int>(tcp::Option::NoDelay),
                          OptDir::Set) == Status::Ok,
           "wave6 tcp nodelay");
    expect(net.setsockopt(sock.value, Level::Udp, static_cast<int>(udp::Option::Encap),
                          OptDir::Set) == Status::Ok,
           "wave6 udp encap");

    Vfs vfs;
    expect(vfs.mount_root() == Status::Ok, "wave6 vfs mount");
    auto vn = vfs.open("/tmp", {.want = CapabilityRights::Read}, root);
    expect(vn.status == Status::Ok, "wave6 open");
    expect(vfs.vop_dispatch(vn.value, vop::Op::Read) == Status::Ok, "wave6 vop read");
    expect(vfs.vop_dispatch(vn.value, vop::Op::Readdir) == Status::Ok, "wave6 vop readdir");
    expect(vfs.vop_dispatch(vn.value, vop::Op::Seekhole) == Status::Ok, "wave6 vop seekhole");
    expect(vop_table_size() >= 25, "wave6 vop table expanded");

    GeomStack geom;
    auto prov = geom.publish({.name = "da0", .sector_count = 1024},
                             CapabilityRights::Read | CapabilityRights::Write, root);
    expect(prov.status == Status::Ok, "wave6 geom publish");
    ReqHeader hdr{.version = kGctlVersion, .narg = 1};
    expect(geom.ioctl_ctl(prov.value, hdr, false) == Status::Ok, "wave6 geom ctl");

    ZfsPool pool;
    expect(pool.import_pool("guid-1") == Status::Ok, "wave6 zfs import");
    auto ds = pool.mount("tank/root", DatasetKind::FileSystem,
                         CapabilityRights::Read | CapabilityRights::Map, root);
    expect(ds.status == Status::Ok, "wave6 zfs mount");
    expect(pool.ioctl(ds.value, Ioc::ObjsetStats) == Status::Ok, "wave6 zfs ioctl read");
    expect(pool.require_feature(SpaFeature::Lz4Compress) == Status::Ok, "wave6 zfs lz4");
    expect(feature_guid(SpaFeature::Encryption) != nullptr, "wave6 zfs feature guid");
    expect(feature_table_size() >= 15, "wave6 zfs feature table");
    expect(feature_table_size() >= 35, "wave6 zfs spa features expanded");
    expect(props::prop_table_size() >= 8, "wave6 zfs props table");
    expect(props::validate_attr(props::PropAttr::Inherit) == Status::Ok, "wave6 zfs prop attr");
    expect(zio::validate_flags(static_cast<unsigned>(zio::Flag::Read)) == Status::Ok,
           "wave6 zio flags");
    expect(spa::validate_state(spa::State::Active) == Status::Ok, "wave6 spa state");
    expect(spa::is_importable(spa::State::Exported), "wave6 spa importable");
    expect(vlan::validate_vid(100) == Status::Ok, "wave6 vlan vid");
    expect(altq::disc_table_size() >= 4, "wave6 altq table");
    expect(in6::validate_hoplimit(64) == Status::Ok, "wave6 in6 hoplimit");
    expect(gre::validate_header(gre::Header{.flags_version = 0, .proto = gre::kProtoIp})
               == Status::Ok,
           "wave6 gre header");
    expect(epair::validate_role(epair::Role::Host) == Status::Ok, "wave6 epair role");
    expect(tmpfs::validate(tmpfs::MountArgs{.max_size_bytes = 4096}) == Status::Ok, "wave6 tmpfs size");
    expect(pipefs::validate_io_len(512) == Status::Ok, "wave6 pipefs io");
    expect(extattr::validate_entry(extattr::Entry{.namespace_id = 1, .name_len = 4})
               == Status::Ok,
           "wave6 extattr entry");
    expect(stripe::validate_width(4) == Status::Ok, "wave6 geom stripe");
    expect(concat::validate_provider_count(2) == Status::Ok, "wave6 geom concat");
    expect(dmu::validate_objset_type(dmu::ObjsetType::Meta) == Status::Ok, "wave6 zfs dmu table");
    expect(vdev::validate_state(vdev::State::Healthy) == Status::Ok, "wave6 zfs vdev");
    expect(arc::validate_flags(0) == Status::Ok, "wave6 zfs arc");
    expect(dsl::validate_hold(static_cast<unsigned>(dsl::HoldFlag::Recursive)) == Status::Ok,
           "wave6 zfs dsl");
    expect(lagg::validate_proto(lagg::Proto::Lacp) == Status::Ok, "wave6 lagg lacp");
    expect(gif::validate_mtu(gif::kMtuDefault) == Status::Ok, "wave6 gif mtu");
    expect(tun::validate_mtu(tun::kMtuDefault) == Status::Ok, "wave6 tun mtu");
    expect(tap::validate_mru(tap::kMruMax) == Status::Ok, "wave6 tap mru");
    expect(loopback::is_loopback(static_cast<unsigned>(loopback::Flag::Loopback)),
           "wave6 loopback flag");
    expect(ipfw::validate_rule_num(ipfw::kDefaultRule) == Status::Ok, "wave6 ipfw rule");
    expect(carp::validate_header(carp::Header{.vhid = 1, .advbase = 1}) == Status::Ok,
           "wave6 carp header");
    expect(ndp::validate_state(ndp::LlState::Reachable) == Status::Ok, "wave6 ndp state");
    expect(ip6::validate_header(ip6::Header{.vfc = 0x60, .hlim = 64}) == Status::Ok,
           "wave6 ip6 header");
    expect(sctp::validate_header(sctp::Header{.src_port = 5000, .dest_port = 5000}) == Status::Ok,
           "wave6 sctp header");
    expect(netmap::validate_api(netmap::kApi) == Status::Ok, "wave6 netmap api");
    expect(netgraph::validate_name_len(8, netgraph::kNodeSize) == Status::Ok,
           "wave6 netgraph name");
    expect(nfs::validate_version(nfs::kVer4) == Status::Ok, "wave6 nfs v4");
    expect(procfs::validate_node(procfs::NodeType::Curproc) == Status::Ok, "wave6 procfs node");
    expect(devfs::validate_rule_id(1) == Status::Ok, "wave6 devfs rule");
    expect(fdescfs::validate_node(fdescfs::NodeId::Root) == Status::Ok, "wave6 fdescfs node");
    expect(linprocfs::validate_node(linprocfs::Node::Cpuinfo) == Status::Ok,
           "wave6 linprocfs node");
    expect(fusefs::validate_version(fusefs::kMajorVersion, fusefs::kMinorVersion) == Status::Ok,
           "wave6 fusefs version");
    expect(raid3::validate_width(3) == Status::Ok, "wave6 geom raid3");
    expect(eli::validate_algo(eli::Algo::AesXts) == Status::Ok, "wave6 geom eli");
    expect(journal::validate_state(journal::State::Active) == Status::Ok, "wave6 geom journal");
    expect(gate::validate_cmd(gate::Cmd::Create) == Status::Ok, "wave6 geom gate");
    expect(multipath::validate_provider_count(2) == Status::Ok, "wave6 geom multipath");
    expect(zio_checksum::table_size() >= 5, "wave6 zio checksum table");
    expect(zio_checksum::validate_algo(zio_checksum::Algorithm::Fletcher4) == Status::Ok,
           "wave6 zio checksum fletcher4");
    expect(dlt::validate_type(static_cast<unsigned>(dlt::Type::En10Mb)) == Status::Ok,
           "wave6 dlt en10mb");
    expect(media::validate_subtype(media::Subtype::ThousandT) == Status::Ok,
           "wave6 ifmedia 1000T");
    expect(pfil::validate_type(pfil::Type::Ip4) == Status::Ok, "wave6 pfil ip4");
    expect(vxlan::validate_vni(100) == Status::Ok, "wave6 vxlan vni");
    expect(lacp::validate_timer(lacp::Timer::Periodic) == Status::Ok, "wave6 lacp timer");
    expect(clone::validate_unit(1) == Status::Ok, "wave6 clone unit");
    expect(ext2fs::validate_magic(ext2fs::kMagic) == Status::Ok, "wave6 ext2 magic");
    expect(smbfs::validate_version(smbfs::kVersion) == Status::Ok, "wave6 smbfs version");
    expect(raid::validate_version(raid::kVersion) == Status::Ok, "wave6 geom raid ver");
    expect(mountver::validate_version(mountver::kVersion) == Status::Ok, "wave6 mountver ver");
    expect(zio_flag::validate_failure_mode(zio_flag::FailureMode::Wait) == Status::Ok,
           "wave6 zio failure mode");
    expect(compress::validate_algo(compress::Algorithm::Lz4) == Status::Ok,
           "wave6 zfs compress lz4");
}

void test_wave7_stand_arch() {
    using namespace pbsd;
    using namespace pbsd::stand;
    using namespace pbsd::stand::pe;
    using namespace pbsd::stand::efi;
    using namespace pbsd::stand::efi::protocols;
    using namespace pbsd::stand::bootinfo;
    using namespace pbsd::stand::common;
    using namespace pbsd::stand::loader;
    using namespace pbsd::stand::reboot;
    using namespace pbsd::stand::gpt;
    using namespace pbsd::stand::kmod;
    using namespace pbsd::stand::elf;
    using namespace pbsd::stand::env;
    using namespace pbsd::stand::devinfo;
    using namespace pbsd::stand::zfsboot;
    using namespace pbsd::stand::net;
    using namespace pbsd::arch::amd64;
    using namespace pbsd::arch::amd64::msr;
    using namespace pbsd::arch::amd64::cpufunc;
    using namespace pbsd::arch::amd64::pmap;
    using namespace pbsd::arch::amd64::apic;
    using namespace pbsd::arch::amd64::exception;
    using namespace pbsd::arch::amd64::cr;
    using namespace pbsd::arch::arm64;
    using namespace pbsd::arch::arm64::sysreg;
    using namespace pbsd::arch::arm64::pmap;
    using namespace pbsd::arch::arm64::gic;

    expect(efi::kPageSize == 4096, "wave7 efi page size");
    expect(is_loader_memory(MemoryType::LoaderCode), "wave7 efi loader mem");
    expect(loader_cmd_table_size() >= 8, "wave7 loader cmd table");
    expect(validate_boot_flags(kBootFRootFromZfs) == Status::Ok, "wave7 boot flags");
    expect(validate_elf_machine(ElfClass::Elf64, ElfMachine::Amd64) == Status::Ok,
           "wave7 elf64 amd64");
    MemoryDescriptor md{.type = static_cast<std::uint32_t>(MemoryType::Conventional),
                        .page_count = 16};
    expect(validate_memory_descriptor(md) == Status::Ok, "wave7 efi mem desc");
    expect(pages_to_size(1) == 4096, "wave7 efi pages to size");

    EfiLoader loader;
    expect(loader.init_firmware() == Status::Ok, "wave7 efi init");
    BootParams bp{};
    bp.kernel_path = "/boot/kernel";
    bp.pe_dos.e_magic = kDosSignature;
    bp.pe_dos.e_lfanew = sizeof(DosHeader);
    bp.pe_nt.signature = kNtSignature;
    bp.pe_nt.file_header.machine = kFileMachineAmd64;
    bp.pe_nt.file_header.characteristics = kFileExecutable;
    bp.module_load_addr = 0x100000;
    bp.pe_nt.optional_header.address_of_entry_point = 0x1000;
    expect(loader.load_kernel(bp) == Status::Ok, "wave7 pe load");
    expect(loader.entry_point() == 0x101000, "wave7 entry point");

    arch::amd64::RegisterFrame frame{.rsp = 16};
    expect(arch::amd64::validate_frame(frame) == Status::Ok, "wave7 amd64 frame");
    expect(arch::amd64::validate_long_mode(static_cast<unsigned long long>(msr::EferFlag::Lme))
               == Status::Ok,
           "wave7 efer");
    expect(msr::validate_cr4_vmx(static_cast<unsigned long long>(msr::Cr4Flag::VmxE)
                                 | static_cast<unsigned long long>(msr::Cr4Flag::Pae))
               == Status::Ok,
           "wave7 cr4 vmx");
    expect(arch::amd64::cpufunc::validate_cr0(
               static_cast<unsigned long long>(arch::amd64::cpufunc::Cr0Flag::Pe)
               | static_cast<unsigned long long>(arch::amd64::cpufunc::Cr0Flag::Pg))
               == Status::Ok,
           "wave7 cr0 paging");
    expect(arch::amd64::cpufunc::validate_cr4_vmx(
               static_cast<unsigned long long>(arch::amd64::cpufunc::Cr4Flag::VmxE)
               | static_cast<unsigned long long>(arch::amd64::cpufunc::Cr4Flag::Pae))
               == Status::Ok,
           "wave7 cpufunc cr4 vmx");
    expect(guest_exit_reason_count() >= 12, "wave7 vmx exit table");
    expect(msr_table_size() >= 12, "wave7 msr table");
    expect(is_vmx_msr(kMsrIa32VmxBasic), "wave7 vmx msr range");
    expect(validate_msr_access(kMsrTsc, false) == Status::Ok, "wave7 rdmsr tsc");
    expect(protocol_table_size() >= 8, "wave7 efi protocol table");
    expect(validate_protocol(Protocol::GraphicsOutput) == Status::Ok, "wave7 efi gop");
    expect(guid_equal(guid_for(Protocol::BlockIo), kProtocolTable[6].guid), "wave7 blockio guid");
    BootInfo bi{.version = kBootinfoVersion, .size = kBiSizeOffset, .memsizes_valid = 1,
                .basemem_kb = 640, .extmem_kb = 64512};
    expect(validate_bootinfo(bi) == Status::Ok, "wave7 bootinfo");
    expect(has_bootinfo_flag(static_cast<unsigned>(RebootFlag::Bootinfo)), "wave7 rb bootinfo");
    expect(loader::cmd_table_size() >= 8, "wave7 loader cmd table");
    expect(loader::validate_cmd(loader::Cmd::Boot) == Status::Ok, "wave7 loader boot");
    expect(reboot::validate_flags(static_cast<unsigned>(reboot::Flag::Dump)) == Status::Ok,
           "wave7 reboot dump");
    expect(reboot::wants_dump(static_cast<unsigned>(reboot::Flag::Dump)), "wave7 reboot wants dump");

    arch::arm64::RegisterFrame aframe{.sp = 16};
    expect(arch::arm64::validate_frame(aframe) == Status::Ok, "wave7 arm64 frame");
    expect(guest_exit_code_count() >= 8, "wave7 arm64 exit table");
    expect(validate_sctlr(sctlr_el1_reset()) == Status::Ok, "wave7 sctlr el1");
    expect(validate_hcr_el2(static_cast<unsigned long long>(HcrEl2Flag::Vm)) == Status::Ok,
           "wave7 hcr el2");
    expect(parange_bits(IdAa64Mmfr0Parange::Range64G) == 36, "wave7 parange");
    expect(arch::arm64::exception::validate_ec(arch::arm64::exception::Ec::Svc64) == Status::Ok,
           "wave7 arm64 ec");
    expect(arch::arm64::exception::is_sync_abort(arch::arm64::exception::Ec::DataAbtCurrent),
           "wave7 arm64 sync abort");
    expect(gpt::validate_header(gpt::Header{.signature = gpt::kSignature,
                                            .header_size = gpt::kHeaderSize})
               == Status::Ok,
           "wave7 gpt header");
    expect(kmod::validate_flags(static_cast<unsigned>(kmod::Flag::Loader)) == Status::Ok,
           "wave7 kmod flags");
    expect(elf::validate_ehdr(elf::Class::Elf64, elf::Machine::Amd64) == Status::Ok,
           "wave7 elf64");
    expect(arch::amd64::pmap::validate_pte(
               static_cast<unsigned long long>(arch::amd64::pmap::PteFlag::Present))
               == Status::Ok,
           "wave7 amd64 pte");
    expect(apic::validate_delivery(apic::DeliveryMode::Fixed) == Status::Ok, "wave7 apic mode");
    expect(arch::arm64::pmap::validate_pte(
               static_cast<unsigned long long>(arch::arm64::pmap::Attr::Valid))
               == Status::Ok,
           "wave7 arm64 pte");
    expect(gic::validate_irq(32, gic::IrqType::Spi) == Status::Ok, "wave7 gic irq");
    expect(env::var_table_size() >= 4, "wave7 stand env table");
    expect(env::validate_name_len(8) == Status::Ok, "wave7 stand env name");
    expect(devinfo::validate_device(devinfo::Device{.bus = devinfo::Bus::Pci,
                                                    .kind = devinfo::Kind::Disk,
                                                    .unit = 0})
               == Status::Ok,
           "wave7 stand devinfo");
    expect(zfsboot::validate_name(zfsboot::BootEnv{.name = "default"}) == Status::Ok,
           "wave7 zfsboot name");
    expect(pbsd::arch::amd64::exception::validate_vector(
               pbsd::arch::amd64::exception::Vector::PageFault) == Status::Ok,
           "wave7 amd64 exception");
    expect(pbsd::arch::arm64::cpufunc::validate_barrier(
               pbsd::arch::arm64::cpufunc::Barrier::Dsb) == Status::Ok,
           "wave7 arm64 barrier");
    expect(::pbsd::stand::net::validate_proto(::pbsd::stand::net::Proto::Bootp) == Status::Ok,
           "wave7 stand net bootp");
    expect(cr::validate_paging(static_cast<unsigned>(cr::Cr0::Pe) | static_cast<unsigned>(cr::Cr0::Pg))
               == Status::Ok,
           "wave7 amd64 cr0 paging");
}

void test_wave8_compositor() {
    using namespace pbsd;
    using namespace pbsd::compositor;
    using namespace pbsd::compositor::wayland;
    using namespace pbsd::compositor::wayland::layer;
    using namespace pbsd::bifrost;
    using namespace pbsd::bifrost::hypervisor;
    using namespace pbsd::bifrost::virtio;
    using namespace pbsd::bifrost::nested;
    using namespace pbsd::bifrost::ept;
    using namespace pbsd::bifrost::vmcb;
    using namespace pbsd::bifrost::vmcs;
    using namespace pbsd::bifrost::npt;
    using namespace pbsd::bifrost::ioapic;
    using namespace pbsd::bifrost::lapic;
    using namespace pbsd::bifrost::vpid;
    using namespace pbsd::bifrost::exit;
    using namespace pbsd::compositor::wayland::seat;
    using namespace pbsd::compositor::wayland::xdg;
    using namespace pbsd::compositor::wayland::output;
    using namespace pbsd::compositor::wayland::buffer;
    using namespace pbsd::compositor::drm;
    expect(validate_mmio_status(static_cast<unsigned char>(MmioStatus::Acknowledge)
                                | static_cast<unsigned char>(MmioStatus::Driver)
                                | static_cast<unsigned char>(MmioStatus::DriverOk))
               == Status::Ok,
           "wave8 virtio status");
    auto gpu = pv_gpu(0xF000'3000u);
    expect(attach_pv_stub(gpu) == Status::Ok, "wave8 pv gpu attach");

    Compositor comp;
    expect(comp.init() == Status::Ok, "wave8 compositor init");
    expect(comp.aero_theme().blur_radius_px == 24.0f, "wave8 aero defaults");

    static char json[] = R"({"blurRadius":24,"glassTint":[0.12,0.22,0.40,0.65],"panelOpacity":0.72,"dialogOpacity":0.78,"saturationBoost":1.15,"noiseStrength":0.04})";
    expect(comp.load_aero_json_fragment(json) == Status::Ok, "wave8 aero json");
    expect(comp.aero_theme().panel_opacity == 0.72f, "wave8 aero panel opacity");
    expect(comp.apply_aero_to_surface(1) == Status::Ok, "wave8 aero surface");

    WireHeader wh{.object_id = 1,
                  .opcode = static_cast<std::uint16_t>(DisplayRequest::Sync),
                  .size = 8};
    expect(comp.dispatch_wayland(wh) == Status::Ok, "wave8 wl_display sync");

    WireHeader xdg{.object_id = 2,
                   .opcode = static_cast<std::uint16_t>(XdgToplevelRequest::SetTitle),
                   .size = 16};
    expect(comp.dispatch_wayland(xdg) == Status::Ok, "wave8 xdg set title");
    WireHeader ptr{.object_id = 3,
                   .opcode = static_cast<std::uint16_t>(
                       pbsd::compositor::wayland::seat::PointerRequest::SetCursor),
                   .size = 24};
    expect(comp.dispatch_wayland(ptr) == Status::Ok, "wave8 pointer set cursor");
    expect(validate_shm_format(
               static_cast<unsigned>(pbsd::compositor::wayland::ShmFormat::CArgb8888))
               == Status::Ok,
           "wave8 shm format");
    expect(comp.bind_output(OutputRole::Desktop, 1) == Status::Ok, "wave8 bind output");
    expect(is_xdg_positioner_request(static_cast<unsigned>(XdgPositionerRequest::SetAnchor)),
           "wave8 xdg positioner");
    expect(is_xdg_decoration_request(static_cast<unsigned>(XdgToplevelDecorationRequest::SetMode)),
           "wave8 xdg decoration");
    expect(layer::validate_layer(static_cast<unsigned>(layer::Layer::Top)) == Status::Ok,
           "wave8 layer shell");
    expect(layer::validate_configure(layer::ConfigureEvent{.width = 1920, .height = 1080,
                                                           .serial = 1})
               == Status::Ok,
           "wave8 layer configure");
    expect(seat::validate_capabilities(static_cast<unsigned>(seat::Capability::Pointer))
               == Status::Ok,
           "wave8 seat caps");
    expect(seat::has_capability(static_cast<unsigned>(seat::Capability::Pointer)
                                | static_cast<unsigned>(seat::Capability::Keyboard),
                                seat::Capability::Pointer),
           "wave8 seat has pointer");
    expect(xdg::validate_wm_capabilities(static_cast<unsigned>(xdg::WmCapability::Maximize))
               == Status::Ok,
           "wave8 xdg wm caps");
    expect(xdg::is_toplevel_request(static_cast<unsigned>(xdg::ToplevelRequest::SetTitle)),
           "wave8 xdg toplevel req");
    expect(nest_stub_table_size() >= 4, "wave8 nested stub table");
    EptViolation ev1{.guest_pa = 0x1000, .backend = hypervisor::Backend::Vmx};
    expect(validate_violation(ev1) == Status::Ok, "wave8 ept violation");
    expect(vmcb::validate_intercept(static_cast<unsigned>(vmcb::Ctrl1::Cpuid), 0) == Status::Ok,
           "wave8 vmcb intercept");
    expect(vmcb::backend_for_vmcb() == hypervisor::Backend::Svm, "wave8 vmcb backend");
    NestConfig nest{.level = NestLevel::L1, .backend = Backend::Vmx,
                    .features = static_cast<unsigned int>(NestFeature::Ept)};
    expect(attach_nested_stub(nest) == Status::Ok, "wave8 nested attach");
    hypervisor::Amd64VmExit nested_out{};
    expect(dispatch_nested_exit(NestLevel::L1, Backend::Vmx,
                                static_cast<unsigned>(VmxExit::Cpuid), &nested_out)
               == Status::Ok,
           "wave8 nested exit");
    WireHeader layer_hdr{.object_id = 4,
                         .opcode = static_cast<std::uint16_t>(LayerSurfaceRequest::SetAnchor),
                         .size = 16};
    expect(comp.dispatch_wayland(layer_hdr) == Status::Ok, "wave8 layer surface anchor");
    expect(layer::validate_layer(static_cast<unsigned>(layer::Layer::Top)) == Status::Ok,
           "wave8 layer top");
    expect(layer::validate_anchor(static_cast<unsigned>(layer::Anchor::Top)
                                  | static_cast<unsigned>(layer::Anchor::Left)) == Status::Ok,
           "wave8 layer anchor");
    EptViolation ev2{.backend = Backend::Vmx, .guest_pa = 0x1000, .type = ViolationType::Write};
    expect(validate_violation(ev2) == Status::Ok, "wave8 ept violation write");
    hypervisor::Amd64VmExit ept_out{};
    expect(map_violation_to_exit(ev2, &ept_out) == Status::Ok, "wave8 ept map exit");
    expect(vmcs::validate_field(vmcs::Field::ExitReason) == Status::Ok, "wave8 vmcs field");
    expect(vmcs::backend_for_vmcs() == hypervisor::Backend::Vmx, "wave8 vmcs backend");
    expect(npt::validate_fault(npt::Fault{.gpa = 0x2000, .flags = 1}) == Status::Ok,
           "wave8 npt fault");
    expect(npt::backend_for_npt() == hypervisor::Backend::Svm, "wave8 npt backend");
    expect(output::validate_mode(output::Mode{.width = 1920, .height = 1080}) == Status::Ok,
           "wave8 wl output mode");
    expect(buffer::validate_format(buffer::ShmFormat::CArgb8888) == Status::Ok,
           "wave8 wl buffer fmt");
    expect(buffer::bytes_per_pixel(buffer::ShmFormat::Crgb565) == 2, "wave8 wl bpp");
    expect(ioapic::validate_index(0) == Status::Ok, "wave8 ioapic index");
    expect(lapic::validate_mode(lapic::Mode::Periodic) == Status::Ok, "wave8 lapic mode");
    expect(vpid::validate_tag(1, vpid::TagKind::Vpid) == Status::Ok, "wave8 vpid tag");
    expect(exit::validate_code(exit::Code::Vmx) == Status::Ok, "wave8 vm exit vmx");
    expect(exit::is_nested(exit::Code::Svm), "wave8 svm nested exit");
    expect(drm::validate_fourcc(drm::Fourcc::Argb8888) == Status::Ok, "wave8 drm fourcc");
}

void test_wave9_pkg() {
    using namespace pbsd::pkg;
    using namespace pbsd::pkg::manifest;
    using namespace pbsd::pkg::repo;
    using namespace pbsd::pkg::signature;

    Transaction tx{};
    AtomicUpdater updater;
    expect(updater.begin(tx) == pbsd::Status::Ok, "wave9 pkg begin");
    PackageRef pkg_ref{};
    std::strncpy(pkg_ref.name, "pbsd-base", sizeof(pkg_ref.name) - 1);
    std::strncpy(pkg_ref.digest_hex, "deadbeef", sizeof(pkg_ref.digest_hex) - 1);
    expect(updater.stage(tx, pkg_ref) == pbsd::Status::Ok, "wave9 pkg stage");
    expect(updater.commit(tx) == pbsd::Status::Ok, "wave9 pkg commit");
    expect(tx.state == pbsd::pkg::TxState::Applied, "wave9 pkg applied");

    Entry me{.type = EntryType::File};
    std::strncpy(me.path, "/etc/rc.conf", sizeof(me.path) - 1);
    std::strncpy(me.digest_hex, "abc123", sizeof(me.digest_hex) - 1);
    me.size = 42;
    expect(validate_entry(me) == pbsd::Status::Ok, "wave9 manifest entry");

    RepoRef repo{};
    std::strncpy(repo.name, "pbsd", sizeof(repo.name) - 1);
    std::strncpy(repo.url, "https://pkg.pbsd.local", sizeof(repo.url) - 1);
    std::strncpy(repo.fingerprint_hex, "ff00", sizeof(repo.fingerprint_hex) - 1);
    repo.flags = static_cast<unsigned>(RepoFlag::Signed);
    expect(validate_repo(repo) == pbsd::Status::Ok, "wave9 repo ref");

    SignatureBlock sig{.algo = Algo::Ed25519};
    std::strncpy(sig.sig_hex, "cafe", sizeof(sig.sig_hex) - 1);
    expect(verify_digest("deadbeef", sig) == pbsd::Status::Ok, "wave9 signature");
}

void test_hostname_port() {
    using namespace pbsd::userland::bin::hostname;

    char a0[] = "hostname";
    char a1[] = "-s";
    char* argv[] = {a0, a1, nullptr};
    auto req = parse_args(2, argv);
    expect(req.has_value() && req.value.opts.mode == DisplayMode::Short, "hostname -s");

    char host[] = "foo.example.com";
    const char* view = display_name(host, DisplayMode::Short);
    expect(view != nullptr && std::strcmp(view, "foo") == 0, "hostname short transform");
}

void test_wave9_kde_theme() {
    using namespace pbsd;
    using namespace pbsd::kde::plasma::aero;
    using namespace pbsd::kde::plasma::volume;
    using namespace pbsd::kde::kwin::effects::fade;
    using namespace pbsd::theme::plasma::aero::panel;
    using namespace pbsd::theme::plasma::aero::icons;

    expect(validate_theme_id(kThemeId) == Status::Ok, "burst9 kde theme id");
    expect(theme_id_matches(kThemeId), "burst9 kde theme match");
    expect(validate_volume(kDefaultVolume) == Status::Ok, "burst9 plasma volume");
    expect(kEffectId[0] == 'f', "burst9 kwin fade id");
    expect(kHeight == 40, "burst9 aero panel height");
    expect(size_for(Context::StartMenu) == kLarge, "burst9 aero icon size");
}

} // namespace

int main() {
    test_si1_rights_algebra();
    test_si2_no_ambient_echo();
    test_si3_wx();
    test_si4_zeroed_alloc();
    test_si5_no_blocking_in_isr();
    test_si6_lineage_revocation();
    test_handles_and_vm();
    test_si7_uda_capability();
    test_si7_virtio_net();
    test_si7_nvme();
    test_si7_virtio_scsi();
    test_si7_ahci();
    test_si7_e1000();
    test_si7_xhci();
    test_si7_virtio_console();
    test_si7_usbhid();
    test_si7_virtio_gpu();
    test_si7_virtio_random();
    test_si7_virtio_balloon();
    test_si7_nvme_admin();
    test_si7_ixgbe();
    test_si7_uhci();
    test_si7_ehci();
    test_si7_ns8250();
    test_si7_gpio();
    test_si7_acpi_button();
    test_si7_igb();
    test_si7_em();
    test_si7_em_82574();
    test_si7_re();
    test_si7_igc();
    test_si7_hda();
    test_si7_smbus();
    test_si7_ichwd();
    test_si7_acpi_thermal();
    test_si7_vtnet_mq();
    test_si7_ahci_cam();
    test_si7_nvme_io();
    test_si7_igb_vf();
    test_si7_uart_pci();
    test_si7_acpi_video();
    test_si7_cam_integration();
    test_si7_new_descriptors();
    test_si7_burst9_descriptors();
    test_si7_engine_deepening();
    test_wave4_net_stack();
    test_si8_vmhandle_isolation();
    test_wave4_kernel_tables();
    test_wave5_kernel_ipc();
    test_wave4_io_socket();
    test_wave6_net_fs_geom_zfs();
    test_wave9_kde_theme();
    test_wave7_stand_arch();
    test_wave8_compositor();
    test_wave9_pkg();
    test_hostname_port();

    if (failures == 0) {
        std::printf("SI harness: ALL PASS\n");
        return 0;
    }
    std::printf("SI harness: %d failure(s)\n", failures);
    return 1;
}
