#ifndef __vtkImageVectorRotate_h
#define __vtkImageVectorRotate_h

#include "vtkImageAlgorithm.h"

class vtkImageData;

class vtkImageVectorRotate : public vtkImageAlgorithm
{
public:
  vtkTypeMacro(vtkImageVectorRotate,vtkImageAlgorithm);
  static vtkImageVectorRotate *New();

  vtkSetMacro(XRotation, double);
  vtkGetMacro(XRotation, double);

  vtkSetMacro(YRotation, double);
  vtkGetMacro(YRotation, double);

  vtkSetMacro(ZRotation, double);
  vtkGetMacro(ZRotation, double);
  
protected:
  vtkImageVectorRotate();
  
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  double XRotation;
  double YRotation;
  double ZRotation;
  
private:
  vtkImageVectorRotate(const vtkImageVectorRotate&);  // Not implemented.
  void operator=(const vtkImageVectorRotate&);  // Not implemented.

};

#endif
