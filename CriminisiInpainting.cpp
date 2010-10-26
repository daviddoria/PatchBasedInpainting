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
#include "itkRigid2DTransform.h"
#include "itkUnaryFunctorImageFilter.h"
#include "itkVariableLengthVector.h"

#include <vnl/vnl_double_2.h>

#include "RotateVectors.h"

typedef itk::Image< itk::CovariantVector<float, 2>, 2 > VectorImageType;
typedef itk::Image< unsigned char, 2 > UnsignedCharImageType;
typedef itk::Image< itk::Vector<unsigned char, 2>, 2 > ColorImageType;
typedef itk::Image< float, 2 > FloatImageType;
typedef itk::ConstantBoundaryCondition<UnsignedCharImageType>  BoundaryConditionType;
typedef itk::ConstNeighborhoodIterator<UnsignedCharImageType, BoundaryConditionType>::NeighborhoodType UnsignedCharNeighborhoodType;
typedef itk::ConstNeighborhoodIterator<FloatImageType, BoundaryConditionType>::NeighborhoodType FloatNeighborhoodType;

typedef  itk::ImageFileWriter< UnsignedCharImageType  > UnsignedCharImageWriterType;
typedef  itk::ImageFileWriter< FloatImageType  > FloatImageWriterType;
typedef  itk::ImageFileWriter< VectorImageType  > VectorImageWriterType;

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
    }

    void Inpaint();
    void SetImage(ColorImageType::Pointer image){this->Image = image;}
    void SetMask(UnsignedCharImageType::Pointer mask){this->Mask = mask;}

  private:

    void UpdateConfidenceImage(FloatImageType::IndexType sourcePixel, FloatImageType::IndexType targetPixel);
    void UpdateIsophoteImage(FloatImageType::IndexType sourcePixel, FloatImageType::IndexType targetPixel);

    UnsignedCharImageType::SizeType GetPatchSize()
    {
      UnsignedCharImageType::SizeType patchSize;

      patchSize[0] = (this->PatchRadius[0]*2)+1;
      patchSize[1] = (this->PatchRadius[1]*2)+1;

      return patchSize;
    }

    UnsignedCharImageType::Pointer Image;
    UnsignedCharImageType::Pointer Mask;
    FloatImageType::Pointer ConfidenceImage;
    UnsignedCharImageType::SizeType PatchRadius;

    FloatImageType::Pointer MeanDifferenceImage;
    VectorImageType::Pointer IsophoteImage;
    UnsignedCharImageType::Pointer BoundaryImage;
    VectorImageType::Pointer BoundaryNormals;

    void ComputeIsophotes();
    bool HasMoreToInpaint(UnsignedCharImageType::Pointer mask);
    void FindBoundary();
    void ComputeBoundaryNormals();

    void CreateBlankUnsignedCharPatch(UnsignedCharImageType::Pointer patch);
    void CreateBlankFloatPatch(FloatImageType::Pointer patch);
    void CreateConstantFloatPatch(FloatImageType::Pointer patch, float value);

    void ComputeAllPriorities(VectorImageType::Pointer isophotes, UnsignedCharImageType::Pointer boundary,
                              VectorImageType::Pointer boundaryNormals, FloatImageType::Pointer priorityImage);
    float ComputePriority(UnsignedCharNeighborhoodType imagePatch, UnsignedCharNeighborhoodType maskPatch,
                          FloatNeighborhoodType confidencePatch,
                          itk::CovariantVector<float, 2> isophote, itk::CovariantVector<float, 2> boundaryNormal);
    float ComputeConfidenceTerm(UnsignedCharNeighborhoodType imagePatch, UnsignedCharNeighborhoodType maskPatch, FloatNeighborhoodType confidencePatch);
    float ComputeDataTerm(itk::CovariantVector<float, 2> isophote, itk::CovariantVector<float, 2> boundaryNormal);

    void CopyPatchIntoImage(UnsignedCharImageType::Pointer patch, UnsignedCharImageType::Pointer &image, UnsignedCharImageType::IndexType position);
    void CopyPatchIntoImage(FloatImageType::Pointer patch, FloatImageType::Pointer &image, FloatImageType::IndexType position);
    void CopyPatchIntoImage(VectorImageType::Pointer patch, VectorImageType::Pointer &image, VectorImageType::IndexType position);

    FloatImageType::IndexType FindHighestPriority(FloatImageType::Pointer priorityImage);

    UnsignedCharImageType::IndexType FindBestPatchMatch(UnsignedCharImageType::IndexType pixel);

    void UpdateMask(UnsignedCharImageType::IndexType pixel);

    UnsignedCharImageType::RegionType GetRegionAroundPixel(UnsignedCharImageType::IndexType pixel);


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
  // Initialize confidence image. The confidence is 0 in the hole to be filled, and 1 elsewhere.
  // This is the inverse of the mask.

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

  // Convert the inverted mask to floats to serve as the initial confidence image
  typedef itk::CastImageFilter< UnsignedCharImageType, FloatImageType > CastFilterType;
  CastFilterType::Pointer castFilter = CastFilterType::New();
  castFilter->SetInput(invertIntensityFilter->GetOutput());
  castFilter->Update();

  this->ConfidenceImage = castFilter->GetOutput();

  // Mask the input image with the inverted maskActor (blank the region in the input image that we will fill in)
  typedef itk::MaskImageFilter< UnsignedCharImageType, UnsignedCharImageType > MaskFilterType;
  MaskFilterType::Pointer maskFilter = MaskFilterType::New();
  maskFilter->SetInput1(this->Image);
  maskFilter->SetInput2(invertIntensityFilter->GetOutput());
  maskFilter->InPlaceOn();
  maskFilter->Update();;

  this->Image = maskFilter->GetOutput();

  ComputeIsophotes();

  int iteration = 0;
  while(HasMoreToInpaint(this->Mask))
    {
    std::cout << "Iteration: " << iteration << std::endl;

    {
    FloatImageWriterType::Pointer writer = FloatImageWriterType::New();
    std::stringstream padded;
    padded << "Confidence_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
    writer->SetFileName(padded.str());
    writer->SetInput(this->ConfidenceImage);
    writer->Update();
    }
    FindBoundary();

    {
    UnsignedCharImageWriterType::Pointer writer = UnsignedCharImageWriterType::New();
    std::stringstream padded;
    padded << "Boundary_" << std::setfill('0') << std::setw(4) << iteration << ".jpg";
    writer->SetFileName(padded.str());
    writer->SetInput(this->BoundaryImage);
    writer->Update();
    }

    ComputeBoundaryNormals();

    {
    VectorImageWriterType::Pointer writer = VectorImageWriterType::New();
    std::stringstream padded;
    padded << "BoundaryNormals_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
    writer->SetFileName(padded.str());
    writer->SetInput(this->BoundaryNormals);
    writer->Update();
    }

    FloatImageType::Pointer priorityImage = FloatImageType::New();
    priorityImage->SetRegions(this->Image->GetLargestPossibleRegion());
    priorityImage->Allocate();

    ComputeAllPriorities(this->IsophoteImage, this->BoundaryImage, this->BoundaryNormals, priorityImage);
    {
    FloatImageWriterType::Pointer writer = FloatImageWriterType::New();
    std::stringstream padded;
    padded << "Priorities_" << std::setfill('0') << std::setw(4) << iteration << ".mhd";
    writer->SetFileName(padded.str());
    writer->SetInput(priorityImage);
    writer->Update();
    }


    FloatImageType::IndexType pixelToFill = FindHighestPriority(priorityImage);
    std::cout << "Filling: " << pixelToFill << std::endl;

    UnsignedCharImageType::IndexType bestMatchPixel = FindBestPatchMatch(pixelToFill);
    std::cout << "Best match found at: " << bestMatchPixel << std::endl;

    // Extract the best patch
    typedef itk::RegionOfInterestImageFilter< UnsignedCharImageType,
                                                UnsignedCharImageType > ExtractFilterType;

    ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();

    UnsignedCharImageType::IndexType start;
    start[0] = bestMatchPixel[0] - this->PatchRadius[0];
    start[1] = bestMatchPixel[1] - this->PatchRadius[1];

    UnsignedCharImageType::RegionType desiredRegion;
    desiredRegion.SetSize(this->GetPatchSize());
    desiredRegion.SetIndex(start);

    extractFilter->SetRegionOfInterest(desiredRegion);
    extractFilter->SetInput(this->Image);
    extractFilter->Update();

    CopyPatchIntoImage(extractFilter->GetOutput(), this->Image, pixelToFill);

    {
    UnsignedCharImageWriterType::Pointer writer = UnsignedCharImageWriterType::New();
    std::stringstream padded;
    padded << "FilledImage_" << std::setfill('0') << std::setw(4) << iteration << ".jpg";
    writer->SetFileName(padded.str());
    writer->SetInput(this->Image);
    writer->Update();
    }

    UpdateConfidenceImage(bestMatchPixel, pixelToFill);
    UpdateIsophoteImage(bestMatchPixel, pixelToFill);

    // Update the mask
    UnsignedCharImageType::Pointer blankPatch = UnsignedCharImageType::New();
    CreateBlankUnsignedCharPatch(blankPatch);
    CopyPatchIntoImage(blankPatch, this->Mask, pixelToFill);

    {
    UnsignedCharImageWriterType::Pointer writer = UnsignedCharImageWriterType::New();
    std::stringstream padded;
    padded << "Mask_" << std::setfill('0') << std::setw(4) << iteration << ".jpg";
    writer->SetFileName(padded.str());
    writer->SetInput(this->Mask);
    writer->Update();
    }
    iteration++;
    } // end main while loop
}

void CriminisiInpainting::ComputeIsophotes()
{
  typedef itk::GradientImageFilter<
      itk::Image< unsigned char, 2 >, float, float>  GradientFilterType;
  GradientFilterType::Pointer gradientFilter = GradientFilterType::New();
  gradientFilter->SetInput(this->Image);
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

  UnsignedCharImageType::RegionType region(start, this->GetPatchSize());

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

  FloatImageType::RegionType region(start, this->GetPatchSize());

  patch->SetRegions(region);
  patch->Allocate();

  itk::ImageRegionIterator<FloatImageType> imageIterator(patch, patch->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
  {
    imageIterator.Set(0);

    ++imageIterator;
  }
}


void CriminisiInpainting::CreateConstantFloatPatch(FloatImageType::Pointer patch, float value)
{
  FloatImageType::IndexType start;
  start[0] = 0;
  start[1] = 0;

  FloatImageType::RegionType region(start, this->GetPatchSize());

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

  // Paste it into the mask
  typedef itk::PasteImageFilter <UnsignedCharImageType, UnsignedCharImageType >
          PasteImageFilterType;

  PasteImageFilterType::Pointer pasteFilter
          = PasteImageFilterType::New ();
  pasteFilter->SetInput(0, this->Mask);
  pasteFilter->SetInput(1, blackPatch);
  pasteFilter->SetSourceRegion(blackPatch->GetLargestPossibleRegion());
  pasteFilter->SetDestinationIndex(pixel);
  pasteFilter->Update();

  this->Mask = pasteFilter->GetOutput();
}

void CriminisiInpainting::ComputeBoundaryNormals()
{
  typedef itk::GradientImageFilter<
      itk::Image< unsigned char, 2 >, float, float>  GradientFilterType;
  GradientFilterType::Pointer gradientFilter = GradientFilterType::New();
  gradientFilter->SetInput(this->Mask);
  gradientFilter->Update();

  typedef itk::MaskImageFilter< VectorImageType, UnsignedCharImageType, VectorImageType > MaskFilterType;
  MaskFilterType::Pointer maskFilter = MaskFilterType::New();
  maskFilter->SetInput1(gradientFilter->GetOutput());
  maskFilter->SetInput2(this->Mask);
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

UnsignedCharImageType::IndexType CriminisiInpainting::FindBestPatchMatch(UnsignedCharImageType::IndexType queryPixel)
{
  this->MeanDifferenceImage->SetRegions(this->Image->GetLargestPossibleRegion());
  this->MeanDifferenceImage->Allocate();

  // Setup a neighborhood iterator over the whole image
  itk::ConstNeighborhoodIterator<UnsignedCharImageType, BoundaryConditionType> imageIterator(this->PatchRadius, this->Image, this->Image->GetLargestPossibleRegion());
  imageIterator.GoToBegin();

  itk::ImageRegionIterator<FloatImageType> meanDifferenceIterator(this->MeanDifferenceImage, this->MeanDifferenceImage->GetLargestPossibleRegion());
  meanDifferenceIterator.GoToBegin();

  // Setup desired region
  UnsignedCharImageType::IndexType start;
  start[0] = queryPixel[0] - this->PatchRadius[0];
  start[1] = queryPixel[1] - this->PatchRadius[1];

  UnsignedCharImageType::SizeType size = this->GetPatchSize();

  UnsignedCharImageType::RegionType desiredRegion;
  desiredRegion.SetSize(size);
  desiredRegion.SetIndex(start);

  // Setup an iterator over the fixed image patch
  itk::ImageRegionConstIterator<UnsignedCharImageType> imagePatchIterator(this->Image, desiredRegion);
  imagePatchIterator.GoToBegin();

  // Setup an iterator over the fixed mask patch
  itk::ImageRegionConstIterator<UnsignedCharImageType> maskPatchIterator(this->Mask, desiredRegion);
  maskPatchIterator.GoToBegin();


  unsigned int numberOfPixelsInNeighborhood = this->GetPatchSize()[0] * this->GetPatchSize()[1];
  while(!imageIterator.IsAtEnd())
    {
    double sumDifference = 0;

    for(unsigned int i = 0; i < numberOfPixelsInNeighborhood; i++)
      {

      unsigned char maskPixel = maskPatchIterator.Get();
      if(maskPixel == 0) // We are in the source region
        {
        unsigned char imagePixel = imagePatchIterator.Get();
        double difference = imagePixel - imageIterator.GetPixel(i);
        sumDifference += difference*difference;
        }
      else // pixels from the target region should not be compared
        {
        }
      ++imagePatchIterator;
      ++maskPatchIterator;
      }

    meanDifferenceIterator.Set(sumDifference);
    ++meanDifferenceIterator;
    imagePatchIterator.GoToBegin();
    maskPatchIterator.GoToBegin();
    ++imageIterator;
    }

    {
    FloatImageWriterType::Pointer writer = FloatImageWriterType::New();
    writer->SetFileName("FullCorrelation.mhd");
    writer->SetInput(this->MeanDifferenceImage);
    writer->Update();
    }

  // Compute the maximum value of the difference image. We will use this to fill the constant blank patch
  typedef itk::MinimumMaximumImageCalculator <FloatImageType>
          ImageCalculatorFilterType;

  ImageCalculatorFilterType::Pointer imageCalculatorFilter
          = ImageCalculatorFilterType::New ();
  imageCalculatorFilter->SetImage(this->MeanDifferenceImage);
  imageCalculatorFilter->Compute();

  float highestValue = imageCalculatorFilter->GetMaximum();

  // Paste a blank patch into the correlation image so that we don't match a region within the current patch
  FloatImageType::Pointer blankPatch = FloatImageType::New();
  CreateConstantFloatPatch(blankPatch, highestValue);
  CopyPatchIntoImage(blankPatch, this->MeanDifferenceImage, queryPixel);

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
  position[0] -= this->PatchRadius[0];
  position[1] -= this->PatchRadius[1];

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
  position[0] -= this->PatchRadius[0];
  position[1] -= this->PatchRadius[1];

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
  position[0] -= this->PatchRadius[0];
  position[1] -= this->PatchRadius[1];

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

void CriminisiInpainting::ComputeAllPriorities(VectorImageType::Pointer isophotes, UnsignedCharImageType::Pointer boundary,
                                               VectorImageType::Pointer boundaryNormals, FloatImageType::Pointer priorityImage)
{
  // Only compute priorities for pixels on the boundary
  itk::ImageRegionConstIterator<UnsignedCharImageType> boundaryIterator(boundary, boundary->GetLargestPossibleRegion());
  itk::ConstNeighborhoodIterator<UnsignedCharImageType, BoundaryConditionType> imageIterator(this->PatchRadius, this->Image, this->Image->GetLargestPossibleRegion());
  itk::ConstNeighborhoodIterator<UnsignedCharImageType, BoundaryConditionType> maskIterator(this->PatchRadius, this->Mask, this->Mask->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator<VectorImageType> isophoteIterator(isophotes, isophotes->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator<VectorImageType> boundaryNormalIterator(boundaryNormals, boundaryNormals->GetLargestPossibleRegion());
  itk::ConstNeighborhoodIterator<FloatImageType> confidenceIterator(this->PatchRadius, this->ConfidenceImage, this->ConfidenceImage->GetLargestPossibleRegion());

  // This is the only non-const iterator
  itk::ImageRegionIterator<FloatImageType> priorityIterator(priorityImage, priorityImage->GetLargestPossibleRegion());

  boundaryIterator.GoToBegin();
  imageIterator.GoToBegin();
  maskIterator.GoToBegin();
  priorityIterator.GoToBegin();
  isophoteIterator.GoToBegin();
  boundaryNormalIterator.GoToBegin();
  confidenceIterator.GoToBegin();

  // The main loop is over the boundary image. We only want to compute priorities at boundary pixels
  while(!imageIterator.IsAtEnd())
    {
    // If the pixel is not on the boundary, skip it and set its priority to zero.
    if(boundaryIterator.Get() == 0)
      {
      priorityIterator.Set(0);
      ++imageIterator;
      ++boundaryIterator;
      ++priorityIterator;
      ++maskIterator;
      ++isophoteIterator;
      ++boundaryNormalIterator;
      ++confidenceIterator;
      continue;
      }

    imageIterator.GetNeighborhood();
    maskIterator.GetNeighborhood();
    confidenceIterator.GetNeighborhood();
    isophoteIterator.Get();
    boundaryNormalIterator.Get();

    float priority = ComputePriority(imageIterator.GetNeighborhood(), maskIterator.GetNeighborhood(), confidenceIterator.GetNeighborhood(),
                                                         isophoteIterator.Get(), boundaryNormalIterator.Get());
    //std::cout << "Priority: " << priority << std::endl;
    priorityIterator.Set(priority);

    ++imageIterator;
    ++boundaryIterator;
    ++priorityIterator;
    ++maskIterator;
    ++isophoteIterator;
    ++boundaryNormalIterator;
    ++confidenceIterator;
    }
}

float CriminisiInpainting::ComputePriority(UnsignedCharNeighborhoodType imagePatch, UnsignedCharNeighborhoodType maskPatch,
                                           FloatNeighborhoodType confidencePatch,
                                           itk::CovariantVector<float, 2> isophote, itk::CovariantVector<float, 2> boundaryNormal)
{
  double confidence = ComputeConfidenceTerm(imagePatch, maskPatch, confidencePatch);
  double data = ComputeDataTerm(isophote, boundaryNormal);

  return confidence + data;
}

float CriminisiInpainting::ComputeConfidenceTerm(UnsignedCharNeighborhoodType imagePatch, UnsignedCharNeighborhoodType maskPatch, FloatNeighborhoodType confidencePatch)
{
  // sum of the confidences of patch pixels in the source region / area of the patch

  unsigned int numberOfPixels = this->GetPatchSize()[0]*this->GetPatchSize()[1];

  double sum = 0;

  for(unsigned int i = 0; i < numberOfPixels; i++)
    {
    if(maskPatch[i] == 0) // Pixel is in the source region
      {
      sum += confidencePatch[i];
      }
    }

  double areaOfPatch = static_cast<double>(numberOfPixels);
  return sum/areaOfPatch;
}

float CriminisiInpainting::ComputeDataTerm(itk::CovariantVector<float, 2> isophote, itk::CovariantVector<float, 2> boundaryNormal)
{
  double alpha = 255; // for grey scale images
  // D(p) = |dot(isophote direction at p, normal of the front at p)|/alpha

  vnl_double_2 vnlIsophote(isophote[0], isophote[1]);

  vnl_double_2 vnlNormal(boundaryNormal[0], boundaryNormal[1]);

  double dot = std::abs(dot_product(vnlIsophote,vnlNormal));

  return dot/alpha;
}

UnsignedCharImageType::RegionType CriminisiInpainting::GetRegionAroundPixel(UnsignedCharImageType::IndexType pixel)
{
  pixel[0] -= this->GetPatchSize()[0]/2;
  pixel[1] -= this->GetPatchSize()[1]/2;

  UnsignedCharImageType::RegionType region;
  region.SetIndex(pixel);
  region.SetSize(this->GetPatchSize());

  return region;
}

void CriminisiInpainting::UpdateConfidenceImage(FloatImageType::IndexType sourcePixel, FloatImageType::IndexType targetPixel)
{
  // Copy confidences from best patch
  typedef itk::RegionOfInterestImageFilter< FloatImageType,
                                            FloatImageType > ExtractFilterType;

  ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();

  sourcePixel[0] -= this->PatchRadius[0];
  sourcePixel[1] -= this->PatchRadius[1];

  FloatImageType::RegionType desiredRegion;
  desiredRegion.SetSize(this->GetPatchSize());
  desiredRegion.SetIndex(sourcePixel);

  extractFilter->SetRegionOfInterest(desiredRegion);
  extractFilter->SetInput(this->ConfidenceImage);
  extractFilter->Update();

  CopyPatchIntoImage(extractFilter->GetOutput(), this->ConfidenceImage, targetPixel);

  // recompute confidences (not in Criminisi paper)
  /*
  // loop over all pixels in the target region which were just filled and compute their confidence and store it in this->ConfidenceImage
  itk::ImageRegionIterator<FloatImageType, BoundaryConditionType> confidenceIterator(this->ConfidenceImage, region);
  itk::ImageRegionIterator<UnsignedCharImageType, BoundaryConditionType> maskIterator(this->ConfidenceImage, region);
  confidenceIterator.GoToBegin();
  maskIterator.GoToBegin();

  while(!maskIterator.IsAtEnd())
    {
    if(!maskIterator.Get()) // this was a target pixel, compute its confidence

    confidenceIterator.Set(C);

    ++imageIterator;
  }
  */
}


void CriminisiInpainting::UpdateIsophoteImage(FloatImageType::IndexType sourcePixel, FloatImageType::IndexType targetPixel)
{
  // Copy confidences from best patch
  typedef itk::RegionOfInterestImageFilter< VectorImageType,
                                            VectorImageType > ExtractFilterType;

  ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();

  VectorImageType::IndexType start;
  start[0] = sourcePixel[0] - this->PatchRadius[0];
  start[1] = sourcePixel[1] - this->PatchRadius[1];

  VectorImageType::RegionType desiredRegion;
  desiredRegion.SetSize(this->GetPatchSize());
  desiredRegion.SetIndex(start);

  extractFilter->SetRegionOfInterest(desiredRegion);
  extractFilter->SetInput(this->IsophoteImage);
  extractFilter->Update();

  CopyPatchIntoImage(extractFilter->GetOutput(), this->IsophoteImage, targetPixel);
}