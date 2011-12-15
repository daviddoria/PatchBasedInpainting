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
#include "Helpers.h"
#include "HelpersOutput.h"
#include "Mask.h"
#include "PatchBasedInpainting.h"
#include "Types.h"

// ITK
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

  HelpersOutput::WriteImage<RGBImageType>(rgbImage, "Test/TestIsophotes.rgb.mha");

  typedef itk::RGBToLuminanceImageFilter< RGBImageType, FloatScalarImageType > LuminanceFilterType;
  LuminanceFilterType::Pointer luminanceFilter = LuminanceFilterType::New();
  luminanceFilter->SetInput(rgbImage);
  luminanceFilter->Update();

  FloatScalarImageType::Pointer blurredLuminance = FloatScalarImageType::New();
  // Blur with a Gaussian kernel
  unsigned int kernelRadius = 5;
  Helpers::MaskedBlur<FloatScalarImageType>(luminanceFilter->GetOutput(), maskReader->GetOutput(), kernelRadius, blurredLuminance);

  HelpersOutput::WriteImage<FloatScalarImageType>(blurredLuminance, "Test/TestIsophotes.blurred.mha");


  PatchBasedInpainting inpainting;
  inpainting.SetMask(maskReader->GetOutput());
  inpainting.SetImage(imageReader->GetOutput());
  //inpainting.ComputeMaskedIsophotes(blurredLuminance, maskReader->GetOutput());

  //Helpers::WriteImage<FloatVector2ImageType>(inpainting.GetIsophoteImage(), );
  //HelpersOutput::Write2DVectorImage(inpainting.GetIsophoteImage(), "Test/TestIsophotes.isophotes.mha");

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

  //inpainting.FindBoundary();

  //std::vector<itk::Index<2> > borderPixels = Helpers::GetNonZeroPixels<UnsignedCharScalarImageType>(inpainting.GetBoundaryImage(), targetRegion);

  itk::RGBPixel<unsigned char> black;
  black.SetRed(0);
  black.SetGreen(0);
  black.SetBlue(0);

  itk::RGBPixel<unsigned char> red;
  red.SetRed(255);
  red.SetGreen(0);
  red.SetBlue(0);

  itk::RGBPixel<unsigned char> darkRed;
  darkRed.SetRed(100);
  darkRed.SetGreen(0);
  darkRed.SetBlue(0);

  itk::RGBPixel<unsigned char> yellow;
  yellow.SetRed(255);
  yellow.SetGreen(255);
  yellow.SetBlue(0);

  itk::RGBPixel<unsigned char> green;
  green.SetRed(0);
  green.SetGreen(255);
  green.SetBlue(0);

  itk::RGBPixel<unsigned char> darkGreen;
  darkGreen.SetRed(0);
  darkGreen.SetGreen(100);
  darkGreen.SetBlue(0);

  itk::RGBPixel<unsigned char> blue;
  blue.SetRed(0);
  blue.SetGreen(0);
  blue.SetBlue(255);

  RGBImageType::Pointer output = RGBImageType::New();
  output->SetRegions(imageReader->GetOutput()->GetLargestPossibleRegion());
  output->Allocate();
  output->FillBuffer(black);

  Helpers::BlankAndOutlineRegion<RGBImageType>(output, targetRegion, black, red);
  Helpers::BlankAndOutlineRegion<RGBImageType>(output, sourceRegion, black, green);

  RGBImageType::Pointer target = RGBImageType::New();
  target->SetRegions(imageReader->GetOutput()->GetLargestPossibleRegion());
  target->Allocate();
  Helpers::BlankAndOutlineRegion<RGBImageType>(target, targetRegion, black, red);

  RGBImageType::Pointer source = RGBImageType::New();
  source->SetRegions(imageReader->GetOutput()->GetLargestPossibleRegion());
  source->Allocate();
  Helpers::BlankAndOutlineRegion<RGBImageType>(source, sourceRegion, black, green);

  itk::Offset<2> offset = targetIndex - sourceIndex;
  /*
  for(unsigned int pixelId = 0; pixelId < borderPixels.size(); ++pixelId)
    {
    itk::Index<2> targetPatchSourceSideBoundaryPixel = borderPixels[pixelId];
    itk::Index<2> sourcePatchTargetSideBoundaryPixel;
    //bool valid = GetAdjacentBoundaryPixel(currentPixel, candidatePairs[sourcePatchId], adjacentBoundaryPixel);
    bool valid = inpainting.GetAdjacentBoundaryPixel(targetPatchSourceSideBoundaryPixel, patchPair, sourcePatchTargetSideBoundaryPixel);

    target->SetPixel(targetPatchSourceSideBoundaryPixel, darkRed);
    source->SetPixel(sourcePatchTargetSideBoundaryPixel, darkGreen);

    if(!valid)
      {
      continue;
      }

    // Bring the adjacent pixel back to the target region.
    itk::Index<2> targetPatchTargetSideBoundaryPixel = sourcePatchTargetSideBoundaryPixel + offset;

    output->SetPixel(targetPatchSourceSideBoundaryPixel, darkRed);

    output->SetPixel(targetPatchTargetSideBoundaryPixel, blue);
    output->SetPixel(sourcePatchTargetSideBoundaryPixel, darkGreen);
    }
  */

  HelpersOutput::WriteImage<RGBImageType>(output, "Test/FollowIsophotes.Output.mha");
  HelpersOutput::WriteImage<RGBImageType>(target, "Test/FollowIsophotes.Target.mha");
  HelpersOutput::WriteImage<RGBImageType>(source, "Test/FollowIsophotes.Source.mha");

  return EXIT_SUCCESS;
}
