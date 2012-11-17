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

#ifndef TargetPatchWriterVisitor_HPP
#define TargetPatchWriterVisitor_HPP

#include <boost/graph/graph_traits.hpp>

// Custom
#include "Visitors/InpaintingVisitors/InpaintingVisitorParent.h"

// Submodules
#include <Mask/Mask.h>
#include <Utilities/Debug/Debug.h>
#include <ITKHelpers/ITKHelpers.h>

// ITK
#include "itkImage.h"
#include "itkImageRegion.h"

/**
  * This visitor writes out information and images at each iteration.
 */
template <typename TGraph, typename TImage>
struct TargetPatchWriterVisitor : public InpaintingVisitorParent<TGraph>, public Debug
{
  const TImage* Image;

  const std::string Prefix;

  const unsigned int PatchHalfWidth;

  typedef typename boost::graph_traits<TGraph>::vertex_descriptor VertexDescriptorType;

  TargetPatchWriterVisitor(TImage* const image, const std::string& prefix,
                           const unsigned int patchHalfWidth,
                           const std::string& visitorName = "DebugVisitor") :
    InpaintingVisitorParent<TGraph>(visitorName),
    Image(image), Prefix(prefix), PatchHalfWidth(patchHalfWidth)
  {

  }

  void DiscoverVertex(VertexDescriptorType v) override
  {
    itk::Index<2> index = ITKHelpers::CreateIndex(v);

    itk::ImageRegion<2> region =
        ITKHelpers::GetRegionInRadiusAroundPixel(index, this->PatchHalfWidth);

    itk::ImageRegion<2> croppedRegion = region;
    croppedRegion.Crop(this->Image->GetLargestPossibleRegion());

    ITKHelpers::WriteRegionAsRGBImage(this->Image, region,
                                      Helpers::GetSequentialFileName(this->Prefix,
                                                                     this->GetDebugIteration(), "png", 3));

    this->IncrementDebugIteration();
  }

};

#endif
