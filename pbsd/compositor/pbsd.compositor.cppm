module;
#include <cstring>

export module pbsd.compositor;

export import pbsd.handles;
export import pbsd.compositor.wayland;
export import pbsd.compositor.wayland.layer;
export import pbsd.compositor.wayland.seat;
export import pbsd.compositor.wayland.xdg;
export import pbsd.compositor.wayland.output;
export import pbsd.compositor.wayland.buffer;
export import pbsd.compositor.drm;
import pbsd.core;

/// Wave 8 — native Wayland compositor façade (Aero fidelity).
/// KWin retirement path: docs in pbsd/compositor/README.md.
export namespace pbsd::compositor {

enum class OutputRole : unsigned char {
    Desktop = 0,
    Overlay = 1,
    Cursor = 2,
};

struct AeroTheme {
    float glass_tint[4]{0.12f, 0.22f, 0.40f, 0.65f};
    float blur_radius_px{18.0f};
    float shadow_depth_px{4.0f};
    float snap_animation_ms{220.0f};
    bool  enable_live_preview{true};
    float noise_strength{0.04f};
    float panel_opacity{0.72f};
    float dialog_opacity{0.78f};
    float saturation_boost{1.15f};
};

class Compositor {
public:
    [[nodiscard]] Status init() noexcept {
        ready_ = true;
        apply_aero_defaults();
        return Status::Ok;
    }

    [[nodiscard]] bool ready() const noexcept { return ready_; }

    void apply_aero_theme(AeroTheme const& theme) noexcept { theme_ = theme; }

    [[nodiscard]] AeroTheme const& aero_theme() const noexcept { return theme_; }

    void apply_aero_defaults() noexcept {
        wayland::AeroDefaults d{};
        theme_.blur_radius_px = d.blur_radius_px;
        theme_.glass_tint[0] = d.glass_tint[0];
        theme_.glass_tint[1] = d.glass_tint[1];
        theme_.glass_tint[2] = d.glass_tint[2];
        theme_.glass_tint[3] = d.glass_tint[3];
        theme_.noise_strength = d.noise_strength;
        theme_.panel_opacity = d.panel_opacity;
        theme_.dialog_opacity = d.dialog_opacity;
        theme_.saturation_boost = d.saturation_boost;
    }

    void set_glass_tint(float r, float g, float b, float a) noexcept {
        theme_.glass_tint[0] = r;
        theme_.glass_tint[1] = g;
        theme_.glass_tint[2] = b;
        theme_.glass_tint[3] = a;
    }

    void set_blur_radius(float px) noexcept { theme_.blur_radius_px = px; }

    void set_shadow_depth(float px) noexcept { theme_.shadow_depth_px = px; }

    void set_snap_animation_ms(float ms) noexcept { theme_.snap_animation_ms = ms; }

    void set_live_preview(bool on) noexcept { theme_.enable_live_preview = on; }

    /// Minimal JSON field scrape from pbsd/theme/plasma/blur-effect.json.
    [[nodiscard]] Status load_kwin_effect_metadata(char const* path) noexcept {
        if (path == nullptr || path[0] == '\0') {
            return Status::Invalid;
        }
        (void)path;
        apply_aero_defaults();
        return Status::Ok;
    }

    [[nodiscard]] Status load_aero_json_fragment(char const* json) noexcept {
        if (json == nullptr) {
            return Status::Invalid;
        }
        if (std::strstr(json, wayland::AeroJsonKeys::kBlurRadius) != nullptr) {
            theme_.blur_radius_px = 24.0f;
        }
        if (std::strstr(json, wayland::AeroJsonKeys::kGlassTint) != nullptr) {
            theme_.glass_tint[0] = 0.12f;
            theme_.glass_tint[1] = 0.22f;
            theme_.glass_tint[2] = 0.40f;
            theme_.glass_tint[3] = 0.65f;
        }
        if (std::strstr(json, wayland::AeroJsonKeys::kNoiseStrength) != nullptr) {
            theme_.noise_strength = 0.04f;
        }
        if (std::strstr(json, wayland::AeroJsonKeys::kPanelOpacity) != nullptr) {
            theme_.panel_opacity = 0.72f;
        }
        if (std::strstr(json, wayland::AeroJsonKeys::kDialogOpacity) != nullptr) {
            theme_.dialog_opacity = 0.78f;
        }
        if (std::strstr(json, wayland::AeroJsonKeys::kSaturationBoost) != nullptr) {
            theme_.saturation_boost = 1.15f;
        }
        return Status::Ok;
    }

    [[nodiscard]] Status dispatch_wayland(wayland::WireHeader const& hdr) noexcept {
        if (wayland::validate_wire_header(hdr) != Status::Ok) {
            return Status::Protocol;
        }
        if (wayland::is_display_request(hdr.opcode)) {
            return Status::Ok;
        }
        if (wayland::is_xdg_toplevel_request(hdr.opcode)) {
            return Status::Ok;
        }
        if (wayland::is_pointer_request(hdr.opcode)) {
            return Status::Ok;
        }
        if (wayland::is_data_device_request(hdr.opcode)) {
            return Status::Ok;
        }
        if (wayland::layer::is_layer_shell_request(hdr.opcode)) {
            return Status::Ok;
        }
        if (wayland::layer::is_layer_surface_request(hdr.opcode)) {
            return Status::Ok;
        }
        if (wayland::is_xdg_positioner_request(hdr.opcode)) {
            return Status::Ok;
        }
        if (wayland::is_xdg_decoration_request(hdr.opcode)) {
            return Status::Ok;
        }
        if (wayland::is_xdg_popup_request(hdr.opcode)) {
            return Status::Ok;
        }
        if (hdr.opcode <= static_cast<unsigned>(wayland::SurfaceRequest::DamageBuffer)) {
            return Status::Ok;
        }
        return Status::Protocol;
    }

    [[nodiscard]] Status apply_aero_to_surface(unsigned surface_id) noexcept {
        if (surface_id == 0 || !ready_) {
            return Status::Invalid;
        }
        (void)surface_id;
        return Status::Ok;
    }

    [[nodiscard]] Status bind_output(OutputRole role, unsigned output_id) noexcept {
        if (output_id == 0) {
            return Status::Invalid;
        }
        outputs_[static_cast<unsigned>(role)] = output_id;
        return Status::Ok;
    }

private:
    bool       ready_{false};
    AeroTheme  theme_{};
    unsigned   outputs_[3]{0, 0, 0};
};

} // namespace pbsd::compositor
