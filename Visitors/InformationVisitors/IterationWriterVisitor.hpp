/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#ifndef IterationWriterVisitor_HPP
#define IterationWriterVisitor_HPP

// Boost
#include <boost/graph/graph_traits.hpp>

// Custom
#include "Visitors/InpaintingVisitors/InpaintingVisitorParent.h"

// Submodules
#include <Mask/Mask.h>
#include <ITKHelpers/ITKHelpers.h>

// ITK
#include "itkImage.h"
#include "itkImageRegion.h"

/**
  * This visitor writes out information and images at each iteration.
 */
template <typename TGraph, typename TImage>
struct IterationWriterVisitor : public InpaintingVisitorParent<TGraph>
{
  typedef InpaintingVisitorParent<TGraph> Superclass;
  typedef typename Superclass::VertexDescriptorType VertexDescriptorType;

  /** The image to write at every iteration. */
  const TImage* Image;

  /** The mask to write at every iteration. */
  const Mask* MaskImage;

  /** A counter used to name output files. */
  unsigned int NumberOfFinishedVertices = 0;

  std::string Prefix;

  IterationWriterVisitor(const TImage* const image, const Mask* const mask,
                         const std::string& prefix = "FilledIteration",
                         const std::string& visitorName = "IterationWriterVisitor") :
    InpaintingVisitorParent<TGraph>(visitorName),
    Image(image), MaskImage(mask), Prefix(prefix)
  {

  }

  void FinishVertex(VertexDescriptorType target, VertexDescriptorType sourceNode) override
  {
    ITKHelpers::WriteRGBImage(this->MaskImage,
                              Helpers::GetSequentialFileName(this->Prefix + "_mask",
                                                             this->NumberOfFinishedVertices, "png"));
    ITKHelpers::WriteRGBImage(this->Image,
                              Helpers::GetSequentialFileName(this->Prefix,
                                                             this->NumberOfFinishedVertices, "png"));

  // MHA versions
//    ITKHelpers::WriteImage(this->MaskImage,
//                           Helpers::GetSequentialFileName(this->Prefix + "_mask",
//                                                          this->NumberOfFinishedVertices, "png"));
////                                                        this->NumberOfFinishedVertices, "mha"));
//    ITKHelpers::WriteImage(this->Image,
//                           Helpers::GetSequentialFileName(this->Prefix,
//                                                          this->NumberOfFinishedVertices, "png"));
////                                                          this->NumberOfFinishedVertices, "mha"));

    this->NumberOfFinishedVertices++;
  }

};

#endif
