#ifndef DebugVisitor_HPP
#define DebugVisitor_HPP

#include <boost/graph/graph_traits.hpp>

// Custom
#include "Visitors/InpaintingVisitorParent.h"
#include "ImageProcessing/Mask.h"
#include "Helpers/OutputHelpers.h"
#include "Helpers/ITKHelpers.h"
#include "Helpers/BoostHelpers.h"

// ITK
#include "itkImage.h"
#include "itkImageRegion.h"

/**
  * This visitor writes out information and images at each iteration.
 */
template <typename TGraph, typename TImage, typename TBoundaryStatusMap>
struct DebugVisitor : public InpaintingVisitorParent<TGraph>
{
  TImage* Image;
  Mask* MaskImage;

  const unsigned int HalfWidth;
  unsigned int NumberOfFinishedVertices;

  TBoundaryStatusMap& BoundaryStatusMap;
  
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  DebugVisitor(TImage* const image, Mask* const mask, const unsigned int halfWidth, TBoundaryStatusMap& in_boundaryStatusMap) :
  Image(image), MaskImage(mask), HalfWidth(halfWidth), NumberOfFinishedVertices(0), BoundaryStatusMap(in_boundaryStatusMap)
  {

  }

  void InitializeVertex(VertexDescriptorType v) const
  {

  };

  void DiscoverVertex(VertexDescriptorType v) const
  {

  };

  void PotentialMatchMade(VertexDescriptorType target, VertexDescriptorType source)
  {
    std::cout << "Match made: target: " << target[0] << " " << target[1]
              << " with source: " << source[0] << " " << source[1] << std::endl;
  };

  void PaintVertex(VertexDescriptorType target, VertexDescriptorType source) const
  {

  };

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source) const
  {
    return true;
  };

  void FinishVertex(VertexDescriptorType target, VertexDescriptorType sourceNode)
  {
    {
    itk::Index<2> sourceIndex = ITKHelpers::CreateIndex(sourceNode);

    itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourceIndex, HalfWidth);

    OutputHelpers::WriteVectorImageRegionAsRGB(Image, sourceRegion,
                                               Helpers::GetSequentialFileName("sourcePatch",
                                                                              this->NumberOfFinishedVertices, "png"));
    }

    {
    // Construct the region around the vertex
    itk::Index<2> indexToFinish = ITKHelpers::CreateIndex(target);

    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(indexToFinish, HalfWidth);

    OutputHelpers::WriteVectorImageRegionAsRGB(Image, region,
                                               Helpers::GetSequentialFileName("targetPatch",
                                                                              this->NumberOfFinishedVertices, "png"));
    OutputHelpers::WriteRegion(MaskImage, region,
                               Helpers::GetSequentialFileName("maskPatch", this->NumberOfFinishedVertices, "png"));
    }

    OutputHelpers::WriteImage(MaskImage, Helpers::GetSequentialFileName("mask", this->NumberOfFinishedVertices, "png"));
    OutputHelpers::WriteImage(MaskImage, Helpers::GetSequentialFileName("mask", this->NumberOfFinishedVertices, "mha"));
    OutputHelpers::WriteVectorImageAsRGB(Image, Helpers::GetSequentialFileName("output", this->NumberOfFinishedVertices, "png"));
    OutputHelpers::WriteImage(Image, Helpers::GetSequentialFileName("output", this->NumberOfFinishedVertices, "mha"));

    this->NumberOfFinishedVertices++;

    std::cout << "Finished node " << this->NumberOfFinishedVertices << std::endl;

    typedef itk::Image<unsigned char, 2> IndicatorImageType;
    IndicatorImageType::Pointer boundaryIndicatorImage = IndicatorImageType::New();
    boundaryIndicatorImage->SetRegions(Image->GetLargestPossibleRegion());
    boundaryIndicatorImage->Allocate();
    BoostHelpers::WritePropertyMapAsImage(BoundaryStatusMap, boundaryIndicatorImage.GetPointer(),
                                          Helpers::GetSequentialFileName("boundary", this->NumberOfFinishedVertices, "png"));
  };

  void InpaintingComplete() const
  {
  }
};

#endif
