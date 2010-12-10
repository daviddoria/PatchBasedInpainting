/*=========================================================================
 *
 *  Copyright David Doria 2010 daviddoria@gmail.com
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
#include "itkRegionOfInterestImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkRigid2DTransform.h"
#include "itkSubtractImageFilter.h"
#include "itkUnaryFunctorImageFilter.h"
#include "itkVariableLengthVector.h"

typedef itk::Image< itk::CovariantVector<float, 2>, 2 > VectorImageType;
typedef itk::Image< unsigned char, 2 > UnsignedCharImageType;
typedef itk::Image< itk::CovariantVector<unsigned char, 3>, 2 > ColorImageType;
typedef itk::Image< float, 2 > FloatImageType;

typedef itk::ConstantBoundaryCondition<FloatImageType>  FloatBoundaryConditionType;
typedef itk::ConstantBoundaryCondition<UnsignedCharImageType>  UnsignedCharBoundaryConditionType;
typedef itk::ConstantBoundaryCondition<ColorImageType>  ColorBoundaryConditionType;

typedef itk::ConstNeighborhoodIterator<UnsignedCharImageType, UnsignedCharBoundaryConditionType>::NeighborhoodType UnsignedCharNeighborhoodType;
typedef itk::ConstNeighborhoodIterator<FloatImageType, UnsignedCharBoundaryConditionType>::NeighborhoodType FloatNeighborhoodType;
typedef itk::ConstNeighborhoodIterator<ColorImageType, ColorBoundaryConditionType>::NeighborhoodType ColorNeighborhoodType;

typedef  itk::ImageFileReader< UnsignedCharImageType  > UnsignedCharImageReaderType;
typedef  itk::ImageFileReader< ColorImageType  > ColorImageReaderType;


class CriminisiInpainting
{
  public:

    CriminisiInpainting();

    void Inpaint();
    void SetImage(ColorImageType::Pointer image);
    void SetInputMask(UnsignedCharImageType::Pointer mask);

    // Debugging
    void SetWriteIntermediateImages(bool);

  private:
    // Debugging
    bool WriteIntermediateImages;

    // Data members
    ColorImageType::Pointer Image;
    //ColorImageType::Pointer Patch;
    UnsignedCharImageType::Pointer InputMask;
    UnsignedCharImageType::Pointer Mask;
    FloatImageType::Pointer ConfidenceImage;
    UnsignedCharImageType::SizeType PatchRadius;

    FloatImageType::Pointer MeanDifferenceImage;
    VectorImageType::Pointer IsophoteImage;
    UnsignedCharImageType::Pointer BoundaryImage;
    VectorImageType::Pointer BoundaryNormals;

    FloatImageType::Pointer PriorityImage;

    // Functions

    void Initialize();

    // Debugging
    void DebugTests();
    void DebugWriteAllImages(itk::Index<2> pixelToFill, itk::Index<2> bestMatchPixel, unsigned int iteration);
    void DebugWritePatch(itk::Index<2> pixel, std::string filePrefix, unsigned int iteration);
    void DebugWritePixelToFill(itk::Index<2> pixelToFill, unsigned int iteration);
    void DebugWritePatchToFillLocation(itk::Index<2> pixelToFill, unsigned int iteration);

    itk::CovariantVector<float, 2> GetAverageIsophote(itk::Index<2> queryPixel);
    bool IsValidPatch(itk::Index<2> queryPixel, unsigned int radius);

    void ColorToGrayscale(ColorImageType::Pointer colorImage, UnsignedCharImageType::Pointer grayscaleImage);

    unsigned int GetNumberOfPixelsInPatch();

    itk::Size<2> GetPatchSize();

    void FindBoundary();
    void ComputeIsophotes();
    bool HasMoreToInpaint(UnsignedCharImageType::Pointer mask);

    void ComputeBoundaryNormals();

    void ExpandMask();

    // Criminisi specific functions
    void ComputeAllPriorities();
    float ComputePriority(itk::Index<2> queryPixel);
    float ComputeConfidenceTerm(itk::Index<2> queryPixel);
    float ComputeDataTerm(itk::Index<2> queryPixel);

    itk::Index<2> FindHighestPriority(FloatImageType::Pointer priorityImage);

    void UpdateMask(itk::Index<2> pixel);

};

#endif