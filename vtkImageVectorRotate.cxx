#include "vtkImageVectorRotate.h"

#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkTransform.h"

vtkStandardNewMacro(vtkImageVectorRotate);

vtkImageVectorRotate::vtkImageVectorRotate()
{
  this->XRotation = 0;
  this->YRotation = 0;
  this->ZRotation = 0;
}

int vtkImageVectorRotate::RequestData(vtkInformation *vtkNotUsed(request),
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

  output->DeepCopy(input);

  vtkSmartPointer<vtkTransform> transform =
    vtkSmartPointer<vtkTransform>::New();
  transform->RotateX(XRotation);
  transform->RotateY(YRotation);
  transform->RotateZ(ZRotation);

  int extent[6];
  input->GetExtent(extent);
  
  for(vtkIdType i = extent[0]; i <= extent[1]; i++)
    {
    for(vtkIdType j = extent[2]; j <= extent[3]; j++)
      {
      for(vtkIdType k = extent[4]; k <= extent[5]; k++)
        {
        double* pixel = static_cast<double*>(output->GetScalarPointer(i,j,k));
        transform->TransformPoint(pixel, pixel);
        }
      }
    }

  // Without these lines, the output will appear correct but will not work as the input to any other filters
  output->SetExtent(input->GetExtent());
  output->SetUpdateExtent(output->GetExtent());
  output->SetWholeExtent(output->GetExtent());

  return 1;
}