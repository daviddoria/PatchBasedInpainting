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

#ifndef BoundaryNormals_H
#define BoundaryNormals_H

// Submodules
#include <Mask/Mask.h>
#include <ITKHelpers/ITKHelpers.h>
#include <Utilities/Debug/Debug.h>

/** This class computes the boundary normals of a mask at the valid side of the mask boundary. */
class BoundaryNormals : public Debug
{
public:

  /** Constructor. */
  BoundaryNormals(const Mask* const mask) : MaskImage(mask){}

  /** Comput the boundary normals. 'TNormalsImage' should be a type that has an
    * operator[] for two components (a 2-vector).*/
  template <typename TNormalsImage>
  void ComputeBoundaryNormals(TNormalsImage* const boundaryNormals, const float blurVariance = 0.0f);

private:

  const Mask* MaskImage;
};

#include "BoundaryNormals.hpp"

#endif
