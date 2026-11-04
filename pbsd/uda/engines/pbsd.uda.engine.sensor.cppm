module;
#include <concepts>
#include <cstdint>

export module pbsd.uda.engine.sensor;

export import pbsd.uda.interp;
import pbsd.core;

export namespace pbsd::uda {

enum class SensorKind : std::uint8_t {
    Entropy  = 0,
    Balloon  = 1,
    Gpio     = 2,
    Button   = 3,
    Thermal  = 4,
    Watchdog = 5,
    Smbus    = 6,
};

struct EntropyRequest {
    std::uint32_t bytes{};
};

struct BalloonStat {
    std::uint16_t tag{};
    std::uint64_t val{};
};

struct SensorEvent {
    SensorKind    kind{};
    std::uint8_t  notify_code{};
    std::uint32_t payload{};
};

struct GpioEdge {
    std::uint8_t pin{};
    bool         rising{};
};

class SensorEngine {
public:
    static constexpr std::size_t kMaxPending = 16;

    [[nodiscard]] static constexpr DeviceClass device_class() noexcept {
        return DeviceClass::Sensor;
    }

    explicit SensorEngine(SoftMmio& mem, DeviceGrant grant = {}) noexcept
        : mem_(mem), grant_(grant.mmio_valid() ? grant
                                               : DeviceGrant{{0, SoftMmio::kSize}, {}}) {}

    [[nodiscard]] Status probe(const Descriptor& d) noexcept {
        if (validate_descriptor(d) != Status::Ok) {
            return Status::Invalid;
        }
        if (d.device_class != DeviceClass::Sensor) {
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
        entropy_pending_ = 0;
        event_count_ = 0;
        watchdog_pets_ = 0;
        BytecodeInterpreter interp{grant_, mem_};
        return interp.run(desc_->reset_sequence);
    }

    [[nodiscard]] DeviceGrant grant() const noexcept { return grant_; }

    [[nodiscard]] Status submit_entropy(EntropyRequest req) noexcept {
        if (!running_) {
            return Status::Invalid;
        }
        if (req.bytes == 0 || req.bytes > 4096) {
            return Status::Invalid;
        }
        if (entropy_pending_ >= kMaxPending) {
            return Status::Busy;
        }
        entropy_queue_[entropy_pending_++] = req;
        return Status::Ok;
    }

    [[nodiscard]] Status submit_balloon_stat(BalloonStat stat) noexcept {
        if (!running_) {
            return Status::Invalid;
        }
        if (event_count_ >= kMaxPending) {
            return Status::Busy;
        }
        events_[event_count_++] = SensorEvent{SensorKind::Balloon, static_cast<std::uint8_t>(stat.tag),
                                              static_cast<std::uint32_t>(stat.val & 0xFFFFFFFFu)};
        return Status::Ok;
    }

    [[nodiscard]] Status inject_event(SensorEvent ev) noexcept {
        if (!running_ || event_count_ >= kMaxPending) {
            return Status::Busy;
        }
        events_[event_count_++] = ev;
        return Status::Ok;
    }

    [[nodiscard]] Status poll_event(SensorEvent* out) noexcept {
        if (!running_ || event_count_ == 0) {
            return Status::NotFound;
        }
        if (out) {
            *out = events_[0];
        }
        for (std::size_t i = 1; i < event_count_; ++i) {
            events_[i - 1] = events_[i];
        }
        --event_count_;
        return Status::Ok;
    }

    [[nodiscard]] Status complete_entropy(EntropyRequest* out) noexcept {
        if (!running_ || entropy_pending_ == 0) {
            return Status::NotFound;
        }
        if (out) {
            *out = entropy_queue_[0];
        }
        for (std::size_t i = 1; i < entropy_pending_; ++i) {
            entropy_queue_[i - 1] = entropy_queue_[i];
        }
        --entropy_pending_;
        return Status::Ok;
    }

    [[nodiscard]] Status submit_thermal(std::int32_t tenths_kelvin,
                                      std::uint8_t notify_code) noexcept {
        if (!running_) {
            return Status::Invalid;
        }
        return inject_event(
            SensorEvent{SensorKind::Thermal, notify_code,
                        static_cast<std::uint32_t>(tenths_kelvin)});
    }

    [[nodiscard]] Status pet_watchdog() noexcept {
        if (!running_) {
            return Status::Invalid;
        }
        ++watchdog_pets_;
        return inject_event(
            SensorEvent{SensorKind::Watchdog, 0,
                        static_cast<std::uint32_t>(watchdog_pets_)});
    }

    [[nodiscard]] Status submit_smbus(std::uint8_t addr, std::uint8_t cmd) noexcept {
        if (!running_) {
            return Status::Invalid;
        }
        if (addr < 0x10 || addr >= 0x70) {
            return Status::Invalid;
        }
        return inject_event(
            SensorEvent{SensorKind::Smbus, cmd,
                        static_cast<std::uint32_t>(addr)});
    }

    [[nodiscard]] Status submit_gpio_edge(std::uint8_t pin, bool rising) noexcept {
        if (!running_) {
            return Status::Invalid;
        }
        if (gpio_edge_count_ >= kMaxPending) {
            return Status::Busy;
        }
        gpio_edges_[gpio_edge_count_++] = GpioEdge{pin, rising};
        return inject_event({SensorKind::Gpio, pin, rising ? 1u : 0u});
    }

    [[nodiscard]] Status poll_gpio_edge(GpioEdge* out) noexcept {
        if (!running_ || gpio_edge_count_ == 0) {
            return Status::NotFound;
        }
        if (out) {
            *out = gpio_edges_[0];
        }
        for (std::size_t i = 1; i < gpio_edge_count_; ++i) {
            gpio_edges_[i - 1] = gpio_edges_[i];
        }
        --gpio_edge_count_;
        return Status::Ok;
    }

    [[nodiscard]] std::uint64_t watchdog_pets() const noexcept {
        return watchdog_pets_;
    }

    [[nodiscard]] Status submit_thermal_batch(const std::int32_t* samples,
                                              std::size_t count,
                                              std::uint8_t notify_code) noexcept {
        if (!running_ || samples == nullptr || count == 0) {
            return Status::Invalid;
        }
        for (std::size_t i = 0; i < count; ++i) {
            if (submit_thermal(samples[i], notify_code) != Status::Ok) {
                return Status::Busy;
            }
        }
        return Status::Ok;
    }

private:
    SoftMmio&         mem_;
    DeviceGrant       grant_;
    const Descriptor* desc_{};
    EntropyRequest    entropy_queue_[kMaxPending]{};
    SensorEvent       events_[kMaxPending]{};
    GpioEdge          gpio_edges_[kMaxPending]{};
    std::size_t       entropy_pending_{0};
    std::size_t       event_count_{0};
    std::size_t       gpio_edge_count_{0};
    std::uint64_t     watchdog_pets_{0};
    bool              running_{false};
};

} // namespace pbsd::uda
