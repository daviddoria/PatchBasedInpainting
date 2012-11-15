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

#ifndef PriorityImprovedCriminisi_H
#define PriorityImprovedCriminisi_H

#include "PriorityCriminisi.h"

// Submodules
#include <Utilities/Debug/Debug.h>

/** The difference between this funciton and Criminisi's original data term computation (ComputeDataTermCriminisi)
  * is that we claim there is no reason to penalize the priority of linear structures that don't have a perpendicular incident
  * angle with the boundary. Of course, we don't want to continue structures that are almost parallel with the boundary, but above
  * a threshold, the strength of the isophote should be more important than the angle of incidence.*/
template <typename TImage>
class PriorityImprovedCriminisi : public PriorityCriminisi<TImage>
{
  protected:

  float ComputeDataTerm(const itk::Index<2>& queryPixel) const override
  {
    FloatVector2Type isophote = this->IsophoteImage->GetPixel(queryPixel);
    FloatVector2Type boundaryNormal = this->BoundaryNormalsImage->GetPixel(queryPixel);

    DebugMessage<FloatVector2Type>("Isophote: ", isophote);
    DebugMessage<FloatVector2Type>("Boundary normal: ", boundaryNormal);
    // D(p) = |dot(isophote at p, normalized normal of the front at p)|/alpha

    vnl_double_2 vnlIsophote(isophote[0], isophote[1]);

    vnl_double_2 vnlNormal(boundaryNormal[0], boundaryNormal[1]);

    float dataTerm = 0.0f;

    float angleBetween = Helpers::AngleBetween(isophote, boundaryNormal);
    if(angleBetween < 20)
    {
      float projectionMagnitude = isophote.GetNorm() * cos(angleBetween);

      dataTerm = projectionMagnitude;
    }
    else
    {
      dataTerm = isophote.GetNorm();
    }

    return dataTerm;
  }

};

#endif
