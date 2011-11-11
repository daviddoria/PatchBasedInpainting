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
