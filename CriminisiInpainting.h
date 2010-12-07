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
typedef itk::Image< itk::CovariantVector<int, 3>, 2 > CIELABImageType;
typedef itk::Image< float, 2 > FloatImageType;

typedef itk::ConstantBoundaryCondition<FloatImageType>  FloatBoundaryConditionType;
typedef itk::ConstantBoundaryCondition<UnsignedCharImageType>  UnsignedCharBoundaryConditionType;
typedef itk::ConstantBoundaryCondition<ColorImageType>  ColorBoundaryConditionType;
typedef itk::ConstantBoundaryCondition<CIELABImageType>  CIELABBoundaryConditionType;

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
    void SetImage(ColorImageType::Pointer image){this->Image->Graft(image);}
    void SetInputMask(UnsignedCharImageType::Pointer mask){this->InputMask->Graft(mask);}

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
    template <class T>
    void CopyPatchIntoTargetRegion(typename T::Pointer patch, typename T::Pointer image, itk::Index<2> position);
    itk::CovariantVector<int ,3> RGBtoCIELAB(itk::CovariantVector<unsigned char, 3> rgb);
    void CreateCIELABImage();

    // Debugging
    void DebugTests();
    void WriteDebugImages(itk::Index<2> pixelToFill, unsigned int iteration);

    itk::CovariantVector<float, 2> GetAverageIsophote(itk::Index<2> queryPixel);
    bool IsValidPatch(itk::Index<2> queryPixel, unsigned int radius);

    void ReplaceValue(FloatImageType::Pointer iamge, float oldValue, float newValue);
    void CreateBorderMask(FloatImageType::Pointer mask);
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

    template <class T>
    void CopyPatchIntoImage(typename T::Pointer patch, typename T::Pointer &image, itk::Index<2> position);

    itk::Index<2> FindHighestPriority(FloatImageType::Pointer priorityImage);

    itk::Index<2> FindBestPatchMatch(itk::Index<2> pixel);

    void UpdateMask(itk::Index<2> pixel);

    // General operations
    itk::ImageRegion<2> GetRegionAroundPixel(itk::Index<2> pixel);
    itk::ImageRegion<2> GetRegionAroundPixel(itk::Index<2> pixel, unsigned int radius);

    template <class T>
    void CreateBlankPatch(typename T::Pointer patch);

    template <class T>
    void CreateConstantPatch(typename T::Pointer patch, unsigned char value);

    void CreateConstantFloatBlockingPatch(FloatImageType::Pointer patch, float value);

    template <class T>
    void WriteScaledImage(typename T::Pointer image, std::string filename);

    template <class T>
    void WriteImage(typename T::Pointer image, std::string filename);
};

#endif