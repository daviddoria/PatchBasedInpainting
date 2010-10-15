#include "vtkImageIsophotes.h"
#include "vtkImageVectorRotate.h"

#include "vtkImageData.h"
#include "vtkImageGradient.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

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
  gradientFilter->SetInputConnection(input->GetProducerPort());
  gradientFilter->SetDimensionality(3);
  gradientFilter->Update();

  vtkSmartPointer<vtkImageData> gradient =
    vtkSmartPointer<vtkImageData>::New();
  gradient->ShallowCopy(gradientFilter->GetOutput());

  // Normalize the gradient
  int extent[6];
  gradient->GetExtent(extent);
  for(vtkIdType i = extent[0]; i <= extent[1]; i++)
    {
    for(vtkIdType j = extent[2]; j <= extent[3]; j++)
      {
      for(vtkIdType k = extent[4]; k <= extent[5]; k++)
        {
        double* pixel = static_cast<double*>(gradient->GetScalarPointer(i,j,k));
//        std::cout << "pixel: " << pixel[0] << " " << pixel[1] << " " << pixel[2] << std::endl;
        vtkMath::Normalize2D(pixel);
  //      std::cout << "pixel after: " << pixel[0] << " " << pixel[1] << " " << pixel[2] << std::endl;
        }
      }
    }

  vtkSmartPointer<vtkImageVectorRotate> vectorRotate =
    vtkSmartPointer<vtkImageVectorRotate>::New();
  vectorRotate->SetZRotation(90);
  vectorRotate->SetInputConnection(gradient->GetProducerPort());
  vectorRotate->Update();

  output->ShallowCopy(vectorRotate->GetOutput());
  
  // Without these lines, the output will appear real but will not work as the input to any other filters
  output->SetExtent(input->GetExtent());
  output->SetUpdateExtent(output->GetExtent());
  output->SetWholeExtent(output->GetExtent());

  return 1;
}