/*
Copyright (C) 2010 David Doria, daviddoria@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Helpers.h"

itk::ImageRegion<2> GetRegionAroundPixel(itk::Index<2> pixel, unsigned int radius)
{
  // This function returns a Region with the specified 'radius' centered at 'pixel'. By the definition of the radius of a square patch, the output region is (radius*2 + 1)x(radius*2 + 1).

  // The "index" is the lower left corner, so we need to subtract the radius from the center to obtain it
  pixel[0] -= radius;
  pixel[1] -= radius;

  itk::ImageRegion<2> region;
  region.SetIndex(pixel);
  itk::Size<2> size;
  size[0] = radius*2 + 1;
  size[1] = radius*2 + 1;
  region.SetSize(size);

  return region;
}

template <>
void CreateBlankPatch<UnsignedCharImageType>(UnsignedCharImageType::Pointer patch, unsigned int sideLength)
{
  CreateConstantPatch<UnsignedCharImageType>(patch, 0, sideLength);
}