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

#ifndef Testing_H
#define Testing_H

#include "ImageProcessing/Mask.h"

namespace Testing
{
  static unsigned int TestImageSize = 100;

  bool ValuesEqual(const float a, const float b, const float epsilon = 1e-6);

  void GetFullyValidMask(Mask* const mask);

  void GetHalfValidMask(Mask* const mask);

  itk::ImageRegion<2> GetImageRegion();

  // Function templates
  template<typename T>
  bool ArraysEqual(const T* const a, const T* const b, const unsigned int length, const float epsilon = 1e-6);

  template<typename T>
  std::string ArrayString(const T* const a, const unsigned int length);

  template<typename T>
  void OutputArray(const T* const a, const unsigned int length);

  template<typename T>
  bool VectorsEqual(const std::vector<T>& a, const std::vector<T>& b);

  template<typename TImage>
  bool ImagesEqual(const TImage* const image1, const TImage* const image2);

  template<typename TImage>
  void GetBlankImage(TImage* const image);

  template<typename TImage>
  void GetHalfConstantImage(TImage* const image, const typename TImage::PixelType& leftSideConstant, const typename TImage::PixelType& rightSideConstant);

  template<typename TImage>
  void GetBlankImage(TImage* const image, const unsigned int numberOfChannels);


} // end namespace

#include "Testing.hxx"

#endif
