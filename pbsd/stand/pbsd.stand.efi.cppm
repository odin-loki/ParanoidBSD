module;
#include <cstddef>
#include <cstdint>

export module pbsd.stand.efi;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/efi/include — EFI boot services / memory map constants.
export namespace pbsd::stand::efi {

inline constexpr std::uint64_t kPageSize     = 4096;
inline constexpr std::uint64_t kPageMask     = kPageSize - 1;
inline constexpr std::uint32_t kMemoryUc    = 0x00000001;
inline constexpr std::uint32_t kMemoryWc    = 0x00000002;
inline constexpr std::uint32_t kMemoryWt    = 0x00000004;
inline constexpr std::uint32_t kMemoryWb    = 0x00000008;
inline constexpr std::uint32_t kMemoryRuntime = 0x80000000;

enum class MemoryType : unsigned int {
    Reserved            = 0,
    LoaderCode          = 1,
    LoaderData          = 2,
    BootServicesCode    = 3,
    BootServicesData    = 4,
    RuntimeServicesCode = 5,
    RuntimeServicesData = 6,
    Conventional        = 7,
    Unusable            = 8,
    AcpiReclaim         = 9,
    AcpiNvs             = 10,
    Mmio                = 11,
    MmioPort            = 12,
    PalCode             = 13,
    Persistent          = 14,
    Max,
};

enum class EfiStatus : std::uint64_t {
    Success             = 0,
    LoadError           = 1 | (1ull << 63),
    InvalidParameter    = 2 | (1ull << 63),
    Unsupported         = 3 | (1ull << 63),
    BadBufferSize       = 4 | (1ull << 63),
    BufferTooSmall      = 5 | (1ull << 63),
    NotReady            = 6 | (1ull << 63),
    OutOfResources      = 9 | (1ull << 63),
};

struct MemoryDescriptor {
    std::uint32_t type{};
    std::uint32_t pad{};
    std::uint64_t physical_start{};
    std::uint64_t virtual_start{};
    std::uint64_t page_count{};
    std::uint64_t attribute{};
};

struct TableHeader {
    std::uint64_t signature{};
    std::uint32_t revision{};
    std::uint32_t header_size{};
    std::uint32_t crc32{};
    std::uint32_t reserved{};
};

inline constexpr std::uint64_t kSystemTableSignature = 0x5453595320494249ull; // IBI SYST
inline constexpr std::uint64_t kBootServicesGuidLo   = 0x961578bf935c4122ull;
inline constexpr std::uint64_t kBootServicesGuidHi   = 0x564e3789d4217dc9ull;
inline constexpr std::uint64_t kRuntimeServicesGuidLo = 0x7a49bf935c4122ull;
inline constexpr std::uint64_t kRuntimeServicesGuidHi = 0x564ef5639d7217dcull;

enum class BootService : unsigned int {
    AllocatePages       = 0,
    FreePages           = 1,
    GetMemoryMap        = 2,
    AllocatePool        = 3,
    ExitBootServices    = 4,
    LoadImage           = 5,
    StartImage          = 6,
    SetWatchdogTimer    = 7,
    Stall               = 8,
};

[[nodiscard]] inline constexpr std::size_t pages_to_size(std::uint64_t pages) noexcept {
    return static_cast<std::size_t>(pages * kPageSize);
}

[[nodiscard]] inline constexpr std::uint64_t size_to_pages(std::size_t bytes) noexcept {
    return (static_cast<std::uint64_t>(bytes) + kPageMask) / kPageSize;
}

[[nodiscard]] inline Status validate_memory_descriptor(MemoryDescriptor const& d) noexcept {
    if (d.type >= static_cast<std::uint32_t>(MemoryType::Max)) {
        return Status::Invalid;
    }
    if (d.page_count == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline constexpr bool is_runtime_memory(MemoryDescriptor const& d) noexcept {
    return (d.attribute & kMemoryRuntime) != 0;
}

[[nodiscard]] inline constexpr bool efi_error(std::uint64_t code) noexcept {
    return (code & (1ull << 63)) != 0;
}

[[nodiscard]] inline constexpr bool is_loader_memory(MemoryType t) noexcept {
    return t == MemoryType::LoaderCode || t == MemoryType::LoaderData;
}

[[nodiscard]] inline constexpr bool needs_exit_boot_services(BootService svc) noexcept {
    return svc == BootService::ExitBootServices;
}

} // namespace pbsd::stand::efi
