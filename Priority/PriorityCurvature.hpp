/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
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

#ifndef PriorityCurvature_HPP
#define PriorityCurvature_HPP

#include "PriorityCurvature.h"

// Custom
#include "Helpers/Helpers.h"
#include "ITKHelpers/ITKHelpers.h"
#include "ITKVTKHelpers/ITKVTKHelpers.h"

// VTK
#include <vtkSmartPointer.h>
#include <vtkStructuredGrid.h>

template <typename TNode>
PriorityCurvature<TNode>::PriorityCurvature(vtkStructuredGrid* const structuredGrid,
                                            const unsigned int patchRadius) : PatchRadius(patchRadius)
{
  ITKVTKHelpers::CreateScalarImageFromStructuredGridArray(structuredGrid, "Curvature", this->CurvatureImage.GetPointer());
}

template <typename TNode>
void PriorityCurvature<TNode>::Update(const TNode& sourceNode, const TNode& targetNode)
{
  itk::Index<2> sourceCenter = {{sourceNode[0], sourceNode[1]}};
  itk::ImageRegion<2> sourceRegion = ITKHelpers::GetRegionInRadiusAroundPixel(sourceCenter, PatchRadius);

  itk::Index<2> targetCenter = {{targetNode[0], targetNode[1]}};
  itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetCenter, PatchRadius);

  ITKHelpers::CopySelfRegion(this->CurvatureImage.GetPointer(), sourceRegion, targetRegion);
}

template <typename TNode>
float PriorityCurvature<TNode>::ComputePriority(const TNode& queryNode) const
{
  itk::Index<2> queryPixel = {{queryNode[0], queryNode[1]}};
  float priority = this->CurvatureImage->GetPixel(queryPixel);
  return priority;
}

#endif
