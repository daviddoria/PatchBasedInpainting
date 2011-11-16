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

/* The purpose of this class is simply to run the core algorithm
 * separate thread so that the progress bar can run during the computation.
*/

#ifndef PROGRESSTHREAD_H
#define PROGRESSTHREAD_H

#include <QThread>

#include "PatchBasedInpainting.h"

class ProgressThreadObject : public QThread
{
Q_OBJECT
public:

signals:
  // This signal is emitted to start the progress bar
  void StartProgressSignal();

  // This signal is emitted to stop the progress bar
  void StopProgressSignal();

  void RefreshSignal();
  
  void IterationCompleteSignal();

};

class ProgressThread : public ProgressThreadObject
{
public:

  ProgressThread();
  
  // This function is called when the thread is started
  void run();

  // This function is called when the thread is stopped
  void exit();

  void StopInpainting();

  void SetObject(PatchBasedInpainting*);
  PatchBasedInpainting* GetObject();
  
private:
  // We need a pointer to this object so we can perform the computations in this thread
  PatchBasedInpainting* Inpainting;
  bool Stop;
};

#endif