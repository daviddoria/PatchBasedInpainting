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

#include "PriorityCriminisi.h"
#include "../ImageProcessing/Mask.h"
#include "../Testing/Testing.h"

int main()
{
  FloatVectorImageType::Pointer image = FloatVectorImageType::New();
  Testing::GetBlankImage(image.GetPointer(), 4);

  Mask::Pointer mask = Mask::New();
  Testing::GetFullyValidMask(mask.GetPointer());

  unsigned int patchRadius = 5;
  PriorityCriminisi<FloatVectorImageType> priority(image, mask, patchRadius);

  itk::Index<2> filledPixel;
  filledPixel.Fill(0);
  priority.Update(filledPixel);

  itk::Index<2> queryPixel = {{0,0}};
  priority.ComputePriority(queryPixel);

  return EXIT_SUCCESS;
}
