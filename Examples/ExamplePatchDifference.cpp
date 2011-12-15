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

#include "Types.h"
#include "SelfPatchCompareAll.h"

#include "itkImageFileReader.h"

int main(int argc, char *argv[])
{
  if(argc != 3)
    {
    std::cerr << "Only gave " << argc << " arguments!" << std::endl;
    std::cerr << "Required arguments: image mask" << std::endl;
    return EXIT_FAILURE;
    }
  std::string imageFilename = argv[1];
  std::string maskFilename = argv[2];
  std::cout << "Reading image: " << imageFilename << std::endl;
  std::cout << "Reading mask: " << maskFilename << std::endl;

  typedef itk::ImageFileReader<FloatVectorImageType> VectorImageReaderType;
  VectorImageReaderType::Pointer imageReader = VectorImageReaderType::New();
  imageReader->SetFileName(imageFilename.c_str());
  imageReader->Update();

  typedef itk::ImageFileReader<Mask> MaskReaderType;
  MaskReaderType::Pointer maskReader = MaskReaderType::New();
  maskReader->SetFileName(maskFilename.c_str());
  maskReader->Update();

  itk::Size<2> size;
  size.Fill(21);

  itk::Index<2> sourceCorner;
  sourceCorner[0] = 319;
  sourceCorner[1] = 292;
  itk::ImageRegion<2> sourceRegion(sourceCorner, size);

  itk::Index<2> targetCorner;
  targetCorner[0] = 193;
  targetCorner[1] = 218;
  itk::ImageRegion<2> targetRegion(targetCorner, size);

  Patch targetPatch(targetRegion);
  Patch sourcePatch(sourceRegion);

  CandidatePairs candidatePairs(targetPatch);
  candidatePairs.AddPairFromPatch(sourcePatch);

  SelfPatchCompareAll patchCompare(imageReader->GetOutput()->GetNumberOfComponentsPerPixel(), candidatePairs);
  patchCompare.SetImage(imageReader->GetOutput());
  patchCompare.SetMask(maskReader->GetOutput());

  patchCompare.ComputeAllDifferences();

  //float totalAbsoluteDifference = patchCompare.SlowTotalAbsoluteDifference(sourceRegion);
  //float totalSquaredDifference = patchCompare.SlowTotalSquaredDifference(sourceRegion);

  std::cerr << "Average Absolute Difference: " << candidatePairs[0].GetAverageAbsoluteDifference() << std::endl;
  //std::cerr << "Total Squared Difference: " << totalSquaredDifference << std::endl;

  return EXIT_SUCCESS;
}
