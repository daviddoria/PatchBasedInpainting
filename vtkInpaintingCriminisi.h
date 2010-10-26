#ifndef __vtkInpaintingCriminisi_h
#define __vtkInpaintingCriminisi_h

#include "vtkImageAlgorithm.h"
#include "vtkSmartPointer.h"

class vtkImageData;

#include <vector>

struct Pixel
{
public:
  Pixel() : x(-1), y(-1), Priority(0) {}

  int x;
  int y;
  double Priority;
};

class vtkInpaintingCriminisi : public vtkImageAlgorithm
{
public:
  vtkTypeMacro(vtkInpaintingCriminisi,vtkImageAlgorithm);
  static vtkInpaintingCriminisi *New();

  // Description:
  // Given a nxn (n odd) patch, find the center of the best matching patch in the image
  // Public for testing purposes.
  void FindBestPatchMatch(vtkImageData* patch, vtkImageData* image, int bestPatchCenter[2]);

  vtkSetMacro(MaximumIterations, int);

  vtkImageData* GetIntermediateOutput();
  
protected:

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
  // Check if Mask is emtpy.
  bool IsDone();

  vtkSmartPointer<vtkImageData> Output;
  vtkSmartPointer<vtkImageData> Image;
  vtkSmartPointer<vtkImageData> Mask;

  int MaximumIterations;
  
  vtkInpaintingCriminisi();
  ~vtkInpaintingCriminisi(){}

  // Description:
  // Perform an iteration of the fast digital inpainting algorithm.
  void Iterate(int iteration);

  // Description:
  // Find the pixels on the border of the target region and source region.
  void FindFrontPixels();

  // Description:
  // Compute the priority of every pixel on the front
  void ComputePriorities();

  // Description:
  // Compute the 'priority' of a pixel. This is the sum of the ConfidenceTerm
  // and DataTerm.
  double ComputePriority(int x, int y);

  // Description:
  // Perform an iteration of the fast digital inpainting algorithm.
  double ComputeConfidenceTerm(int x, int y);

  // Description:
  // Perform an iteration of the fast digital inpainting algorithm.
  double ComputeDataTerm(int x, int y);

  // Description:
  // Maintain a list of the current front/border pixels
  std::vector<Pixel> Border;

private:
  vtkInpaintingCriminisi(const vtkInpaintingCriminisi&);  // Not implemented.
  void operator=(const vtkInpaintingCriminisi&);  // Not implemented.

};

#endif
