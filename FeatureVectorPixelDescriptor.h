#ifndef FeatureVectorPixelDescriptor_H
#define FeatureVectorPixelDescriptor_H

#include "PixelDescriptor.h"

/**
\class FeatureVectorPixelDescriptor
\brief Describes a pixel using a feature vector.
*/
class FeatureVectorPixelDescriptor : public PixelDescriptor
{
  /** The feature vector.*/
  std::vector<float> FeatureVector;
};

#endif
