#ifndef PriorityViewerWidget_HPP
#define PriorityViewerWidget_HPP

#include "PriorityViewerWidget.h" // Appease syntax parser

// ITK
#include "itkCastImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMaskImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

// VTK
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkImageProperty.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>

// Submodules
#include <Helpers/Helpers.h>
#include <ITKHelpers/ITKHelpers.h>
#include <ITKVTKHelpers/ITKVTKHelpers.h>
#include <VTKHelpers/VTKHelpers.h>
#include <QtHelpers/QtHelpers.h>

// Custom
#include "InteractorStyleImageWithDrag.h"

template <typename TPriority, typename TBoundaryStatusMapType>
PriorityViewerWidget<TPriority, TBoundaryStatusMapType>::PriorityViewerWidget(TPriority* const priorityFunction,
                                                                              const itk::Size<2>& imageSize,
                                                                              TBoundaryStatusMapType* boundaryStatusMap) :
  PriorityFunction(priorityFunction), ImageSize(imageSize), PreviouslyDisplayed(false),
  BoundaryStatusMap(boundaryStatusMap)
{
  this->setupUi(this);

  this->PriorityImage = PriorityImageType::New();
  itk::ImageRegion<2> fullRegion(ITKHelpers::CornerRegion(this->ImageSize));
  this->PriorityImage->SetRegions(fullRegion);
  this->PriorityImage->Allocate();

  this->InteractorStyle = vtkSmartPointer<InteractorStyleImageWithDrag>::New();

  this->Renderer = vtkSmartPointer<vtkRenderer>::New();
  this->qvtkWidget->GetRenderWindow()->AddRenderer(this->Renderer);

  this->Renderer->AddViewProp(this->ImageLayer.ImageSlice);

  // Without this here (and then setting it to true later after it is populated),
  // you will get "This data object does not contain the requested extent
  this->ImageLayer.ImageSlice->VisibilityOff();

  this->InteractorStyle->SetCurrentRenderer(this->Renderer);
  this->qvtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(this->InteractorStyle);
  this->InteractorStyle->Init();

  // Setup control of the viewing orientation
  this->ItkVtkCamera = new ITKVTKCamera(this->InteractorStyle->GetImageStyle(), this->Renderer,
                                        this->qvtkWidget->GetRenderWindow());
  this->ItkVtkCamera->SetCameraPositionPNG();
}

template <typename TPriority, typename TBoundaryStatusMapType>
void PriorityViewerWidget<TPriority, TBoundaryStatusMapType>::slot_UpdateImage()
{
  // std::cout << "PriorityViewerWidget::slot_UpdateImage." << std::endl;

  ITKHelpers::SetImageToConstant(this->PriorityImage.GetPointer(), 0);
  // Compute the priority at every boundary pixel
  itk::ImageRegionIterator<PriorityImageType> imageIterator(this->PriorityImage,
                                                            this->PriorityImage->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
  {
    typename TBoundaryStatusMapType::key_type node = Helpers::ConvertFrom<typename TBoundaryStatusMapType::key_type, itk::Index<2> >(imageIterator.GetIndex());
    if(get(*(this->BoundaryStatusMap), node))
    {
      imageIterator.Set(this->PriorityFunction->ComputePriority(imageIterator.GetIndex()));
    }

    ++imageIterator;
  }

  // Rescale the priorities to be displayed as a grayscale image
  ITKVTKHelpers::ITKScalarImageToScaledVTKImage(this->PriorityImage.GetPointer(), this->ImageLayer.ImageData);
  this->ImageLayer.ImageSlice->VisibilityOn();

  if(!this->PreviouslyDisplayed)
  {
    this->Renderer->ResetCamera();
    this->PreviouslyDisplayed = true;
  }

  this->qvtkWidget->GetRenderWindow()->Render();
}

#endif
