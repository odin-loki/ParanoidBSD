# SPEC: UDA Descriptor Schema & Interpreter

**Status:** Frozen for Wave 1–5  
**Invariants:** SI-7  
**Modules:** `pbsd.uda.schema`, `pbsd.uda.interp`, `pbsd.uda.engine`  
**Port context:** ParanoidBSD full C++23 port — UDA replaces ad-hoc C driver probe tables with declarative, provenance-bound descriptors.

## Descriptor

Declarative chip description only. Populated from public datasheets / standards (PROVENANCE). Never derived from existing driver source.

| Field | Type | Required | Notes |
|-------|------|----------|-------|
| `name` | `string_view` | yes | Stable id, e.g. `virtio-blk-1.0` |
| `provenance` | `string_view` | yes | PROVENANCE key / citation |
| `device_class` | `DeviceClass` | yes | Block, Net, Hid, … |
| `vendor_id` | `uint32_t` | optional | PCI/USB id when applicable |
| `device_id` | `uint32_t` | optional | |
| `init_sequence` | `span<const RegInsn>` | yes | Bytecode run at attach |
| `reset_sequence` | `span<const RegInsn>` | optional | Run before init on reset |

Descriptors are immutable at runtime — stored in `.rodata` or constexpr-generated BMIs.

## Bytecode (`RegOp`)

| Op | Operands | Effect |
|----|----------|--------|
| `Write8/16/32` | offset, value | MMIO store relative to grant base |
| `Read8/32` | offset, dest | MMIO load |
| `WaitUs` | microseconds | Bounded busy-wait |
| `CheckEq` | offset, expect | Fail attach if mismatch |
| `Done` | — | End sequence |

All offsets are relative to the granted MMIO window. Absolute physical addresses in descriptors are forbidden (SI-7).

## Capabilities

The interpreter receives:

- `MmioCapability` — `{ base_offset, length }` window into device BAR
- `IrqCapability` — optional MSI/legacy line binding

The engine never maps MMIO itself; the host kernel grants windows before `Engine::start()`.

## SI-7 enforcement

Interpreter executes only against granted capabilities. Out-of-bounds access returns `Status::Denied` — never panics, never touches host memory outside the grant.

## Engine concept (static polymorphism)

```cpp
template<typename E>
concept Engine = requires(E e, const Descriptor& d) {
    { E::device_class() } -> std::same_as<DeviceClass>;
    { e.probe(d) } -> std::same_as<Status>;
    { e.start() } -> std::same_as<Status>;
    { e.stop() } -> std::same_as<Status>;
};
```

Engines compose the interpreter; device-specific logic stays in constexpr descriptor tables where possible.

## Wave plan

| Wave | Deliverable |
|------|-------------|
| 1 | Schema + interpreter + `SoftMmio` test double |
| 5 | `virtio_blk` descriptor; first in-tree attach via kmod |
| 6+ | Net / FS class engines |

## Proof artifacts

- `pbsd/uda/descriptors/virtio_blk.cppm` — OASIS virtio 1.2 §5.2 descriptor
- `pbsd/tests/si_harness.cpp` — SI-7 out-of-bounds denial test
