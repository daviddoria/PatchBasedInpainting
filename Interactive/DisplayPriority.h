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

#ifndef DisplayPriority_H
#define DisplayPriority_H

class DisplayPriority
{

  virtual std::vector<NamedVTKImage> GetNamedImages();
  static std::vector<std::string> GetImageNames();
};


template <typename TImage>
std::vector<NamedVTKImage> Priority<TImage>::GetNamedImages()
{
  std::vector<NamedVTKImage> namedImages;

  NamedVTKImage priorityImage;
  priorityImage.Name = "Priority";
  vtkSmartPointer<vtkImageData> priorityImageVTK = vtkSmartPointer<vtkImageData>::New();
  ITKVTKHelpers::ITKScalarImageToScaledVTKImage<FloatScalarImageType>(this->PriorityImage, priorityImageVTK);
  priorityImage.ImageData = priorityImageVTK;

  namedImages.push_back(priorityImage);

  return namedImages;
}

template <typename TImage>
std::vector<std::string> Priority<TImage>::GetImageNames()
{
  std::vector<std::string> imageNames;
  imageNames.push_back("Priority");
  return imageNames;
}


#endif
