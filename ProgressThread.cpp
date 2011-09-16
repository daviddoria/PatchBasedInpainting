#include "ProgressThread.h"

ProgressThread::ProgressThread()
{
  this->Stop = false;
}

CriminisiInpainting* ProgressThread::GetObject()
{
  return this->Inpainting;
}

void ProgressThread::StopInpainting()
{
  this->Stop = true;
}

void ProgressThread::run()
{
  std::cout << "ProgressThread::run()" << std::endl;
  // When the thread is started, emit the signal to start the marquee progress bar
  emit StartProgressSignal();

  // Start the procedure
  this->Stop = false;

  //DebugMessage("Initializing...");
  this->Inpainting->Initialize();

  while(this->Inpainting->HasMoreToInpaint() && !this->Stop)
    {
    this->Inpainting->Iterate();
    RefreshSignal();
    }

  // When the function is finished, end the thread
  exit();
}

void ProgressThread::exit()
{
  // When the thread is stopped, emit the signal to stop the marquee progress bar
  emit StopProgressSignal();
}

void ProgressThread::SetObject(CriminisiInpainting* inpainting)
{
  this->Inpainting = inpainting;
}
