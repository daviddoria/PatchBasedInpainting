/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#ifndef CriminisiInpainting_h
#define CriminisiInpainting_h

// Custom
#include "Helpers.h"
#include "Types.h"
class EventThrower;

// ITK
#include "itkAddImageFilter.h"
#include "itkBinaryContourImageFilter.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryErodeImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkConstNeighborhoodIterator.h"
#include "itkConstantBoundaryCondition.h"
#include "itkCovariantVector.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkFlatStructuringElement.h"
#include "itkGradientImageFilter.h"
#include "itkImage.h"
#include "itkImageDuplicator.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkInvertIntensityImageFilter.h"
#include "itkMaskImageFilter.h"
#include "itkMinimumMaximumImageCalculator.h"
#include "itkPasteImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkRigid2DTransform.h"
#include "itkSubtractImageFilter.h"
#include "itkUnaryFunctorImageFilter.h"
//#include "itkVariableLengthVector.h"

// VTK
#include <vtkSmartPointer.h>

// STL
#include <iomanip> // setfill, setw

// Qt
#include <QObject>

class CriminisiInpainting : public QObject
{
  Q_OBJECT
signals:
  void RefreshSignal();
public:

  // Constructor
  CriminisiInpainting();

  // The real work is done here.
  void Inpaint();
  
  // Specify the image to inpaint.
  void SetImage(FloatVectorImageType::Pointer image);
  
  // Specify the region to inpaint.
  void SetMask(MaskImageType::Pointer mask);
  
  // Specify the size of the patches to copy.
  void SetPatchRadius(unsigned int);

  // Specify if you want to see debugging outputs.
  void SetDebug(bool);

  // Compute the confidence values for pixels that were just inpainted.
  void UpdateConfidences(itk::ImageRegion<2>);
  
  // Get the output of the inpainting.
  FloatVectorImageType::Pointer GetResult();
  
  // Get the current confidence image
  FloatScalarImageType::Pointer GetConfidenceImage();

  // Get the current confidence image
  FloatScalarImageType::Pointer GetPriorityImage();
  
  // Get the current boundary image
  UnsignedCharScalarImageType::Pointer GetBoundaryImage();
  
  // This object allows this class to invoke VTK events.
  vtkSmartPointer<EventThrower> Thrower;
  
private:

  // Image to inpaint. This should not be modified throughout the algorithm.
  FloatVectorImageType::Pointer OriginalImage;
  
  // The result of the inpainting.
  FloatVectorImageType::Pointer CurrentImage;

  // The mask specifying the region to inpaint. This does not change throughout the algorithm - it is the original mask.
  UnsignedCharScalarImageType::Pointer OriginalMask;
  
  // This mask is updated as patches are copied.
  UnsignedCharScalarImageType::Pointer CurrentMask;
  
  // Keep track of the confidence of each pixel
  FloatScalarImageType::Pointer ConfidenceImage;
  
  // The patch radius.
  itk::Size<2> PatchRadius;

  // Store the computed isophotes.
  FloatVector2ImageType::Pointer IsophoteImage;
  
  // Keep track of the edge of the region to inpaint.
  UnsignedCharScalarImageType::Pointer BoundaryImage;
  
  // Store the computed boundary normals.
  FloatVector2ImageType::Pointer BoundaryNormals;

  // Keep track of the priority of each pixel.
  FloatScalarImageType::Pointer PriorityImage;

  // Functions
  void Initialize();

  // Debugging
  void DebugWriteAllImages(itk::Index<2> pixelToFill, itk::Index<2> bestMatchPixel, unsigned int iteration);
  void DebugWritePatch(itk::Index<2> pixel, std::string filePrefix, unsigned int iteration);
  void DebugWritePatch(itk::ImageRegion<2> region, std::string filename);

  void DebugWritePatch(itk::Index<2> pixel, std::string filename);
  void DebugWritePixelToFill(itk::Index<2> pixelToFill, unsigned int iteration);
  void DebugWritePatchToFillLocation(itk::Index<2> pixelToFill, unsigned int iteration);

  itk::CovariantVector<float, 2> GetAverageIsophote(itk::Index<2> queryPixel);
  bool IsValidPatch(itk::Index<2> queryPixel, unsigned int radius);
  bool IsValidPatch(itk::ImageRegion<2> patch);

  unsigned int GetNumberOfPixelsInPatch();

  itk::Size<2> GetPatchSize();

  // Find the boundary of a region.
  void FindBoundary();
  
  // Compute the isophotes.
  void ComputeIsophotes();
  
  // Determine whether or not the inpainting is completed by seeing if there are any pixels in the mask that still need to be filled.
  bool HasMoreToInpaint(MaskImageType::Pointer mask);

  // Compute the normals of a region boundary.
  void ComputeBoundaryNormals();

  // ?
  void ExpandMask();

  // Criminisi specific functions
  // Compute the priorities at all boundary pixels.
  void ComputeAllPriorities();
  
  // Compute the priority of a specific pixel.
  float ComputePriority(itk::Index<2> queryPixel);
  
  // Compute the Confidence at a pixel.
  float ComputeConfidenceTerm(itk::Index<2> queryPixel);
  
  // Compute the Data at a pixel.
  float ComputeDataTerm(itk::Index<2> queryPixel);

  // Find the highest priority patch to be filled.
  itk::Index<2> FindHighestPriority(FloatScalarImageType::Pointer priorityImage);

  // Update the mask so that the pixel that was filled is marked as filled.
  void UpdateMask(itk::Index<2> pixel);

  // Locate all patches that are completely inside of the image and completely inside of the source region
  void ComputeSourcePatches();
  
  // Store the list of source patches computed with ComputeSourcePatches()
  std::vector<itk::ImageRegion<2> > SourcePatches;
  
  bool Debug;
  void DebugMessage(const std::string&);
  
  template <typename T>
  void DebugMessage(const std::string& message, T value);
};

#include "CriminisiInpainting.hxx"

#endif