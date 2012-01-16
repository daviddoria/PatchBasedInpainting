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

#include "NamedITKImageCollection.h"

#include "Helpers/Helpers.h"
#include "Helpers/ITKHelpers.h"
#include "Mask.h"
#include "MaskOperations.h"

#include <stdexcept>

void NamedITKImageCollection::CopySelfPatchIntoHoleOfTargetRegion(const Mask* mask, const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion)
{
//   for(unsigned int imageId = 0; imageId < this->size(); ++imageId)
//     {
//     itk::ImageBase<2>* image = (*this)[imageId].Image;
// 
//     if(dynamic_cast<FloatScalarImageType*>(image))
//       {
//       std::cout << "Image " << imageId << " is FloatScalarImageType" << std::endl;
//       MaskOperations::CopySelfPatchIntoHoleOfTargetRegion<FloatScalarImageType>(dynamic_cast<FloatScalarImageType*>(image), mask, sourceRegion, targetRegion);
//       }
//     else if(dynamic_cast<UnsignedCharScalarImageType*>(image))
//       {
//       std::cout << "Image " << imageId << " is UnsignedCharScalarImageType" << std::endl;
//       MaskOperations::CopySelfPatchIntoHoleOfTargetRegion<UnsignedCharScalarImageType>(dynamic_cast<UnsignedCharScalarImageType*>(image), mask, sourceRegion, targetRegion);
//       }
//     else if(dynamic_cast<Mask*>(image))
//       {
//       std::cout << "Image " << imageId << " is Mask" << std::endl;
//       MaskOperations::CopySelfPatchIntoHoleOfTargetRegion<Mask>(dynamic_cast<Mask*>(image), mask, sourceRegion, targetRegion);
//       }
//     else if(dynamic_cast<FloatVectorImageType*>(image))
//       {
//       std::cout << "Image " << imageId << " is FloatVectorImageType" << std::endl;
//       MaskOperations::CopySelfPatchIntoHoleOfTargetRegion<FloatVectorImageType>(dynamic_cast<FloatVectorImageType*>(image), mask, sourceRegion, targetRegion);
//       }
//     else if(dynamic_cast<FloatVector2ImageType*>(image))
//       {
//       std::cout << "Image " << imageId << " is FloatVector2ImageType" << std::endl;
//       MaskOperations::CopySelfPatchIntoHoleOfTargetRegion<FloatVector2ImageType>(dynamic_cast<FloatVector2ImageType*>(image), mask, sourceRegion, targetRegion);
//       }
//     else if(dynamic_cast<IntImageType*>(image))
//       {
//       std::cout << "Image " << imageId << " is IntImageType" << std::endl;
//       MaskOperations::CopySelfPatchIntoHoleOfTargetRegion<IntImageType>(dynamic_cast<IntImageType*>(image), mask, sourceRegion, targetRegion);
//       }
//     else
//       {
//       std::cout << "Image " << imageId << " is Invalid type!" << std::endl;
//       std::cerr << "Cannot cast to any of the specified types!" << std::endl;
//       }
//     }
}

NamedITKImage NamedITKImageCollection::FindImageByName(const std::string& imageName) const
{
//   for(unsigned int i = 0; i < this->size(); ++i)
//     {
//     if((*this)[i].Name == imageName)
//       {
//       return (*this)[i];
//       }
//     }
//   std::stringstream ss;
//   ss << "No image named " << imageName << " found!";
//   throw std::runtime_error(ss.str());
}
