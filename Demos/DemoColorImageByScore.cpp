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
#include "PatchBasedInpainting.h"
#include "HelpersOutput.h"
#include "Mask.h"
#include "PatchPair.h"
#include "PixelDifference.h"
#include "SelfPatchCompare.h"
#include "Types.h"

// ITK
#include "itkImageFileReader.h"

// Boost
#include <boost/bind.hpp>

int main(int argc, char *argv[])
{
  if(argc != 3)
    {
    std::cerr << "Required arguments: image mask" << std::endl;
    return EXIT_FAILURE;
    }
  std::string imageFilename = argv[1];
  std::string maskFilename = argv[2];
  std::cout << "Reading image: " << imageFilename << std::endl;
  std::cout << "Reading mask: " << maskFilename << std::endl;

  typedef itk::ImageFileReader<FloatVectorImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFilename.c_str());
  imageReader->Update();

  typedef itk::ImageFileReader<Mask> MaskReaderType;
  MaskReaderType::Pointer maskReader = MaskReaderType::New();
  maskReader->SetFileName(maskFilename.c_str());
  maskReader->Update();

  std::shared_ptr<SelfPatchCompare> patchCompare(new SelfPatchCompare);
  patchCompare->SetNumberOfComponentsPerPixel(imageReader->GetOutput()->GetNumberOfComponentsPerPixel());
  //patchCompare->FunctionsToCompute.push_back(boost::bind(&SelfPatchCompare::SetPatchAverageAbsoluteSourceDifference,patchCompare,_1));
  //patchCompare->FunctionsToCompute.push_back(boost::bind(&SelfPatchCompare::SetPatchColorDifference,patchCompare,_1));
  patchCompare->FunctionsToCompute.push_back(boost::bind(&SelfPatchCompare::SetPatchDepthDifference,patchCompare.get(),_1));
  //patchCompare->SetImage(imageReader->GetOutput());
  //patchCompare->SetMask(maskReader->GetOutput());

  PatchBasedInpainting inpainting;
  inpainting.SetMask(maskReader->GetOutput());
  inpainting.SetImage(imageReader->GetOutput());
  //inpainting.PatchSortFunction = &SortByDepthDifference;
  //inpainting.SetPatchCompare(patchCompare);
  inpainting.Initialize();
  inpainting.Iterate();

  CandidatePairs candidatePairs = inpainting.GetPotentialCandidatePairs()[0];
  //patchCompare->SetPairs(&candidatePairs);
  //patchCompare->ComputeAllSourceDifferences();
  WriteImageOfScores(candidatePairs, imageReader->GetOutput()->GetLargestPossibleRegion(), "ScoreImage_TargetPatch.mha");

  return EXIT_SUCCESS;
}
