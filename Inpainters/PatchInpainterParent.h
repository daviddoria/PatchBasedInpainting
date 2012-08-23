#ifndef PatchInpainterParent_HPP
#define PatchInpainterParent_HPP

struct PatchInpainterParent
{
  virtual void PaintPatch(const itk::Index<2>& targetPatchCenter, const itk::Index<2>& sourcePatchCenter) = 0;
};

#endif
