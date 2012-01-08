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

#include "InpaintingComputationObject.h"

InpaintingComputationObject::InpaintingComputationObject() : Operation(ALLSTEPS), Stop(false)
{

}

// PatchBasedInpainting* ProgressThread::GetObject()
// {
//   return this->Inpainting;
// }

void InpaintingComputationObject::StopInpainting()
{
  this->Stop = true;
}

void InpaintingComputationObject::AllSteps()
{
  // Start the procedure
  this->Stop = false;

  while(this->Inpainting->HasMoreToInpaint() && !this->Stop)
    {
    // TODO: this object will disappear before getting to the slot!
    PatchPair usedPatchPair = this->Inpainting->Iterate();
    emit IterationComplete(&usedPatchPair);
    }

  // When the function is finished, end the thread
  emit InpaintingComplete();
}

void InpaintingComputationObject::SingleStep()
{
  // TODO: This object will be deleted before the slot receives it!
  PatchPair usedPatchPair = this->Inpainting->Iterate();
  emit IterationComplete(&usedPatchPair);
}

void InpaintingComputationObject::start()
{
  if(this->Operation == ALLSTEPS)
    {
    AllSteps();
    }
  else if(this->Operation == SINGLESTEP)
    {
    SingleStep();
    }
}

void InpaintingComputationObject::SetObject(PatchBasedInpainting<FloatVectorImageType>* const inpainting)
{
  this->Inpainting = inpainting;
}
