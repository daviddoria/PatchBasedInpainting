#ifndef PatchInpainter_HPP
#define PatchInpainter_HPP

struct PatchInpainter
{
  template <typename TVertex, typename TGraph, typename TVisitor>
  TVertex operator()(TVertex target_patch_center, TVertex source_patch_center, TGraph& g, TVisitor visitor)
  {

  }
};

#endif
