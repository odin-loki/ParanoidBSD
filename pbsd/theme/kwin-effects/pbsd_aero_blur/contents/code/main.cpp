/*
    PBSD Aero Blur — KWin effect factory (Wave 3)
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "pbsd_aero_blur.h"

namespace KWin
{

KWIN_EFFECT_FACTORY_SUPPORTED_ENABLED(PbsdAeroBlurEffect,
                                      "metadata.json.stripped",
                                      return PbsdAeroBlurEffect::supported();
                                      ,
                                      return PbsdAeroBlurEffect::enabledByDefault();)

} // namespace KWin

#include "main.moc"
