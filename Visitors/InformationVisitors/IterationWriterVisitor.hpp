#ifndef DebugVisitor_HPP
#define DebugVisitor_HPP

#include <boost/graph/graph_traits.hpp>

// Custom
#include "Visitors/SimpleVisitors/VisitorSuperclass.hpp"
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
template <typename TVertexDescriptor, typename TImage>
struct IterationWriterVisitor : public VisitorSuperclass<TVertexDescriptor>
{
  TImage* Image;
  Mask* MaskImage;

  unsigned int NumberOfFinishedVertices;
  IterationWriterVisitor(TImage* const image, Mask* const mask,
                         const std::string& visitorName = "IterationWriterVisitor") :
  Image(image), MaskImage(mask), NumberOfFinishedVertices(0)
  {

  }

  void PaintPatch(TVertexDescriptor target, TVertexDescriptor source) const {}

  void FinishVertex(TVertexDescriptor target, TVertexDescriptor sourceNode)
  {
    OutputHelpers::WriteImage(MaskImage, Helpers::GetSequentialFileName("mask",
                                                                        this->NumberOfFinishedVertices, "mha"));
    OutputHelpers::WriteImage(Image, Helpers::GetSequentialFileName("output",
                                                                    this->NumberOfFinishedVertices, "mha"));

    this->NumberOfFinishedVertices++;

  };

  void InpaintingComplete() const  {  }
};

#endif
