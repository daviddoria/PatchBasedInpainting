#include "PatchBasedInpaintingGUI.h"

#include "InteractorStyleImageNoLevel.h"

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

void PatchBasedInpaintingGUI::on_radCompareOriginal_clicked()
{
  this->Inpainting.SetCompareToOriginal();
}

void PatchBasedInpaintingGUI::on_radCompareBlurred_clicked()
{
  this->Inpainting.SetCompareToBlurred();
}

void PatchBasedInpaintingGUI::on_radCompareCIELAB_clicked()
{
  this->Inpainting.SetCompareToCIELAB();
}
  
void PatchBasedInpaintingGUI::on_chkLive_clicked()
{
  // When "Live" is checked, we do not want to be able to click 'next' and 'previous'
  this->btnDisplayNextStep->setEnabled(!this->chkLive->isChecked());
  this->btnDisplayPreviousStep->setEnabled(!this->chkLive->isChecked());
  
  this->btnGoToIteration->setEnabled(!this->chkLive->isChecked());
  this->txtGoToIteration->setEnabled(!this->chkLive->isChecked());
}

void PatchBasedInpaintingGUI::on_btnGoToIteration_clicked()
{
  unsigned int requestedIteration = this->txtGoToIteration->text().toUInt();
  if(requestedIteration < this->AllPotentialCandidatePairs.size() && requestedIteration >= 0)
    {
    this->IterationToDisplay = requestedIteration;
    ChangeDisplayedIteration();
    }
}

void PatchBasedInpaintingGUI::on_btnResort_clicked()
{
  for(unsigned int iteration = 0; iteration < this->AllPotentialCandidatePairs.size(); iteration++)
    {
    for(unsigned int forwardLookId = 0; forwardLookId < this->AllPotentialCandidatePairs[iteration].size(); forwardLookId++)
      {
      CandidatePairs& candidatePairs = this->AllPotentialCandidatePairs[iteration][forwardLookId];

      if(this->radSortByAverageAbsoluteDifference->isChecked())
        {
        std::sort(candidatePairs.begin(), candidatePairs.end(), SortByAverageAbsoluteDifference);
        }
      else if(this->radSortByAverageSquaredDifference->isChecked())
        {
        std::sort(candidatePairs.begin(), candidatePairs.end(), SortByAverageSquaredDifference);
        }
      else if(this->radSortByBoundaryPixelDifference->isChecked())
        {
        std::sort(candidatePairs.begin(), candidatePairs.end(), SortByBoundaryPixelDifference);
        }
      else if(this->radSortByBoundaryIsophoteAngleDifference->isChecked())
        {
        std::sort(candidatePairs.begin(), candidatePairs.end(), SortByBoundaryIsophoteAngleDifference);
        }
      else if(this->radSortByBoundaryIsophoteStrengthDifference->isChecked())
        {
        std::sort(candidatePairs.begin(), candidatePairs.end(), SortByBoundaryIsophoteStrengthDifference);
        }
      else if(this->radSortByTotalScore->isChecked())
        {
        std::sort(candidatePairs.begin(), candidatePairs.end(), SortByTotalScore);
        }
      else
        {
        std::cerr << "No valid sorting criterion selected." << std::endl;
        }
      }
    }

  SetupTopPatchesTable();
  //on_topPatchesTableWidget_currentCellChanged(0, 0);
  
  Refresh();
}

void PatchBasedInpaintingGUI::on_chkHighlightUsedPatches_clicked()
{
  Refresh();
}

void PatchBasedInpaintingGUI::on_chkImage_clicked()
{
  Refresh();
}

void PatchBasedInpaintingGUI::on_chkMask_clicked()
{
  Refresh();
}

void PatchBasedInpaintingGUI::on_chkPriority_clicked()
{
  Refresh();
}

void PatchBasedInpaintingGUI::on_chkDisplayForwardLookPatchLocations_clicked()
{
  Refresh();
}

void PatchBasedInpaintingGUI::on_chkDisplaySourcePatchLocations_clicked()
{
  Refresh();
}

void PatchBasedInpaintingGUI::on_chkConfidence_clicked()
{
  Refresh();
}

void PatchBasedInpaintingGUI::on_chkConfidenceMap_clicked()
{
  Refresh();
}

void PatchBasedInpaintingGUI::on_chkBoundary_clicked()
{
  Refresh();
}

void PatchBasedInpaintingGUI::on_chkIsophotes_clicked()
{
  Refresh();
}

void PatchBasedInpaintingGUI::on_chkData_clicked()
{
  Refresh();
}

void PatchBasedInpaintingGUI::on_chkBoundaryNormals_clicked()
{
  Refresh();
}

void PatchBasedInpaintingGUI::on_chkPotentialPatches_clicked()
{
  Refresh();
}
/*
void Form::SetCameraPosition1()
{
  double leftToRight[3] = {-1,0,0};
  double bottomToTop[3] = {0,1,0};
  SetCameraPosition(leftToRight, bottomToTop);
}

void Form::SetCameraPosition2()
{
  double leftToRight[3] = {-1,0,0};
  double bottomToTop[3] = {0,-1,0};

  SetCameraPosition(leftToRight, bottomToTop);
}
*/

void PatchBasedInpaintingGUI::SetCameraPosition()
{
  double leftToRight[3] = {this->CameraLeftToRightVector[0], this->CameraLeftToRightVector[1], this->CameraLeftToRightVector[2]};
  double bottomToTop[3] = {this->CameraBottomToTopVector[0], this->CameraBottomToTopVector[1], this->CameraBottomToTopVector[2]};
  this->InteractorStyle->SetImageOrientation(leftToRight, bottomToTop);

  this->Renderer->ResetCamera();
  this->Renderer->ResetCameraClippingRange();
  this->qvtkWidget->GetRenderWindow()->Render();
}

void PatchBasedInpaintingGUI::on_actionFlipImageVertically_activated()
{
  this->CameraBottomToTopVector[1] *= -1;
  SetCameraPosition();
}

void PatchBasedInpaintingGUI::on_actionFlipImageHorizontally_activated()
{
  this->CameraLeftToRightVector[0] *= -1;
  SetCameraPosition();
}

void PatchBasedInpaintingGUI::SetCheckboxVisibility(const bool visible)
{
  chkImage->setEnabled(visible);
  chkPriority->setEnabled(visible);
  chkConfidence->setEnabled(visible);
  chkConfidenceMap->setEnabled(visible);
  chkBoundary->setEnabled(visible);
  chkIsophotes->setEnabled(visible);
  chkData->setEnabled(visible);
  chkBoundaryNormals->setEnabled(visible);
  chkMask->setEnabled(visible);
  chkPotentialPatches->setEnabled(visible);
}
