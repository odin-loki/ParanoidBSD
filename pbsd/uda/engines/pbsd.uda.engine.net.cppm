module;
#include <concepts>
#include <cstdint>

export module pbsd.uda.engine.net;

export import pbsd.uda.interp;
import pbsd.core;

export namespace pbsd::uda {

struct NetTxDesc {
    std::uint8_t  flags{};
    std::uint8_t  gso_type{};
    std::uint16_t hdr_len{};
    std::uint16_t gso_size{};
    std::uint16_t csum_start{};
    std::uint16_t csum_offset{};
    std::uint16_t length{};
};

struct NetRxDesc {
    std::uint16_t length{};
    std::uint8_t  flags{};
    std::uint8_t  gso_type{};
};

class NetEngine {
public:
    static constexpr std::size_t kMaxPending = 32;

    [[nodiscard]] static constexpr DeviceClass device_class() noexcept {
        return DeviceClass::Network;
    }

    explicit NetEngine(SoftMmio& mem, DeviceGrant grant = {}) noexcept
        : mem_(mem), grant_(grant.mmio_valid() ? grant
                                               : DeviceGrant{{0, SoftMmio::kSize}, {}}) {}

    [[nodiscard]] Status probe(const Descriptor& d) noexcept {
        if (validate_descriptor(d) != Status::Ok) {
            return Status::Invalid;
        }
        if (d.device_class != DeviceClass::Network) {
            return Status::Invalid;
        }
        desc_ = &d;
        return Status::Ok;
    }

    [[nodiscard]] Status start() noexcept {
        if (!desc_) {
            return Status::Invalid;
        }
        BytecodeInterpreter interp{grant_, mem_};
        const auto st = interp.run(desc_->init_sequence);
        if (st == Status::Ok) {
            running_ = true;
        }
        return st;
    }

    [[nodiscard]] Status stop() noexcept {
        if (!desc_) {
            return Status::Invalid;
        }
        running_ = false;
        tx_pending_ = 0;
        rx_pending_ = 0;
        BytecodeInterpreter interp{grant_, mem_};
        return interp.run(desc_->reset_sequence);
    }

    [[nodiscard]] DeviceGrant grant() const noexcept { return grant_; }

    [[nodiscard]] Status submit_tx(NetTxDesc desc) noexcept {
        if (!running_) {
            return Status::Invalid;
        }
        if (desc.length == 0) {
            return Status::Invalid;
        }
        if (tx_pending_ >= kMaxPending) {
            return Status::Busy;
        }
        tx_queue_[tx_pending_++] = desc;
        ++tx_count_;
        return Status::Ok;
    }

    [[nodiscard]] Status submit_tso(NetTxDesc desc, std::uint16_t mss) noexcept {
        if (!running_) {
            return Status::Invalid;
        }
        if (desc.length == 0 || mss == 0) {
            return Status::Invalid;
        }
        desc.gso_type = 1;
        desc.gso_size = mss;
        desc.flags |= 0x01;
        return submit_tx(desc);
    }

    [[nodiscard]] Status set_link_up(bool up) noexcept {
        if (!running_) {
            return Status::Invalid;
        }
        link_up_ = up;
        return Status::Ok;
    }

    [[nodiscard]] bool link_up() const noexcept { return link_up_; }

    [[nodiscard]] Status note_rx_csum(bool ok) noexcept {
        if (!running_) {
            return Status::Invalid;
        }
        if (ok) {
            ++rx_csum_ok_;
        } else {
            ++rx_csum_fail_;
        }
        return Status::Ok;
    }

    [[nodiscard]] std::uint64_t rx_csum_ok() const noexcept { return rx_csum_ok_; }
    [[nodiscard]] std::uint64_t rx_csum_fail() const noexcept { return rx_csum_fail_; }

    [[nodiscard]] Status poll_rx(NetRxDesc* out) noexcept {
        if (!running_) {
            return Status::Invalid;
        }
        if (rx_pending_ == 0) {
            return Status::NotFound;
        }
        if (out) {
            *out = rx_queue_[0];
        }
        for (std::size_t i = 1; i < rx_pending_; ++i) {
            rx_queue_[i - 1] = rx_queue_[i];
        }
        --rx_pending_;
        return Status::Ok;
    }

    [[nodiscard]] Status inject_rx(NetRxDesc desc) noexcept {
        if (!running_ || rx_pending_ >= kMaxPending) {
            return Status::Busy;
        }
        rx_queue_[rx_pending_++] = desc;
        return Status::Ok;
    }

    [[nodiscard]] std::uint64_t tx_count() const noexcept { return tx_count_; }

    [[nodiscard]] Status join_multicast(std::uint32_t group) noexcept {
        if (!running_) {
            return Status::Invalid;
        }
        if (group == 0) {
            return Status::Invalid;
        }
        mcast_mask_ |= group;
        ++mcast_joins_;
        return Status::Ok;
    }

    [[nodiscard]] Status leave_multicast(std::uint32_t group) noexcept {
        if (!running_) {
            return Status::Invalid;
        }
        mcast_mask_ &= ~group;
        return Status::Ok;
    }

    [[nodiscard]] bool multicast_member(std::uint32_t group) const noexcept {
        return (mcast_mask_ & group) != 0;
    }

    [[nodiscard]] Status bpf_tap(std::uint16_t length) noexcept {
        if (!running_) {
            return Status::Invalid;
        }
        bpf_tap_bytes_ += length;
        ++bpf_tap_count_;
        return Status::Ok;
    }

    [[nodiscard]] std::uint64_t bpf_tap_count() const noexcept { return bpf_tap_count_; }

    [[nodiscard]] Status set_mtu(std::uint16_t mtu) noexcept {
        if (!running_) {
            return Status::Invalid;
        }
        if (mtu < 68 || mtu > 65536) {
            return Status::Invalid;
        }
        mtu_ = mtu;
        return Status::Ok;
    }

    [[nodiscard]] std::uint16_t mtu() const noexcept { return mtu_; }

    [[nodiscard]] Status submit_rx_batch(const NetRxDesc* descs, std::size_t count) noexcept {
        if (!running_ || descs == nullptr || count == 0) {
            return Status::Invalid;
        }
        if (rx_pending_ + count > kMaxPending) {
            return Status::Busy;
        }
        for (std::size_t i = 0; i < count; ++i) {
            rx_queue_[rx_pending_++] = descs[i];
        }
        return Status::Ok;
    }

    [[nodiscard]] Status complete_tx(NetTxDesc* out) noexcept {
        if (!running_ || tx_pending_ == 0) {
            return Status::NotFound;
        }
        if (out) {
            *out = tx_queue_[0];
        }
        for (std::size_t i = 1; i < tx_pending_; ++i) {
            tx_queue_[i - 1] = tx_queue_[i];
        }
        --tx_pending_;
        return Status::Ok;
    }

private:
    SoftMmio&         mem_;
    DeviceGrant       grant_;
    const Descriptor* desc_{};
    NetTxDesc         tx_queue_[kMaxPending]{};
    NetRxDesc         rx_queue_[kMaxPending]{};
    std::size_t       tx_pending_{0};
    std::size_t       rx_pending_{0};
    std::uint64_t     tx_count_{0};
    std::uint64_t     rx_csum_ok_{0};
    std::uint64_t     rx_csum_fail_{0};
    std::uint64_t     bpf_tap_count_{0};
    std::uint64_t     bpf_tap_bytes_{0};
    std::uint64_t     mcast_joins_{0};
    std::uint32_t     mcast_mask_{0};
    std::uint16_t     mtu_{1500};
    bool              link_up_{false};
    bool              running_{false};
};

} // namespace pbsd::uda
