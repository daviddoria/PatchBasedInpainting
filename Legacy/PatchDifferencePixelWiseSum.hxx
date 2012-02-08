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
PatchDifferencePixelWiseSum<TImage, TPixelDifference>::PatchDifferencePixelWiseSum() : PatchDifference<TImage, TPixelDifference>()
{

}

template <typename TImage, typename TPixelDifference>
float PatchDifferencePixelWiseSum<TImage, TPixelDifference>::Difference(const PatchPair<TImage>& patchPair, const std::vector<itk::Offset<2> >& offsetsToCompare)  const
{
  float totalDifference = 0.0f;
  float numberOfComponentsPerPixel = this->Image->GetNumberOfComponentsPerPixel();

  itk::Index<2> targetCorner = patchPair.GetTargetPatch().GetRegion().GetIndex();
  itk::Index<2> sourceCorner = patchPair.GetSourcePatch()->GetRegion().GetIndex();

  // Create empty pixel containers that we will fill.
  typename TImage::PixelType sourcePixel(numberOfComponentsPerPixel);
  typename TImage::PixelType targetPixel(numberOfComponentsPerPixel);

  // Instantiate the distance function that has been specified as a template parameter.
  float difference = 0;

  for(unsigned int pixelId = 0; pixelId < offsetsToCompare.size(); ++pixelId)
    {
    sourcePixel = this->Image->GetPixel(sourceCorner + offsetsToCompare[pixelId]);
    targetPixel = this->Image->GetPixel(targetCorner + offsetsToCompare[pixelId]);

    difference = TPixelDifference::Difference(sourcePixel, targetPixel);

    totalDifference += difference;
    } // end pixel loop

  return totalDifference;
}
