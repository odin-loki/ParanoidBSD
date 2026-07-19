// PBSD UDA — Hardware Descriptor Schema
// Author: Odin Loch
// PROVENANCE: Original design. Descriptors populated from public datasheets only.
// Each descriptor MUST have a corresponding PROVENANCE.md entry citing its datasheet.

#pragma once
#include <cstdint>
#include <span>
#include <string_view>

namespace pbsd::uda {

// Register operation types for the bytecode interpreter
enum class RegOp : uint8_t {
    Write8  = 0x01,  // Write 8-bit value to offset
    Write16 = 0x02,
    Write32 = 0x03,
    Read8   = 0x11,  // Read 8-bit value from offset (result in accumulator)
    Read32  = 0x13,
    WaitUs  = 0x20,  // Wait N microseconds
    CheckEq = 0x30,  // Loop until [offset] & mask == expected (poll/wait)
    Done    = 0xFF,  // End of sequence
};

struct RegInsn {
    RegOp    op;
    uint32_t offset;   // MMIO offset from device base
    uint32_t value;    // Write value or expected value for CheckEq
    uint32_t mask;     // Mask for CheckEq operations
    uint32_t timeout;  // Microseconds for CheckEq timeout (0 = no timeout)
};

// Device class
enum class DeviceClass : uint8_t {
    Block   = 0x01,
    Network = 0x02,
    Display = 0x03,
    Input   = 0x04,
    Sensor  = 0x05,
};

// Hardware descriptor — one per chip model
// Populated entirely from public datasheet; never from existing driver source
struct Descriptor {
    std::string_view name;           // e.g. "virtio-blk-1.0"
    std::string_view provenance;     // e.g. "OASIS virtio-v1.2 spec §5.2"
    DeviceClass      device_class;
    uint16_t         vendor_id;      // PCI vendor ID (0 = non-PCI/platform device)
    uint16_t         device_id;      // PCI device ID
    std::span<const RegInsn> init_sequence;   // Bytecode: device initialisation
    std::span<const RegInsn> reset_sequence;  // Bytecode: soft reset
    // Per-class parameters follow in class-specific descriptor subtype
};

} // namespace pbsd::uda
