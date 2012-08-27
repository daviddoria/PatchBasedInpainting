#ifndef DebugVisitor_HPP
#define DebugVisitor_HPP

#include <boost/graph/graph_traits.hpp>

// Custom
#include "Visitors/InpaintingVisitorParent.h"
#include "Mask/Mask.h"
#include "ITKHelpers/ITKHelpers.h"
#include "BoostHelpers/BoostHelpers.h"

// ITK
#include "itkImage.h"
#include "itkImageRegion.h"

/**
  * This visitor writes out information and images at each iteration.
 */
template <typename TGraph, typename TImage, typename TBoundaryStatusMap, typename TBoundaryNodeQueue>
struct DebugVisitor : public InpaintingVisitorParent<TGraph>
{
  TImage* Image;
  Mask* MaskImage;

  const unsigned int HalfWidth;
  unsigned int NumberOfFinishedVertices;

  TBoundaryStatusMap& BoundaryStatusMap;
  TBoundaryNodeQueue& BoundaryNodeQueue;

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;
  
  DebugVisitor(TImage* const image, Mask* const mask, const unsigned int halfWidth, TBoundaryStatusMap& boundaryStatusMap,
               TBoundaryNodeQueue& boundaryNodeQueue, const std::string& visitorName = "DebugVisitor") :
  InpaintingVisitorParent<TGraph>(visitorName),
  Image(image), MaskImage(mask), HalfWidth(halfWidth), NumberOfFinishedVertices(0),
  BoundaryStatusMap(boundaryStatusMap), BoundaryNodeQueue(boundaryNodeQueue)
  {

  }

  void PaintPatch(VertexDescriptorType target, VertexDescriptorType source) const {}
  
  void InitializeVertex(VertexDescriptorType v) const
  {

  };

  void DiscoverVertex(VertexDescriptorType v) const
  {

  };

  void PotentialMatchMade(VertexDescriptorType targetNode, VertexDescriptorType sourceNode)
  {
    std::cout << "Match made: target: " << targetNode[0] << " " << targetNode[1]
              << " with source: " << sourceNode[0] << " " << sourceNode[1] << std::endl;
  std::cout << "Writing pair " << NumberOfFinishedVertices << std::endl;
  
    {
    itk::Index<2> sourceIndex = ITKHelpers::CreateIndex(sourceNode);

    itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourceIndex, this->HalfWidth);

    ITKHelpers::WriteVectorImageRegionAsRGB(Image, sourceRegion,
                                               Helpers::GetSequentialFileName("sourcePatch",
                                                                              this->NumberOfFinishedVertices, "png"));
    }

    {
    // Construct the region around the vertex
    itk::Index<2> indexToFinish = ITKHelpers::CreateIndex(targetNode);

    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(indexToFinish, this->HalfWidth);

    ITKHelpers::WriteVectorImageRegionAsRGB(Image, region,
                                               Helpers::GetSequentialFileName("targetPatch",
                                                                              this->NumberOfFinishedVertices, "png"));
    ITKHelpers::WriteRegionAsRGBImage(this->MaskImage, region,
                               Helpers::GetSequentialFileName("maskPatch", this->NumberOfFinishedVertices, "png"));


    typename TImage::PixelType holeColor;
    holeColor.SetSize(Image->GetNumberOfComponentsPerPixel());
    holeColor[0] = 255;
    holeColor[1] = 0;
    holeColor[2] = 0;
    MaskOperations::WriteMaskedRegionPNG(this->Image, this->MaskImage, region,
                                         Helpers::GetSequentialFileName("maskedTargetPatch", this->NumberOfFinishedVertices, "png"),
                       holeColor);
    }
  }

  void PaintVertex(VertexDescriptorType target, VertexDescriptorType source) const
  {

  };

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source) const
  {
    return true;
  }

  void FinishVertex(VertexDescriptorType target, VertexDescriptorType sourceNode)
  {
    //OutputHelpers::WriteImage(MaskImage, Helpers::GetSequentialFileName("mask", this->NumberOfFinishedVertices, "png"));
    ITKHelpers::WriteImage(MaskImage, Helpers::GetSequentialFileName("mask",
                                                                        this->NumberOfFinishedVertices, "mha"));
    //OutputHelpers::WriteVectorImageAsRGB(Image, Helpers::GetSequentialFileName("output", this->NumberOfFinishedVertices, "png"));
    ITKHelpers::WriteImage(Image, Helpers::GetSequentialFileName("output",
                                                                    this->NumberOfFinishedVertices, "mha"));

    ITKHelpers::WriteRGBImage(Image, Helpers::GetSequentialFileName("output",
                                                                    this->NumberOfFinishedVertices, "png"));

    typename TImage::PixelType holeColor;
    holeColor.SetSize(Image->GetNumberOfComponentsPerPixel());
    holeColor[0] = 255;
    holeColor[1] = 0;
    holeColor[2] = 0;

    MaskOperations::WriteMaskedRegionPNG(Image, MaskImage, Image->GetLargestPossibleRegion(), Helpers::GetSequentialFileName("maskedOutput", this->NumberOfFinishedVertices, "png"),
                       holeColor);


    typedef itk::Image<unsigned char, 2> IndicatorImageType;

    IndicatorImageType::Pointer boundaryStatusMapImage = IndicatorImageType::New();
    boundaryStatusMapImage->SetRegions(Image->GetLargestPossibleRegion());
    boundaryStatusMapImage->Allocate();
//     BoostHelpers::WritePropertyMapAsImage(BoundaryStatusMap, boundaryStatusMapImage.GetPointer(),
//                                           Helpers::GetSequentialFileName("boundaryStatusMap",
//                                           this->NumberOfFinishedVertices, "png"));

    IndicatorImageType::Pointer validBoundaryNodeImage = IndicatorImageType::New();
    validBoundaryNodeImage->SetRegions(Image->GetLargestPossibleRegion());
    validBoundaryNodeImage->Allocate();
//     BoostHelpers::WriteValidQueueNodesAsImage(BoundaryNodeQueue, BoundaryStatusMap,
//                                                 validBoundaryNodeImage.GetPointer(),
//                                           Helpers::GetSequentialFileName("boundaryQueueValidNodes",
//                                                this->NumberOfFinishedVertices, "png"));

    IndicatorImageType::Pointer allBoundaryNodeImage = IndicatorImageType::New();
    allBoundaryNodeImage->SetRegions(Image->GetLargestPossibleRegion());
    allBoundaryNodeImage->Allocate();
//     BoostHelpers::WriteAllQueueNodesAsImage(BoundaryNodeQueue, allBoundaryNodeImage.GetPointer(),
//                                           Helpers::GetSequentialFileName("boundaryQueueAllNodes",
//                                            this->NumberOfFinishedVertices, "png"));

    this->NumberOfFinishedVertices++;

    // std::cout << "Finished node " << this->NumberOfFinishedVertices << std::endl;

  }

  void InpaintingComplete() const
  {
  }
};

#endif
