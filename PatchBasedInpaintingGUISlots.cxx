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
#include "InteractorStyleImageNoLevel.h"
#include "FileSelector.h"
#include "HelpersOutput.h"

// VTK
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

// QT
#include <QFileDialog>
#include <QTextEdit>

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
  if(this->IterationToDisplay < this->IntermediateImages.size() - 1)
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
  std::cout << "on_ForwardLookTableView_currentCellChanged" << std::endl;
  
  if(currentIndex.row() < 0)
    {
    std::cout << "on_ForwardLookTableView_currentCellChanged: row < 0!" << std::endl;
    return;
    }
  
  if(currentIndex.row() > static_cast<int>(this->AllPotentialCandidatePairs[this->IterationToDisplay - 1].size() - 1))
    {
    std::cerr << "Requested display of forward look patch " << currentIndex.row() << " but there are only " << this->AllPotentialCandidatePairs[this->IterationToDisplay - 1].size() - 1 << std::endl;
    }

  std::cerr << "Requested display of forward look patch " << currentIndex.row() << std::endl;
  
  this->ForwardLookToDisplay = currentIndex.row();
  this->SourcePatchToDisplay = 0;
  
  ChangeDisplayedForwardLookPatch();
  
  SetupTopPatchesTable();
  ChangeDisplayedTopPatch();
  
}


void PatchBasedInpaintingGUI::slot_TopPatchesTableView_changed(const QModelIndex& currentIndex, const QModelIndex& previousIndex)
{
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
    
    this->SourcePatchToDisplay = currentIndex.row();
    ChangeDisplayedTopPatch();
    
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
  DebugMessage("on_btnInpaint_clicked()");
  
  //Initialize();
  
  Refresh();
  
  DebugMessage("Starting ComputationThread...");
  
  this->Inpainting.SetMaxForwardLookPatches(this->txtNumberOfForwardLook->text().toUInt());
  this->Inpainting.SetNumberOfTopPatchesToSave(this->txtNumberOfTopPatchesToSave->text().toUInt());
  
  ComputationThread.start();
}


void PatchBasedInpaintingGUI::on_btnStep_clicked()
{
  this->Inpainting.SetDebugImages(this->chkDebugImages->isChecked());
  this->Inpainting.SetDebugMessages(this->chkDebugMessages->isChecked());
  this->Inpainting.SetMaxForwardLookPatches(this->txtNumberOfForwardLook->text().toUInt());
  this->Inpainting.SetNumberOfTopPatchesToSave(this->txtNumberOfTopPatchesToSave->text().toUInt());
  PatchPair usedPair = this->Inpainting.Iterate();
  
  this->UsedPatchPairs.push_back(usedPair);
  
  IterationComplete();
}


void PatchBasedInpaintingGUI::on_btnStop_clicked()
{
  this->ComputationThread.StopInpainting();
}

void PatchBasedInpaintingGUI::on_btnReset_clicked()
{
  slot_Refresh();
}
  

void PatchBasedInpaintingGUI::on_btnInitialize_clicked()
{
  Initialize();
}


void PatchBasedInpaintingGUI::on_actionHelp_activated()
{
  QTextEdit* help=new QTextEdit();

  help->setReadOnly(true);
  help->append("<h1>Patch Based Inpainting</h1>\
  Load an image and a mask. <br/>\
  Set the settings such as patch size. <br/>\
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
  // When the ProgressThread emits the StopProgressSignal, we need to display the result of the segmentation

  this->progressBar->hide();
}

void PatchBasedInpaintingGUI::slot_Refresh()
{
  DebugMessage("RefreshSlot()");

  Refresh();
}


void PatchBasedInpaintingGUI::slot_IterationComplete()
{
  DebugMessage("IterationCompleteSlot()");
  IterationComplete();
}