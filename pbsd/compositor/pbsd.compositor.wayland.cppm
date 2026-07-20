module;
#include <cstdint>

export module pbsd.compositor.wayland;

import pbsd.core;

/// PROVENANCE: Wayland core protocol (wayland.xml) + pbsd/theme/plasma/blur-effect.json
export namespace pbsd::compositor::wayland {

/// wl_display interface (requests / events).
enum class DisplayRequest : unsigned int {
    Sync        = 0,
    GetRegistry = 1,
};

enum class DisplayEvent : unsigned int {
    Error    = 0,
    DeleteId = 1,
};

/// wl_registry
enum class RegistryRequest : unsigned int { Bind = 0 };
enum class RegistryEvent : unsigned int {
    Global       = 0,
    GlobalRemove = 1,
};

/// wl_compositor
enum class CompositorRequest : unsigned int {
    CreateSurface = 0,
    CreateRegion  = 1,
};

/// wl_surface (subset)
enum class SurfaceRequest : unsigned int {
    Destroy            = 0,
    Attach             = 1,
    Damage             = 2,
    Frame              = 3,
    SetOpaqueRegion    = 4,
    SetInputRegion     = 5,
    Commit             = 6,
    SetBufferTransform = 7,
    SetBufferScale     = 8,
    DamageBuffer       = 9,
};

enum class SurfaceEvent : unsigned int {
    Enter                    = 0,
    Leave                    = 1,
    PreferredBufferScale     = 2,
    PreferredBufferTransform = 3,
};

/// wl_seat (subset)
enum class SeatRequest : unsigned int {
    GetPointer      = 0,
    GetKeyboard     = 1,
    GetTouch        = 2,
    Release         = 3,
};

enum class SeatCapability : unsigned int {
    Pointer  = 1u << 0,
    Keyboard = 1u << 1,
    Touch    = 1u << 2,
};

/// wl_buffer
enum class BufferRequest : unsigned int {
    Destroy = 0,
    Create  = 1,
};

/// wl_shm
enum class ShmRequest : unsigned int {
    CreatePool = 0,
};

enum class ShmFormat : unsigned int {
    CArgb8888 = 0,
    CXrgb8888 = 1,
};

/// wl_output
enum class OutputRequest : unsigned int {
    Release = 0,
};

enum class OutputEvent : unsigned int {
    Geometry       = 0,
    Mode           = 1,
    Done           = 2,
    Scale          = 3,
    Name           = 4,
    Description    = 5,
};

enum class OutputSubpixel : unsigned int {
    Unknown       = 0,
    None          = 1,
    HorizontalRgb = 2,
    HorizontalBgr = 3,
    VerticalRgb   = 4,
    VerticalBgr   = 5,
};

enum class OutputTransform : unsigned int {
    Normal     = 0,
    Rotate90   = 1,
    Rotate180  = 2,
    Rotate270  = 3,
    Flipped    = 4,
    Flipped90  = 5,
    Flipped180 = 6,
    Flipped270 = 7,
};

/// xdg_popup
enum class XdgPopupRequest : unsigned int {
    Destroy        = 0,
    Grab           = 1,
    Reposition     = 2,
};

enum class XdgPopupEvent : unsigned int {
    Configure = 0,
    PopupDone = 1,
};

/// wl_data_device
enum class DataDeviceRequest : unsigned int {
    StartDrag = 0,
    SetSelection = 1,
};

enum class DataDeviceEvent : unsigned int {
    DataOffer = 0,
    Enter     = 1,
    Leave     = 2,
    Motion    = 3,
    Drop      = 4,
    Selection = 5,
};

/// wl_pointer
enum class PointerRequest : unsigned int {
    SetCursor = 0,
    Release   = 1,
};

enum class PointerEvent : unsigned int {
    Enter  = 0,
    Leave  = 1,
    Motion = 2,
    Button = 3,
    Axis   = 4,
    Frame  = 5,
    AxisSource = 6,
    AxisStop   = 7,
    AxisDiscrete = 8,
};

/// wl_keyboard
enum class KeyboardRequest : unsigned int {
    Release = 0,
};

enum class KeyboardEvent : unsigned int {
    Keymap    = 0,
    Enter     = 1,
    Leave     = 2,
    Key       = 3,
    Modifiers = 4,
    RepeatInfo = 5,
};

/// xdg_wm_base
enum class XdgWmBaseRequest : unsigned int {
    Destroy              = 0,
    CreatePositioner     = 1,
    CreateXdgSurface     = 2,
    CreateXdgPopup       = 3,
    Pong                 = 4,
};

enum class XdgWmBaseEvent : unsigned int {
    Ping = 0,
};

/// xdg_surface
enum class XdgSurfaceRequest : unsigned int {
    Destroy        = 0,
    GetToplevel    = 1,
    GetPopup       = 2,
    SetWindowGeometry = 3,
    AckConfigure   = 4,
};

enum class XdgSurfaceEvent : unsigned int {
    Configure = 0,
};

/// xdg_toplevel
enum class XdgToplevelRequest : unsigned int {
    Destroy          = 0,
    SetParent        = 1,
    SetTitle         = 2,
    SetAppId         = 3,
    ShowWindowMenu   = 4,
    Move             = 5,
    Resize           = 6,
    SetMaxSize       = 7,
    SetMinSize       = 8,
    SetMaximized     = 9,
    UnsetMaximized   = 10,
    SetFullscreen    = 11,
    UnsetFullscreen  = 12,
    SetMinimized     = 13,
};

enum class XdgToplevelEvent : unsigned int {
    Configure       = 0,
    Close           = 1,
    ConfigureBounds = 2,
    WmCapabilities  = 3,
};

/// xdg_toplevel wm_capabilities bitfield (xdg-shell stable).
enum class XdgWmCapability : unsigned int {
    WindowMenu = 1u << 0,
    Maximize   = 1u << 1,
    Fullscreen = 1u << 2,
    Minimize   = 1u << 3,
};

/// xdg_positioner (xdg-shell stable).
enum class XdgPositionerRequest : unsigned int {
    Destroy                 = 0,
    SetSize                 = 1,
    SetAnchorRect           = 2,
    SetAnchor               = 3,
    SetGravity              = 4,
    SetConstraintAdjustment = 5,
    SetOffset               = 6,
};

enum class XdgPositionerAnchor : unsigned int {
    None   = 0,
    Top    = 1,
    Bottom = 2,
    Left   = 3,
    Right  = 4,
};

enum class XdgPositionerGravity : unsigned int {
    None        = 0,
    Top         = 1,
    Bottom      = 2,
    Left        = 3,
    Right       = 4,
    TopLeft     = 5,
    BottomLeft  = 6,
    TopRight    = 7,
    BottomRight = 8,
};

/// xdg_decoration_manager + xdg_toplevel_decoration.
enum class XdgDecorationManagerRequest : unsigned int {
    Destroy                  = 0,
    GetToplevelDecoration    = 1,
    CreateToplevelDecoration = 2,
};

enum class XdgToplevelDecorationRequest : unsigned int {
    Destroy   = 0,
    SetMode   = 1,
    UnsetMode = 2,
};

enum class XdgDecorationMode : unsigned int {
    ClientSide = 1,
    ServerSide = 2,
};

enum class XdgToplevelDecorationEvent : unsigned int {
    Configure = 0,
};

struct WireHeader {
    std::uint32_t object_id{};
    std::uint16_t opcode{};
    std::uint16_t size{};
};

struct AeroJsonKeys {
    static constexpr char kBlurRadius[]      = "blurRadius";
    static constexpr char kGlassTint[]       = "glassTint";
    static constexpr char kNoiseStrength[]   = "noiseStrength";
    static constexpr char kPanelOpacity[]    = "panelOpacity";
    static constexpr char kDialogOpacity[]   = "dialogOpacity";
    static constexpr char kSaturationBoost[] = "saturationBoost";
};

/// Default Aero values from pbsd/theme/plasma/blur-effect.json.
struct AeroDefaults {
    float blur_radius_px{24.0f};
    float glass_tint[4]{0.12f, 0.22f, 0.40f, 0.65f};
    float noise_strength{0.04f};
    float panel_opacity{0.72f};
    float dialog_opacity{0.78f};
    float saturation_boost{1.15f};
};

[[nodiscard]] inline Status validate_wire_header(WireHeader const& h) noexcept {
    if (h.size < 8 || h.size > 4096) {
        return Status::Protocol;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool is_display_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(DisplayRequest::GetRegistry);
}

[[nodiscard]] inline bool is_xdg_toplevel_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(XdgToplevelRequest::SetMinimized);
}

[[nodiscard]] inline bool is_xdg_positioner_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(XdgPositionerRequest::SetOffset);
}

[[nodiscard]] inline bool is_xdg_decoration_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(XdgToplevelDecorationRequest::UnsetMode);
}

[[nodiscard]] inline bool is_xdg_popup_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(XdgPopupRequest::Reposition);
}

[[nodiscard]] inline bool is_pointer_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(PointerRequest::Release);
}

[[nodiscard]] inline bool is_data_device_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(DataDeviceRequest::SetSelection);
}

[[nodiscard]] inline Status validate_shm_format(unsigned fmt) noexcept {
    if (fmt > static_cast<unsigned>(ShmFormat::CXrgb8888)) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::compositor::wayland
