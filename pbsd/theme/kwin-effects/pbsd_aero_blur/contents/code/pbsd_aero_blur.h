/*
    PBSD Aero Blur — KWin effect (Wave 3)
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <effect/effect.h>

namespace KWin
{

class PbsdAeroBlurEffect : public Effect
{
    Q_OBJECT

public:
    PbsdAeroBlurEffect();
    ~PbsdAeroBlurEffect() override;

    static bool supported();
    static bool enabledByDefault();

    void reconfigure(ReconfigureFlags flags) override;
    void prePaintScreen(ScreenPrePaintData &data, std::chrono::milliseconds presentTime) override;
    void drawWindow(const RenderTarget &renderTarget, const RenderViewport &viewport,
                    EffectWindow *w, int mask, const Region &deviceRegion,
                    WindowPaintData &data) override;

    bool provides(Feature feature) override;
    bool isActive() const override;

    int requestedEffectChainPosition() const override { return 20; }

private:
    void loadConfig();
    void ensureShaders();

    bool active_{true};
    float blur_radius_{24.f};
    float noise_strength_{0.04f};
    float glass_tint_[4]{0.12f, 0.22f, 0.40f, 0.65f};
    float saturation_{1.15f};
    float contrast_{1.05f};
    float panel_opacity_{0.72f};
};

} // namespace KWin
