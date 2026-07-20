module;
#include <cstddef>
#include <cstdint>

export module pbsd.geom;

import pbsd.core;
export import pbsd.handles;
export import pbsd.geom.ctl;
export import pbsd.geom.disk;
export import pbsd.geom.slice;
export import pbsd.geom.flashmap;
export import pbsd.geom.part;
export import pbsd.geom.mirror;
export import pbsd.geom.label;
export import pbsd.geom.taste;
export import pbsd.geom.concat;
export import pbsd.geom.stripe;
export import pbsd.geom.raid3;
export import pbsd.geom.eli;
export import pbsd.geom.journal;
export import pbsd.geom.gate;
export import pbsd.geom.multipath;
export import pbsd.geom.cache;
export import pbsd.geom.shsec;
export import pbsd.geom.virstor;
export import pbsd.geom.raid;
export import pbsd.geom.mountver;
export import pbsd.geom.nop;
export import pbsd.geom.zero;
export import pbsd.geom.uzip;
export import pbsd.geom.gunion;
export import pbsd.geom.shred;
export import pbsd.geom.linux_lvm;
export import pbsd.geom.subr;
export import pbsd.geom.io;
export import pbsd.geom.event;
export import pbsd.geom.vfs;
export import pbsd.geom.ccd;
export import pbsd.geom.bsd;
export import pbsd.geom.core;
export import pbsd.geom.bsdlabel;
export import pbsd.geom.dump;

export import pbsd.geom.part_gpt;
export import pbsd.geom.part_mbr;
export import pbsd.geom.eli_key;
export import pbsd.geom.journal_ufs;
export import pbsd.geom.mirror_ctl;
export import pbsd.geom.raid_ctl;
export import pbsd.geom.label_gpt;
export import pbsd.geom.label_ufs;
export import pbsd.geom.orphan;
export import pbsd.geom.up;
export import pbsd.geom.subr_disk;
export import pbsd.geom.eli_crypto;
export import pbsd.geom.eli_ctl;
export import pbsd.geom.eli_hmac;
export import pbsd.geom.eli_integrity;
export import pbsd.geom.eli_key_cache;
export import pbsd.geom.eli_privacy;
export import pbsd.geom.pkcs5v2;
export import pbsd.geom.geom_kern;
export import pbsd.geom.geom_dev;
export import pbsd.geom.label_disk_ident;
export import pbsd.geom.label_ext2fs;
export import pbsd.geom.label_flashmap;
export import pbsd.geom.label_iso9660;
export import pbsd.geom.label_msdosfs;
export import pbsd.geom.label_reiserfs;
export import pbsd.geom.label_ufs_id;
export import pbsd.geom.geom_bsd_enc;
export import pbsd.geom.label_ntfs;
export import pbsd.geom.label_swaplinux;
export import pbsd.geom.label_swap;
export import pbsd.geom.label_vtoc8;
export import pbsd.geom.label_gpt_uuid;
export import pbsd.geom.multipath_ctl;
export import pbsd.geom.raid_cls;
export import pbsd.geom.raid_tr;
export import pbsd.geom.journal_core;
export import pbsd.geom.part_apm;
export import pbsd.geom.part_bsd;
export import pbsd.geom.raid_tr_raid5;
export import pbsd.geom.uzip_lzma;
export import pbsd.geom.raid_tr_raid0;
export import pbsd.geom.raid_tr_raid1;
export import pbsd.geom.raid_tr_raid1e;
export import pbsd.geom.raid_tr_concat;
export import pbsd.geom.raid_md_intel;
export import pbsd.geom.raid_md_ddf;
export import pbsd.geom.raid_md_nvidia;
export import pbsd.geom.raid_md_promise;
export import pbsd.geom.raid_md_sii;
export import pbsd.geom.raid_md_jmicron;
export import pbsd.geom.part_ldm;
export import pbsd.geom.part_ebr;
export import pbsd.geom.part_bsd64;
export import pbsd.geom.raid3_ctl;
export import pbsd.geom.uzip_zstd;
export import pbsd.geom.uzip_zlib;
export import pbsd.geom.uzip_wrkthr;
export import pbsd.geom.virstor_md;
/// Wave 6 — GEOM provider/consumer façade
export namespace pbsd::geom {

struct ProviderObject {
    static void release(ProviderObject* p) noexcept { (void)p; }
};

using ProviderHandle = UniqueHandle<ProviderObject>;

struct GeomLabel {
    const char* name{};
    std::size_t sector_size{512};
    unsigned long long sector_count{0};
    ctl::GeomFlag flags{ctl::GeomFlag::Wither};
};

class GeomStack {
public:
    [[nodiscard]] Result<ProviderHandle> publish(GeomLabel label, CapabilityRights rights,
                                                 LineageId lineage) noexcept {
        if (label.name == nullptr || label.sector_count == 0) {
            return {Status::Invalid, ProviderHandle{}};
        }
        if (!has_right(rights, CapabilityRights::Read)
            || !has_right(rights, CapabilityRights::Write)) {
            return {Status::Denied, ProviderHandle{}};
        }
        (void)label;
        return {Status::Ok, ProviderHandle{reinterpret_cast<ProviderObject*>(1), rights, lineage}};
    }

    [[nodiscard]] Status consume(const ProviderHandle& prov) noexcept {
        if (!prov.valid() || !prov.has_right(CapabilityRights::Read)) {
            return Status::Denied;
        }
        return Status::Ok;
    }

    /// GEOM_CTL ioctl path — validates gctl_req header before dispatch.
    [[nodiscard]] Status ioctl_ctl(const ProviderHandle& prov,
                                   ctl::ReqHeader const& req,
                                   bool writing) noexcept {
        if (!prov.valid()) {
            return Status::Invalid;
        }
        if (ctl::validate_req(req) != Status::Ok) {
            return Status::Protocol;
        }
        if (writing && !prov.has_right(CapabilityRights::Write)) {
            return Status::Denied;
        }
        if (!writing && !prov.has_right(CapabilityRights::Read)) {
            return Status::Denied;
        }
        return Status::Ok;
    }

private:
    bool active_{false};
};

} // namespace pbsd::geom
