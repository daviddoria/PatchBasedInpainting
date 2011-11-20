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

template <typename T>
void PatchBasedInpainting::SetPriorityFunction()
{
  // This must be done internal to PatchBasedInpainting because we want to make sure it is initialized with all of
  // the correct properties.

  if(this->OriginalImage->GetLargestPossibleRegion() != this->CurrentOutputImage->GetLargestPossibleRegion())
    {
    std::cerr << "The CurrentOutputImage must have been initialized from OriginalImage before calling this function." << std::endl;
    exit(-1);
    }

  // std::cout << "SetPriorityFunction() image size: " << CurrentOutputImage->GetLargestPossibleRegion().GetSize() << std::endl;
  if(this->PriorityFunction)
    {
    delete this->PriorityFunction;
    }
  this->PriorityFunction = new T(this->CurrentOutputImage, this->MaskImage, this->PatchRadius[0]);
}
