#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkJPEGReader.h>
#include <vtkImageActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkRenderer.h>
#include <vtkObjectFactory.h>
#include <vtkCommand.h>


#include "itkBinaryContourImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkConstNeighborhoodIterator.h"
#include "itkConstantBoundaryCondition.h"
#include "itkCovariantVector.h"
#include "itkDiscreteGaussianImageFilter.h"
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
#include "itkUnaryFunctorImageFilter.h"
#include "itkVariableLengthVector.h"

#include <vnl/vnl_double_2.h>

#include "RotateVectors.h"

typedef itk::Image< itk::CovariantVector<float, 2>, 2 > VectorImageType;
typedef itk::Image< unsigned char, 2 > UnsignedCharImageType;
typedef itk::Image< itk::Vector<unsigned char, 3>, 2 > ColorImageType;
typedef itk::Image< float, 2 > FloatImageType;

typedef itk::ConstantBoundaryCondition<FloatImageType>  FloatBoundaryConditionType;
typedef itk::ConstantBoundaryCondition<UnsignedCharImageType>  ScalarBoundaryConditionType;
typedef itk::ConstantBoundaryCondition<ColorImageType>  ColorBoundaryConditionType;

typedef itk::ConstNeighborhoodIterator<UnsignedCharImageType, ScalarBoundaryConditionType>::NeighborhoodType UnsignedCharNeighborhoodType;
typedef itk::ConstNeighborhoodIterator<FloatImageType, ScalarBoundaryConditionType>::NeighborhoodType FloatNeighborhoodType;
typedef itk::ConstNeighborhoodIterator<ColorImageType, ColorBoundaryConditionType>::NeighborhoodType ColorNeighborhoodType;

typedef  itk::ImageFileWriter< UnsignedCharImageType  > UnsignedCharImageWriterType;
typedef  itk::ImageFileWriter< FloatImageType  > FloatImageWriterType;
typedef  itk::ImageFileWriter< VectorImageType  > VectorImageWriterType;
typedef  itk::ImageFileWriter< ColorImageType  > ColorImageWriterType;

typedef  itk::ImageFileReader< UnsignedCharImageType  > UnsignedCharImageReaderType;
typedef  itk::ImageFileReader< ColorImageType  > ColorImageReaderType;

class CriminisiInpainting
{
  public:

    CriminisiInpainting()
    {
      this->PatchRadius[0] = 5;
      this->PatchRadius[1] = 5;

      this->BoundaryImage = UnsignedCharImageType::New();
      this->BoundaryNormals = VectorImageType::New();
      this->MeanDifferenceImage = FloatImageType::New();
      this->IsophoteImage = VectorImageType::New();
      this->PriorityImage = FloatImageType::New();
      this->Patch = NULL;
    }

    void Inpaint();
    void SetImage(ColorImageType::Pointer image){this->Image = image;}
    void SetMask(UnsignedCharImageType::Pointer mask){this->Mask = mask;}

  private:
    void DebugTests();
    void WriteDebugImages(FloatImageType::IndexType pixelToFill, unsigned int iteration);
    void WriteValidIsophoteImage();
    itk::CovariantVector<float, 2> GetAverageIsophote(UnsignedCharImageType::IndexType queryPixel);
    bool IsValidPatch(UnsignedCharImageType::IndexType queryPixel, unsigned int radius);
    
    void ReplaceValue(FloatImageType::Pointer iamge, float oldValue, float newValue);
    void CreateBorderMask(FloatImageType::Pointer mask);
    void ColorToGrayscale(ColorImageType::Pointer colorImage, UnsignedCharImageType::Pointer grayscaleImage);

    unsigned int GetNumberOfPixelsInPatch();

    void UpdateConfidenceImage(FloatImageType::IndexType sourcePixel, FloatImageType::IndexType targetPixel);
    void UpdateIsophoteImage(FloatImageType::IndexType sourcePixel, FloatImageType::IndexType targetPixel);

    UnsignedCharImageType::SizeType GetUnsignedCharPatchSize();

    ColorImageType::SizeType GetColorPatchSize();

    ColorImageType::Pointer Image;
    ColorImageType::Pointer Patch;
    UnsignedCharImageType::Pointer Mask;
    FloatImageType::Pointer ConfidenceImage;
    UnsignedCharImageType::SizeType PatchRadius;

    FloatImageType::Pointer MeanDifferenceImage;
    VectorImageType::Pointer IsophoteImage;
    UnsignedCharImageType::Pointer BoundaryImage;
    VectorImageType::Pointer BoundaryNormals;

    FloatImageType::Pointer PriorityImage;

    void ComputeIsophotes();
    bool HasMoreToInpaint(UnsignedCharImageType::Pointer mask);
    void FindBoundary();
    void ComputeBoundaryNormals();

    void CreateBlankUnsignedCharPatch(UnsignedCharImageType::Pointer patch);
    void CreateBlankFloatPatch(FloatImageType::Pointer patch);
    void CreateConstantUnsignedCharPatch(UnsignedCharImageType::Pointer patch, unsigned char value);
    void CreateConstantFloatPatch(FloatImageType::Pointer patch, float value);
    void CreateConstantFloatBlockingPatch(FloatImageType::Pointer patch, float value);

    void ComputeAllPriorities();
    float ComputePriority(UnsignedCharImageType::IndexType queryPixel);
    float ComputeConfidenceTerm(UnsignedCharImageType::IndexType queryPixel);
    float ComputeDataTerm(UnsignedCharImageType::IndexType queryPixel);

    void CopyPatchIntoImage(UnsignedCharImageType::Pointer patch, UnsignedCharImageType::Pointer &image, UnsignedCharImageType::IndexType position);
    void CopyPatchIntoImage(FloatImageType::Pointer patch, FloatImageType::Pointer &image, FloatImageType::IndexType position);
    void CopyPatchIntoImage(VectorImageType::Pointer patch, VectorImageType::Pointer &image, VectorImageType::IndexType position);
    void CopyPatchIntoImage(ColorImageType::Pointer patch, ColorImageType::Pointer &image, ColorImageType::IndexType position);

    FloatImageType::IndexType FindHighestPriority(FloatImageType::Pointer priorityImage);

    ColorImageType::IndexType FindBestPatchMatch(ColorImageType::IndexType pixel);

    void UpdateMask(UnsignedCharImageType::IndexType pixel);

    UnsignedCharImageType::RegionType GetRegionAroundPixel(UnsignedCharImageType::IndexType pixel);
    UnsignedCharImageType::RegionType GetRegionAroundPixel(UnsignedCharImageType::IndexType pixel, unsigned int radius);
    ColorImageType::RegionType GetColorRegionAroundPixel(ColorImageType::IndexType pixel);

};


// Define interaction style
class CustomStyle : public vtkInteractorStyleImage
{
  public:
    static CustomStyle* New();
    vtkTypeMacro(CustomStyle, vtkInteractorStyleImage);

    CustomStyle()
    {
      this->InpaintedActor = vtkSmartPointer<vtkImageActor>::New();
    }

    void CallbackFunction(vtkObject* caller,
                    long unsigned int eventId,
                    void* callData )
    {
      std::cout << "CustomStyle::CallbackFunction called." << std::endl;
      /*
      this->InpaintedActor->SetInput(intermediate);
      this->RightRenderer->ResetCamera();
      this->RightRenderer->Render();
      this->Interactor->GetRenderWindow()->Render();
      */
    }

    void OnKeyPress()
    {
      // Get the keypress
      std::string key = this->Interactor->GetKeySym();

      if(key.compare("s") == 0) // 'S'tart
        {
        /*
        this->RightRenderer->AddActor(this->InpaintedActor);
        this->Inpainting->SetMaximumIterations(1000);
        this->Inpainting->SetInputConnection(0, this->Image->GetProducerPort());
        this->Inpainting->SetInputConnection(1, this->Mask->GetProducerPort());
        this->Inpainting->AddObserver(vtkCommand::WarningEvent, this, &CustomStyle::CallbackFunction);
        this->Inpainting->Update();

        this->InpaintedActor->SetInput(this->Inpainting->GetOutput());
        this->RightRenderer->ResetCamera();
        this->RightRenderer->Render();
        */
        }

      // Forward events
      vtkInteractorStyleTrackballCamera::OnKeyPress();
    }

  vtkRenderer* RightRenderer;
  vtkImageData* Image;
  vtkImageData* Mask;
  vtkSmartPointer<vtkImageActor> InpaintedActor;

};
vtkStandardNewMacro(CustomStyle);

int main(int argc, char *argv[])
{
  if(argc != 3)
    {
    std::cerr << "Required arguments: image.jpg imageMask.jpg" << std::endl;
    return EXIT_FAILURE;
    }
  std::string imageFilename = argv[1];
  std::string maskFilename = argv[2];
  std::cout << "Reading image: " << imageFilename << std::endl;
  std::cout << "Reading mask: " << maskFilename << std::endl;

  ColorImageReaderType::Pointer imageReader = ColorImageReaderType::New();
  imageReader->SetFileName(imageFilename.c_str());
  imageReader->Update();

  UnsignedCharImageReaderType::Pointer maskReader = UnsignedCharImageReaderType::New();
  maskReader->SetFileName(maskFilename.c_str());
  maskReader->Update();

  typedef itk::BinaryThresholdImageFilter <UnsignedCharImageType, UnsignedCharImageType>
          BinaryThresholdImageFilterType;

  BinaryThresholdImageFilterType::Pointer thresholdFilter
          = BinaryThresholdImageFilterType::New();
  thresholdFilter->SetInput(maskReader->GetOutput());
  thresholdFilter->SetLowerThreshold(122);
  thresholdFilter->SetUpperThreshold(255);
  thresholdFilter->SetInsideValue(255);
  thresholdFilter->SetOutsideValue(0);
  thresholdFilter->InPlaceOn();
  thresholdFilter->Update();

  CriminisiInpainting Inpainting;
  Inpainting.SetImage(imageReader->GetOutput());
  Inpainting.SetMask(thresholdFilter->GetOutput());
  Inpainting.Inpaint();

  /*
  vtkSmartPointer<vtkImageActor> originalActor =
    vtkSmartPointer<vtkImageActor>::New();
  originalActor->SetInput(imageReader->GetOutput());

  vtkSmartPointer<vtkImageActor> maskActor =
    vtkSmartPointer<vtkImageActor>::New();
  maskActor->SetInput(maskReader->GetOutput());
  maskActor->SetOpacity(.5);

  // There will be one render window
  vtkSmartPointer<vtkRenderWindow> renderWindow =
    vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->SetSize(1000, 500);

  // And one interactor
  vtkSmartPointer<vtkRenderWindowInteractor> interactor =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  interactor->SetRenderWindow(renderWindow);

  // Define viewport ranges
  // (xmin, ymin, xmax, ymax)
  double leftViewport[4] = {0.0, 0.0, 0.5, 1.0};
  double rightViewport[4] = {0.5, 0.0, 1.0, 1.0};

  // Setup both renderers
  vtkSmartPointer<vtkRenderer> leftRenderer =
    vtkSmartPointer<vtkRenderer>::New();
  renderWindow->AddRenderer(leftRenderer);
  leftRenderer->SetViewport(leftViewport);
  leftRenderer->SetBackground(.6, .5, .4);

  vtkSmartPointer<vtkRenderer> rightRenderer =
    vtkSmartPointer<vtkRenderer>::New();
  renderWindow->AddRenderer(rightRenderer);
  rightRenderer->SetViewport(rightViewport);
  rightRenderer->SetBackground(.4, .5, .6);

  // Add the sphere to the left and the cube to the right
  leftRenderer->AddActor(originalActor);
  //leftRenderer->AddActor(maskActor);

  leftRenderer->ResetCamera();
  rightRenderer->ResetCamera();

  renderWindow->Render();

  vtkSmartPointer<CustomStyle> style =
    vtkSmartPointer<CustomStyle>::New();
  style->RightRenderer = rightRenderer;
  style->Image = imageReader->GetOutput();
  style->Mask = maskReader->GetOutput();
  interactor->SetInteractorStyle(style);

  interactor->Start();
  */
  return EXIT_SUCCESS;
}

void CriminisiInpainting::Inpaint()
{
  this->PriorityImage->SetRegions(this->Image->GetLargestPossibleRegion());
  this->PriorityImage->Allocate();

  // Clone the mask
  typedef itk::ImageDuplicator< UnsignedCharImageType > DuplicatorType;
  DuplicatorType::Pointer duplicator = DuplicatorType::New();
  duplicator->SetInputImage(this->Mask);
  duplicator->Update();

  // Invert the mask
  typedef itk::InvertIntensityImageFilter <UnsignedCharImageType>
          InvertIntensityImageFilterType;

  InvertIntensityImageFilterType::Pointer invertIntensityFilter
          = InvertIntensityImageFilterType::New();
  invertIntensityFilter->SetInput(duplicator->GetOutput());
  invertIntensityFilter->InPlaceOn();
  invertIntensityFilter->Update();

  // Convert the inverted mask to floats and scale them to between 0 and 1
  // to serve as the initial confidence image
  typedef itk::RescaleIntensityImageFilter< UnsignedCharImageType, FloatImageType > RescaleFilterType;
  RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
  rescaleFilter->SetInput(invertIntensityFilter->GetOutput());
  rescaleFilter->SetOutputMinimum(0);
  rescaleFilter->SetOutputMaximum(1);
  rescaleFilter->Update();

  this->ConfidenceImage = rescaleFilter->GetOutput();

  // Mask the input image with the inverted maskActor (blank the region in the input image that we will fill in)
  typedef itk::MaskImageFilter< ColorImageType, UnsignedCharImageType, ColorImageType > MaskFilterType;
  MaskFilterType::Pointer maskFilter = MaskFilterType::New();
  maskFilter->SetInput1(this->Image);
  maskFilter->SetInput2(invertIntensityFilter->GetOutput());
  maskFilter->InPlaceOn();
  ColorImageType::PixelType green;
  green[0] = 0;
  green[1] = 255;
  green[2] = 0;
  maskFilter->SetOutsideValue(green);
  maskFilter->Update();;

  this->Image = maskFilter->GetOutput();

  ComputeIsophotes();
  //WriteValidIsophoteImage();

  int iteration = 0;
  while(HasMoreToInpaint(this->Mask))
    {
    std::cout << "Iteration: " << iteration << std::endl;
    
    FindBoundary();

    ComputeBoundaryNormals();

    ComputeAllPriorities();

    FloatImageType::IndexType pixelToFill = FindHighestPriority(this->PriorityImage);
    std::cout << "Filling: " << pixelToFill << std::endl;

    ColorImageType::IndexType bestMatchPixel = FindBestPatchMatch(pixelToFill);
    std::cout << "Best match found at: " << bestMatchPixel << std::endl;

    // Extract the best patch
    typedef itk::RegionOfInterestImageFilter< ColorImageType,
                                                ColorImageType > ExtractFilterType;

    ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
    extractFilter->SetRegionOfInterest(GetColorRegionAroundPixel(bestMatchPixel));
    extractFilter->SetInput(this->Image);
    extractFilter->Update();
    this->Patch = extractFilter->GetOutput();

    CopyPatchIntoImage(this->Patch, this->Image, pixelToFill);

    UpdateConfidenceImage(bestMatchPixel, pixelToFill);
    UpdateIsophoteImage(bestMatchPixel, pixelToFill);

    // Update the mask
    UpdateMask(pixelToFill);

    // Sanity check everything
    //DebugTests();
    //WriteDebugImages(pixelToFill, iteration);
    
    iteration++;
    } // end main while loop

  {
  ColorImageWriterType::Pointer writer = ColorImageWriterType::New();
  writer->SetFileName("result.jpg");
  writer->SetInput(this->Image);
  writer->Update();
  }
}

void CriminisiInpainting::WriteDebugImages(FloatImageType::IndexType pixelToFill, unsigned int iteration)
{
  // Create a blank image with the pixel to fill colored white
  {
  UnsignedCharImageType::Pointer pixelImage = UnsignedCharImageType::New();
  pixelImage->SetRegions(this->Mask->GetLargestPossibleRegion());
  pixelImage->Allocate();

  itk::ImageRegionIterator<UnsignedCharImageType> iterator(pixelImage, pixelImage->GetLargestPossibleRegion());

  while(!iterator.IsAtEnd())
    {
    if(iterator.GetIndex()[0] == pixelToFill[0] && iterator.GetIndex()[1] == pixelToFill[1])
      {
      iterator.Set(255);
      }
    else
      {
      iterator.Set(0);
      }
    ++iterator;
    }
  UnsignedCharImageWriterType::Pointer writer = UnsignedCharImageWriterType::New();
  std::stringstream padded;
  padded << "PixelToFill_" << std::setfill('0') << std::setw(4) << iteration << ".jpg";
  writer->SetFileName(padded.str());
  writer->SetInput(pixelImage);
  writer->Update();
  }

  // Create a blank image with the patch that has been filled colored white
  {
  UnsignedCharImageType::Pointer patchImage = UnsignedCharImageType::New();
  patchImage->SetRegions(this->Mask->GetLargestPossibleRegion());
  patchImage->Allocate();
  // Make image black
  itk::ImageRegionIterator<UnsignedCharImageType> blackIterator(patchImage, patchImage->GetLargestPossibleRegion());

  while(!blackIterator.IsAtEnd())
    {
    blackIterator.Set(0);
    ++blackIterator;
    }

  UnsignedCharImageType::Pointer patch = UnsignedCharImageType::New();
  CreateConstantUnsignedCharPatch(patch, 255);

  UnsignedCharImageType::IndexType lowerLeft;
  lowerLeft[0] = pixelToFill[0] - this->Patch->GetLargestPossibleRegion().GetSize()[0]/2;
  lowerLeft[1] = pixelToFill[1] - this->Patch->GetLargestPossibleRegion().GetSize()[1]/2;
  typedef itk::PasteImageFilter <UnsignedCharImageType, UnsignedCharImageType >
          PasteImageFilterType;
  
  PasteImageFilterType::Pointer pasteFilter
          = PasteImageFilterType::New ();
  pasteFilter->SetInput(0, patchImage);
  pasteFilter->SetInput(1, patch);
  pasteFilter->SetSourceRegion(patch->GetLargestPossibleRegion());
  pasteFilter->SetDestinationIndex(lowerLeft);
  pasteFilter->InPlaceOn();
  pasteFilter->Update();
  UnsignedCharImageWriterType::Pointer writer = UnsignedCharImageWriterType::New();
  std::stringstream padded;
  padded << "PatchToFill_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  writer->SetFileName(padded.str());
  writer->SetInput(pasteFilter->GetOutput());
  writer->Update();
  }

  
  {
  ColorImageWriterType::Pointer writer = ColorImageWriterType::New();
  std::stringstream padded;
  padded << "Patch_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  writer->SetFileName(padded.str());
  writer->SetInput(this->Patch);
  writer->Update();
  }

  {
  VectorImageWriterType::Pointer writer = VectorImageWriterType::New();
  std::stringstream padded;
  padded << "Isophotes" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  writer->SetFileName(padded.str());
  writer->SetInput(this->IsophoteImage);
  writer->Update();
  }

  {
  FloatImageWriterType::Pointer writer = FloatImageWriterType::New();
  std::stringstream padded;
  padded << "Confidence_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  writer->SetFileName(padded.str());
  writer->SetInput(this->ConfidenceImage);
  writer->Update();
  }

  {
  UnsignedCharImageWriterType::Pointer writer = UnsignedCharImageWriterType::New();
  std::stringstream padded;
  //padded << "Boundary_" << std::setfill('0') << std::setw(4) << iteration << ".jpg";
  padded << "Boundary_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  writer->SetFileName(padded.str());
  writer->SetInput(this->BoundaryImage);
  writer->Update();
  }
  {
  VectorImageWriterType::Pointer writer = VectorImageWriterType::New();
  std::stringstream padded;
  padded << "BoundaryNormals_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  writer->SetFileName(padded.str());
  writer->SetInput(this->BoundaryNormals);
  writer->Update();
  }
  {
  FloatImageWriterType::Pointer writer = FloatImageWriterType::New();
  std::stringstream padded;
  padded << "Priorities_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  writer->SetFileName(padded.str());
  writer->SetInput(this->PriorityImage);
  writer->Update();
  }

  {
  FloatImageWriterType::Pointer writer = FloatImageWriterType::New();
  std::stringstream padded;
  padded << "Difference_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  writer->SetFileName(padded.str());
  writer->SetInput(this->MeanDifferenceImage);
  writer->Update();
  }

  {
  UnsignedCharImageWriterType::Pointer writer = UnsignedCharImageWriterType::New();
  std::stringstream padded;
  padded << "Mask_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
  writer->SetFileName(padded.str());
  writer->SetInput(this->Mask);
  writer->Update();
  }

  {
  ColorImageWriterType::Pointer writer = ColorImageWriterType::New();
  std::stringstream padded;
  padded << "FilledImage_" << std::setfill('0') << std::setw(4) << iteration << ".jpg";
  writer->SetFileName(padded.str());
  writer->SetInput(this->Image);
  writer->Update();
  }
}

void CriminisiInpainting::ComputeIsophotes()
{
  UnsignedCharImageType::Pointer grayscaleImage = UnsignedCharImageType::New();
  ColorToGrayscale(this->Image, grayscaleImage);

  typedef itk::DiscreteGaussianImageFilter<
          UnsignedCharImageType, FloatImageType >  filterType;

  // Create and setup a Gaussian filter
  filterType::Pointer gaussianFilter = filterType::New();
  gaussianFilter->SetInput(grayscaleImage);
  gaussianFilter->SetVariance(2);
  gaussianFilter->Update();

  typedef itk::GradientImageFilter<
      FloatImageType, float, float>  GradientFilterType;
  GradientFilterType::Pointer gradientFilter = GradientFilterType::New();
  gradientFilter->SetInput(gaussianFilter->GetOutput());
  gradientFilter->Update();

  // Rotate the gradient 90 degrees to obtain isophotes
  typedef itk::UnaryFunctorImageFilter<VectorImageType, VectorImageType,
                                  RotateVectors<
    VectorImageType::PixelType,
    VectorImageType::PixelType> > FilterType;

  FilterType::Pointer rotateFilter = FilterType::New();
  rotateFilter->SetInput(gradientFilter->GetOutput());
  rotateFilter->Update();

  this->IsophoteImage = rotateFilter->GetOutput();

}

bool CriminisiInpainting::HasMoreToInpaint(UnsignedCharImageType::Pointer mask)
{
  itk::ImageRegionIterator<UnsignedCharImageType> imageIterator(mask,mask->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    if(imageIterator.Get() != 0)
      {
      return true;
      }

    ++imageIterator;
    }

  return false;
}

void CriminisiInpainting::FindBoundary()
{
  typedef itk::BinaryContourImageFilter <UnsignedCharImageType, UnsignedCharImageType >
          binaryContourImageFilterType;

  binaryContourImageFilterType::Pointer binaryContourFilter
          = binaryContourImageFilterType::New ();
  binaryContourFilter->SetInput(this->Mask);
  binaryContourFilter->Update();

  this->BoundaryImage = binaryContourFilter->GetOutput();
}

void CriminisiInpainting::CreateBlankUnsignedCharPatch(UnsignedCharImageType::Pointer patch)
{
  UnsignedCharImageType::IndexType start;
  start[0] = 0;
  start[1] = 0;

  UnsignedCharImageType::RegionType region(start, this->GetUnsignedCharPatchSize());

  patch->SetRegions(region);
  patch->Allocate();

  itk::ImageRegionIterator<UnsignedCharImageType> imageIterator(patch, patch->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    imageIterator.Set(0);
    ++imageIterator;
    }
}

void CriminisiInpainting::CreateBlankFloatPatch(FloatImageType::Pointer patch)
{
  FloatImageType::IndexType start;
  start[0] = 0;
  start[1] = 0;

  FloatImageType::RegionType region(start, this->GetUnsignedCharPatchSize());

  patch->SetRegions(region);
  patch->Allocate();

  itk::ImageRegionIterator<FloatImageType> imageIterator(patch, patch->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    imageIterator.Set(0);
    ++imageIterator;
    }
}


void CriminisiInpainting::CreateConstantUnsignedCharPatch(UnsignedCharImageType::Pointer patch, unsigned char value)
{
  UnsignedCharImageType::IndexType start;
  start[0] = 0;
  start[1] = 0;

  UnsignedCharImageType::RegionType region(start, this->GetUnsignedCharPatchSize());

  patch->SetRegions(region);
  patch->Allocate();

  itk::ImageRegionIterator<UnsignedCharImageType> imageIterator(patch, patch->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    imageIterator.Set(value);
    ++imageIterator;
    }
}

void CriminisiInpainting::CreateConstantFloatPatch(FloatImageType::Pointer patch, float value)
{
  FloatImageType::IndexType start;
  start[0] = 0;
  start[1] = 0;

  FloatImageType::RegionType region(start, this->GetUnsignedCharPatchSize());

  patch->SetRegions(region);
  patch->Allocate();

  itk::ImageRegionIterator<FloatImageType> imageIterator(patch, patch->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    imageIterator.Set(value);
    ++imageIterator;
    }
}

void CriminisiInpainting::CreateConstantFloatBlockingPatch(FloatImageType::Pointer patch, float value)
{
  // Create a 3-patch x 3-patch patch
  FloatImageType::IndexType start;
  start[0] = 0;
  start[1] = 0;

  FloatImageType::SizeType size;
  size[0] = 3*this->GetUnsignedCharPatchSize()[0];
  size[1] = 3*this->GetUnsignedCharPatchSize()[1];

  FloatImageType::RegionType region(start, size);

  patch->SetRegions(region);
  patch->Allocate();

  itk::ImageRegionIterator<FloatImageType> imageIterator(patch, patch->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    imageIterator.Set(value);
    ++imageIterator;
    }
}

void CriminisiInpainting::UpdateMask(UnsignedCharImageType::IndexType pixel)
{
  // Create a black patch
  UnsignedCharImageType::Pointer blackPatch = UnsignedCharImageType::New();
  CreateBlankUnsignedCharPatch(blackPatch);

  pixel[0] -= blackPatch->GetLargestPossibleRegion().GetSize()[0]/2;
  pixel[1] -= blackPatch->GetLargestPossibleRegion().GetSize()[1]/2;
  
  // Paste it into the mask
  typedef itk::PasteImageFilter <UnsignedCharImageType, UnsignedCharImageType >
          PasteImageFilterType;

  PasteImageFilterType::Pointer pasteFilter
          = PasteImageFilterType::New();
  pasteFilter->SetInput(0, this->Mask);
  pasteFilter->SetInput(1, blackPatch);
  pasteFilter->SetSourceRegion(blackPatch->GetLargestPossibleRegion());
  pasteFilter->SetDestinationIndex(pixel);
  pasteFilter->InPlaceOn();
  pasteFilter->Update();

  this->Mask = pasteFilter->GetOutput();
}

void CriminisiInpainting::ComputeBoundaryNormals()
{
  typedef itk::DiscreteGaussianImageFilter<
          UnsignedCharImageType, FloatImageType >  filterType;

  // Create and setup a Gaussian filter
  filterType::Pointer gaussianFilter = filterType::New();
  gaussianFilter->SetInput(this->Mask);
  gaussianFilter->SetVariance(2);
  gaussianFilter->Update();

  typedef itk::GradientImageFilter<
      FloatImageType, float, float>  GradientFilterType;
  GradientFilterType::Pointer gradientFilter = GradientFilterType::New();
  gradientFilter->SetInput(gaussianFilter->GetOutput());
  gradientFilter->Update();

  typedef itk::MaskImageFilter< VectorImageType, UnsignedCharImageType, VectorImageType > MaskFilterType;
  MaskFilterType::Pointer maskFilter = MaskFilterType::New();
  maskFilter->SetInput1(gradientFilter->GetOutput());
  maskFilter->SetInput2(this->BoundaryImage);
  maskFilter->Update();

  this->BoundaryNormals = maskFilter->GetOutput();
}

FloatImageType::IndexType CriminisiInpainting::FindHighestPriority(FloatImageType::Pointer priorityImage)
{
  typedef itk::MinimumMaximumImageCalculator <FloatImageType>
          ImageCalculatorFilterType;

  ImageCalculatorFilterType::Pointer imageCalculatorFilter
          = ImageCalculatorFilterType::New ();
  imageCalculatorFilter->SetImage(priorityImage);
  imageCalculatorFilter->Compute();

  return imageCalculatorFilter->GetIndexOfMaximum();
}

ColorImageType::IndexType CriminisiInpainting::FindBestPatchMatch(ColorImageType::IndexType queryPixel)
{
  this->MeanDifferenceImage->SetRegions(this->Image->GetLargestPossibleRegion());
  this->MeanDifferenceImage->Allocate();

  // Setup a neighborhood iterator over the whole image
  itk::ConstNeighborhoodIterator<ColorImageType, ColorBoundaryConditionType> imageIterator(this->PatchRadius, this->Image, this->Image->GetLargestPossibleRegion());
  imageIterator.GoToBegin();

  itk::ImageRegionIterator<FloatImageType> meanDifferenceIterator(this->MeanDifferenceImage, this->MeanDifferenceImage->GetLargestPossibleRegion());
  meanDifferenceIterator.GoToBegin();

  // Setup an iterator over the fixed image patch
  itk::ImageRegionConstIterator<ColorImageType> fixedImagePatchIterator(this->Image, GetRegionAroundPixel(queryPixel));
  fixedImagePatchIterator.GoToBegin();

  // Setup an iterator over the fixed mask patch
  itk::ImageRegionConstIterator<UnsignedCharImageType> fixedMaskPatchIterator(this->Mask, GetRegionAroundPixel(queryPixel));
  fixedMaskPatchIterator.GoToBegin();

  unsigned int numberOfPixelsInNeighborhood = this->GetUnsignedCharPatchSize()[0] * this->GetUnsignedCharPatchSize()[1];
  // Loop over every pixel in the image with a neighborhood
  while(!imageIterator.IsAtEnd())
    {
    double sumDifference = 0;
    fixedImagePatchIterator.GoToBegin();
    fixedMaskPatchIterator.GoToBegin();
    // Only consider pixels which have a neighborhood entirely inside of the image
    if(!imageIterator.InBounds())
      {
      meanDifferenceIterator.Set(-1);
      ++imageIterator;
      ++meanDifferenceIterator;
      continue;
      }

    bool valid = true;
    // Loop over all pixels in the neighborhood of the current pixel
    for(unsigned int i = 0; i < numberOfPixelsInNeighborhood; i++)
      {
      ColorImageType::PixelType imagePixel = imageIterator.GetPixel(i);
      if(imagePixel[0] == 0 && imagePixel[1] == 255 && imagePixel[2] == 0)
        {
        valid = false;
        }
      if(fixedMaskPatchIterator.Get() == 0) // We are in the source region
        {
        //double difference = (imagePatchIterator.Get() - imageIterator.GetPixel(i)).GetNorm();
        //sumDifference += difference*difference;
        ColorImageType::PixelType patchPixel = fixedImagePatchIterator.Get();

        for(unsigned int p = 0; p < 3; p++)
          {
          sumDifference += std::abs(static_cast<float>(imagePixel[p]) - static_cast<float>(patchPixel[p]));
          }
        }
      else // pixels from the target region should not be compared
        {
        }
      ++fixedImagePatchIterator;
      ++fixedMaskPatchIterator;
      }

    if(valid)
      {
      meanDifferenceIterator.Set(sumDifference);
      }
    else
      {
      meanDifferenceIterator.Set(-1);
      }
    ++meanDifferenceIterator;
    ++imageIterator;
    }

  // Compute the maximum value of the difference image. We will use this to fill the constant blank patch
  typedef itk::MinimumMaximumImageCalculator <FloatImageType>
          ImageCalculatorFilterType;

  ImageCalculatorFilterType::Pointer imageCalculatorFilter
          = ImageCalculatorFilterType::New ();
  imageCalculatorFilter->SetImage(this->MeanDifferenceImage);
  imageCalculatorFilter->Compute();

  float highestValue = imageCalculatorFilter->GetMaximum();

  // Paste a blank patch into the correlation image so that we don't match a region within the current patch or anywhere near it
  FloatImageType::Pointer blankPatch = FloatImageType::New();
  CreateConstantFloatBlockingPatch(blankPatch, highestValue);
  CopyPatchIntoImage(blankPatch, this->MeanDifferenceImage, queryPixel);

  ReplaceValue(this->MeanDifferenceImage, -1, highestValue);
    {
    FloatImageWriterType::Pointer writer = FloatImageWriterType::New();
    writer->SetFileName("BlankedCorrelation.mhd");
    writer->SetInput(this->MeanDifferenceImage);
    writer->Update();
    }

  imageCalculatorFilter->SetImage(this->MeanDifferenceImage);
  imageCalculatorFilter->Compute();

  UnsignedCharImageType::IndexType bestMatchIndex = imageCalculatorFilter->GetIndexOfMinimum();
  std::cout << "BestMatchIndex: " << bestMatchIndex << std::endl;
  return bestMatchIndex;
}

void CriminisiInpainting::CopyPatchIntoImage(UnsignedCharImageType::Pointer patch, UnsignedCharImageType::Pointer &image, UnsignedCharImageType::IndexType position)
{
  typedef itk::PasteImageFilter <UnsignedCharImageType, UnsignedCharImageType >
          PasteImageFilterType;

  // We have passed the center of the patch, but the paste filter wants the lower left corner of the patch
  position[0] -= patch->GetLargestPossibleRegion().GetSize()[0]/2;
  position[1] -= patch->GetLargestPossibleRegion().GetSize()[1]/2;

  PasteImageFilterType::Pointer pasteFilter
          = PasteImageFilterType::New ();
  pasteFilter->SetInput(0, image);
  pasteFilter->SetInput(1, patch);
  pasteFilter->SetSourceRegion(patch->GetLargestPossibleRegion());
  pasteFilter->SetDestinationIndex(position);
  pasteFilter->InPlaceOn();
  pasteFilter->Update();

  image = pasteFilter->GetOutput();
}


void CriminisiInpainting::CopyPatchIntoImage(ColorImageType::Pointer patch, ColorImageType::Pointer &image, ColorImageType::IndexType position)
{
  typedef itk::PasteImageFilter <ColorImageType, ColorImageType >
          PasteImageFilterType;

  // We have passed the center of the patch, but the paste filter wants the lower left corner of the patch
  position[0] -= patch->GetLargestPossibleRegion().GetSize()[0]/2;
  position[1] -= patch->GetLargestPossibleRegion().GetSize()[1]/2;

  PasteImageFilterType::Pointer pasteFilter
          = PasteImageFilterType::New ();
  pasteFilter->SetInput(0, image);
  pasteFilter->SetInput(1, patch);
  pasteFilter->SetSourceRegion(patch->GetLargestPossibleRegion());
  pasteFilter->SetDestinationIndex(position);
  pasteFilter->InPlaceOn();
  pasteFilter->Update();

  image = pasteFilter->GetOutput();
}

void CriminisiInpainting::CopyPatchIntoImage(VectorImageType::Pointer patch, VectorImageType::Pointer &image, VectorImageType::IndexType position)
{
  typedef itk::PasteImageFilter <VectorImageType, VectorImageType >
          PasteImageFilterType;

  // We have passed the center of the patch, but the paste filter wants the lower left corner of the patch
  position[0] -= patch->GetLargestPossibleRegion().GetSize()[0]/2;
  position[1] -= patch->GetLargestPossibleRegion().GetSize()[1]/2;

  PasteImageFilterType::Pointer pasteFilter
          = PasteImageFilterType::New ();
  pasteFilter->SetInput(0, image);
  pasteFilter->SetInput(1, patch);
  pasteFilter->SetSourceRegion(patch->GetLargestPossibleRegion());
  pasteFilter->SetDestinationIndex(position);
  pasteFilter->InPlaceOn();
  pasteFilter->Update();

  image = pasteFilter->GetOutput();
}

void CriminisiInpainting::CopyPatchIntoImage(FloatImageType::Pointer patch, FloatImageType::Pointer &image, FloatImageType::IndexType position)
{
  typedef itk::PasteImageFilter <FloatImageType, FloatImageType >
          PasteImageFilterType;

  // We have passed the center of the patch, but the paste filter wants the lower left corner of the patch
  position[0] -= patch->GetLargestPossibleRegion().GetSize()[0]/2;
  position[1] -= patch->GetLargestPossibleRegion().GetSize()[1]/2;

  PasteImageFilterType::Pointer pasteFilter
          = PasteImageFilterType::New ();
  pasteFilter->SetInput(0, image);
  pasteFilter->SetInput(1, patch);
  pasteFilter->SetSourceRegion(patch->GetLargestPossibleRegion());
  pasteFilter->SetDestinationIndex(position);
  pasteFilter->InPlaceOn();
  pasteFilter->Update();

  image = pasteFilter->GetOutput();
}

void CriminisiInpainting::WriteValidIsophoteImage()
{
  VectorImageType::Pointer validIsophotes = VectorImageType::New();
  validIsophotes->Graft(this->IsophoteImage);

  itk::ImageRegionIterator<VectorImageType> iterator(validIsophotes,validIsophotes->GetLargestPossibleRegion());

  itk::NeighborhoodIterator<VectorImageType> neighborhoodIterator(this->PatchRadius, validIsophotes, validIsophotes->GetLargestPossibleRegion() );

  itk::CovariantVector<float, 2> emptyVector;
  emptyVector[0] = 0;
  emptyVector[1] = 0;
  
  while(!iterator.IsAtEnd())
    {
    //std::cout << "iterator: " << iterator.GetIndex() << std::endl;
    //std::cout << "neighborhood iterator: " << neighborhoodIterator.GetIndex() << std::endl;
    if(neighborhoodIterator.InBounds())
      {
      iterator.Set(GetAverageIsophote(iterator.GetIndex()));
      }
    else
      {
      iterator.Set(emptyVector);
      }
    ++iterator;
    ++neighborhoodIterator;
    }
  
  VectorImageWriterType::Pointer writer = VectorImageWriterType::New();
  writer->SetFileName("ValidIsophotes.mhd");
  writer->SetInput(validIsophotes);
  writer->Update();
}

void CriminisiInpainting::ComputeAllPriorities()
{
  // Only compute priorities for pixels on the boundary
  itk::ImageRegionConstIterator<UnsignedCharImageType> boundaryIterator(this->BoundaryImage, this->BoundaryImage->GetLargestPossibleRegion());
  itk::ImageRegionIterator<FloatImageType> priorityIterator(this->PriorityImage, this->PriorityImage->GetLargestPossibleRegion());

  boundaryIterator.GoToBegin();
  priorityIterator.GoToBegin();

  // The main loop is over the boundary image. We only want to compute priorities at boundary pixels
  while(!boundaryIterator.IsAtEnd())
    {
    // If the pixel is not on the boundary, skip it and set its priority to zero.
    if(boundaryIterator.Get() == 0)
      {
      priorityIterator.Set(0);
      ++boundaryIterator;
      ++priorityIterator;
      continue;
      }

    float priority = ComputePriority(boundaryIterator.GetIndex());
    //std::cout << "Priority: " << priority << std::endl;
    priorityIterator.Set(priority);

    ++boundaryIterator;
    ++priorityIterator;
    }
}

float CriminisiInpainting::ComputePriority(UnsignedCharImageType::IndexType queryPixel)
{
  double confidence = ComputeConfidenceTerm(queryPixel);
  double data = ComputeDataTerm(queryPixel);

  return confidence + data;
}


float CriminisiInpainting::ComputeConfidenceTerm(UnsignedCharImageType::IndexType queryPixel)
{
  itk::ImageRegionConstIterator<UnsignedCharImageType> maskIterator(this->Mask, GetRegionAroundPixel(queryPixel));
  itk::ImageRegionConstIterator<FloatImageType> confidenceIterator(this->ConfidenceImage, GetRegionAroundPixel(queryPixel));

  // confidence = sum of the confidences of patch pixels in the source region / area of the patch

  unsigned int numberOfPixels = GetNumberOfPixelsInPatch();

  float sum = 0;

  while(!maskIterator.IsAtEnd())
    {
    if(maskIterator.Get() == 0) // Pixel is in the source region
      {
      sum += confidenceIterator.Get();
      }
    ++confidenceIterator;
    ++maskIterator;
    }

  float areaOfPatch = static_cast<float>(numberOfPixels);
  return sum/areaOfPatch;
}

float CriminisiInpainting::ComputeDataTerm(UnsignedCharImageType::IndexType queryPixel)
{
  itk::CovariantVector<float, 2> isophote = this->IsophoteImage->GetPixel(queryPixel);
  //itk::CovariantVector<float, 2> isophote = GetAverageIsophote(queryPixel);
  itk::CovariantVector<float, 2> boundaryNormal = this->BoundaryNormals->GetPixel(queryPixel);

  double alpha = 255; // for grey scale images
  // D(p) = |dot(isophote direction at p, normal of the front at p)|/alpha

  vnl_double_2 vnlIsophote(isophote[0], isophote[1]);

  vnl_double_2 vnlNormal(boundaryNormal[0], boundaryNormal[1]);

  double dot = std::abs(dot_product(vnlIsophote,vnlNormal));

  return dot/alpha;
}

bool CriminisiInpainting::IsValidPatch(UnsignedCharImageType::IndexType queryPixel, unsigned int radius)
{
  // Check if the patch is inside the image
  if(!this->Mask->GetLargestPossibleRegion().IsInside(GetRegionAroundPixel(queryPixel,radius)))
    {
    return false;
    }

  // Check if all the pixels in the patch are in the valid region of the image
  itk::ImageRegionConstIterator<UnsignedCharImageType> iterator(this->Mask,GetRegionAroundPixel(queryPixel, radius));
    while(!iterator.IsAtEnd())
    {
    if(iterator.Get()) // valid(unmasked) pixels are 0, masked pixels are 1 (non-zero)
      {
      return false;
      }

    ++iterator;
    }

  return true;
}

itk::CovariantVector<float, 2> CriminisiInpainting::GetAverageIsophote(UnsignedCharImageType::IndexType queryPixel)
{
  if(!this->Mask->GetLargestPossibleRegion().IsInside(GetRegionAroundPixel(queryPixel)))
    {
    itk::CovariantVector<float, 2> v;
    v[0] = 0; v[1] = 0;
    return v;
    }
    
  itk::ImageRegionConstIterator<UnsignedCharImageType> iterator(this->Mask,GetRegionAroundPixel(queryPixel));

  std::vector<itk::CovariantVector<float, 2> > vectors;
  
  while(!iterator.IsAtEnd())
    {
    if(IsValidPatch(iterator.GetIndex(), 3))
      {
      vectors.push_back(this->IsophoteImage->GetPixel(iterator.GetIndex()));
      }

    ++iterator;
    }

  itk::CovariantVector<float, 2> averageVector;
  averageVector[0] = 0;
  averageVector[1] = 0;

  if(vectors.size() == 0)
    {
    return averageVector;
    }
    
  for(unsigned int i = 0; i < vectors.size(); i++)
    {
    averageVector[0] += vectors[i][0];
    averageVector[1] += vectors[i][1];
    }
  averageVector[0] /= vectors.size();
  averageVector[1] /= vectors.size();

  return averageVector;
}

UnsignedCharImageType::RegionType CriminisiInpainting::GetRegionAroundPixel(UnsignedCharImageType::IndexType pixel)
{
  return GetRegionAroundPixel(pixel, this->GetUnsignedCharPatchSize()[0]/2);
}

UnsignedCharImageType::RegionType CriminisiInpainting::GetRegionAroundPixel(UnsignedCharImageType::IndexType pixel, unsigned int radius)
{
  pixel[0] -= radius;
  pixel[1] -= radius;

  UnsignedCharImageType::RegionType region;
  region.SetIndex(pixel);
  UnsignedCharImageType::SizeType size;
  size[0] = radius*2 + 1;
  size[1] = radius*2 + 1;
  region.SetSize(size);

  return region;
}

ColorImageType::RegionType CriminisiInpainting::GetColorRegionAroundPixel(ColorImageType::IndexType pixel)
{
  pixel[0] -= this->GetColorPatchSize()[0]/2;
  pixel[1] -= this->GetColorPatchSize()[1]/2;

  ColorImageType::RegionType region;
  region.SetIndex(pixel);
  region.SetSize(this->GetColorPatchSize());

  return region;
}

void CriminisiInpainting::UpdateConfidenceImage(FloatImageType::IndexType sourcePixel, FloatImageType::IndexType targetPixel)
{
  // loop over all pixels in the target region which were just filled and update their
  itk::ImageRegionIterator<FloatImageType> confidenceIterator(this->ConfidenceImage, GetRegionAroundPixel(targetPixel));
  itk::ImageRegionIterator<UnsignedCharImageType> maskIterator(this->Mask, GetRegionAroundPixel(targetPixel));
  confidenceIterator.GoToBegin();
  maskIterator.GoToBegin();

  while(!maskIterator.IsAtEnd())
    {
    if(maskIterator.Get()) // this was a target pixel, compute its confidence
      {
      confidenceIterator.Set(ComputeConfidenceTerm(maskIterator.GetIndex()));
      }
    ++confidenceIterator;
    ++maskIterator;
    }
}

void CriminisiInpainting::UpdateIsophoteImage(FloatImageType::IndexType sourcePixel, FloatImageType::IndexType targetPixel)
{
  // Copy confidences from best patch
  typedef itk::RegionOfInterestImageFilter< VectorImageType,
                                            VectorImageType > ExtractFilterType;

  ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
  extractFilter->SetRegionOfInterest(GetRegionAroundPixel(sourcePixel));
  extractFilter->SetInput(this->IsophoteImage);
  extractFilter->Update();

  VectorImageType::Pointer image = extractFilter->GetOutput();

  itk::ImageRegionIterator<VectorImageType> imageIterator(image,image->GetLargestPossibleRegion());
  itk::ImageRegionIterator<UnsignedCharImageType> maskIterator(this->Mask, GetRegionAroundPixel(sourcePixel));
  imageIterator.GoToBegin();
  maskIterator.GoToBegin();

  // "clear" the pixels which are in the target region.
  VectorImageType::PixelType blankPixel;
  blankPixel[0] = 0;
  blankPixel[1] = 0;

  while(!imageIterator.IsAtEnd())
  {
    if(maskIterator.Get() != 0) // we are in the target region
      {
      imageIterator.Set(blankPixel);
      }

    ++imageIterator;
    ++maskIterator;
  }

  CopyPatchIntoImage(image, this->IsophoteImage, targetPixel);
}

void CriminisiInpainting::ColorToGrayscale(ColorImageType::Pointer colorImage, UnsignedCharImageType::Pointer grayscaleImage)
{
  grayscaleImage->SetRegions(colorImage->GetLargestPossibleRegion());
  grayscaleImage->Allocate();

  itk::ImageRegionConstIterator<ColorImageType> colorImageIterator(colorImage,colorImage->GetLargestPossibleRegion());
  itk::ImageRegionIterator<UnsignedCharImageType> grayscaleImageIterator(grayscaleImage,grayscaleImage->GetLargestPossibleRegion());

  ColorImageType::PixelType largestPixel;
  largestPixel[0] = 255;
  largestPixel[1] = 255;
  largestPixel[2] = 255;

  float largestNorm = largestPixel.GetNorm();

  while(!colorImageIterator.IsAtEnd())
  {

    grayscaleImageIterator.Set(colorImageIterator.Get().GetNorm()*(255./largestNorm));

    ++colorImageIterator;
    ++grayscaleImageIterator;
  }
}

unsigned int CriminisiInpainting::GetNumberOfPixelsInPatch()
{
  return this->GetUnsignedCharPatchSize()[0]*this->GetUnsignedCharPatchSize()[1];
}

void CriminisiInpainting::CreateBorderMask(FloatImageType::Pointer image)
{
  itk::NeighborhoodIterator<FloatImageType, FloatBoundaryConditionType> imageIterator(this->PatchRadius, image, image->GetLargestPossibleRegion());
  imageIterator.GoToBegin();
  while(!imageIterator.IsAtEnd())
    {
    if(imageIterator.InBounds())
      {
      imageIterator.SetCenterPixel(1);
      }
    else
      {
      imageIterator.SetCenterPixel(0);
      }
    ++imageIterator;
    }
}

void CriminisiInpainting::ReplaceValue(FloatImageType::Pointer image, float oldValue, float newValue)
{
  itk::ImageRegionIterator<FloatImageType> imageIterator(image, image->GetLargestPossibleRegion());
  imageIterator.GoToBegin();
  while(!imageIterator.IsAtEnd())
    {
    if(imageIterator.Get() == oldValue)
      {
      imageIterator.Set(newValue);
      }
    ++imageIterator;
    }
}

UnsignedCharImageType::SizeType CriminisiInpainting::GetUnsignedCharPatchSize()
{
  UnsignedCharImageType::SizeType patchSize;

  patchSize[0] = (this->PatchRadius[0]*2)+1;
  patchSize[1] = (this->PatchRadius[1]*2)+1;

  return patchSize;
}

ColorImageType::SizeType CriminisiInpainting::GetColorPatchSize()
{
  ColorImageType::SizeType patchSize;

  patchSize[0] = (this->PatchRadius[0]*2)+1;
  patchSize[1] = (this->PatchRadius[1]*2)+1;

  return patchSize;
}

void CriminisiInpainting::DebugTests()
{
  // Check to make sure patch doesn't have any masked pixels
  itk::ImageRegionIterator<ColorImageType> patchIterator(this->Patch,this->Patch->GetLargestPossibleRegion());

  while(!patchIterator.IsAtEnd())
    {
    ColorImageType::PixelType val = patchIterator.Get();
    if(val[0] == 0 && val[1] == 255 && val[2] == 0)
      {
      std::cerr << "Patch has a blank pixel!" << std::endl;
      exit(-1);
      }
    ++patchIterator;
    }
}