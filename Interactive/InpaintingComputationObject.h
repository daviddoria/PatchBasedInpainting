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

#ifndef InpaintingComputationObject_H
#define InpaintingComputationObject_H

#include <QObject>

#include "PatchPair.h"
#include "PatchBasedInpainting.h"

class InpaintingComputationObject : public QObject
{
Q_OBJECT

signals:

  // This signal is emitted when an iteration is complete.
  void IterationComplete(const PatchPair*);

  // This signal is emitted when the entire inpainting is complete.
  void InpaintingComplete();

public slots:
  // This function is called when the thread is started.
  void start();

public:
  InpaintingComputationObject();

  // Store the type of operation to perform.
  enum OPERATION {ALLSTEPS, SINGLESTEP};
  OPERATION Operation;

  // Perform the entire inpainting operation.
  void AllSteps();

  // Perform one iteration of the inpainting.
  void SingleStep();

  // Set the Stop flag so that the inpainting will stop before the next iteration.
  void StopInpainting();

  // Provide the object with which to do the computation.
  void SetObject(PatchBasedInpainting<FloatVectorImageType>* const patchBasedInpainting);

private:
  // We need a pointer to this object so we can perform the computations in this thread
  PatchBasedInpainting<FloatVectorImageType>* Inpainting;

  // This flag can be set from another thread (by calling StopInpainting()) to indicate that we want to stop the computation at the next possible opportunity.
  bool Stop;
};

#endif
