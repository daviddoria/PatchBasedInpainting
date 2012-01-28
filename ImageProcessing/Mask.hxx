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

#include <vtkImageData.h>

template<typename TImage, typename TColor>
void Mask::ApplyToVectorImage(TImage* const image, const TColor& color) const
{
  // Using generics, we allow any Color class that has .red(), .green(), and .blue() member functions
  // to be used to specify the color.
  if(image->GetLargestPossibleRegion() != this->GetLargestPossibleRegion())
    {
    std::cerr << "Image and mask must be the same size!" << std::endl
              << "Image region: " << image->GetLargestPossibleRegion() << std::endl
              << "Mask region: " << this->GetLargestPossibleRegion() << std::endl;
    return;
    }

  // Color the hole pixels in the image.
  typename TImage::PixelType holeValue;
  holeValue.SetSize(image->GetNumberOfComponentsPerPixel());
  holeValue.Fill(0);
  if(image->GetNumberOfComponentsPerPixel() >= 3)
    {
    holeValue[0] = color.red();
    holeValue[1] = color.green();
    holeValue[2] = color.blue();
    }

  itk::ImageRegionConstIterator<Mask> maskIterator(this, this->GetLargestPossibleRegion());

  while(!maskIterator.IsAtEnd())
    {
    if(this->IsHole(maskIterator.GetIndex()))
      {
      image->SetPixel(maskIterator.GetIndex(), holeValue);
      }

    ++maskIterator;
    }
}

template<typename TImage, typename TColor>
void Mask::ApplyColorToImage(const TImage* const image, const TColor& color) const
{
  // Using generics, we allow any Color class that has .red(), .green(), and .blue() member functions
  // to be used to specify the color.

  if(image->GetLargestPossibleRegion() != this->GetLargestPossibleRegion())
    {
    std::cerr << "Image and mask must be the same size!" << std::endl
              << "Image region: " << image->GetLargestPossibleRegion() << std::endl
              << "Mask region: " << this->GetLargestPossibleRegion() << std::endl;
    return;
    }

  // Color the hole pixels in the image.
  typename TImage::PixelType holeValue;
  holeValue.Fill(0);
  if(image->GetNumberOfComponentsPerPixel() >= 3)
    {
    holeValue[0] = color.red();
    holeValue[1] = color.green();
    holeValue[2] = color.blue();
    }

  itk::ImageRegionConstIterator<Mask> maskIterator(this, this->GetLargestPossibleRegion());

  while(!maskIterator.IsAtEnd())
    {
    if(this->IsHole(maskIterator.GetIndex()))
      {
      image->SetPixel(maskIterator.GetIndex(), holeValue);
      }

    ++maskIterator;
    }
}


template<typename TImage>
void Mask::ApplyToImage(TImage* const image, const typename TImage::PixelType& holeValue) const
{
  // Using generics, we allow any Color class that has .red(), .green(), and .blue() member functions
  // to be used to specify the color.

  if(image->GetLargestPossibleRegion() != this->GetLargestPossibleRegion())
    {
    std::cerr << "Image and mask must be the same size!" << std::endl
              << "Image region: " << image->GetLargestPossibleRegion() << std::endl
              << "Mask region: " << this->GetLargestPossibleRegion() << std::endl;
    return;
    }

  itk::ImageRegionConstIterator<Mask> maskIterator(this, this->GetLargestPossibleRegion());

  while(!maskIterator.IsAtEnd())
    {
    if(this->IsHole(maskIterator.GetIndex()))
      {
      image->SetPixel(maskIterator.GetIndex(), holeValue);
      }

    ++maskIterator;
    }
}

template <typename TColor>
void Mask::MakeVTKImage(vtkImageData* const image, const TColor& validColor, const TColor& holeColor, const bool holeTransparent, const bool validTransparent) const
{
  int dims[3];
  dims[0] = this->GetLargestPossibleRegion().GetSize()[0];
  dims[1] = this->GetLargestPossibleRegion().GetSize()[1];
  dims[2] = 1;

  image->SetScalarTypeToUnsignedChar();
  image->SetNumberOfScalarComponents(4);
  image->SetDimensions(dims);
  image->AllocateScalars();

  for(int i = 0; i < dims[0]; ++i)
    {
    for(int j = 0; j < dims[1]; ++j)
      {
      // Get a pointer to the pixel so we can modify it.
      unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(i,j,0));
      itk::Index<2> index;
      index[0] = i;
      index[1] = j;
      if(this->IsValid(index))
        {
        pixel[0] = validColor.red();
        pixel[1] = validColor.green();
        pixel[2] = validColor.blue();

        if(validTransparent)
          {
          pixel[3] = 0; // invisible
          }
        else
          {
          pixel[3] = 255; // visible
          }
        }
      else // Pixel is a hole
        {
        pixel[0] = holeColor.red();
        pixel[1] = holeColor.green();
        pixel[2] = holeColor.blue();

        if(holeTransparent)
          {
          pixel[3] = 0; // invisible
          }
        else
          {
          pixel[3] = 255; // visible
          }
        }

      }
    }
}

// Create a mask from a mask image.
template<typename TImage>
void Mask::CreateFromImage(const TImage* image, const typename TImage::PixelType& holeColor)
{
  this->SetRegions(image->GetLargestPossibleRegion());
  this->Allocate();

  itk::ImageRegionConstIterator<TImage> imageIterator(image, image->GetLargestPossibleRegion());

  std::cout << "Hole color: " << holeColor << std::endl;
  unsigned int counter = 0;
  while(!imageIterator.IsAtEnd())
    {
    typename TImage::PixelType currentPixel = imageIterator.Get();
    std::cout << "Current color: " << currentPixel << std::endl;
    if(currentPixel == holeColor)
      {
      this->SetPixel(imageIterator.GetIndex(), this->HoleValue);
      counter++;
      }
    else
      {
      this->SetPixel(imageIterator.GetIndex(), this->ValidValue);
      }

    ++imageIterator;
    }
  std::cout << "There were " << counter << " mask pixels." << std::endl;
}
