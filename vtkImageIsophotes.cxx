#include "vtkImageIsophotes.h"

#include "vtkObjectFactory.h"
#include "vtkImageConvolve.h"
#include "vtkImageData.h"
#include "vtkCommand.h"

// For testing only
#include "vtkCommand.h"
#include "vtkJPEGWriter.h"
#include <sstream>

vtkStandardNewMacro(vtkImageIsophotes);

int vtkImageIsophotes::RequestData(vtkInformation *vtkNotUsed(request),
                                             vtkInformationVector **inputVector,
                                             vtkInformationVector *outputVector)
{
  // Get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // Get the input and ouptut
  vtkImageData *input = vtkImageData::SafeDownCast(
      inInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkImageData *output = vtkImageData::SafeDownCast(
      outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkSmartPointer<vtkImageGradient> gradientFilter =
    vtkSmartPointer<vtkImageGradient>::New();
  gradientFilter->SetInputConnection(this->Image->GetProducerPort());
  gradientFilter->Update();

  // Normalize the gradient
  vtkSmartPointer<vtkImageData> normalizedGradient =
    vtkSmartPointer<vtkImageData>::New();
  normalizedGradient->ShallowCopy(gradientFilter->GetOutput());
  int extents[6];
  normalizedGradient->GetExtent(extents);
  for(vtkIdType i = extents[0]; i < extents[1]; i++)
    {
    for(vtkIdType j = extents[2]; j < extents[3]; j++)
      {
      double* pixel = static_cast<double*>(normalizedGradient->GetScalarPointer(i,j,0));
      vtkMath::Normalize2D(pixel);
      }
    }
    
  vtkSmartPointer<vtkImageData> image =
    vtkSmartPointer<vtkImageData>::New();
  image->ShallowCopy(input);

  image->SetScalarComponentFromDouble(0,0,0,0, 5.0);

  output->ShallowCopy(image);

  // Without these lines, the output will appear real but will not work as the input to any other filters
  output->SetExtent(input->GetExtent());
  output->SetUpdateExtent(output->GetExtent());
  output->SetWholeExtent(output->GetExtent());

  return 1;
}