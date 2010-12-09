/*
Copyright (C) 2010 David Doria, daviddoria@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
    //CIELABImageType::Pointer CIELABImage;
    ColorImageType::Pointer Patch;
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
    void DebugWriteAllImages(itk::Index<2> pixelToFill, unsigned int iteration);
    void DebugWritePatchToFill(itk::Index<2> pixelToFill, unsigned int iteration);
    void DebugWritePixelToFill(itk::Index<2> pixelToFill, unsigned int iteration);
    void DebugWritePatchToFillLocation(itk::Index<2> pixelToFill, unsigned int iteration);

    itk::CovariantVector<float, 2> GetAverageIsophote(itk::Index<2> queryPixel);
    bool IsValidPatch(itk::Index<2> queryPixel, unsigned int radius);

    void ColorToGrayscale(ColorImageType::Pointer colorImage, UnsignedCharImageType::Pointer grayscaleImage);

    unsigned int GetNumberOfPixelsInPatch();

    void UpdateConfidenceImage(itk::Index<2> sourcePixel, itk::Index<2> targetPixel);
    void UpdateIsophoteImage(itk::Index<2> sourcePixel, itk::Index<2> targetPixel);

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

    template <class T>
    void WriteScaledImage(typename T::Pointer image, std::string filename);

};

#endif