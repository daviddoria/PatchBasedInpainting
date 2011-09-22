#include "InpaintingVisualizationStack.h"

InpaintingVisualizationStack::InpaintingVisualizationStack()
{
  this->Image = FloatVectorImageType::New();
  this->MaskImage = Mask::New();
  this->Boundary = UnsignedCharScalarImageType::New();
  this->Confidence = FloatScalarImageType::New();
  this->ConfidenceMap = FloatScalarImageType::New();
  this->Priority = FloatScalarImageType::New();
  this->Data = FloatScalarImageType::New();
  this->BoundaryNormals = FloatVector2ImageType::New();
  this->Isophotes = FloatVector2ImageType::New();
  this->PotentialTargetPatchesImage = UnsignedCharScalarImageType::New();
}
