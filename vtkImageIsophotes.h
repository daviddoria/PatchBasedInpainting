#ifndef __vtkImageIsophotes_h
#define __vtkImageIsophotes_h

#include "vtkImageAlgorithm.h"

class vtkImageData;

class vtkImageIsophotes : public vtkImageAlgorithm
{
public:
  vtkTypeMacro(vtkImageIsophotes,vtkImageAlgorithm);
  static vtkImageIsophotes *New();

protected:
  vtkImageIsophotes(){}
  ~vtkImageIsophotes(){}
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  
private:
  vtkImageIsophotes(const vtkImageIsophotes&);  // Not implemented.
  void operator=(const vtkImageIsophotes&);  // Not implemented.

};

#endif
