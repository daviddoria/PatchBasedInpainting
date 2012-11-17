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

// Custom
#include "Utilities/PatchHelpers.h"
#include "DifferenceFunctions/Patch/GMHDifference.hpp"

// Submodules
#include <Helpers/ParallelSort.h>
#include <Mask/MaskOperations.h>

int main(int argc, char *argv[])
{
  if(argc < 2)
  {
    std::cerr << "Required arguments: fileName.png" << std::endl;
    return EXIT_FAILURE;
  }
//  typedef itk::Image<itk::CovariantVector<int, 3>, 2> ImageType;
  typedef itk::Image<itk::CovariantVector<unsigned char, 3>, 2> ImageType;

  std::string fileName = argv[1];

  typedef itk::Image<itk::CovariantVector<unsigned char, 3>, 2> ImageType;

  typedef std::vector<ImageType::Pointer> PatchesContainerType;
  PatchesContainerType patches =
    PatchHelpers::ReadTopPatchesGrid(fileName, 31, 10, 10);

  itk::Size<2> patchSize = patches[0]->GetLargestPossibleRegion().GetSize();

  std::vector<float> differences;

  unsigned int referencePatchId = 1;

  for(unsigned int i = 0; i < patches.size(); ++i)
  {
    itk::Index<2> dualCorner = {{0,0}};
    itk::Size<2> dualSize = {{patchSize[0]*2, patchSize[1]}};
    itk::ImageRegion<2> dualRegion(dualCorner, dualSize);
    ImageType::Pointer dualImage = ImageType::New();
    dualImage->SetRegions(dualRegion);
    dualImage->Allocate();

    Mask::Pointer mask = Mask::New();
    mask->SetRegions(dualRegion);
    mask->Allocate();
    ITKHelpers::SetImageToConstant(mask.GetPointer(), mask->GetValidValue());

    itk::Index<2> corner1 = {{0,0}};
    itk::ImageRegion<2> region1(corner1, patchSize);

    itk::Index<2> corner2 = {{static_cast<itk::Index<2>::IndexValueType>(patchSize[0]), 0}};
    itk::ImageRegion<2> region2(corner2, patchSize);


    ITKHelpers::CopyRegion(patches[referencePatchId].GetPointer(), dualImage.GetPointer(),
        patches[0]->GetLargestPossibleRegion(), region1);
    ITKHelpers::CopyRegion(patches[i].GetPointer(), dualImage.GetPointer(),
                           patches[0]->GetLargestPossibleRegion(), region2);

    ImageType::Pointer blurredImage = ImageType::New();
    float blurVariance = 1.2f;
    MaskOperations::MaskedBlur(dualImage.GetPointer(), mask, blurVariance, blurredImage.GetPointer());

//    GMHDifference<ImageType> gmhDifference(dualImage.GetPointer(), mask, 30);
    GMHDifference<ImageType> gmhDifference(blurredImage.GetPointer(), mask, 30);
    float difference = gmhDifference.Difference(region1, region2);

    differences.push_back(difference);
//    std::cout << difference << std::endl;
  }

  typedef ParallelSort<float> ParallelSortType;

  ParallelSortType::IndexedVector sortedDifferences =
      ParallelSortType::ParallelSortAscending(differences);

  PatchesContainerType sortedPatches(sortedDifferences.size());

  for(unsigned int i = 0; i < sortedDifferences.size(); ++i)
  {
    sortedPatches[i] = patches[sortedDifferences[i].index];
  }

  PatchHelpers::WriteTopPatchesGrid(sortedPatches,
                           10, 10,
                           "SortedPatches.png");


  return EXIT_SUCCESS;
}

