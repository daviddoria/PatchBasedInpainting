// Based on "Image Inpainting" by Bertalmio, Sapiro, Caselles, and Ballester#ifndef __vtkInpainting_h
#ifndef __vtkFastDigitalInpainting_h
#define __vtkFastDigitalInpainting_h

#include "vtkInpainting.h"

class vtkImageData;

class vtkFastDigitalInpainting : public vtkInpainting
{
public:
  vtkTypeMacro(vtkFastDigitalInpainting,vtkInpainting);
  static vtkFastDigitalInpainting *New();

protected:
  vtkFastDigitalInpainting(){}
  ~vtkFastDigitalInpainting(){}

  // Description:
  // Perform an iteration of the fast digital inpainting algorithm.
  void Iterate(int iteration);



private:
  vtkFastDigitalInpainting(const vtkFastDigitalInpainting&);  // Not implemented.
  void operator=(const vtkFastDigitalInpainting&);  // Not implemented.

};

#endif
