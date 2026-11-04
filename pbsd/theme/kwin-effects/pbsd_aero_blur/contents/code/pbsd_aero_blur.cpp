/*
    PBSD Aero Blur — KWin effect (Wave 3)
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "pbsd_aero_blur.h"

#include "core/rendertarget.h"
#include "core/renderviewport.h"
#include "effect/effecthandler.h"
#include "opengl/glplatform.h"
#include "window.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

namespace KWin
{

namespace {

QJsonObject readPbsdConfig()
{
    const QStringList paths = {
        QStringLiteral("/usr/local/share/kwin/effects/pbsd_aero_blur/contents/config/pbsd_aero_blur.json"),
        QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                               QStringLiteral("kwin/effects/pbsd_aero_blur/contents/config/pbsd_aero_blur.json")),
    };
    for (const QString &path : paths) {
        if (path.isEmpty()) {
            continue;
        }
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            continue;
        }
        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isObject()) {
            return doc.object();
        }
    }
    return {};
}

} // namespace

PbsdAeroBlurEffect::PbsdAeroBlurEffect()
{
    loadConfig();
    ensureShaders();
}

PbsdAeroBlurEffect::~PbsdAeroBlurEffect() = default;

bool PbsdAeroBlurEffect::supported()
{
    return effects && effects->compositingType() == OpenGLCompositing
        && GLPlatform::instance()->supports(GLPlatform::Blur);
}

bool PbsdAeroBlurEffect::enabledByDefault()
{
    return true;
}

void PbsdAeroBlurEffect::reconfigure(ReconfigureFlags flags)
{
    Q_UNUSED(flags)
    loadConfig();
}

void PbsdAeroBlurEffect::loadConfig()
{
    const QJsonObject root = readPbsdConfig();
    blur_radius_ = root.value(QStringLiteral("blurRadius")).toDouble(24.0);
    noise_strength_ = root.value(QStringLiteral("noiseStrength")).toDouble(0.04);
    saturation_ = root.value(QStringLiteral("saturationBoost")).toDouble(1.15);
    contrast_ = root.value(QStringLiteral("contrastBoost")).toDouble(1.05);
    panel_opacity_ = root.value(QStringLiteral("panelOpacity")).toDouble(0.72);

    const bool decorationBlur = root.value(QStringLiteral("decorationBlur")).toBool(true);
    const QString decorationPlugin = root.value(QStringLiteral("decorationPlugin")).toString(
        QStringLiteral("pbsd_aero"));
    Q_UNUSED(decorationBlur)
    Q_UNUSED(decorationPlugin)

    const QJsonArray tint = root.value(QStringLiteral("glassTint")).toArray();
    if (tint.size() >= 4) {
        glass_tint_[0] = float(tint.at(0).toDouble(0.12));
        glass_tint_[1] = float(tint.at(1).toDouble(0.22));
        glass_tint_[2] = float(tint.at(2).toDouble(0.40));
        glass_tint_[3] = float(tint.at(3).toDouble(0.65));
    }

    if (KSharedConfigPtr config = KSharedConfig::openConfig(QStringLiteral("kwinrc"))) {
        KConfigGroup group = config->group(QStringLiteral("Effect-pbsd_aero_blur"));
        active_ = group.readEntry("enabled", true);
    }
}

void PbsdAeroBlurEffect::ensureShaders()
{
    // Packaged under contents/shaders/ and embedded via pbsd_aero_blur.qrc:
    //   :/effects/pbsd_aero_blur/contents/shaders/{vertex,downsample,upsample,
    //   onscreen,onscreen_rounded,noise,composite}.{vert,frag}
    static const char *const kShaderPaths[] = {
        ":/effects/pbsd_aero_blur/contents/shaders/vertex.vert",
        ":/effects/pbsd_aero_blur/contents/shaders/downsample.frag",
        ":/effects/pbsd_aero_blur/contents/shaders/upsample.frag",
        ":/effects/pbsd_aero_blur/contents/shaders/onscreen.frag",
        ":/effects/pbsd_aero_blur/contents/shaders/onscreen_rounded.frag",
        ":/effects/pbsd_aero_blur/contents/shaders/onscreen_rounded.vert",
        ":/effects/pbsd_aero_blur/contents/shaders/noise.frag",
        ":/effects/pbsd_aero_blur/contents/shaders/composite.frag",
    };
    Q_UNUSED(kShaderPaths)
    Q_UNUSED(noise_strength_)
}

void PbsdAeroBlurEffect::prePaintScreen(ScreenPrePaintData &data, std::chrono::milliseconds presentTime)
{
    Q_UNUSED(presentTime)
    if (!active_) {
        return;
    }
    data.mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS;
}

void PbsdAeroBlurEffect::drawWindow(const RenderTarget &renderTarget, const RenderViewport &viewport,
                                    EffectWindow *w, int mask, const Region &deviceRegion,
                                    WindowPaintData &data)
{
    Q_UNUSED(renderTarget)
    Q_UNUSED(viewport)
    Q_UNUSED(w)
    Q_UNUSED(mask)
    Q_UNUSED(deviceRegion)
    Q_UNUSED(data)
    Q_UNUSED(blur_radius_)
    Q_UNUSED(glass_tint_)
    Q_UNUSED(saturation_)
    Q_UNUSED(contrast_)
    Q_UNUSED(panel_opacity_)
    // Wave 3: metadata + shader stubs ship; full Dual Kawase pass lands in Wave 8 compositor bridge.
}

bool PbsdAeroBlurEffect::provides(Feature feature)
{
    return feature == Blur;
}

bool PbsdAeroBlurEffect::isActive() const
{
    return active_ && supported();
}

} // namespace KWin
