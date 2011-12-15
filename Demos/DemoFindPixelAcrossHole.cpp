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
#include "HelpersOutput.h"
#include "Mask.h"
#include "Types.h"

// ITK
#include "itkImageFileReader.h"
#include "itkMaskImageFilter.h"
#include "itkRGBToLuminanceImageFilter.h"

static void CreateMask(Mask::Pointer mask);

int main(int argc, char *argv[])
{
  Mask::Pointer mask = Mask::New();
  CreateMask(mask);

  itk::Index<2> queryPixel;
  queryPixel[0] = 5;
  queryPixel[1] = 5;

  FloatVector2Type direction;
  direction[0] = 1;
  direction[1] = 1;
  direction.Normalize();

  itk::Index<2> pixelAcross = mask->FindPixelAcrossHole(queryPixel, direction);

  std::cout << "Pixel across: " << pixelAcross << std::endl;

  //HelpersOutput::WriteImage<Mask>(blurredLuminance, "Test/TestIsophotes.blurred.mha");

  return EXIT_SUCCESS;
}

static void CreateMask(Mask::Pointer mask)
{
  itk::Size<2> size;
  size.Fill(20);

  itk::Index<2> start;
  start.Fill(0);

  itk::ImageRegion<2> region(start,size);

  mask->SetRegions(region);
  mask->Allocate();
  mask->FillBuffer(mask->GetValidValue());

  itk::ImageRegionIterator<Mask> iterator(mask, mask->GetLargestPossibleRegion());

  while(!iterator.IsAtEnd())
    {
    if(iterator.GetIndex()[0] > 5 && iterator.GetIndex()[0] < 15 &&
       iterator.GetIndex()[1] > 5 && iterator.GetIndex()[1] < 15)
      {
      mask->SetPixel(iterator.GetIndex(), mask->GetHoleValue());
      }

    ++iterator;
    }
}
