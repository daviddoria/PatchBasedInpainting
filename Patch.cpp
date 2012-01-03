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

#include "Patch.h"

Patch::Patch(const itk::ImageRegion<2>& region)
{
  this->Region = region;
}

itk::ImageRegion<2> Patch::GetRegion() const
{
  return this->Region;
}

std::ostream& operator<<(std::ostream& output, const Patch &patch)
{
  output << "Patch: " << patch.GetRegion() << std::endl;
  return output;
}

bool Patch::operator==(const Patch& other) const
{
  if(this->Region == other.Region)
    {
    return true;
    }
  return false;
}

bool Patch::operator!=(const Patch& other) const
{
  return !operator==(other);
}

bool Patch::operator<(const Patch &other) const
{
  if(this->Region.GetIndex()[0] < other.Region.GetIndex()[0])
    {
    return true;
    }
  else if (other.Region.GetIndex()[0] < this->Region.GetIndex()[0])
    {
    return false;
    }

  if (this->Region.GetIndex()[1] < other.Region.GetIndex()[1])
    {
    return true;
    }
  else if (other.Region.GetIndex()[1] < this->Region.GetIndex()[1])
    {
    return false;
    }
  assert(0); // This should never be reached
  return true;
}
