/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
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

#ifndef CompositePatchInpainter_HPP
#define CompositePatchInpainter_HPP

#include "PatchInpainterParent.h"

// STL
#include <memory>

struct CompositePatchInpainter
{
public:
  void AddInpainter(std::shared_ptr<PatchInpainterParent> inpainter)
  {
    this->Inpainters.push_back(inpainter);
  }

  void AddInpainter(PatchInpainterParent* inpainter)
  {
    this->Inpainters.push_back(std::shared_ptr<PatchInpainterParent>(inpainter));
  }

  void PaintPatch(const itk::Index<2>& targetPatchCenter, const itk::Index<2>& sourcePatchCenter)
  {
    for(std::size_t i = 0; i < this->Inpainters.size(); ++i)
    {
      this->Inpainters[i]->PaintPatch(targetPatchCenter, sourcePatchCenter);
    }
  }

private:
  std::vector<std::shared_ptr<PatchInpainterParent> > Inpainters;
};

#endif
