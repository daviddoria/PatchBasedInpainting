#ifndef __vtkInpaintingCriminisi_h
#define __vtkInpaintingCriminisi_h

#include "vtkInpainting.h"

class vtkImageData;

class vtkInpaintingCriminisi : public vtkInpainting
{
public:
  vtkTypeMacro(vtkInpaintingCriminisi,vtkInpainting);
  static vtkInpaintingCriminisi *New();

protected:
  vtkInpaintingCriminisi(){}
  ~vtkInpaintingCriminisi(){}

  // Description:
  // Perform an iteration of the fast digital inpainting algorithm.
  void Iterate(int iteration);



private:
  vtkInpaintingCriminisi(const vtkInpaintingCriminisi&);  // Not implemented.
  void operator=(const vtkInpaintingCriminisi&);  // Not implemented.

};

#endif
