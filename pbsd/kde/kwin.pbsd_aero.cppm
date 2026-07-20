export module pbsd.kde.kwin.pbsd_aero;

export import pbsd.handles;
import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.kwin.decorations;
import pbsd.kde.kwin.blur;
import pbsd.kde.kwin.compositing;

/// Wave 3 — KWin blur/decoration bridge (hosted stub; links nucleus flags).
export namespace pbsd::kde::kwin {

struct EffectObject {
    bool enabled{false};

    static void release(EffectObject* p) noexcept {
        if (p != nullptr) {
            p->enabled = false;
        }
    }
};

using EffectHandle = UniqueHandle<EffectObject>;

class AeroBlurEffect {
public:
    [[nodiscard]] Result<EffectHandle> create(LineageTree& tree,
                                              plasma::aero::BlurParams params) noexcept {
        const LineageId id = tree.create_root();
        if (id == kInvalidLineage) {
            return {Status::NoMemory, EffectHandle{}};
        }
        auto* obj = new EffectObject{true};
        params_ = params;
        return {Status::Ok, EffectHandle{obj, CapabilityRights::Read, id}};
    }

    [[nodiscard]] plasma::aero::BlurParams params() const noexcept { return params_; }

    void apply_tint(float r, float g, float b, float a) noexcept {
        params_.tint = {r, g, b, a};
    }

    [[nodiscard]] static plasma::aero::BlurParams params_from_defaults() noexcept {
        return blur::default_params();
    }

    [[nodiscard]] static plasma::aero::BlurParams params_for_panel() noexcept {
        return compositing::panel_blur();
    }

    [[nodiscard]] static plasma::aero::BlurParams params_for_dialog() noexcept {
        return compositing::dialog_blur();
    }

    [[nodiscard]] static plasma::aero::BlurParams params_for_start_menu() noexcept {
        return compositing::start_menu_blur();
    }

    [[nodiscard]] static plasma::aero::BlurParams params_for_decoration() noexcept {
        return compositing::decoration_blur();
    }

    [[nodiscard]] static plasma::aero::BlurParams params_for_glow() noexcept {
        return compositing::glow_blur();
    }

    [[nodiscard]] static plasma::aero::BlurParams params_for_system_link() noexcept {
        return compositing::system_link_blur();
    }

private:
    plasma::aero::BlurParams params_{plasma::aero::default_blur()};
};

[[nodiscard]] inline const char* effect_plugin_id() noexcept { return "pbsd_aero_blur"; }
[[nodiscard]] inline const char* effect_metadata_path() noexcept {
    return "kwin-effects/pbsd_aero_blur/metadata.json";
}
[[nodiscard]] inline const char* effect_config_path() noexcept {
    return "kwin/effects/pbsd_aero_blur/contents/config/pbsd_aero_blur.json";
}
[[nodiscard]] inline const char* effect_shader_dir() noexcept {
    return "kwin/effects/pbsd_aero_blur/contents/shaders";
}
[[nodiscard]] inline const char* decoration_config_path() noexcept {
    return "kwin/decoration/pbsd_aero/contents/config.json";
}
[[nodiscard]] inline const char* decoration_metadata_path() noexcept {
    return decorations::kMetadataPath;
}

} // namespace pbsd::kde::kwin
