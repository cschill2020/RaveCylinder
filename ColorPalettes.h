#ifndef __INC_COLORPALETTES_H
#define __INC_COLORPALETTES_H

#include "ColorUtils.h"

/// Declarations for the predefined color palettes supplied by FastLED.
namespace ravecylinder {

extern const CRGBPalette16 CloudColors;
extern const CRGBPalette16 LavaColors;
extern const CRGBPalette16 OceanColors;
extern const CRGBPalette16 ForestColors;
extern const CRGBPalette16 RainbowColors;
extern const CRGBPalette16 RainbowStripeColors;
extern const CRGBPalette16 PartyColors;
extern const CRGBPalette16 HeatColors;

DECLARE_GRADIENT_PALETTE( Rainbow_gp);

} // namespace ravecylinder

#endif