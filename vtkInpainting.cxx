#include "vtkInpainting.h"

#include "vtkImageData.h"
#include "vtkImageThreshold.h"
#include "vtkImageMagnitude.h"
#include "vtkCommand.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkDataObject.h"
#include "vtkSmartPointer.h"
#include "vtkImageGradient.h"
#include "vtkImageLaplacian.h"
#include "vtkImageLuminance.h"

// Testing only
#include "vtkJPEGWriter.h"

vtkInpainting::vtkInpainting()
{
  this->SetNumberOfInputPorts(2);

  this->MaximumIterations = 10;

  this->Output = vtkSmartPointer<vtkImageData>::New();
  this->Image = vtkSmartPointer<vtkImageData>::New();
  this->Mask = vtkSmartPointer<vtkImageData>::New();
}

vtkImageData* vtkInpainting::GetIntermediateOutput()
{
  return this->Output;
}

int vtkInpainting::RequestData(vtkInformation *vtkNotUsed(request),
                                             vtkInformationVector **inputVector,
                                             vtkInformationVector *outputVector)
{
  // Get the info objects
  vtkInformation *inInfo0 = inputVector[0]->GetInformationObject(0);
  vtkInformation *inInfo1 = inputVector[1]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // Get the inputs
  vtkImageData *image = vtkImageData::SafeDownCast(
      inInfo0->Get(vtkDataObject::DATA_OBJECT()));

  vtkSmartPointer<vtkImageLuminance> imageLuminanceFilter =
    vtkSmartPointer<vtkImageLuminance>::New();
  imageLuminanceFilter->SetInputConnection(image->GetProducerPort());
  imageLuminanceFilter->Update();
  this->Image->ShallowCopy(imageLuminanceFilter->GetOutput());

  {
  vtkSmartPointer<vtkJPEGWriter> imageLuminanceWriter =
    vtkSmartPointer<vtkJPEGWriter>::New();
  imageLuminanceWriter->SetFileName("ImageMagnitude.jpg");
  imageLuminanceWriter->SetInputConnection(imageLuminanceFilter->GetOutputPort());
  imageLuminanceWriter->Write();
  }
  
  std::cout << "Image has " << this->Image->GetNumberOfScalarComponents() << " components." << std::endl;

  vtkImageData* mask = vtkImageData::SafeDownCast(
      inInfo1->Get(vtkDataObject::DATA_OBJECT()));

  vtkSmartPointer<vtkImageMagnitude> maskMagnitudeFilter =
    vtkSmartPointer<vtkImageMagnitude>::New();
  maskMagnitudeFilter->SetInputConnection(mask->GetProducerPort());
  maskMagnitudeFilter->Update();

  {
  vtkSmartPointer<vtkJPEGWriter> maskMagnitudeWriter =
    vtkSmartPointer<vtkJPEGWriter>::New();
  maskMagnitudeWriter->SetFileName("MaskMagnitude.jpg");
  maskMagnitudeWriter->SetInputConnection(maskMagnitudeFilter->GetOutputPort());
  maskMagnitudeWriter->Write();
  }
  
  vtkSmartPointer<vtkImageThreshold> maskThresholdFilter =
    vtkSmartPointer<vtkImageThreshold>::New();
  maskThresholdFilter->SetInputConnection(maskMagnitudeFilter->GetOutputPort());
  maskThresholdFilter->SetOutValue(0);
  maskThresholdFilter->SetInValue(255);
  maskThresholdFilter->ReplaceInOn();
  maskThresholdFilter->ReplaceOutOn();
  maskThresholdFilter->ThresholdByUpper(30); // Anything above 30 will be white (seems like a hack...)
  maskThresholdFilter->Update();
  this->Mask->ShallowCopy(maskThresholdFilter->GetOutput());
  std::cout << "Mask has " << this->Mask->GetNumberOfScalarComponents() << " components." << std::endl;
  std::cout << "Mask is type " << this->Mask->GetScalarTypeAsString() << std::endl;

  {
  vtkSmartPointer<vtkJPEGWriter> maskWriter =
    vtkSmartPointer<vtkJPEGWriter>::New();
  maskWriter->SetFileName("InitialMask.jpg");
  maskWriter->SetInputConnection(this->Mask->GetProducerPort());
  maskWriter->Write();
  }
  
  // Get the input and ouptut
  vtkImageData *output = vtkImageData::SafeDownCast(
      outInfo->Get(vtkDataObject::DATA_OBJECT()));

  this->Output->ShallowCopy(imageLuminanceFilter->GetOutput());
  ClearImageInRegion(this->Output, this->Mask);

  {
  vtkSmartPointer<vtkJPEGWriter> imageWriter =
    vtkSmartPointer<vtkJPEGWriter>::New();
  imageWriter->SetFileName("InitialImage.jpg");
  imageWriter->SetInputConnection(this->Output->GetProducerPort());
  imageWriter->Write();
  }
  
  unsigned int iteration = 0;
  while(!IsDone() && (iteration < this->MaximumIterations))
    {
    Iterate(iteration);
    iteration++;
    std::cout << "Finished " << iteration << " iterations." << std::endl;
    this->InvokeEvent(vtkCommand::WarningEvent, NULL);
    }

  output->ShallowCopy(this->Output);
  output->SetExtent(image->GetExtent());
  output->SetUpdateExtent(output->GetExtent());
  output->SetWholeExtent(output->GetExtent());
  return 1;
}

void vtkInpainting::FindMaskBoundaryPixels(vtkImageData* boundaryImage)
{
  // Find all white pixels that have a black neighbor and store them in boundaryPixels.
  // This function currently assumes we are no where near the border!

  int extent[6];
  this->Mask->GetExtent(extent);

  boundaryImage->SetExtent(extent);
  boundaryImage->SetScalarTypeToUnsignedChar();
  boundaryImage->SetNumberOfScalarComponents(1);

  for(int i = extent[0]; i < extent[1]; i++)
    {
    for(int j = extent[2]; j < extent[3]; j++)
      {
      unsigned char* maskPixel = static_cast<unsigned char*>(this->Mask->GetScalarPointer(i,j,0));
      unsigned char* outputPixel = static_cast<unsigned char*>(boundaryImage->GetScalarPointer(i,j,0));
      // Look for white mask pixels with a black neighbor. Mark them as white in the boundaryImage.
      // Mark the rest of the boundary image as black.
      if(maskPixel[0] == 255) // pixel is white
        {
        unsigned char* top = static_cast<unsigned char*>(this->Mask->GetScalarPointer(i,j-1,0));
        if(top[0] == 0)
          {
          outputPixel[0] = 255;
          continue;
          }
        unsigned char* bottom = static_cast<unsigned char*>(this->Mask->GetScalarPointer(i,j+1,0));
        if(bottom[0] == 0)
          {
          outputPixel[0] = 255;
          continue;
          }
        unsigned char* left = static_cast<unsigned char*>(this->Mask->GetScalarPointer(i-1,j,0));
        if(left[0] == 0)
          {
          outputPixel[0] = 255;
          continue;
          }
        unsigned char* right = static_cast<unsigned char*>(this->Mask->GetScalarPointer(i+1,j,0));
        if(right[0] == 0)
          {
          outputPixel[0] = 255;
          continue;
          }

        // If we get to here, the pixels is white, but has no black neighbors, so it is not a boundary pixel.
        outputPixel[0] = 0;
        }
      else // If the pixel is not white, it has no chance of being a boundary pixel.
        {
        outputPixel[0] = 0;
        }
      }
    }
}

bool vtkInpainting::IsDone()
{
  // If there are any white pixels left, we are not done.
  int extent[6];
  this->Mask->GetExtent(extent);

  for(int i = extent[0]; i < extent[1]; i++)
    {
    for(int j = extent[2]; j < extent[3]; j++)
      {
      unsigned char* pixel = static_cast<unsigned char*>(this->Mask->GetScalarPointer(i,j,0));
      if(pixel[0] == 255)
        {
        return false;
        }
      }
    }

  return true;
}


void vtkInpainting::ClearImageInRegion(vtkImageData* image, vtkImageData* region)
{
  int extent[6];
  image->GetExtent(extent);

  for(int i = extent[0]; i < extent[1]; i++)
    {
    for(int j = extent[2]; j < extent[3]; j++)
      {
      unsigned char* regionPixel = static_cast<unsigned char*>(region->GetScalarPointer(i,j,0));
      if(regionPixel[0] == 255)
        {
        unsigned char* imagePixel = static_cast<unsigned char*>(image->GetScalarPointer(i,j,0));
        imagePixel[0] = 255; // Here, "clear" is white
        }
      }
    }
}

void vtkInpainting::CopyImageInRegion(vtkImageData* image, vtkImageData* region, vtkImageData* input)
{
  int extent[6];
  image->GetExtent(extent);

  for(int i = extent[0]; i < extent[1]; i++)
    {
    for(int j = extent[2]; j < extent[3]; j++)
      {
      unsigned char* regionPixel = static_cast<unsigned char*>(region->GetScalarPointer(i,j,0));
      if(regionPixel[0] == 255)
        {
        unsigned char* imagePixel = static_cast<unsigned char*>(image->GetScalarPointer(i,j,0));
        unsigned char* inputPixel = static_cast<unsigned char*>(input->GetScalarPointer(i,j,0));
        imagePixel[0] = inputPixel[0];
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkInpainting::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
