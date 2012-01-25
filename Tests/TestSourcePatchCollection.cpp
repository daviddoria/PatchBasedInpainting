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

#include "SourcePatchCollection.h"

#include "ImageProcessing/Mask.h"

static void CreateMask(Mask* mask);

int main(int argc, char*argv[])
{
  Mask::Pointer mask = Mask::New();
  CreateMask(mask);

  FloatScalarImageType::Pointer image = FloatScalarImageType::New();

  const unsigned int patchRadius = 5;
  SourcePatchCollection<FloatScalarImageType> sourcePatchCollection(image, mask, patchRadius);

  if(sourcePatchCollection.GetNumberOfPatches() != 0)
    {
    std::cerr << "Should be 0 patches after construction!" << std::endl;
    return EXIT_FAILURE;
    }

  SourcePatchCollection<FloatScalarImageType>::PatchContainer patches = sourcePatchCollection.FindSourcePatchesInRegion(mask->GetLargestPossibleRegion());

  sourcePatchCollection.AddPatches(patches);
  unsigned int correctNumberOfPatches = 2160;
  if(sourcePatchCollection.GetNumberOfPatches() != correctNumberOfPatches)
    {
    std::cerr << "Should be " << correctNumberOfPatches << " patches after construction, but there are " << sourcePatchCollection.GetNumberOfPatches() << std::endl;
    return EXIT_FAILURE;
    }

  // Iterate over all patches
  unsigned int patchCounter = 0;
  for(SourcePatchCollection<FloatScalarImageType>::Iterator patchIterator = sourcePatchCollection.begin(); patchIterator != sourcePatchCollection.end(); ++patchIterator)
    {
    patchCounter++;
    }

  if(sourcePatchCollection.GetNumberOfPatches() != patchCounter)
    {
    std::cerr << "Iterated over " << patchCounter << " patches but should have been " << sourcePatchCollection.GetNumberOfPatches() << std::endl;
    return EXIT_FAILURE;
    }

  sourcePatchCollection.Clear();

  if(sourcePatchCollection.GetNumberOfPatches() != 0)
    {
    std::cerr << "Should be 0 patches after clearing." << std::endl;
    return EXIT_FAILURE;
    }

  // TODO Test this.
  // const Patch* patch = sourcePatchCollection.GetPatch(const itk::ImageRegion<2>& region);
  return EXIT_SUCCESS;
}

void CreateMask(Mask* mask)
{
  itk::Index<2> maskCorner;
  maskCorner.Fill(0);

  itk::Size<2> maskSize;
  maskSize.Fill(100);

  itk::ImageRegion<2> maskRegion(maskCorner, maskSize);

  mask->SetRegions(maskRegion);
  mask->Allocate();

  unsigned int indeterminateValue = (static_cast<unsigned int>(mask->GetHoleValue()) + static_cast<unsigned int>(mask->GetValidValue()))/2;

  // Make the left half of the mask the hole, the center indeterminate, and the right half valid.
  for(unsigned int column = 0; column < maskSize[0]; ++column)
    {
    for(unsigned int row = 0; row < maskSize[1]; ++row)
      {
      itk::Index<2> currentIndex;
      currentIndex[0] = column;
      currentIndex[1] = row;
      if(column < maskSize[0] / 3)
        {
        mask->SetPixel(currentIndex, mask->GetHoleValue());
        }
      else if(column < 2*maskSize[0]/3)
        {
        mask->SetPixel(currentIndex, indeterminateValue);
        }
      else
        {
        mask->SetPixel(currentIndex, mask->GetValidValue());
        }
      }
    }
}
