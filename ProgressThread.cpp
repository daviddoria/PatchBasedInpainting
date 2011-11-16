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

#include "ProgressThread.h"

ProgressThread::ProgressThread()
{
  this->Stop = false;
}

PatchBasedInpainting* ProgressThread::GetObject()
{
  return this->Inpainting;
}

void ProgressThread::StopInpainting()
{
  this->Stop = true;
}

void ProgressThread::run()
{
  //std::cout << "ProgressThread::run()" << std::endl;
  // When the thread is started, emit the signal to start the marquee progress bar
  emit StartProgressSignal();

  // Start the procedure
  this->Stop = false;

  //DebugMessage("Initializing...");
  
  //RefreshSignal();
  
  while(this->Inpainting->HasMoreToInpaint() && !this->Stop)
    {
    this->Inpainting->Iterate();
    IterationCompleteSignal();
    }

  // When the function is finished, end the thread
  exit();
}

void ProgressThread::exit()
{
  // When the thread is stopped, emit the signal to stop the marquee progress bar
  emit StopProgressSignal();
}

void ProgressThread::SetObject(PatchBasedInpainting* inpainting)
{
  this->Inpainting = inpainting;
}
