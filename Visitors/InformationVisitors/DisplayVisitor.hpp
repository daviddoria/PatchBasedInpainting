#ifndef DisplayVisitor_HPP
#define DisplayVisitor_HPP

#include <boost/graph/graph_traits.hpp>

// Custom
#include "Visitors/InpaintingVisitorParent.h"
#include "ImageProcessing/Mask.h"
#include "Helpers/HelpersOutput.h"
#include "Helpers/ITKHelpers.h"

// ITK
#include "itkImage.h"
#include "itkImageRegion.h"

// VTK
#include <vtkRenderWindow.h>

/**

 */
template <typename TGraph, typename TImage>
struct DisplayVisitor : public InpaintingVisitorParent<TGraph>
{
  TImage* Image;
  Mask* MaskImage;

  const unsigned int HalfWidth;
  unsigned int NumberOfFinishedVertices;

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  DebugVisitor(TImage* const image, Mask* const mask, const unsigned int halfWidth) :
  Image(image), MaskImage(mask), HalfWidth(halfWidth), NumberOfFinishedVertices(0)
  {

  }
  
  void initialize_vertex(VertexDescriptorType v, TGraph& g) const
  { 

  };

  void discover_vertex(VertexDescriptorType v, TGraph& g) const 
  { 
    // Construct the region around the vertex
    itk::Index<2> indexToFinish;
    indexToFinish[0] = v[0];
    indexToFinish[1] = v[1];

    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(indexToFinish, HalfWidth);

    HelpersOutput::WriteVectorImageRegionAsRGB(Image, region, Helpers::GetSequentialFileName("targetPatch", this->NumberOfFinishedVertices, "png"));
    HelpersOutput::WriteRegion(MaskImage, region, Helpers::GetSequentialFileName("maskPatch", this->NumberOfFinishedVertices, "png"));
  };

  void vertex_match_made(VertexDescriptorType target, VertexDescriptorType source, TGraph& g) const
  {
    std::cout << "Match made: target: " << target[0] << " " << target[1]
              << " with source: " << source[0] << " " << source[1] << std::endl;
  };

  void paint_vertex(VertexDescriptorType target, VertexDescriptorType source, TGraph& g) const
  {
    
  };

  bool accept_painted_vertex(VertexDescriptorType v, TGraph& g) const
  {
    return true;
  };

  void finish_vertex(VertexDescriptorType v, VertexDescriptorType sourceNode, TGraph& g)
  {
    // Debug only
    itk::Index<2> sourceIndex;
    sourceIndex[0] = sourceNode[0];
    sourceIndex[1] = sourceNode[1];

    itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourceIndex, HalfWidth);

    HelpersOutput::WriteVectorImageRegionAsRGB(Image, sourceRegion, Helpers::GetSequentialFileName("sourcePatch", this->NumberOfFinishedVertices, "png"));

    HelpersOutput::WriteImage(MaskImage, Helpers::GetSequentialFileName("debugMask", this->NumberOfFinishedVertices, "png"));
    HelpersOutput::WriteVectorImageAsRGB(Image, Helpers::GetSequentialFileName("output", this->NumberOfFinishedVertices, "png"));
    this->NumberOfFinishedVertices++;

    std::cout << "Finished node " << this->NumberOfFinishedVertices << std::endl;
  };

};

#endif
