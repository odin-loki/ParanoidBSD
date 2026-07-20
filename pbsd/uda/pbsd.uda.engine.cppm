module;
#include <concepts>

export module pbsd.uda.engine;

export import pbsd.uda.engine.block;
export import pbsd.uda.engine.net;
export import pbsd.uda.engine.display;
export import pbsd.uda.engine.input;
export import pbsd.uda.engine.sensor;
export import pbsd.uda.engine.crypto;
export import pbsd.uda.engine.usb;
export import pbsd.uda.engine.pci;
export import pbsd.uda.interp;
import pbsd.core;

export namespace pbsd::uda {

template<typename E>
concept Engine = requires(E e, const Descriptor& d) {
    { E::device_class() } -> std::same_as<DeviceClass>;
    { e.probe(d) } -> std::same_as<Status>;
    { e.start() } -> std::same_as<Status>;
    { e.stop() } -> std::same_as<Status>;
    { e.grant() } -> std::same_as<DeviceGrant>;
};

static_assert(Engine<BlockEngine>);
static_assert(Engine<NetEngine>);
static_assert(Engine<DisplayEngine>);
static_assert(Engine<InputEngine>);
static_assert(Engine<SensorEngine>);
static_assert(Engine<CryptoEngine>);
static_assert(Engine<UsbEngine>);
static_assert(Engine<PciEngine>);

} // namespace pbsd::uda
