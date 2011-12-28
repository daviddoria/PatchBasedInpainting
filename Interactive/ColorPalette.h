#ifndef COLOR_PALETTE_H
#define COLOR_PALETTE_H

#include <QColor>

class ColorPalette
{
public:
  ColorPalette();
  
  QColor UsedTargetPatch;
  QColor UsedSourcePatch;
  QColor AllForwardLookPatch;
  QColor SelectedForwardLookPatch;
  QColor AllSourcePatch;
  QColor SelectedSourcePatch;
  QColor CenterPixel;
  QColor Mask;
  QColor Hole;
  QColor SceneBackground;
  QColor UserPatch;
};

#endif
