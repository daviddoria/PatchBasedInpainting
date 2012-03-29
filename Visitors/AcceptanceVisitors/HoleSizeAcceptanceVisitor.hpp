#ifndef HoleSizeAcceptanceVisitor_HPP
#define HoleSizeAcceptanceVisitor_HPP

#include <boost/graph/graph_traits.hpp>

// Parent class
#include "Visitors/AcceptanceVisitors/AcceptanceVisitorParent.h"

// Custom
#include "ITKHelpers/ITKHelpers.h"

// ITK
#include "itkImageRegion.h"

/**

 */
template <typename TGraph>
struct HoleSizeAcceptanceVisitor : public AcceptanceVisitorParent<TGraph>
{
  Mask* MaskImage;

  const unsigned int HalfWidth;

  float HolePixelRatio;
  
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  HoleSizeAcceptanceVisitor(Mask* const mask, const unsigned int halfWidth, const float holePixelRatio, const std::string& visitorName = "HoleSizeAcceptanceVisitor") :
  AcceptanceVisitorParent<TGraph>(visitorName), MaskImage(mask), HalfWidth(halfWidth), HolePixelRatio(holePixelRatio)
  {

  }

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source, float& computedEnergy) const
  {
    itk::Index<2> targetPixel = ITKHelpers::CreateIndex(target);
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, HalfWidth);

    unsigned int numberOfHolePixels = MaskImage->CountHolePixels(targetRegion);

    float ratio = static_cast<float>(numberOfHolePixels)/static_cast<float>(targetRegion.GetNumberOfPixels());
    std::cout << "Hole pixel ratio: " << ratio << std::endl;
    if(ratio < HolePixelRatio)
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
