#ifndef BoundaryEnergyAcceptanceVisitor_HPP
#define BoundaryEnergyAcceptanceVisitor_HPP

#include <boost/graph/graph_traits.hpp>

// Parent class
#include "Visitors/AcceptanceVisitors/AcceptanceVisitorParent.h"

// Custom
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
struct BoundaryEnergyAcceptanceVisitor : public AcceptanceVisitorParent<TGraph>
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

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source) const
  {
        BoundaryEnergy<TImage> boundaryEnergy(Image, MaskImage);

    itk::Index<2> targetPixel = ITKHelpers::CreateIndex(target);
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, HalfWidth);

    itk::Index<2> sourcePixel = ITKHelpers::CreateIndex(source);
    itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourcePixel, HalfWidth);

    // Compute boundary energy
    float energy = boundaryEnergy(sourceRegion, targetRegion);
    std::cout << "Energy: " << energy << std::endl;

    float energyThreshold = 100;
    if(energy < energyThreshold)
      {
      std::cout << "Match accepted." << std::endl;
      return true;
      }
    else
      {
      std::cout << "Match rejected." << std::endl;
      return false;
      }
  };

};

#endif
