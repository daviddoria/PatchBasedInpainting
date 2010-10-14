// Based on "Image Inpainting" by Bertalmio, Sapiro, Caselles, and Ballester#ifndef __vtkInpainting_h
#ifndef __vtkBertalmioInpainting_h
#define __vtkBertalmioInpainting_h

#include "vtkInpainting.h"

#include <vtkSmartPointer.h>

class vtkImageData;

class vtkBertalmioInpainting : public vtkInpainting
{
public:
  vtkTypeMacro(vtkBertalmioInpainting,vtkInpainting);
  static vtkBertalmioInpainting *New();

protected:
  vtkBertalmioInpainting(){}
  ~vtkBertalmioInpainting(){}

  void Iterate(int iteration);

private:
  vtkBertalmioInpainting(const vtkBertalmioInpainting&);  // Not implemented.
  void operator=(const vtkBertalmioInpainting&);  // Not implemented.

};

#endif
