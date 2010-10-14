#ifndef __vtkInpainting_h
#define __vtkInpainting_h

#include "vtkImageAlgorithm.h"

#include <vtkSmartPointer.h>

class vtkImageData;

class vtkInpainting : public vtkImageAlgorithm
{
public:
  vtkTypeMacro(vtkInpainting,vtkImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkSetMacro(MaximumIterations, int);

  vtkImageData* GetIntermediateOutput();

protected:
  vtkInpainting();
  ~vtkInpainting(){}

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  // Description:
  // Return the current boundary pixels in boundaryPixels.
  void FindMaskBoundaryPixels(vtkImageData* boundaryPixels);

  // Description:
  // Copy the flagged pixels in 'region' of input into 'image'.
  void CopyImageInRegion(vtkImageData* image, vtkImageData* region, vtkImageData* input);

  // Description:
  // Set all of the Image pixels to black where 'region' is white.
  void ClearImageInRegion(vtkImageData* image, vtkImageData* region);

  // Description:
  // Perform an iteration of the inpainting. This modifies both Image and Mask.
  virtual void Iterate(int iteration) = 0;

  // Description:
  // Check if Mask is emtpy.
  bool IsDone();

  vtkSmartPointer<vtkImageData> Output;
  vtkSmartPointer<vtkImageData> Image;
  vtkSmartPointer<vtkImageData> Mask;

  int MaximumIterations;

private:
  vtkInpainting(const vtkInpainting&);  // Not implemented.
  void operator=(const vtkInpainting&);  // Not implemented.

};

#endif
