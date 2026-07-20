module;
#include <cstddef>
#include <cstdint>

export module pbsd.stand.pe;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/efi/include/amd64/pe.h — PE/COFF loader structures.
export namespace pbsd::stand::pe {

inline constexpr std::uint16_t kDosSignature    = 0x5A4D;      // IMAGE_DOS_SIGNATURE
inline constexpr std::uint32_t kNtSignature     = 0x00004550;  // IMAGE_NT_SIGNATURE
inline constexpr std::uint16_t kFileMachineAmd64 = 0x8664;     // IMAGE_FILE_MACHINE_AMD64
inline constexpr std::uint16_t kFileMachineArm64 = 0xAA64;     // IMAGE_FILE_MACHINE_ARM64

inline constexpr std::uint16_t kFileExecutable   = 0x0002;
inline constexpr std::uint16_t kFileDll          = 0x2000;

struct DosHeader {
    std::uint16_t e_magic{};
    std::uint16_t e_cblp{};
    std::uint16_t e_cp{};
    std::uint16_t e_crlc{};
    std::uint16_t e_cparhdr{};
    std::uint16_t e_minalloc{};
    std::uint16_t e_maxalloc{};
    std::uint16_t e_ss{};
    std::uint16_t e_sp{};
    std::uint16_t e_csum{};
    std::uint16_t e_ip{};
    std::uint16_t e_cs{};
    std::uint16_t e_lfarlc{};
    std::uint16_t e_ovno{};
    std::uint16_t e_res[4]{};
    std::uint16_t e_oemid{};
    std::uint16_t e_oeminfo{};
    std::uint16_t e_res2[10]{};
    std::uint32_t e_lfanew{};
};

struct FileHeader {
    std::uint16_t machine{};
    std::uint16_t number_of_sections{};
    std::uint32_t time_date_stamp{};
    std::uint32_t pointer_to_symbol_table{};
    std::uint32_t number_of_symbols{};
    std::uint16_t size_of_optional_header{};
    std::uint16_t characteristics{};
};

struct DataDirectory {
    std::uint32_t virtual_address{};
    std::uint32_t size{};
};

inline constexpr unsigned kNumberOfDirectoryEntries = 16;

struct OptionalHeader64 {
    std::uint16_t magic{};
    std::uint8_t  major_linker_version{};
    std::uint8_t  minor_linker_version{};
    std::uint32_t size_of_code{};
    std::uint32_t size_of_initialized_data{};
    std::uint32_t size_of_uninitialized_data{};
    std::uint32_t address_of_entry_point{};
    std::uint32_t base_of_code{};
    std::uint64_t image_base{};
    std::uint32_t section_alignment{};
    std::uint32_t file_alignment{};
    std::uint16_t major_os_version{};
    std::uint16_t minor_os_version{};
    std::uint16_t major_image_version{};
    std::uint16_t minor_image_version{};
    std::uint16_t major_subsystem_version{};
    std::uint16_t minor_subsystem_version{};
    std::uint32_t win32_version_value{};
    std::uint32_t size_of_image{};
    std::uint32_t size_of_headers{};
    std::uint32_t checksum{};
    std::uint16_t subsystem{};
    std::uint16_t dll_characteristics{};
    std::uint64_t size_of_stack_reserve{};
    std::uint64_t size_of_stack_commit{};
    std::uint64_t size_of_heap_reserve{};
    std::uint64_t size_of_heap_commit{};
    std::uint32_t loader_flags{};
    std::uint32_t number_of_rva_and_sizes{};
    DataDirectory data_directory[kNumberOfDirectoryEntries]{};
};

struct NtHeaders64 {
    std::uint32_t     signature{};
    FileHeader        file_header{};
    OptionalHeader64  optional_header{};
};

[[nodiscard]] inline Status validate_dos(const DosHeader& hdr) noexcept {
    if (hdr.e_magic != kDosSignature) {
        return Status::Invalid;
    }
    if (hdr.e_lfanew < sizeof(DosHeader)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_nt(const NtHeaders64& hdr) noexcept {
    if (hdr.signature != kNtSignature) {
        return Status::Invalid;
    }
    if ((hdr.file_header.characteristics & kFileExecutable) == 0) {
        return Status::Invalid;
    }
    if (hdr.file_header.machine != kFileMachineAmd64
        && hdr.file_header.machine != kFileMachineArm64) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::stand::pe
