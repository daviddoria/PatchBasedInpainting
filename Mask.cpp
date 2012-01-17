#include "Mask.h"

// ITK
#include "itkImageRegionIterator.h"

Mask::Mask() : HoleValue(255), ValidValue(0)
{

}

bool Mask::IsHole(const itk::Index<2>& index) const
{
  if(this->GetPixel(index) == this->HoleValue)
    {
    return true;
    }
  return false;
}

bool Mask::IsValid(const itk::ImageRegion<2>& region) const
{
  // If any of the pixels in the region are invalid, the region is invalid.

  itk::ImageRegionConstIterator<Mask> maskIterator(this, region);

  while(!maskIterator.IsAtEnd())
    {
    if(!this->IsValid(maskIterator.GetIndex()))
      {
      //std::cout << "Mask::IsValid - Pixel " << maskIterator.GetIndex() << " has value " << static_cast<unsigned int>(maskIterator.Get())
      //          << " which makes the region invalid because Mask::ValidValue = " << static_cast<unsigned int>(this->ValidValue) << std::endl;
      return false;
      }

    ++maskIterator;
    }
  return true;
}

bool Mask::IsValid(const itk::Index<2>& index) const
{
  if(this->GetPixel(index) == this->ValidValue)
    {
    return true;
    }
  return false;
}

void Mask::Cleanup()
{
  // We want to interpret pixels that are "pretty much hole value" as holes, and pixels that
  // are "pretty much valid value" as valid. The "do not use" pixels must be very far away from both of these values.
  itk::ImageRegionIterator<Mask> maskIterator(this, this->GetLargestPossibleRegion());

  float tolerance = 4;
  while(!maskIterator.IsAtEnd())
    {
    if(fabs(maskIterator.Get() - this->ValidValue) < tolerance)
      {
      //std::cout << "Setting valid pixel to " << static_cast<unsigned int>(this->ValidValue) << std::endl;
      maskIterator.Set(this->ValidValue);
      }
    else if(fabs(maskIterator.Get() - this->HoleValue) < tolerance)
      {
      //std::cout << "Setting hole pixel to " << static_cast<unsigned int>(this->HoleValue) << std::endl;
      maskIterator.Set(this->HoleValue);
      }
    ++maskIterator;
    }

}

void Mask::SetHoleValue(const unsigned char value)
{
  this->HoleValue = value;
}

void Mask::SetValidValue(const unsigned char value)
{
  this->ValidValue = value;
}

unsigned char Mask::GetHoleValue() const
{
  return this->HoleValue;
}

unsigned char Mask::GetValidValue() const
{
  return this->ValidValue;
}
