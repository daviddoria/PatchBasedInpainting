#ifndef COLOR_PALETTE_H
#define COLOR_PALETTE_H

#include <QColor>

class ColorPalette
{
public:
  ColorPalette();

  QColor UsedTargetPatch;
  QColor UsedSourcePatch;

  QColor SourcePatch;
  QColor ForwardLookPatch;

  QColor SelectedForwardLookPatch;
  QColor SelectedSourcePatch;

  QColor CenterPixel;

  QColor Hole;
  QColor UserPatch;
};

#endif
