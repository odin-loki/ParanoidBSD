module;
#include <cstdint>

export module pbsd.stand.efi.protocols;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/efi/include — EFI protocol GUIDs (subset).
export namespace pbsd::stand::efi::protocols {

struct Guid {
    std::uint32_t data1{};
    std::uint16_t data2{};
    std::uint16_t data3{};
    std::uint8_t  data4[8]{};
};

enum class Protocol : unsigned int {
    LoadedImage              = 0,
    DevicePath               = 1,
    DevicePathToText         = 2,
    DevicePathFromText       = 3,
    GraphicsOutput           = 4,
    SimpleNetwork            = 5,
    SimplePointer            = 6,
    AbsolutePointer          = 7,
    ConsoleControl           = 8,
    BlockIo                  = 9,
    SimpleFileSystem         = 10,
    LoadedImageDevicePath    = 11,
    Ip4                      = 12,
    Ip6                      = 13,
    Count,
};

struct ProtocolEntry {
    Protocol    proto{};
    Guid        guid{};
    const char* name{};
};

inline constexpr ProtocolEntry kProtocolTable[] = {
    {Protocol::LoadedImage, {0x5b1b31a1, 0x9562, 0x11d2, {0x8e, 0x3f, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}, "LoadedImage"},
    {Protocol::DevicePath, {0x09576e91, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}, "DevicePath"},
    {Protocol::GraphicsOutput, {0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}}, "GraphicsOutput"},
    {Protocol::SimpleNetwork, {0xa19832b9, 0xac25, 0x11d3, {0x9a, 0x2d, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d}}, "SimpleNetwork"},
    {Protocol::SimplePointer, {0x31878c87, 0x0b75, 0x11d5, {0x9a, 0x4f, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d}}, "SimplePointer"},
    {Protocol::ConsoleControl, {0xf42f7782, 0x012e, 0x4c12, {0x99, 0x56, 0x49, 0xf9, 0x43, 0x04, 0xf7, 0x21}}, "ConsoleControl"},
    {Protocol::BlockIo, {0x964e5b21, 0x6459, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}, "BlockIo"},
    {Protocol::SimpleFileSystem, {0x964e5b22, 0x6459, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}, "SimpleFileSystem"},
    {Protocol::LoadedImageDevicePath, {0xbc62157e, 0x3e33, 0x4fec, {0x99, 0x20, 0x2d, 0x3b, 0x36, 0xd7, 0x50, 0xdf}}, "LoadedImageDevicePath"},
    {Protocol::DevicePathToText, {0x8b843e20, 0x8132, 0x4854, {0x90, 0xcc, 0x55, 0x1a, 0x4e, 0x4a, 0x7f, 0x1c}}, "DevicePathToText"},
    {Protocol::DevicePathFromText, {0x05c99a21, 0xc70f, 0x4bdb, {0x88, 0xcb, 0x23, 0x0a, 0x4e, 0x4a, 0x7f, 0x1c}}, "DevicePathFromText"},
};

[[nodiscard]] inline constexpr std::size_t protocol_table_size() noexcept {
    return sizeof(kProtocolTable) / sizeof(kProtocolTable[0]);
}

[[nodiscard]] inline constexpr Guid guid_for(Protocol p) noexcept {
    for (auto const& e : kProtocolTable) {
        if (e.proto == p) {
            return e.guid;
        }
    }
    return {};
}

[[nodiscard]] inline Status validate_protocol(Protocol p) noexcept {
    if (p >= Protocol::Count) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline constexpr bool guid_equal(Guid const& a, Guid const& b) noexcept {
    if (a.data1 != b.data1 || a.data2 != b.data2 || a.data3 != b.data3) {
        return false;
    }
    for (std::size_t i = 0; i < 8; ++i) {
        if (a.data4[i] != b.data4[i]) {
            return false;
        }
    }
    return true;
}

} // namespace pbsd::stand::efi::protocols
