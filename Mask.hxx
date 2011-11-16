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

template<typename TImage>
void Mask::ApplyToVectorImage(const typename TImage::Pointer image, const QColor& color)
{
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

template<typename TImage>
void Mask::ApplyToImage(const typename TImage::Pointer image, const QColor& color)
{
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
