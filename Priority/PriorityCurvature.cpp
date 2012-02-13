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

#include "PriorityCurvature.h"

// Custom
#include "Helpers/Helpers.h"
#include "Helpers/ITKHelpers.h"
#include "Helpers/ITKVTKHelpers.h"

// VTK
#include <vtkSmartPointer.h>
#include <vtkStructuredGrid.h>

PriorityCurvature::PriorityCurvature(vtkStructuredGrid* const structuredGrid, const unsigned int patchRadius) : PatchRadius(patchRadius)
{
  ITKVTKHelpers::CreateImageFromStructuredGridArray(structuredGrid, "Curvature", this->CurvatureImage);
}

void PriorityCurvature::Update(const itk::Index<2>& filledPixel)
{
  itk::ImageRegion<2> sourceRegion
  itk::ImageRegion<2> targetRegion
  ITKHelpers::CopySelfRegion(this->CurvatureImage, sourceRegion, targetRegion);
}

float PriorityCurvature::ComputePriority(const itk::Index<2>& queryPixel) const
{
  float priority;
  return priority;
}
