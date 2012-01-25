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

#include "PriorityFactory.h"
#include "Priority.h"
#include "Mask.h"
#include "Testing.h"

int main()
{
  FloatVectorImageType::Pointer image = FloatVectorImageType::New();
  Testing::GetBlankImage(image.GetPointer(), 3);

  Mask::Pointer mask = Mask::New();
  Testing::GetFullyValidMask(mask.GetPointer());

  const unsigned int patchRadius = 5;
  Priority<FloatVectorImageType>* priority = PriorityFactory<FloatVectorImageType>::Create(PriorityFactory<FloatVectorImageType>::RANDOM, image, mask, patchRadius);

  if(!priority)
    {
    throw std::runtime_error("priority was not created correctly!");
    }

  std::vector<std::string> imageNames = PriorityFactory<FloatVectorImageType>::GetImageNames(PriorityFactory<FloatVectorImageType>::RANDOM);

  return EXIT_SUCCESS;
}
