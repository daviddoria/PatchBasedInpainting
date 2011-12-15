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

/* The purpose of this class is simply to run the core algorithm in a
 * separate thread so that the GUI can remain responsive during the computation.
*/

#ifndef COMPUTATIONTHREAD_H
#define COMPUTATIONTHREAD_H

#include <QThread>

#include "PatchPair.h"
#include "PatchBasedInpainting.h"

// This class is named 'ComputationThreadClass' instead of just 'ComputationThread'
// because we often want to name a member variable 'ComputationThread'
class ComputationThreadClass : public QThread
{
Q_OBJECT

signals:
  // This signal is emitted to start the progress bar
  void StartProgressSignal();

  // This signal is emitted to stop the progress bar
  void StopProgressSignal();

  void RefreshSignal();

  void IterationCompleteSignal(const PatchPair&);
  void StepCompleteSignal(const PatchPair&);

public:
  ComputationThreadClass();

  // Store the type of operation to perform.
  enum OPERATION {ALLSTEPS, SINGLESTEP};
  OPERATION Operation;

  // This function is called when the thread is started.
  void run();

  void AllSteps();
  void SingleStep();

  // This function is called when the thread is stopped.
  void exit();

  void StopInpainting();

  // Provide the object with which to do the computation.
  void SetObject(PatchBasedInpainting*);

private:
  // We need a pointer to this object so we can perform the computations in this thread
  PatchBasedInpainting* Inpainting;

  // This flag can be set from another thread (by calling StopInpainting()) to indicate that we want to stop the computation at the next possible opportunity.
  bool Stop;
};

#endif