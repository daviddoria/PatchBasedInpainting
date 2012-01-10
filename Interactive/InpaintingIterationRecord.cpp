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

#include "InpaintingIterationRecord.h"

InpaintingIterationRecord::InpaintingIterationRecord()
{

}

NamedITKImageCollection& InpaintingIterationRecord::GetImages()
{
  return this->Images;
}

// void InpaintingIterationRecord::SetDisplayed(const unsigned int imageId, const bool displayed)
// {
//   this->Display[imageId] = displayed;
// }
// 
// bool InpaintingIterationRecord::IsDisplayed(const unsigned int imageId) const
// {
//   return this->Display[imageId];
// }

void InpaintingIterationRecord::AddImage(const NamedITKImage& namedImage, const bool display)
{
  this->Images.push_back(namedImage);
  this->Display.push_back(display);
}

// NamedITKImage InpaintingIterationRecord::GetImage(const unsigned int imageId) const
// {
//   return this->Images[imageId];
// }
// 
// NamedITKImage InpaintingIterationRecord::GetImageByName(const std::string& imageName) const
// {
//   return this->Images.FindImageByName(imageName);
// }
// 
// unsigned int InpaintingIterationRecord::GetNumberOfImages() const
// {
//   return this->Images.size();
// }
