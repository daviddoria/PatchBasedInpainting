#ifndef VisitorSuperclass_HPP
#define VisitorSuperclass_HPP

template<typename TVertexDescriptor>
struct VisitorSuperclass
{
  virtual void PaintPatch(TVertexDescriptor target, TVertexDescriptor source) const = 0;

  virtual void FinishVertex(TVertexDescriptor target, TVertexDescriptor sourceNode) = 0;

  virtual void InpaintingComplete() const = 0;
};

#endif
