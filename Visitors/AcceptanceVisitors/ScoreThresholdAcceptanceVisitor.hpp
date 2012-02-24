#ifndef ScoreThresholdAcceptanceVisitor_HPP
#define ScoreThresholdAcceptanceVisitor_HPP

#include <boost/graph/graph_traits.hpp>

// Parent class
#include "Visitors/AcceptanceVisitors/AcceptanceVisitorParent.h"

// Custom
#include "Helpers/ITKHelpers.h"

// ITK
#include "itkImageRegion.h"

/**

 */
template <typename TGraph, typename TPropertyMap, typename TDifferenceFunction>
struct ScoreThresholdAcceptanceVisitor : public AcceptanceVisitorParent<TGraph>
{
  Mask* MaskImage;

  const unsigned int HalfWidth;

  TPropertyMap PropertyMap;

  float Threshold;

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  ScoreThresholdAcceptanceVisitor(Mask* const mask, const unsigned int halfWidth, TPropertyMap propertyMap,
                                  const float threshold, const std::string& visitorName = "HoleSizeAcceptanceVisitor") :
  AcceptanceVisitorParent<TGraph>(visitorName), MaskImage(mask), HalfWidth(halfWidth), PropertyMap(propertyMap),
  Threshold(threshold)
  {

  }

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source, float& computedEnergy) const
  {
    itk::Index<2> targetPixel = ITKHelpers::CreateIndex(target);
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, HalfWidth);

    TDifferenceFunction differenceFunction;
    float difference = differenceFunction(get(PropertyMap, target), get(PropertyMap, source));
    
    std::cout << "ScoreThresholdAcceptanceVisitor difference: " << difference << std::endl;
    if(difference < Threshold)
    {
      return true;
    }
    else
    {
      return false;
    }
  };

};

#endif
