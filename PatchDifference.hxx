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

template <typename TImage, typename TPixelDifference>
PatchDifference<TImage, TPixelDifference>::PatchDifference() : Image(NULL)
{

}

template <typename TImage, typename TPixelDifference>
void PatchDifference<TImage, TPixelDifference>::SetImage(const TImage* const image)
{
  this->Image = image;
}

template <typename TImage, typename TPixelDifference>
float PatchDifference<TImage, TPixelDifference>::Difference(const PatchPair<TImage>& patchPair) const
{
  assert(this->Image);

  std::vector<itk::Offset<2> > targetOffsetsToCompare;

  itk::ImageRegionConstIterator<TImage> iterator(this->Image, patchPair.GetTargetPatch().GetRegion());

  itk::Index<2> targetCorner = patchPair.GetTargetPatch().GetCorner();

  // Add all pixels to the list to be compared
  while(!iterator.IsAtEnd())
    {
    itk::Offset<2> offset = iterator.GetIndex() - targetCorner;

    targetOffsetsToCompare.push_back(offset); // We have to multiply the linear offset by the number of components per pixel for the VectorImage type

    ++iterator;
    }
  //std::cout << "Full difference has " << targetOffsetsToCompare.size() << " offsets." << std::endl;
  return Difference(patchPair, targetOffsetsToCompare);
}
