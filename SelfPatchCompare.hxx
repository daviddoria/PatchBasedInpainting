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

template<typename TDifferenceFunction>
float SelfPatchCompare::PatchAverageSourceDifference(const Patch& sourcePatch)
{
  if(this->ValidTargetPatchOffsets.size() <= 0)
    {
    std::cout << "SelfPatchCompare::PatchAverageSourceDifference had 0 ValidTargetPatchOffsets on which to operate!" << std::endl;
    exit(-1);
    }
  float totalDifference = 0.0f;
  
  FloatVectorImageType::InternalPixelType* bufferPointer = this->Image->GetBufferPointer();
  int sourceCornerOffset = this->Image->ComputeOffset(sourcePatch.Region.GetIndex());
  int targetCornerOffset = this->Image->ComputeOffset(this->Pairs->TargetPatch.Region.GetIndex());
  
  int targetToSourceOffsetPixels = sourceCornerOffset - targetCornerOffset;
  int targetToSourceOffset = targetToSourceOffsetPixels * this->NumberOfComponentsPerPixel;

  FloatVectorImageType::PixelType sourcePixel(this->NumberOfComponentsPerPixel);
  
  FloatVectorImageType::PixelType targetPixel(this->NumberOfComponentsPerPixel);
  
  TDifferenceFunction differenceFunction(this->NumberOfComponentsPerPixel);
  float difference = 0;
  for(unsigned int pixelId = 0; pixelId < this->ValidTargetPatchOffsets.size(); ++pixelId)
    {
    for(unsigned int component = 0; component < this->NumberOfComponentsPerPixel; ++component)
      {
      //sourcePixel[component] = bufferPointer[this->ValidTargetPatchOffsets[pixelId] + targetToSourceOffset + component];
      //targetPixel[component] = bufferPointer[this->ValidTargetPatchOffsets[pixelId] + component];
      
      int sourceOffset = this->ValidTargetPatchOffsets[pixelId] + targetToSourceOffset + component;
      int targetOffset = this->ValidTargetPatchOffsets[pixelId] + component;
//       if(sourceOffset < 0 || sourceOffset > this->Image->GetLargestPossibleRegion().GetNumberOfPixels() * this->NumberOfComponentsPerPixel ||
// 	 targetOffset < 0 || targetOffset > this->Image->GetLargestPossibleRegion().GetNumberOfPixels() * this->NumberOfComponentsPerPixel)
// 	{
// 	std::cerr << "Problem: " << std::endl
// 	          << " this->ValidTargetPatchOffsets[pixelId]: " << this->ValidTargetPatchOffsets[pixelId] << std::endl
// 	          << " targetToSourceOffset: " << targetToSourceOffset << std::endl
// 	          << " sourceOffset: " << sourceOffset << std::endl
// 	          << " targetOffset: " << targetOffset << std::endl
// 	          << " sourcePatch: " << sourcePatch.Region.GetIndex() << std::endl
// 		  << " targetPatch: " << this->Pairs->TargetPatch.Region.GetIndex() << std::endl
// 		  << " sourceCornerOffset: " << sourceCornerOffset << std::endl
// 		  << " targetCornerOffset: " << targetCornerOffset << std::endl
// 		  << " computed target index: " << this->Image->ComputeIndex(this->ValidTargetPatchOffsets[pixelId] / this->NumberOfComponentsPerPixel) << std::endl
// 	          << std::endl;
// 		  //std::cout << index[1] * size[0] + index[0] << std::endl;
// 	exit(-1);
// 	}
      
      sourcePixel[component] = bufferPointer[sourceOffset];
      
      targetPixel[component] = bufferPointer[targetOffset];
      }
  
    difference = differenceFunction.Difference(sourcePixel, targetPixel);

    totalDifference += difference;
    }

  float averageDifference = totalDifference/static_cast<float>(this->ValidTargetPatchOffsets.size());
  return averageDifference;
}
