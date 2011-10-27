#include "Form.h"

#include "InteractorStyleImageNoLevel.h"

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

void Form::on_radCompareOriginal_clicked()
{
  this->Inpainting.SetCompareToOriginal();
}

void Form::on_radCompareBlurred_clicked()
{
  this->Inpainting.SetCompareToBlurred();
}

void Form::on_radCompareCIELAB_clicked()
{
  this->Inpainting.SetCompareToCIELAB();
}
  
void Form::on_btnResort_clicked()
{
  for(unsigned int iteration = 0; iteration < this->AllPotentialCandidatePairs.size(); iteration++)
    {
    for(unsigned int forwardLookId = 0; forwardLookId < this->AllPotentialCandidatePairs[iteration].size(); forwardLookId++)
      {
      CandidatePairs& candidatePairs = this->AllPotentialCandidatePairs[iteration][forwardLookId];

      if(this->radSortByTotalAbsoluteDifference->isChecked())
        {
        std::sort(candidatePairs.begin(), candidatePairs.end(), SortByTotalAbsoluteDifference);
        }
      else if(this->radSortByTotalSquaredDifference->isChecked())
        {
        std::sort(candidatePairs.begin(), candidatePairs.end(), SortByTotalSquaredDifference);
        }
      else if(this->radSortByAverageAbsoluteDifference->isChecked())
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

  unsigned int forwardLookId = this->forwardLookingTableWidget->currentRow();
  SetupTopPatchesTable(forwardLookId);
  on_topPatchesTableWidget_cellClicked(0, 0);
  
  Refresh();
}

void Form::on_chkHighlightUsedPatches_clicked()
{
  Refresh();
}

void Form::on_chkImage_clicked()
{
  Refresh();
}

void Form::on_chkMask_clicked()
{
  Refresh();
}

void Form::on_chkPriority_clicked()
{
  Refresh();
}

void Form::on_chkDisplayForwardLookPatchLocations_clicked()
{
  Refresh();
}

void Form::on_chkDisplaySourcePatchLocations_clicked()
{
  Refresh();
}

void Form::on_chkConfidence_clicked()
{
  Refresh();
}

void Form::on_chkConfidenceMap_clicked()
{
  Refresh();
}

void Form::on_chkBoundary_clicked()
{
  Refresh();
}

void Form::on_chkIsophotes_clicked()
{
  Refresh();
}

void Form::on_chkData_clicked()
{
  Refresh();
}

void Form::on_chkBoundaryNormals_clicked()
{
  Refresh();
}

void Form::on_chkPotentialPatches_clicked()
{
  Refresh();
}

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

void Form::SetCameraPosition(double leftToRight[3], double bottomToTop[3])
{
  this->InteractorStyle->SetImageOrientation(leftToRight, bottomToTop);

  this->Renderer->ResetCamera();
  this->Renderer->ResetCameraClippingRange();
  this->qvtkWidget->GetRenderWindow()->Render();
}

void Form::on_actionFlipImage_activated()
{
  if(this->Flipped)
    {
    SetCameraPosition1();
    }   
  else
    {
    SetCameraPosition2();
    }
  this->Flipped = !this->Flipped;
}

void Form::SetCheckboxVisibility(const bool visible)
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
