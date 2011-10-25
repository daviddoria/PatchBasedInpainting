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

#include "Mask.h"
#include "Types.h"
#include "CriminisiInpainting.h"

#include "itkImageFileReader.h"
#include "itkRGBToLuminanceImageFilter.h"

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

  std::cout << "Read image " << imageReader->GetOutput()->GetLargestPossibleRegion() << std::endl;

  typedef itk::ImageFileReader<Mask> MaskReaderType;
  MaskReaderType::Pointer maskReader = MaskReaderType::New();
  maskReader->SetFileName(maskFilename.c_str());
  maskReader->Update();

  std::cout << "Read mask " << maskReader->GetOutput()->GetLargestPossibleRegion() << std::endl;

  // Prepare image
  RGBImageType::Pointer rgbImage = RGBImageType::New();
  Helpers::VectorImageToRGBImage(imageReader->GetOutput(), rgbImage);

  Helpers::WriteImage<RGBImageType>(rgbImage, "Test/TestIsophotes.rgb.mha");

  typedef itk::RGBToLuminanceImageFilter< RGBImageType, FloatScalarImageType > LuminanceFilterType;
  LuminanceFilterType::Pointer luminanceFilter = LuminanceFilterType::New();
  luminanceFilter->SetInput(rgbImage);
  luminanceFilter->Update();

  FloatScalarImageType::Pointer blurredLuminance = FloatScalarImageType::New();
  // Blur with a Gaussian kernel
  unsigned int kernelRadius = 5;
  Helpers::MaskedBlur<FloatScalarImageType>(luminanceFilter->GetOutput(), maskReader->GetOutput(), kernelRadius, blurredLuminance);

  Helpers::WriteImage<FloatScalarImageType>(blurredLuminance, "Test/TestIsophotes.blurred.mha");


  CriminisiInpainting inpainting;
  //inpainting.SetMask(maskReader->GetOutput());
  //inpainting.SetImage(imageReader->GetOutput());
  inpainting.ComputeMaskedIsophotes(blurredLuminance, maskReader->GetOutput());

  //Helpers::WriteImage<FloatVector2ImageType>(inpainting.GetIsophoteImage(), );
  Helpers::Write2DVectorImage(inpainting.GetIsophoteImage(), "Test/TestIsophotes.isophotes.mha");

  itk::Size<2> size;
  size.Fill(21);

  // Target
  itk::Index<2> targetIndex;
  targetIndex[0] = 187;
  targetIndex[1] = 118;
  itk::ImageRegion<2> targetRegion(targetIndex, size);

  // Source
  itk::Index<2> sourceIndex;
  sourceIndex[0] = 176;
  sourceIndex[1] = 118;
  itk::ImageRegion<2> sourceRegion(sourceIndex, size);

  //PatchPair patchPair(Patch(sourceRegion), Patch(targetRegion));
  //PatchPair patchPair;
  Patch sourcePatch(sourceRegion);
  Patch targetPatch(targetRegion);
  PatchPair patchPair(sourcePatch, targetPatch);

  inpainting.FindBoundary();

  std::vector<itk::Index<2> > borderPixels = Helpers::GetNonZeroPixels<UnsignedCharScalarImageType>(inpainting.GetBoundaryImage(), targetRegion);

  for(unsigned int pixelId = 0; pixelId < borderPixels.size(); ++pixelId)
    {
    itk::Index<2> currentPixel = borderPixels[pixelId];
    itk::Index<2> adjacentBoundaryPixel;
    //bool valid = GetAdjacentBoundaryPixel(currentPixel, candidatePairs[sourcePatchId], adjacentBoundaryPixel);
    bool valid = inpainting.GetAdjacentBoundaryPixel(currentPixel, patchPair, adjacentBoundaryPixel);
    if(!valid)
      {
      continue;
      }
    }

  return EXIT_SUCCESS;
}
