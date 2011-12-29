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

#include "PriorityFactory.h"

#include "Helpers.h"
#include "PriorityCriminisi.h"
#include "PriorityDepth.h"
#include "PriorityManual.h"
#include "PriorityOnionPeel.h"
#include "PriorityRandom.h"

// Priority* PriorityFactory::Create(const std::string& priorityType)
// {
//   if(Helpers::StringsMatch(this->cmbPriority->currentText().toStdString(), "Manual"))
//     {
//     this->Inpainting->SetPriorityFunction<PriorityManual>();
//     }
//   else if(Helpers::StringsMatch(this->cmbPriority->currentText().toStdString(), "OnionPeel"))
//     {
//     this->Inpainting->SetPriorityFunction<PriorityOnionPeel>();
//     }
//   else if(Helpers::StringsMatch(this->cmbPriority->currentText().toStdString(), "Random"))
//     {
//     this->Inpainting->SetPriorityFunction<PriorityRandom>();
//     }
//   else if(Helpers::StringsMatch(this->cmbPriority->currentText().toStdString(), "Depth"))
//     {
//     this->Inpainting->SetPriorityFunction<PriorityDepth>();
//     }
//   else if(Helpers::StringsMatch(this->cmbPriority->currentText().toStdString(), "Criminisi"))
//     {
//     this->Inpainting->SetPriorityFunction<PriorityCriminisi>();
//     }
// }

std::vector<std::string> PriorityFactory::GetImageNames(const std::string& priorityType)
{
  std::vector<std::string> priorityImageNames;

  if(Helpers::StringsMatch(priorityType, "Manual"))
    {
    priorityImageNames = PriorityManual::GetImageNames();
    }
  else if(Helpers::StringsMatch(priorityType, "OnionPeel"))
    {
    priorityImageNames = PriorityOnionPeel::GetImageNames();
    }
  else if(Helpers::StringsMatch(priorityType, "Random"))
    {
    priorityImageNames = PriorityRandom::GetImageNames();
    }
  else if(Helpers::StringsMatch(priorityType, "Depth"))
    {
    priorityImageNames = PriorityDepth::GetImageNames();
    }
  else if(Helpers::StringsMatch(priorityType, "Criminisi"))
    {
    priorityImageNames = PriorityCriminisi::GetImageNames();
    }
    
  return priorityImageNames;
}
