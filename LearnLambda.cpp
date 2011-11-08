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

#include "CriminisiInpainting.h"

#include "PatchPair.h"
#include "PixelDifference.h"
#include "SelfPatchCompare.h"

// ITK
#include "itkAddImageFilter.h"
#include "itkImageFileReader.h"
#include "itkXorImageFilter.h"

void ModifyMask(const Mask::Pointer inputMask, const unsigned int radius, Mask::Pointer outputMask);

int main(int argc, char *argv[])
{
  // Verify arguments
  if(argc != 5)
    {
    std::cerr << "Required arguments: image imageMask patchRadius outputPrefix" << std::endl;
    return EXIT_FAILURE;
    }

  // Parse arguments
  std::string imageFilename = argv[1];
  std::string maskFilename = argv[2];

  std::stringstream ssPatchRadius;
  ssPatchRadius << argv[3];
  int patchRadius = 0;
  ssPatchRadius >> patchRadius;

  std::string outputPrefix = argv[4];
  
  // Output arguments
  std::cout << "Reading image: " << imageFilename << std::endl;
  std::cout << "Reading mask: " << maskFilename << std::endl;
  std::cout << "Patch radius: " << patchRadius << std::endl;
  //std::cout << "Output: " << outputFilename << std::endl;

  typedef  itk::ImageFileReader< FloatVectorImageType > ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFilename.c_str());
  imageReader->Update();
  
  FloatVectorImageType::Pointer scaledImage = FloatVectorImageType::New();
//   for(unsigned int channel = 0; channel < imageReader->GetOutput()->GetNumberOfComponentsPerPixel(); ++channel)
//     {
//     Helpers::ScaleChannel<float>(imageReader->GetOutput(), channel, 1.0f, scaledImage);
//     }
  
  // Scale color channels
  for(unsigned int channel = 0; channel < 3; ++channel)
    {
    Helpers::ScaleChannel<float>(imageReader->GetOutput(), channel, 0.33f, scaledImage);
    }

  // Scale depth channel
  Helpers::ScaleChannel<float>(imageReader->GetOutput(), 3, 1.0f, scaledImage);
  
  typedef  itk::ImageFileReader< Mask > MaskReaderType;
  MaskReaderType::Pointer maskReader = MaskReaderType::New();
  maskReader->SetFileName(maskFilename.c_str());
  maskReader->Update();
  
  Mask::Pointer finalMask = Mask::New();
  ModifyMask(maskReader->GetOutput(), patchRadius, finalMask);

  std::vector<float> lambdas;
  for(unsigned int i = 0; i <= 10; ++i)
    {
    lambdas.push_back(0.1f * static_cast<float>(i));
    std::cout << "Using lambda " << lambdas[i] << std::endl;
    }

  // We have to create an empty CandidatePairs object to create the SelfPatchCompare object.
  SelfPatchCompare* patchCompare = new SelfPatchCompare;
  
  std::ofstream fout("scores.txt");
   
  for(unsigned int lambdaId = 0; lambdaId < lambdas.size(); ++lambdaId)
    {
    // Inpaint
    std::cout << "Inpainting with lambda = " << lambdas[lambdaId] << std::endl;
  
    PatchPair::DepthColorLambda = lambdas[lambdaId];
  
    CriminisiInpainting inpainting;
    inpainting.SetDebugFunctionEnterLeave(true);
    inpainting.SetPatchRadius(patchRadius);
    inpainting.SetImage(scaledImage);
    inpainting.SetMask(finalMask);
    inpainting.SetMaxForwardLookPatches(3);
    inpainting.SetPatchCompare(patchCompare);
    inpainting.Initialize();
    //inpainting.PatchSortFunction = &SortByDepthAndColor;
    inpainting.PatchSortFunction = &SortByAverageAbsoluteDifference;
    inpainting.Inpaint();

    // Compute error
    itk::ImageRegionIterator<Mask> iterator(finalMask, finalMask->GetLargestPossibleRegion());
    float depthError = 0.0f;
    float colorError = 0.0f;
    
    while(!iterator.IsAtEnd())
      {
      if(finalMask->IsHole(iterator.GetIndex()))
	{
	colorError += ColorPixelDifference::Difference(scaledImage->GetPixel(iterator.GetIndex()), inpainting.GetCurrentOutputImage()->GetPixel(iterator.GetIndex()));
	depthError += DepthPixelDifference::Difference(scaledImage->GetPixel(iterator.GetIndex()), inpainting.GetCurrentOutputImage()->GetPixel(iterator.GetIndex()));
	}
      ++iterator;
      }
    std::cout << "colorError: " << colorError << std::endl;
    std::cout << "depthError: " << depthError << std::endl;
    
    fout << colorError << " " << depthError << std::endl;
    
    std::stringstream ssFloat;
    ssFloat << outputPrefix << "_float_lambda_" << lambdas[lambdaId] << ".mha";
    Helpers::WriteImage<FloatVectorImageType>(inpainting.GetCurrentOutputImage(), ssFloat.str());
    
    std::stringstream ssRGB;
    ssRGB << outputPrefix << "_RGB_lambda_" << lambdas[lambdaId] << ".mha";
    Helpers::WriteVectorImageAsRGB(inpainting.GetCurrentOutputImage(), ssRGB.str());
    //Helpers::WriteVectorImageAsRGB(inpainting.GetCurrentOutputImage(), Helpers::ReplaceFileExtension(ss.str(), "png"));
    }
    
  fout.close();
  
  return EXIT_SUCCESS;
}

void ModifyMask(const Mask::Pointer inputMask, const unsigned int patchRadius, Mask::Pointer outputMask)
{

  Mask::Pointer originalHole = Mask::New();
  Helpers::DeepCopy<Mask>(inputMask, originalHole);
  Helpers::ChangeValue<Mask>(originalHole, 255, 122);
  
  // Expand (dilate) the hole and subtract the old hole. This leaves a region where we know what the solution of the inpainting should be.
  // Dilate the hole.
  Mask::Pointer dilatedImage = Mask::New();
  Helpers::DilateImage<Mask>(inputMask, dilatedImage, patchRadius);
  Helpers::WriteImage<Mask>(dilatedImage, "Debug/DilatedImage.mha");
  
  // Subtract the old hole
  typedef itk::XorImageFilter <Mask> XorImageFilterType;
  XorImageFilterType::Pointer xorFilter = XorImageFilterType::New();
  xorFilter->SetInput1(inputMask);
  xorFilter->SetInput2(dilatedImage);
  xorFilter->Update();
  Helpers::WriteImage<Mask>(xorFilter->GetOutput(), "Debug/DilatedAndSubtractedImage.mha");

  Mask::Pointer thinMask = Mask::New();
  Helpers::DeepCopy<Mask>(xorFilter->GetOutput(), thinMask);
  
  // At this point we have a thin region around the original hole. We do not need to use this entire region to learn lambda.
  // To reduce the region, we copy large black blocks over random pixels in the thin region.
  std::vector<itk::Index<2> > nonZeroPixels = Helpers::GetNonZeroPixels<Mask>(thinMask);
  
  unsigned int originalNumberOfNonZeroPixels = nonZeroPixels.size();
  
  // Copy zero blocks
  float ratioToKeep = .3;
  while(static_cast<float>(Helpers::CountNonZeroPixels<Mask>(thinMask))/static_cast<float>(originalNumberOfNonZeroPixels) > ratioToKeep)
    {
    unsigned int pixelId = rand() % nonZeroPixels.size();
  
    itk::ImageRegion<2> region = Helpers::GetRegionInRadiusAroundPixel(nonZeroPixels[pixelId], patchRadius*2);
    Helpers::SetRegionToConstant<Mask>(thinMask, region, 0);

    nonZeroPixels = Helpers::GetNonZeroPixels<Mask>(thinMask);
    }
    
  // Combine
  typedef itk::AddImageFilter <Mask, Mask> AddImageFilterType;
  AddImageFilterType::Pointer addFilter = AddImageFilterType::New ();
  addFilter->SetInput1(originalHole);
  addFilter->SetInput2(thinMask);
  addFilter->Update();
  Helpers::WriteImage<Mask>(addFilter->GetOutput(), "Debug/FinalHole.mha");
  
  
  Helpers::DeepCopy<Mask>(addFilter->GetOutput(), outputMask);  
}
