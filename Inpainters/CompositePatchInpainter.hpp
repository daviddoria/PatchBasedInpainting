#ifndef CompositePatchInpainter_HPP
#define CompositePatchInpainter_HPP

#include "PatchInpainterParent.h"

struct CompositePatchInpainter
{
public:
  void AddInpainter(PatchInpainterParent* const inpainter)
  {
    this->Inpainters.push_back(inpainter);
  }

  void PaintPatch(const itk::Index<2>& targetPatchCenter, const itk::Index<2>& sourcePatchCenter)
  {
    for(std::size_t i = 0; i < this->Inpainters.size(); ++i)
    {
      this->Inpainters[i]->PaintPatch(targetPatchCenter, sourcePatchCenter);
    }
  }

private:
  std::vector<PatchInpainterParent*> Inpainters;
};

#endif
