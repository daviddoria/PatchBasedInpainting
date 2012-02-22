#ifndef FullPatchVarianceDifference_HPP
#define FullPatchVarianceDifference_HPP

#include <boost/graph/graph_traits.hpp>

// Parent class
#include "Visitors/AcceptanceVisitors/AcceptanceVisitorParent.h"

// Custom
#include "Helpers/ITKHelpers.h"

// ITK
#include "itkImageRegion.h"

/**

 */
template <typename TGraph, typename TImage>
struct FullPatchVarianceDifference : public AcceptanceVisitorParent<TGraph>
{
  TImage* Image;

  const unsigned int HalfWidth;

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  FullPatchVarianceDifference(TImage* const image, const unsigned int halfWidth) :
  Image(image), HalfWidth(halfWidth)
  {

  }

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source, float& computedEnergy) const
  {
    itk::Index<2> targetPixel = ITKHelpers::CreateIndex(target);
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, HalfWidth);
    typename TImage::PixelType targetRegionAverage = ITKHelpers::VarianceInRegion(Image, targetRegion);

    itk::Index<2> sourcePixel = ITKHelpers::CreateIndex(source);
    itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourcePixel, HalfWidth);
    typename TImage::PixelType sourceRegionAverage = ITKHelpers::VarianceInRegion(Image, sourceRegion);

    // Compute the difference
    computedEnergy = (targetRegionAverage - sourceRegionAverage).GetNorm();
    std::cout << "FullPatchVarianceDifference Energy: " << computedEnergy << std::endl;
    return true;
  };

};

#endif
