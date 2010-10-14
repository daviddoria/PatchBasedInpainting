#include "vtkFastDigitalInpainting.h"

#include "vtkObjectFactory.h"
#include "vtkImageConvolve.h"
#include "vtkImageData.h"
#include "vtkCommand.h"

// For testing only
#include "vtkCommand.h"
#include "vtkJPEGWriter.h"
#include <sstream>

vtkStandardNewMacro(vtkFastDigitalInpainting);

void vtkFastDigitalInpainting::Iterate(int iteration)
{
  double c = 1./8.;
  double kernel[9] = {c,c,c,c,0,c,c,c,c};
  vtkSmartPointer<vtkImageConvolve> convolveFilter =
    vtkSmartPointer<vtkImageConvolve>::New();
  convolveFilter->SetInputConnection(this->Output->GetProducerPort());
  convolveFilter->SetKernel3x3(kernel);
  convolveFilter->Update();

  {
  vtkSmartPointer<vtkJPEGWriter> convolvedWriter =
    vtkSmartPointer<vtkJPEGWriter>::New();
  std::stringstream ss;
  ss << "convovled_" << iteration << ".jpg";
  convolvedWriter->SetFileName(ss.str().c_str());
  convolvedWriter->SetInputConnection(convolveFilter->GetOutputPort());
  convolvedWriter->Write();
  }

  // Update the next step of the output
  CopyImageInRegion(this->Output, this->Mask, convolveFilter->GetOutput());

  {
  vtkSmartPointer<vtkJPEGWriter> outputWriter =
    vtkSmartPointer<vtkJPEGWriter>::New();
  std::stringstream ss;
  ss << "output_" << iteration << ".jpg";
  outputWriter->SetFileName(ss.str().c_str());
  outputWriter->SetInputConnection(this->Output->GetProducerPort());
  outputWriter->Write();
  }

}

/* explicit region boundary only
void vtkFastDigitalInpainting::Iterate(int iteration)
{
  {
  vtkSmartPointer<vtkJPEGWriter> maskWriter =
    vtkSmartPointer<vtkJPEGWriter>::New();
  std::stringstream ss;
  ss << "mask_" << iteration << ".jpg";
  maskWriter->SetFileName(ss.str().c_str());
  maskWriter->SetInputConnection(this->Mask->GetProducerPort());
  maskWriter->Write();
  }

  // Find which pixels will be updated
  vtkSmartPointer<vtkImageData> boundary =
    vtkSmartPointer<vtkImageData>::New();
  FindMaskBoundaryPixels(boundary);

  {
  vtkSmartPointer<vtkJPEGWriter> boundaryWriter =
    vtkSmartPointer<vtkJPEGWriter>::New();
  std::stringstream ss;
  ss << "boundary_" << iteration << ".jpg";
  boundaryWriter->SetFileName(ss.str().c_str());
  boundaryWriter->SetInputConnection(boundary->GetProducerPort());
  boundaryWriter->Write();
  }

  // Blank the next step of the output
  ClearImageInRegion(this->Output, boundary);

  double c = 1./8.;
  double kernel[9] = {c,c,c,c,0,c,c,c,c};
  vtkSmartPointer<vtkImageConvolve> convolveFilter =
    vtkSmartPointer<vtkImageConvolve>::New();
  convolveFilter->SetInputConnection(this->Output->GetProducerPort());
  convolveFilter->SetKernel3x3(kernel);
  convolveFilter->Update();

  {
  vtkSmartPointer<vtkJPEGWriter> convolvedWriter =
    vtkSmartPointer<vtkJPEGWriter>::New();
  std::stringstream ss;
  ss << "convovled_" << iteration << ".jpg";
  convolvedWriter->SetFileName(ss.str().c_str());
  convolvedWriter->SetInputConnection(convolveFilter->GetOutputPort());
  convolvedWriter->Write();
  }
  
  // Update the next step of the output
  CopyImageInRegion(this->Output, boundary, convolveFilter->GetOutput());

  {
  vtkSmartPointer<vtkJPEGWriter> outputWriter =
    vtkSmartPointer<vtkJPEGWriter>::New();
  std::stringstream ss;
  ss << "output_" << iteration << ".jpg";
  outputWriter->SetFileName(ss.str().c_str());
  outputWriter->SetInputConnection(this->Output->GetProducerPort());
  outputWriter->Write();
  }
  
  // Shrink the mask
  ClearImageInRegion(this->Mask, boundary);
}
*/