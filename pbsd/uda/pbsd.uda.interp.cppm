module;
#include <cstdint>
#include <span>

export module pbsd.uda.interp;

export import pbsd.uda.schema;
import pbsd.core;

export namespace pbsd::uda {

/// Hosted/sim backend for Wave 1 tests; kernel maps real MMIO in Wave 5.
struct SoftMmio {
    static constexpr std::size_t kSize = 0x6000;
    std::uint8_t bytes[kSize]{};
    bool uart_dlab_{false};

    void write32(std::uint32_t off, std::uint32_t v) noexcept {
        if (off + 4 > kSize) {
            return;
        }
        const std::uint32_t prev = read32(off);
        if (off == 0x37 && (v & 0x10u) != 0u) { // re/rl Command reset self-clears
            v = 0;
        }
        bytes[off] = static_cast<std::uint8_t>(v);
        bytes[off + 1] = static_cast<std::uint8_t>(v >> 8);
        bytes[off + 2] = static_cast<std::uint8_t>(v >> 16);
        bytes[off + 3] = static_cast<std::uint8_t>(v >> 24);

        // Hosted device model hooks for SI-7 init bytecode (Wave 5 descriptors).
        if (off == 0x14) { // NVMe CC
            const std::uint32_t csts_off = 0x1C;
            if ((v & 0x1u) != 0u) {
                write32(csts_off, read32(csts_off) | 0x1u);
            } else {
                write32(csts_off, read32(csts_off) & ~0x1u);
            }
        }
        if (off == 0x04) {
            if (v == 0x1u) { // virtio-scsi-cam enable
                bytes[0] = 0x02;
                bytes[1] = 0;
                bytes[2] = 0;
                bytes[3] = 0;
            } else if ((v & 0x80000001u) == 0x80000001u) { // AHCI GHC AE|HR self-clear
                const std::uint32_t cleared = read32(off) & ~0x1u;
                bytes[off] = static_cast<std::uint8_t>(cleared);
                bytes[off + 1] = static_cast<std::uint8_t>(cleared >> 8);
                bytes[off + 2] = static_cast<std::uint8_t>(cleared >> 16);
                bytes[off + 3] = static_cast<std::uint8_t>(cleared >> 24);
            }
        }
        // MPT MPI doorbell @ 0x00 — InitHost always transitions to Ready (shared SoftMmio OK).
        if (off == 0x00 && v == 0x04u) {
            constexpr std::uint32_t ready = 0x10000000u;
            bytes[0] = static_cast<std::uint8_t>(ready);
            bytes[1] = static_cast<std::uint8_t>(ready >> 8);
            bytes[2] = static_cast<std::uint8_t>(ready >> 16);
            bytes[3] = static_cast<std::uint8_t>(ready >> 24);
            bytes[0x30] = 0x08;
            bytes[0x31] = 0;
            bytes[0x32] = 0;
            bytes[0x33] = 0;
        } else if (off == 0x00 && (v & 0xF0000000u) == 0x20000000u) {
            bytes[0x30] = 0x08;
            bytes[0x31] = 0;
            bytes[0x32] = 0;
            bytes[0x33] = 0;
        }
        (void)prev;
        if ((off >= 1 && off <= 0x03) || (off == 0x04 && v <= 0xFFu)
            || (off == 0 && uart_dlab_ && v <= 0xFFu)) { // ns8250/uart init
            if (off == 3) {
                uart_dlab_ = (v & 0x80u) != 0u;
            }
            const std::uint32_t lsr_off = 0x05;
            bytes[lsr_off] = 0x60;
            bytes[lsr_off + 1] = 0;
            bytes[lsr_off + 2] = 0;
            bytes[lsr_off + 3] = 0;
        }
        if ((off & ~0x7Fu) == 0x100u && (off & 0x7Fu) == 0x18u) { // AHCI port PCmd
            const std::uint32_t pssts_off = (off & ~0x7Fu) + 0x28u;
            bytes[pssts_off] = 0x03;
            bytes[pssts_off + 1] = 0;
            bytes[pssts_off + 2] = 0;
            bytes[pssts_off + 3] = 0;
        }
        if (off == 0x70) { // virtio-mmio STATUS — already stored above
            (void)0;
        }
        // USB host controllers: USBCMD @ 0x00 (skip UART divisor latches, virtio magic, MPT doorbell, AHCI CAP).
        if (off == 0x00 && !uart_dlab_ && v <= 0xFFFFu && (v & 0xFF000000u) == 0u
            && (v & 0xFFFCu) <= 0x3u) {
            if ((v & 0x2u) != 0u) {
                const std::uint32_t cleared = read32(off) & ~0x2u;
                bytes[off] = static_cast<std::uint8_t>(cleared);
                bytes[off + 1] = static_cast<std::uint8_t>(cleared >> 8);
                bytes[off + 2] = static_cast<std::uint8_t>(cleared >> 16);
                bytes[off + 3] = static_cast<std::uint8_t>(cleared >> 24);
            }
            if ((v & 0x1u) != 0u) {
                const std::uint32_t sts_off = 0x04;
                const std::uint32_t cleared = read32(sts_off) & ~0x1u;
                bytes[sts_off] = static_cast<std::uint8_t>(cleared);
                bytes[sts_off + 1] = static_cast<std::uint8_t>(cleared >> 8);
                bytes[sts_off + 2] = static_cast<std::uint8_t>(cleared >> 16);
                bytes[sts_off + 3] = static_cast<std::uint8_t>(cleared >> 24);
                // EHCI HCH @ USBSTS bit 12
                const std::uint32_t ehci_cleared = read32(sts_off) & ~0x1000u;
                bytes[sts_off] = static_cast<std::uint8_t>(ehci_cleared);
                bytes[sts_off + 1] = static_cast<std::uint8_t>(ehci_cleared >> 8);
                bytes[sts_off + 2] = static_cast<std::uint8_t>(ehci_cleared >> 16);
                bytes[sts_off + 3] = static_cast<std::uint8_t>(ehci_cleared >> 24);
            }
            // UHCI RS clears HCH @ STS offset 0x02 bit 5
            if ((v & 0x1u) != 0u) {
                const std::uint32_t uhci_sts = read32(0x02) & ~0x20u;
                bytes[0x02] = static_cast<std::uint8_t>(uhci_sts);
                bytes[0x03] = static_cast<std::uint8_t>(uhci_sts >> 8);
            }
        }
    }

    [[nodiscard]] std::uint8_t read8(std::uint32_t off) const noexcept {
        if (off >= kSize) {
            return 0;
        }
        return bytes[off];
    }

    [[nodiscard]] std::uint16_t read16(std::uint32_t off) const noexcept {
        if (off + 2 > kSize) {
            return 0;
        }
        return static_cast<std::uint16_t>(bytes[off])
             | (static_cast<std::uint16_t>(bytes[off + 1]) << 8);
    }

    [[nodiscard]] std::uint32_t read32(std::uint32_t off) const noexcept {
        if (off + 4 > kSize) {
            return 0;
        }
        return static_cast<std::uint32_t>(bytes[off])
             | (static_cast<std::uint32_t>(bytes[off + 1]) << 8)
             | (static_cast<std::uint32_t>(bytes[off + 2]) << 16)
             | (static_cast<std::uint32_t>(bytes[off + 3]) << 24);
    }

    void write8(std::uint32_t off, std::uint8_t v) noexcept {
        if (off >= kSize) {
            return;
        }
        bytes[off] = v;
    }

    void write16(std::uint32_t off, std::uint16_t v) noexcept {
        if (off + 2 > kSize) {
            return;
        }
        bytes[off] = static_cast<std::uint8_t>(v);
        bytes[off + 1] = static_cast<std::uint8_t>(v >> 8);
    }
};

class BytecodeInterpreter {
public:
    explicit BytecodeInterpreter(DeviceGrant grant, SoftMmio& mem) noexcept
        : grant_(grant), mem_(mem) {}

    explicit BytecodeInterpreter(MmioCapability cap, SoftMmio& mem) noexcept
        : grant_{cap, {}}, mem_(mem) {}

    [[nodiscard]] Status run(std::span<const RegInsn> seq) noexcept {
        if (!grant_.mmio_valid()) {
            return Status::Invalid;
        }
        std::uint32_t acc = 0;
        for (const RegInsn& insn : seq) {
            switch (insn.op) {
            case RegOp::Done:
                return Status::Ok;
            case RegOp::Write8:
            case RegOp::Write16:
            case RegOp::Write32: {
                const std::size_t sz = insn.op == RegOp::Write8 ? 1u
                                      : insn.op == RegOp::Write16 ? 2u
                                                                  : 4u;
                if (!grant_.mmio.contains(insn.offset, sz)) {
                    return Status::Denied; // SI-7
                }
                mem_.write32(insn.offset, insn.value);
                break;
            }
            case RegOp::Read8:
            case RegOp::Read32: {
                if (!grant_.mmio.contains(insn.offset, 4)) {
                    return Status::Denied;
                }
                acc = mem_.read32(insn.offset);
                (void)acc;
                break;
            }
            case RegOp::WaitUs:
                break;
            case RegOp::CheckEq: {
                if (!grant_.mmio.contains(insn.offset, 4)) {
                    return Status::Denied;
                }
                const auto got = mem_.read32(insn.offset) & insn.mask;
                if (got != (insn.value & insn.mask)) {
                    return Status::Timeout;
                }
                break;
            }
            default:
                return Status::Protocol;
            }
        }
        return Status::Ok;
    }

private:
    DeviceGrant grant_{};
    SoftMmio&   mem_;
};

} // namespace pbsd::uda
