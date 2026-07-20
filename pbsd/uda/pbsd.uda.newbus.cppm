module;
#include <concepts>
#include <cstdint>

export module pbsd.uda.newbus;

export import pbsd.uda.schema;
export import pbsd.uda.engine;
import pbsd.core;

/// Wave 5 — newbus-typed probe/attach concepts (maps to hbsd device_t KPI).
export namespace pbsd::uda::newbus {

using DeviceHandle = std::uintptr_t;
inline constexpr DeviceHandle kNoDevice = 0;

enum class BusPass : unsigned char {
    First  = 0,
    Second = 1,
    Last   = 2,
};

struct DeviceState {
    DeviceHandle  handle{kNoDevice};
    DeviceClass   cls{DeviceClass::Block};
    std::uint16_t vendor_id{};
    std::uint16_t device_id{};
    bool          attached{false};
};

template<typename D>
concept ProbeAttachDriver = requires(D d, DeviceState& dev, const Descriptor& desc) {
    { d.probe(dev, desc) } -> std::same_as<Status>;
    { d.attach(dev, desc) } -> std::same_as<Status>;
    { d.detach(dev) } -> std::same_as<Status>;
};

template<Engine E>
class TypedNewbusNode {
public:
    explicit TypedNewbusNode(E engine) noexcept : engine_(engine) {}

    [[nodiscard]] Status probe(DeviceState& dev, const Descriptor& desc) noexcept {
        if (desc.device_class != E::device_class()) {
            return Status::NotFound;
        }
        dev.vendor_id = desc.vendor_id;
        dev.device_id = desc.device_id;
        dev.cls       = desc.device_class;
        return engine_.probe(desc);
    }

    [[nodiscard]] Status attach(DeviceState& dev, const Descriptor& desc) noexcept {
        auto st = engine_.start();
        if (st == Status::Ok) {
            dev.handle   = reinterpret_cast<DeviceHandle>(&dev);
            dev.attached = true;
        }
        return st;
    }

    [[nodiscard]] Status detach(DeviceState& dev) noexcept {
        if (!dev.attached) {
            return Status::Invalid;
        }
        auto st = engine_.stop();
        dev.attached = false;
        dev.handle   = kNoDevice;
        return st;
    }

private:
    E engine_;
};

} // namespace pbsd::uda::newbus
