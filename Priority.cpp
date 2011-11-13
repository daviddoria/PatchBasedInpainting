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

#include "Priority.h"

#include "Helpers.h"

Priority::Priority(FloatVectorImageType::Pointer image, Mask::Pointer maskImage, unsigned int patchRadius)
                    //Image(image), MaskImage(maskImage), PatchRadius(patchRadius)
{
  this->Image = image;
  this->MaskImage = maskImage;
  this->PatchRadius = patchRadius;

  this->DebugFunctionEnterLeave = true;
  EnterFunction("Priority()");
  this->PriorityImage = FloatScalarImageType::New();
  Helpers::InitializeImage<FloatScalarImageType>(this->PriorityImage, image->GetLargestPossibleRegion());
  Helpers::SetImageToConstant<FloatScalarImageType>(this->PriorityImage, 0.0f);

  this->BoundaryImage = UnsignedCharScalarImageType::New();
  Helpers::InitializeImage<UnsignedCharScalarImageType>(this->BoundaryImage, image->GetLargestPossibleRegion());
  Helpers::SetImageToConstant<UnsignedCharScalarImageType>(this->BoundaryImage, 0u);
  LeaveFunction("Priority()");

  std::cout << "In Priority class, mask pointer is: " << this->MaskImage << std::endl;
}

void Priority::Update(const itk::ImageRegion<2>& filledRegion)
{
  // Do nothing in the default implementation.
}

FloatScalarImageType::Pointer Priority::GetPriorityImage()
{
  return this->PriorityImage;
}

UnsignedCharScalarImageType::Pointer Priority::GetBoundaryImage()
{
  return this->BoundaryImage;
}

float Priority::GetPriority(const itk::Index<2>& queryPixel)
{
  return this->PriorityImage->GetPixel(queryPixel);
}

/*
void Priority::ComputeAllPriorities()
{
  EnterFunction("ComputeAllPriorities()");
  try
  {
    // Only compute priorities for pixels on the boundary
    itk::ImageRegionConstIterator<UnsignedCharScalarImageType> boundaryIterator(this->BoundaryImage, this->BoundaryImage->GetLargestPossibleRegion());

    // Blank the priority image.
    this->PriorityImage->FillBuffer(0);

    // The main loop is over the boundary image. We only want to compute priorities at boundary pixels.
    unsigned int boundaryPixelCounter = 0;
    while(!boundaryIterator.IsAtEnd())
      {
      if(boundaryIterator.Get() != 0) // Pixel is on the boundary
	{
	float priority = ComputePriority(boundaryIterator.GetIndex());
	//DebugMessage<float>("Priority: ", priority);
	this->PriorityImage->SetPixel(boundaryIterator.GetIndex(), priority);
	boundaryPixelCounter++;
	}    
      ++boundaryIterator;
      }
    DebugMessage<unsigned int>("Number of boundary pixels: ", boundaryPixelCounter);
    LeaveFunction("ComputeAllPriorities()");
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeAllPriorities!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}*/

void Priority::UpdateBoundary()
{
  EnterFunction("UpdateBoundary()");
  this->MaskImage->FindBoundary(this->BoundaryImage);
  LeaveFunction("UpdateBoundary()");
}

void Priority::ComputeAllPriorities()
{
  EnterFunction("ComputeAllPriorities()");
  this->MaskImage->FindBoundary(this->BoundaryImage);

  Helpers::SetImageToConstant<FloatScalarImageType>(this->PriorityImage, 0.0f);

  std::vector<itk::Index<2> > boundaryPixels = Helpers::GetNonZeroPixels<UnsignedCharScalarImageType>(this->BoundaryImage);

  for(unsigned int pixelId = 0; pixelId < boundaryPixels.size(); ++pixelId)
    {
    this->PriorityImage->SetPixel(boundaryPixels[pixelId], ComputePriority(boundaryPixels[pixelId]));
    }
  LeaveFunction("ComputeAllPriorities()");
}
