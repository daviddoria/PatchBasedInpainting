#include "ColorPalette.h"

ColorPalette::ColorPalette()
{
  this->UsedTargetPatch = Qt::red;
  this->UsedSourcePatch = Qt::green;
  this->AllForwardLookPatch = Qt::darkCyan;
  this->SelectedForwardLookPatch = Qt::cyan;
  this->AllSourcePatch = Qt::darkMagenta;
  this->SelectedSourcePatch = Qt::magenta;
  this->CenterPixel = Qt::blue;
  this->Mask = Qt::darkGray;
  this->UserPatch = Qt::yellow;

  this->Hole.setRgb(255, 153, 0); // Orange
  this->SceneBackground.setRgb(153, 255, 0); // Lime green
}
