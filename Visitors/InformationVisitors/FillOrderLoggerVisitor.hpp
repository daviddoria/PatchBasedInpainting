#ifndef FillOrderLoggerVisitor_HPP
#define FillOrderLoggerVisitor_HPP

// Custom
#include "Visitors/InpaintingVisitorParent.h"
#include "Helpers/ITKHelpers.h"
#include "Helpers/OutputHelpers.h"

/**
  * This visitor saves the information needed to reproduce the inpainting.
 */
template <typename TGraph>
struct FillOrderLoggerVisitor : public InpaintingVisitorParent<TGraph>
{
  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  typedef itk::Image<unsigned int, 2> ImageType;

  ImageType::Pointer FillOrderImage;
  unsigned int PatchHalfWidth;
  const Mask* MaskImage;
  mutable unsigned int NumberOfPatchesFilled; // This must be changed from a const function
  std::string OutputFileName;
  
  FillOrderLoggerVisitor(const std::string& outputFileName, const Mask* const mask, const unsigned int patchHalfWidth,
                         const std::string& visitorName = "FillOrderLoggerVisitor") :
  InpaintingVisitorParent<TGraph>(visitorName), PatchHalfWidth(patchHalfWidth), MaskImage(mask), NumberOfPatchesFilled(0), OutputFileName(outputFileName)
  {
    FillOrderImage = ImageType::New();
    FillOrderImage->SetRegions(mask->GetLargestPossibleRegion());
    FillOrderImage->Allocate();
    FillOrderImage->FillBuffer(0);
  }

  void PaintPatch(VertexDescriptorType target, VertexDescriptorType source) const
  {
    itk::Index<2> targetIndex = Helpers::ConvertFrom<itk::Index<2>, VertexDescriptorType>(target);
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetIndex, PatchHalfWidth);
    
    itk::ImageRegionIterator<ImageType> imageIterator(FillOrderImage, targetRegion);

    NumberOfPatchesFilled++;
    while(!imageIterator.IsAtEnd())
      {
      if(MaskImage->IsHole(imageIterator.GetIndex()))
        {
        imageIterator.Set(NumberOfPatchesFilled);
        }

      ++imageIterator;
      }
  }

  void InitializeVertex(VertexDescriptorType v) const { };

  void DiscoverVertex(VertexDescriptorType v) const {  };

  void PotentialMatchMade(VertexDescriptorType target, VertexDescriptorType source) {  };

  void PaintVertex(VertexDescriptorType target, VertexDescriptorType source) const {  };

  bool AcceptMatch(VertexDescriptorType target, VertexDescriptorType source) const {  return true; };

  void FinishVertex(VertexDescriptorType targetNode, VertexDescriptorType sourceNode)  { };

  void InpaintingComplete() const
  {
    OutputHelpers::WriteImage(FillOrderImage.GetPointer(), OutputFileName);
  }
};

#endif
