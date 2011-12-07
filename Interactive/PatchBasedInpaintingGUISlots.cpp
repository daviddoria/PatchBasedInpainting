/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "PatchBasedInpaintingGUI.h"

// Custom
#include "InteractorStyleImageWithDrag.h"
#include "FileSelector.h"
#include "HelpersOutput.h"

// VTK
#include <vtkImageSlice.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

// QT
#include <QFileDialog>
#include <QTextEdit>

void PatchBasedInpaintingGUI::on_chkDisplayUserPatch_clicked()
{
  this->UserPatchLayer.ImageSlice->SetVisibility(this->chkDisplayUserPatch->isChecked());
  Refresh();
}

void PatchBasedInpaintingGUI::on_radDisplayColorImage_clicked()
{
  this->spinChannelToDisplay->setEnabled(false);
  this->ImageDisplayStyle.Style = DisplayStyle::COLOR;
  Refresh();
}

void PatchBasedInpaintingGUI::on_radDisplayMagnitudeImage_clicked()
{
  this->spinChannelToDisplay->setEnabled(false);
  this->ImageDisplayStyle.Style = DisplayStyle::MAGNITUDE;
  Refresh();
}

void PatchBasedInpaintingGUI::on_radDisplayChannel_clicked()
{
  this->spinChannelToDisplay->setEnabled(true);
  this->ImageDisplayStyle.Style = DisplayStyle::CHANNEL;
  this->ImageDisplayStyle.Channel = this->spinChannelToDisplay->value();
  Refresh();
}

void PatchBasedInpaintingGUI::on_spinChannelToDisplay_valueChanged(int unused)
{
  this->ImageDisplayStyle.Channel = this->spinChannelToDisplay->value();
  Refresh();
}

void PatchBasedInpaintingGUI::on_cmbCompareImage_activated(int value)
{
  SetCompareImageFromGUI();
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
  if(this->GoToIteration < this->IterationRecords.size() && this->GoToIteration >= 0)
    {
    this->IterationToDisplay = this->GoToIteration;
    ChangeDisplayedIteration();
    }
}
/*
void PatchBasedInpaintingGUI::on_btnResort_clicked()
{
  for(unsigned int iteration = 0; iteration < this->IterationRecords.size(); iteration++)
    {
    for(unsigned int forwardLookId = 0; forwardLookId < this->IterationRecords[iteration].PotentialPairSets.size(); forwardLookId++)
      {
      CandidatePairs& candidatePairs = this->IterationRecords[iteration].PotentialPairSets[forwardLookId];

      if(this->radSortByFullDifference->isChecked())
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
*/
void PatchBasedInpaintingGUI::on_chkHighlightUsedPatches_clicked()
{
  RefreshVTK();
}

void PatchBasedInpaintingGUI::on_chkDisplayImage_clicked()
{
  this->ImageLayer.ImageSlice->SetVisibility(this->chkDisplayImage->isChecked());
  RefreshVTK();
}

void PatchBasedInpaintingGUI::on_chkDisplayMask_clicked()
{
  this->MaskLayer.ImageSlice->SetVisibility(this->chkDisplayMask->isChecked());
  RefreshVTK();
}

void PatchBasedInpaintingGUI::on_chkDisplayForwardLookPatchLocations_clicked()
{
  RefreshVTK();
}

void PatchBasedInpaintingGUI::on_chkDisplaySourcePatchLocations_clicked()
{
  RefreshVTK();
}


void PatchBasedInpaintingGUI::on_chkDisplayBoundary_clicked()
{
  this->BoundaryLayer.ImageSlice->SetVisibility(this->chkDisplayBoundary->isChecked());
  RefreshVTK();
}

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
  chkDisplayImage->setEnabled(visible);
  chkDisplayBoundary->setEnabled(visible);
  chkDisplayMask->setEnabled(visible);
}

void PatchBasedInpaintingGUI::on_btnDisplayPreviousStep_clicked()
{
  if(this->IterationToDisplay > 0)
    {
    this->IterationToDisplay--;
    DebugMessage<unsigned int>("Displaying iteration: ", this->IterationToDisplay);
    ChangeDisplayedIteration();
    }
  else
    {
    DebugMessage("Iteration not changed.");
    }
}


void PatchBasedInpaintingGUI::on_btnDisplayNextStep_clicked()
{
  //std::cout << "IterationToDisplay: " << this->IterationToDisplay
    //        << " Inpainting iteration: " <<  static_cast<int>(this->Inpainting.GetIteration()) << std::endl;
  
  //if(this->IterationToDisplay < this->Inpainting.GetNumberOfCompletedIterations() - 1)
  if(this->IterationToDisplay < this->IterationRecords.size() - 1)
    {
    this->IterationToDisplay++;
    DebugMessage<unsigned int>("Displaying iteration: ", this->IterationToDisplay);
    ChangeDisplayedIteration();
    }
  else
    {
    DebugMessage("Iteration not changed.");
    }
}


void PatchBasedInpaintingGUI::on_actionOpen_activated()
{
  FileSelector* fileSelector(new FileSelector);
  fileSelector->exec();

  int result = fileSelector->result();
  if(result) // The user clicked 'ok'
    {
    std::cout << "User clicked ok." << std::endl;
    OpenImage(fileSelector->GetImageFileName());
    OpenMask(fileSelector->GetMaskFileName(), fileSelector->IsMaskInverted());
    Initialize();
    }
  else
    {
    std::cout << "User clicked cancel." << std::endl;
    // The user clicked 'cancel' or closed the dialog, do nothing.
    }
}


void PatchBasedInpaintingGUI::on_actionSaveResult_activated()
{
  // Get a filename to save
  QString fileName = QFileDialog::getSaveFileName(this, "Save File", ".", "Image Files (*.jpg *.jpeg *.bmp *.png *.mha)");

  DebugMessage<std::string>("Got filename: ", fileName.toStdString());
  if(fileName.toStdString().empty())
    {
    std::cout << "Filename was empty." << std::endl;
    return;
    }

  HelpersOutput::WriteImage<FloatVectorImageType>(this->Inpainting.GetCurrentOutputImage(), fileName.toStdString());

  this->statusBar()->showMessage("Saved result.");
}


void PatchBasedInpaintingGUI::on_chkDebugImages_clicked()
{
  QDir directoryMaker;
  directoryMaker.mkdir("Debug");

  this->Inpainting.SetDebugImages(this->chkDebugImages->isChecked());
  this->DebugImages = this->chkDebugImages->isChecked();

  DebugMessage<bool>("DebugImages: ", this->DebugImages);
}

void PatchBasedInpaintingGUI::on_chkDebugMessages_clicked()
{
  this->Inpainting.SetDebugMessages(this->chkDebugMessages->isChecked());
  this->DebugMessages = this->chkDebugMessages->isChecked();
}

void PatchBasedInpaintingGUI::on_actionQuit_activated()
{
  exit(0);
}


void PatchBasedInpaintingGUI::slot_ForwardLookTableView_changed(const QModelIndex& currentIndex, const QModelIndex& previousIndex)
{
  EnterFunction("slot_ForwardLookTableView_changed()");
  std::cout << "on_ForwardLookTableView_currentCellChanged" << std::endl;
  
  if(currentIndex.row() < 0)
    {
    std::cout << "on_ForwardLookTableView_currentCellChanged: row < 0!" << std::endl;
    return;
    }
  
  if(currentIndex.row() > static_cast<int>(this->RecordToDisplay->PotentialPairSets.size()) - 1)
    {
    std::cerr << "Requested display of forward look patch " << currentIndex.row() << " but there are only "
              << this->RecordToDisplay->PotentialPairSets.size() << std::endl;
    return;
    }

  std::cerr << "Requested display of forward look patch " << currentIndex.row() << std::endl;

  this->ForwardLookToDisplayId = currentIndex.row();
  std::cout << "Set this->ForwardLookToDisplay to " << this->ForwardLookToDisplayId << std::endl;
  // When we select a different forward look patch, there is no way to know which source patch the user wants to see, so show the 0th.
  this->SourcePatchToDisplayId = 0;
  
  ChangeDisplayedForwardLookPatch();

  LeaveFunction("slot_ForwardLookTableView_changed()");
}


void PatchBasedInpaintingGUI::slot_TopPatchesTableView_changed(const QModelIndex& currentIndex, const QModelIndex& previousIndex)
{
  EnterFunction("slot_TopPatchesTableView_changed()");
  try
  {
    if(currentIndex.row() < 0)
      {
      std::cout << "Selected row is < 0!" << std::endl;
      return;
      }
      
    if(currentIndex.row() == previousIndex.row())
      {
      std::cout << "Nothing changed!" << std::endl;
      return;
      }
    
    this->SourcePatchToDisplayId = currentIndex.row();
    std::cout << "Set this->SourcePatchToDisplay to " << this->SourcePatchToDisplayId << std::endl;
    ChangeDisplayedTopPatch();

    LeaveFunction("slot_TopPatchesTableView_changed()");
  }// end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in on_topPatchesTableWidget_currentCellChanged!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}


void PatchBasedInpaintingGUI::on_btnInpaint_clicked()
{
  this->btnStop->setEnabled(true);
  this->btnStep->setEnabled(false);
  this->btnInpaint->setEnabled(false);
  this->btnReset->setEnabled(false);
  ComputationThread.Operation = ComputationThreadClass::ALLSTEPS;
  ComputationThread.start();
}

void PatchBasedInpaintingGUI::on_btnInitialize_clicked()
{
  this->txtNumberOfForwardLook->setEnabled(false);
  this->txtNumberOfTopPatchesToSave->setEnabled(false);
  this->btnReset->setEnabled(true);
  this->btnInpaint->setEnabled(true);
  this->btnStep->setEnabled(true);
  this->btnInitialize->setEnabled(false);
  this->txtPatchRadius->setEnabled(false);
  
  this->cmbPriority->setEnabled(false);
  this->cmbSortBy->setEnabled(false);
  this->cmbCompareImage->setEnabled(false);

  this->chkCompareColor->setEnabled(false);
  this->chkCompareDepth->setEnabled(false);
  this->chkCompareFull->setEnabled(false);
  this->chkCompareHistogramIntersection->setEnabled(false);
  this->chkCompareMembership->setEnabled(false);

  SetPriorityFromGUI();

  this->Inpainting.SetDebugImages(this->chkDebugImages->isChecked());
  this->Inpainting.SetDebugMessages(this->chkDebugMessages->isChecked());
  this->Inpainting.SetMaxForwardLookPatches(this->NumberOfForwardLook);
  this->Inpainting.SetNumberOfTopPatchesToSave(this->NumberOfTopPatchesToSave);
}

void PatchBasedInpaintingGUI::on_btnStep_clicked()
{

  //PatchPair usedPair = this->Inpainting.Iterate();

  //IterationComplete(usedPair);
  
  
  this->btnStep->setEnabled(false);
  this->btnInpaint->setEnabled(false);
  this->btnReset->setEnabled(false);
  ComputationThread.Operation = ComputationThreadClass::SINGLESTEP;
  ComputationThread.start();
}

void PatchBasedInpaintingGUI::on_btnStop_clicked()
{
  this->ComputationThread.StopInpainting();

  this->btnStop->setEnabled(false);
  
  this->btnStep->setEnabled(true);
  this->btnInpaint->setEnabled(true);
  this->btnReset->setEnabled(true);
  
}

void PatchBasedInpaintingGUI::on_btnReset_clicked()
{
  Reset();
}

void PatchBasedInpaintingGUI::on_actionHelp_activated()
{
  QTextEdit* help=new QTextEdit();

  help->setReadOnly(true);
  help->append("<h1>Patch Based Inpainting</h1>\
  Load an image and a mask. <br/>\
  Specify some parameters, such as the patch size, debug verbosity, etc. <br/>\
  To do the complete inpainting, click 'Inpaint'.<br/>\
  To do one step of the inpainting, click 'Step'. This will allow you to inspect forward look candidates and each of their top matches.<br/>\
  <p/>");
  help->show();
}


void PatchBasedInpaintingGUI::slot_StartProgress()
{
  //std::cout << "Form::StartProgressSlot()" << std::endl;
  // Connected to the StartProgressSignal of the ProgressThread member
  this->progressBar->show();
}

void PatchBasedInpaintingGUI::slot_StopProgress()
{
  //std::cout << "Form::StopProgressSlot()" << std::endl;

  // Re-enable some items that should not be changed while the inpainting is running.
  this->txtNumberOfForwardLook->setEnabled(false);
  this->txtNumberOfTopPatchesToSave->setEnabled(false);
  
  this->progressBar->hide();
}

void PatchBasedInpaintingGUI::slot_Refresh()
{
  DebugMessage("RefreshSlot()");

  Refresh();
}

void PatchBasedInpaintingGUI::slot_StepComplete(const PatchPair&)
{
  this->btnStep->setEnabled(true);
  this->btnInpaint->setEnabled(true);
  this->btnReset->setEnabled(true);
}

void PatchBasedInpaintingGUI::slot_IterationComplete(const PatchPair& patchPair)
{
  EnterFunction("IterationCompleteSlot()");
  IterationComplete(patchPair);
  LeaveFunction("IterationCompleteSlot()");
}

void PatchBasedInpaintingGUI::on_txtNumberOfBins_textEdited ( const QString & text )
{
  this->Inpainting.GetClusterColors()->SetNumberOfColors(text.toUInt());
}

void PatchBasedInpaintingGUI::on_txtPatchRadius_textEdited ( const QString & text )
{
  this->PatchRadius = text.toUInt();
}

void PatchBasedInpaintingGUI::on_txtNumberOfTopPatchesToSave_textEdited ( const QString & text )
{
  this->NumberOfTopPatchesToSave = text.toUInt();
}

void PatchBasedInpaintingGUI::on_txtNumberOfForwardLook_textEdited ( const QString & text )
{
  this->NumberOfForwardLook = text.toUInt();
}

void PatchBasedInpaintingGUI::on_txtGoToIteration_textEdited ( const QString & text )
{
  this->GoToIteration = text.toUInt();
}

void PatchBasedInpaintingGUI::on_txtNumberOfTopPatchesToDisplay_textEdited ( const QString & text )
{
  EnterFunction("on_txtNumberOfTopPatchesToDisplay_textEdited()");
  this->NumberOfTopPatchesToDisplay = text.toUInt();
  this->TopPatchesModel->SetNumberOfTopPatchesToDisplay(this->NumberOfTopPatchesToDisplay);
  this->TopPatchesModel->Refresh();
  HighlightSourcePatches();
  LeaveFunction("on_txtNumberOfTopPatchesToDisplay_textEdited()");
}

void PatchBasedInpaintingGUI::on_cmbPriority_activated(int value)
{
  SetPriorityFromGUI();
}

void PatchBasedInpaintingGUI::on_cmbSortBy_activated(int value)
{
  SetSortFunctionFromGUI();
}

void PatchBasedInpaintingGUI::on_chkCompareHistogramIntersection_clicked()
{
  SetComparisonFunctionsFromGUI();
}

void PatchBasedInpaintingGUI::on_chkCompareMembership_clicked()
{
  SetComparisonFunctionsFromGUI();
}

void PatchBasedInpaintingGUI::on_chkCompareFull_clicked()
{
  SetComparisonFunctionsFromGUI();
}

void PatchBasedInpaintingGUI::on_chkCompareColor_clicked()
{
  SetComparisonFunctionsFromGUI();
}

void PatchBasedInpaintingGUI::on_chkCompareDepth_clicked()
{
  SetComparisonFunctionsFromGUI();
}

void PatchBasedInpaintingGUI::on_sldDepthColorLambda_valueChanged()
{
  SetDepthColorLambdaFromGUI();
}
