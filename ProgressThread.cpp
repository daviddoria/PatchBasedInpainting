#include "ProgressThread.h"

void ProgressThread::run()
{
  std::cout << "ProgressThread::run()" << std::endl;
  // When the thread is started, emit the signal to start the marquee progress bar
  emit StartProgressSignal();

  this->Inpainting->Inpaint();

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
