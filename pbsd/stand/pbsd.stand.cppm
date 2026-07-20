module;
#include <cstddef>
#include <cstdint>

export module pbsd.stand;

import pbsd.core;
export import pbsd.stand.pe;
export import pbsd.stand.efi;
export import pbsd.stand.efi.protocols;
export import pbsd.stand.common;
export import pbsd.stand.bootinfo;
export import pbsd.stand.loader;
export import pbsd.stand.reboot;
export import pbsd.stand.gpt;
export import pbsd.stand.kmod;
export import pbsd.stand.elf;
export import pbsd.stand.env;
export import pbsd.stand.devinfo;
export import pbsd.stand.zfsboot;
export import pbsd.stand.net;
export import pbsd.stand.bios;
export import pbsd.stand.ufs;
export import pbsd.stand.iscsi;
export import pbsd.stand.cd9660;
export import pbsd.stand.cdboot;
export import pbsd.stand.pxe;
export import pbsd.stand.bootp;
export import pbsd.stand.disk;
export import pbsd.stand.bcache;
export import pbsd.stand.modload;
export import pbsd.stand.multiboot;
export import pbsd.stand.geli;
export import pbsd.stand.tftp;
export import pbsd.stand.part;
export import pbsd.stand.gzip;
export import pbsd.stand.nfs;
export import pbsd.stand.pkgfs;
export import pbsd.stand.dosfs;
export import pbsd.stand.ext2fs;
export import pbsd.stand.preload;
export import pbsd.stand.ffs;
export import pbsd.stand.zstd;
export import pbsd.stand.smbfs;
export import pbsd.stand.lua;
export import pbsd.stand.nullfs;
export import pbsd.stand.lz4;
export import pbsd.stand.verify;
export import pbsd.stand.hash;
export import pbsd.stand.uuid;
export import pbsd.stand.install;
export import pbsd.stand.decompress;
export import pbsd.stand.xz;
export import pbsd.stand.bzip2;
export import pbsd.stand.manifest;
export import pbsd.stand.signature;
export import pbsd.stand.chain;
export import pbsd.stand.config;
export import pbsd.stand.crc;
export import pbsd.stand.cpio;
export import pbsd.stand.tar;
export import pbsd.stand.md5;
export import pbsd.stand.sha256;
export import pbsd.stand.fat;
export import pbsd.stand.mbr;
export import pbsd.stand.secureboot;
export import pbsd.stand.veriexec;
export import pbsd.stand.console;
export import pbsd.stand.acpi;

/// Wave 7 — EFI loader façade
/// PROVENANCE: hbsd/src/stand/efi, stand/efi/include/amd64/pe.h
export namespace pbsd::stand {

enum class BootStage : unsigned char {
    Firmware = 0,
    Loader   = 1,
    Kernel   = 2,
};

enum class EfiMemoryType : unsigned char {
    Reserved  = 0,
    LoaderCode = 1,
    LoaderData = 2,
    BootServicesCode = 3,
    BootServicesData = 4,
    RuntimeServicesCode = 5,
    RuntimeServicesData = 6,
    Conventional = 7,
};

struct EfiMemoryDescriptor {
    std::uintptr_t physical_start{};
    std::uintptr_t virtual_start{};
    std::size_t    page_count{};
    EfiMemoryType  type{EfiMemoryType::Reserved};
};

struct BootParams {
    const char*           kernel_path{};
    const char*           init_path{};
    EfiMemoryDescriptor*  memory_map{};
    std::size_t           memory_map_entries{};
    std::size_t           memory_map_desc_size{};
    pe::DosHeader         pe_dos{};
    pe::NtHeaders64       pe_nt{};
    std::uintptr_t        module_load_addr{};
};

class EfiLoader {
public:
    [[nodiscard]] Status init_firmware() noexcept {
        stage_ = BootStage::Firmware;
        return Status::Ok;
    }

    [[nodiscard]] Status enter_loader() noexcept {
        stage_ = BootStage::Loader;
        return Status::Ok;
    }

    [[nodiscard]] Status load_kernel(const BootParams& params) noexcept {
        if (params.kernel_path == nullptr) {
            return Status::Invalid;
        }
        if (pe::validate_dos(params.pe_dos) != Status::Ok) {
            return Status::Invalid;
        }
        if (pe::validate_nt(params.pe_nt) != Status::Ok) {
            return Status::Invalid;
        }
        params_ = params;
        stage_  = BootStage::Kernel;
        return Status::Ok;
    }

    [[nodiscard]] Status exit_boot_services() noexcept {
        if (stage_ != BootStage::Loader) {
            return Status::Invalid;
        }
        boot_services_active_ = false;
        return Status::Ok;
    }

    [[nodiscard]] BootStage stage() const noexcept { return stage_; }
    [[nodiscard]] bool boot_services_active() const noexcept { return boot_services_active_; }
    [[nodiscard]] const BootParams& params() const noexcept { return params_; }

    [[nodiscard]] std::uintptr_t entry_point() const noexcept {
        return params_.module_load_addr + params_.pe_nt.optional_header.address_of_entry_point;
    }

private:
    BootStage  stage_{BootStage::Firmware};
    bool       boot_services_active_{true};
    BootParams params_{};
};

} // namespace pbsd::stand
