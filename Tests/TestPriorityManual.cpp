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

#include "PriorityManual.h"
#include "PriorityOnionPeel.h"
#include "Mask.h"
#include "NamedVTKImage.h"

int main()
{
  FloatVectorImageType::Pointer image = FloatVectorImageType::New();
  Mask::Pointer mask = Mask::New();
  unsigned int patchRadius = 5;
  PriorityManual<PriorityOnionPeel> priority(image, mask, patchRadius);

  priority.ComputeAllPriorities();

  itk::ImageRegion<2> filledRegion;
  priority.Update(filledRegion);

  // Get the current priority image
  FloatScalarImageType* priorityImage = priority.GetPriorityImage();

  // Get the current boundary image
  UnsignedCharScalarImageType* boundaryImage = priority.GetBoundaryImage();

  UnsignedCharScalarImageType::Pointer manualPriorityImage = UnsignedCharScalarImageType::New();
  priority.SetManualPriorityImage(manualPriorityImage);

  itk::Index<2> queryPixel;
  priority.GetPriority(queryPixel);

  priority.UpdateBoundary();

  std::vector<NamedVTKImage> namedImages = priority.GetNamedImages();
  std::vector<std::string> imageNames = priority.GetImageNames();
  return 0;
}
