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

int main(int argc, char*argv[])
{
  itk::Index<2> corner;
  corner.Fill(0);

  itk::Size<2> size;
  size.Fill(100);

  itk::ImageRegion<2> region(corner, size);

  Mask::Pointer mask = Mask::New();
  mask->SetRegions(region);
  mask->Allocate();

  unsigned int indeterminateValue = (static_cast<unsigned int>(mask->GetHoleValue()) + static_cast<unsigned int>(mask->GetValidValue()))/2;

  // Make the left half of the mask the hole, the center indeterminate, and the right half valid.
  for(unsigned int column = 0; column < size[0]; ++column)
    {
    for(unsigned int row = 0; row < size[1]; ++row)
      {
      itk::Index<2> currentIndex;
      currentIndex[0] = column;
      currentIndex[1] = row;
      if(column < size[0] / 3)
        {
        mask->SetPixel(currentIndex, mask->GetHoleValue());
        }
      else if(column < 2*size[0]/3)
        {
        mask->SetPixel(currentIndex, indeterminateValue);
        }
      else
        {
        mask->SetPixel(currentIndex, mask->GetValidValue());
        }
      }
    }

  // Test hole pixel
  itk::Index<2> leftHalfPixel;
  leftHalfPixel.Fill(0);
  if(!mask->IsHole(leftHalfPixel))
    {
    std::cerr << "Pixel should be hole!" << std::endl;
    return EXIT_FAILURE;
    }

  if(mask->IsValid(leftHalfPixel))
    {
    std::cerr << "Pixel should not be valid!" << std::endl;
    return EXIT_FAILURE;
    }

  // Test valid pixel
  itk::Index<2> rightHalfPixel;
  rightHalfPixel[0] = size[0] - 1;
  rightHalfPixel[1] = size[1] - 1;

  if(mask->IsHole(rightHalfPixel))
    {
    std::cerr << "Pixel should not be hole!" << std::endl;
    return EXIT_FAILURE;
    }

  if(!mask->IsValid(rightHalfPixel))
    {
    std::cerr << "Pixel should be valid!" << std::endl;
    return EXIT_FAILURE;
    }

  // Test region validity
  itk::Size<2> smallRegionSize;
  smallRegionSize.Fill(3);
  itk::Index<2> smallLeftRegionCorner;
  smallLeftRegionCorner.Fill(0);
  itk::ImageRegion<2> smallLeftRegion(smallLeftRegionCorner, smallRegionSize);

  if(mask->IsValid(smallLeftRegion))
    {
    std::cerr << "Region should not be valid!" << std::endl;
    return EXIT_FAILURE;
    }

  itk::Index<2> smallRightRegionCorner;
  smallRightRegionCorner[0] = size[0] - smallRegionSize[0] - 1;
  smallRightRegionCorner[1] = size[1] - smallRegionSize[0] - 1;
  itk::ImageRegion<2> smallRightRegion(smallRightRegionCorner, smallRegionSize);

  if(!mask->IsValid(smallRightRegion))
    {
    std::cerr << "Region should be valid!" << std::endl;
    return EXIT_FAILURE;
    }

  // TODO: Write tests for the rest of these functions.
  /*
  // Invert the mask by switching the hole and valid pixel values.
  void Invert();

  // Snap the pixel values to either 'hole' or 'valid'.
  void Cleanup();

  // Slightly dilate the hole.
  void ExpandHole();

  // Specify which value should be considered a hole.
  void SetHoleValue(const unsigned char value);

  // Specify which value should be considered valid.
  void SetValidValue(const unsigned char value);

  // Get the value that is considered a hole.
  unsigned char GetHoleValue() const;

  // Get the value that is considered valid.
  unsigned char GetValidValue() const;

  // Print information about the Mask.
  void OutputMembers() const;

  // Copy a mask.
  void DeepCopyFrom(const Mask* inputMask);

  // Find the boundary of the Mask.
  void FindBoundary(UnsignedCharScalarImageType* boundary) const;

  // Recolor the hole pixels in 'image' a specified 'color'.
  template<typename TImage, typename TColor>
  void ApplyColorToImage(const typename TImage::Pointer image, const TColor& color);

  // Change the hole pixels in 'image' to a specified 'holeValue'.
  template<typename TImage>
  void ApplyToImage(TImage* image, const typename TImage::PixelType& holeValue);

  // Recolor the hole pixels in 'image' a specified 'color'.
  template<typename TImage, typename TColor>
  void ApplyToVectorImage(TImage* image, const TColor& color);

  template<typename TColor>
  void MakeVTKImage(vtkImageData* image, const TColor& validColor, const TColor& holeColor, const bool holeTransparent, const bool validTransparent) const;

  // Create a mask from a mask image.
  template<typename TImage>
  void CreateFromImage(const TImage* image, const typename TImage::PixelType& holeColor);

  std::vector<itk::Index<2> > GetValidPixelsInRegion(const itk::ImageRegion<2>& region);
  std::vector<itk::Index<2> > GetHolePixelsInRegion(const itk::ImageRegion<2>& region);
  */

  return EXIT_SUCCESS;
}
