#include "MovablePatch.h"

// Qt
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>

// VTK
#include <vtkImageSlice.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

// Custom
#include "Helpers/Helpers.h"
#include "Helpers/VTKHelpers.h"
#include "Helpers/ITKVTKHelpers.h"
#include "HelpersQt.h"
#include "InteractorStyleImageWithDrag.h"

MovablePatch::MovablePatch(const unsigned int radius, vtkRenderer* const renderer,  QGraphicsView* const view, const QColor color) : Radius(radius), Renderer(renderer), View(view), Color(color), PatchScene(new QGraphicsScene)
{
  this->PatchLayer.ImageSlice->SetPickable(true);
  //this->PatchLayer.ImageSlice->SetVisibility(this->chkDisplayUserPatch->isChecked());

  ITKVTKHelpers::CreateTransparentVTKImage(ITKHelpers::SizeFromRadius(radius), this->PatchLayer.ImageData);
  unsigned char userPatchColor[3];
  HelpersQt::QColorToUCharColor(color, userPatchColor);
  VTKHelpers::BlankAndOutlineImage(this->PatchLayer.ImageData, userPatchColor);

  this->Renderer->AddViewProp(this->PatchLayer.ImageSlice);

  InteractorStyleImageWithDrag::SafeDownCast(this->Renderer->GetRenderWindow()->GetInteractor())->TrackballStyle->AddObserver(CustomTrackballStyle::PatchesMovedEvent, this, &MovablePatch::PatchMoved);

  //this->PatchScene->setBackgroundBrush(brush);
  this->View->setScene(this->PatchScene.data());
}

void MovablePatch::SetVisibility(const bool visibility)
{
  this->PatchLayer.ImageSlice->SetVisibility(visibility);
}

bool MovablePatch::GetVisibility()
{
  return this->PatchLayer.ImageSlice->GetVisibility();
}

itk::ImageRegion<2> MovablePatch::GetRegion()
{
  double position[3];
  this->PatchLayer.ImageSlice->GetPosition(position);

  itk::Index<2> patchCorner;
  patchCorner[0] = position[0];
  patchCorner[1] = position[1];

  itk::Size<2> patchSize = ITKHelpers::SizeFromRadius(this->Radius);

  itk::ImageRegion<2> region(patchCorner, patchSize);

  return region;
}

void MovablePatch::Display()
{
  QGraphicsPixmapItem* item = this->PatchScene->addPixmap(QPixmap::fromImage(this->PatchImage));
  this->View->fitInView(item);
}

void MovablePatch::PatchMoved()
{
  // Snap user patch to integer pixels
  double position[3];
  this->PatchLayer.ImageSlice->GetPosition(position);
  position[0] = round(position[0]);
  position[1] = round(position[1]);
  this->PatchLayer.ImageSlice->SetPosition(position);
  this->Renderer->GetRenderWindow()->Render();

/*
  std::shared_ptr<SelfPatchCompare> patchCompare(new SelfPatchCompare);
  patchCompare->SetImage(dynamic_cast<FloatVectorImageType*>(this->IterationRecords[iterationToCompare].GetImageByName("Image").Image.GetPointer()));
  patchCompare->SetMask(dynamic_cast<Mask*>(this->IterationRecords[iterationToCompare].GetImageByName("Mask").Image.GetPointer()));
  patchCompare->SetNumberOfComponentsPerPixel(this->UserImage->GetNumberOfComponentsPerPixel());
  patchCompare->FunctionsToCompute.push_back(boost::bind(&SelfPatchCompare::SetPatchAverageAbsoluteSourceDifference,patchCompare.get(),_1));
  CandidatePairs candidatePairs(this->IterationRecords[this->IterationToDisplay].PotentialPairSets[this->ForwardLookToDisplayId].TargetPatch);
  Patch userPatch(this->UserPatchRegion);
  candidatePairs.AddPairFromPatch(userPatch);
  patchCompare->SetPairs(&candidatePairs);
  patchCompare->ComputeAllSourceDifferences();

  std::stringstream ss;
  ss << candidatePairs[0].DifferenceMap[PatchPair::AverageAbsoluteDifference];
  lblUserPatchError->setText(ss.str().c_str());*/

}
