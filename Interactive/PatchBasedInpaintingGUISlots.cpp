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
#include "FileSelector.h"
#include "Helpers/HelpersOutput.h"
#include "InteractorStyleImageWithDrag.h"
#include "SelfPatchCompare.h"

// VTK
#include <vtkImageSlice.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

// QT
#include <QFileDialog>
#include <QTextEdit>

void PatchBasedInpaintingGUI::slot_ChangeFileName(QModelIndex index)
{
  //std::cout << "Clicked row " << index.row() << " column " << index.column() << std::endl;
  if (index.column() == TableModelImageInput::FILENAME_COLUMN)
    {
    QString fileName = QFileDialog::getOpenFileName(this, "Open", "", "Files (*.mha, *.png)");
    if(!fileName.isEmpty())
      {
      this->ModelImages->setData(index, fileName, Qt::EditRole);
      }
    }
}

void PatchBasedInpaintingGUI::on_chkDisplayUserPatch_clicked()
{
  this->UserPatch->SetVisibility(this->chkDisplayUserPatch->isChecked());
}

void PatchBasedInpaintingGUI::on_radDisplayColorImage_clicked()
{
  this->spinChannelToDisplay->setEnabled(false);
  this->RecordViewer->GetImageDisplayStyle().Style = DisplayStyle::COLOR;
  Refresh();
}

void PatchBasedInpaintingGUI::on_radDisplayMagnitudeImage_clicked()
{
  this->spinChannelToDisplay->setEnabled(false);
  this->RecordViewer->GetImageDisplayStyle().Style = DisplayStyle::MAGNITUDE;
  Refresh();
}

void PatchBasedInpaintingGUI::on_radDisplayChannel_clicked()
{
  this->spinChannelToDisplay->setEnabled(true);
  this->RecordViewer->GetImageDisplayStyle().Style = DisplayStyle::CHANNEL;
  this->RecordViewer->GetImageDisplayStyle().Channel = this->spinChannelToDisplay->value();
  Refresh();
}

void PatchBasedInpaintingGUI::on_spinChannelToDisplay_valueChanged(int unused)
{
  this->RecordViewer->GetImageDisplayStyle().Channel = this->spinChannelToDisplay->value();
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
  this->RecordDisplayState.Iteration = this->txtGoToIteration->text().toUInt();
  ChangeDisplayedIteration();
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

void PatchBasedInpaintingGUI::on_chkDisplayForwardLookPatchLocations_clicked()
{
  RefreshVTK();
}

void PatchBasedInpaintingGUI::on_chkDisplaySourcePatchLocations_clicked()
{
  RefreshVTK();
}

void PatchBasedInpaintingGUI::on_actionFlipImageVertically_activated()
{
  this->Camera->FlipVertically();
}

void PatchBasedInpaintingGUI::on_actionFlipImageHorizontally_activated()
{
  this->Camera->FlipHorizontally();
}

void PatchBasedInpaintingGUI::on_btnDisplayPreviousStep_clicked()
{
  if(this->RecordDisplayState.Iteration > 0)
    {
    this->RecordDisplayState.Iteration--;
    DebugMessage<unsigned int>("Displaying iteration: ", this->RecordDisplayState.Iteration);
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
  if(this->RecordDisplayState.Iteration < this->IterationRecords.size() - 1)
    {
    this->RecordDisplayState.Iteration++;
    DebugMessage<unsigned int>("Displaying iteration: ", this->RecordDisplayState.Iteration);
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
    this->statusBar()->showMessage("No file selected.");
    return;
    }

  HelpersOutput::WriteImage<FloatVectorImageType>(this->Inpainting->GetCurrentOutputImage(), fileName.toStdString());

  this->statusBar()->showMessage("Saved result.");
}

void PatchBasedInpaintingGUI::on_chkDebugImages_clicked()
{
  this->Inpainting->SetDebugImages(this->chkDebugImages->isChecked());
  this->DebugImages = this->chkDebugImages->isChecked();

  DebugMessage<bool>("DebugImages: ", this->DebugImages);
}

void PatchBasedInpaintingGUI::on_chkDebugMessages_clicked()
{
  this->Inpainting->SetDebugMessages(this->chkDebugMessages->isChecked());
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

  this->RecordDisplayState.ForwardLookId = currentIndex.row();
  std::cout << "Set this->ForwardLookToDisplay to " << this->RecordDisplayState.ForwardLookId << std::endl;
  // When we select a different forward look patch, there is no way to know which source patch the user wants to see, so show the 0th.
  this->RecordDisplayState.SourcePatchId = 0;

  ChangeDisplayedForwardLookPatch();

  LeaveFunction("slot_ForwardLookTableView_changed()");
}

void PatchBasedInpaintingGUI::slot_TopPatchesTableView_changed(const QModelIndex& currentIndex, const QModelIndex& previousIndex)
{
  EnterFunction("slot_TopPatchesTableView_changed()");

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

  this->RecordDisplayState.SourcePatchId = currentIndex.row();
  std::cout << "Set this->SourcePatchToDisplay to " << this->RecordDisplayState.SourcePatchId << std::endl;
  ChangeDisplayedTopPatch();

  LeaveFunction("slot_TopPatchesTableView_changed()");
}

void PatchBasedInpaintingGUI::on_btnInpaint_clicked()
{
  this->btnStop->setEnabled(true);
  this->btnStep->setEnabled(false);
  this->btnInpaint->setEnabled(false);
  this->btnReset->setEnabled(false);
  this->InpaintingComputation->Operation = InpaintingComputationObject::ALLSTEPS;
  this->ComputationThread->start();
}

void PatchBasedInpaintingGUI::on_btnInitialize_clicked()
{
  // We will potentially write debug outputs to the Debug/ directory of the working directory.
  // We need to create this directory if it doesn't exist.
  if(!QDir(QDir::current().filePath("Debug")).exists())
    {
    QDir().mkdir(QDir::current().filePath("Debug"));
    }

  this->tabSettings->setEnabled(false);
  this->tabRecord->setEnabled(false);
  this->tabImages->setEnabled(false);

  this->btnReset->setEnabled(true);
  this->btnInpaint->setEnabled(true);
  this->btnStep->setEnabled(true);
  this->btnInitialize->setEnabled(false);

  if(this->Inpainting)
    {
    delete this->Inpainting;
    }
  this->Inpainting = new PatchBasedInpainting<FloatVectorImageType>(this->UserImage, this->UserMaskImage);
  this->Inpainting->SetPatchRadius(this->Settings.PatchRadius);

  this->Inpainting->SetDebugImages(this->chkDebugImages->isChecked());
  this->Inpainting->SetDebugMessages(this->chkDebugMessages->isChecked());
  //this->Inpainting.SetDebugFunctionEnterLeave(false);

  /*
  // TODO: Fix this
  // Setup the patch comparison function
  this->Inpainting->GetPatchCompare()->SetNumberOfComponentsPerPixel(this->UserImage->GetNumberOfComponentsPerPixel());

  // Setup the sorting function
  //this->Inpainting->PatchSortFunction = new SortByDifference(PatchPair::AverageAbsoluteDifference, PatchSortFunctor::ASCENDING);
  this->Inpainting->PatchSortFunction = std::make_shared<SortByDifference>(PatchPair::AverageAbsoluteDifference, PatchSortFunctor::ASCENDING);
  */

  // Finish initializing
  this->Inpainting->Initialize();

  SetPriorityFromGUI();
  SetCompareImageFromGUI();
  SetComparisonFunctionsFromGUI();
  SetSortFunctionFromGUI();
  SetParametersFromGUI();

  this->Inpainting->SetDebugImages(this->chkDebugImages->isChecked());
  this->Inpainting->SetDebugMessages(this->chkDebugMessages->isChecked());

  this->InpaintingComputation->SetObject(this->Inpainting);
}

void PatchBasedInpaintingGUI::on_btnStep_clicked()
{
  //PatchPair usedPair = this->Inpainting.Iterate();

  //IterationComplete(usedPair);

  this->btnStep->setEnabled(false);
  this->btnInpaint->setEnabled(false);
  this->btnReset->setEnabled(false);
  this->InpaintingComputation->Operation = InpaintingComputationObject::SINGLESTEP;
  this->ComputationThread->start();
}

void PatchBasedInpaintingGUI::on_btnStop_clicked()
{
  this->InpaintingComputation->StopInpainting();

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
  QScopedPointer<QTextEdit> help(new QTextEdit);

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
  EnterFunction("slot_StopProgress()");

  // Re-enable some items that should not be changed while the inpainting is running.
  this->txtNumberOfForwardLook->setEnabled(false);
  this->txtNumberOfTopPatchesToSave->setEnabled(false);

  this->progressBar->hide();
}

void PatchBasedInpaintingGUI::slot_IterationComplete(const PatchPair& patchPair)
{
  EnterFunction("IterationCompleteSlot()");
  this->IterationValidator->setTop(this->IterationValidator->top() + 1);

  this->btnStep->setEnabled(true);
  this->btnInpaint->setEnabled(true);
  this->btnReset->setEnabled(true);
  IterationComplete(patchPair);
  Refresh();
  LeaveFunction("IterationCompleteSlot()");
}

void PatchBasedInpaintingGUI::on_txtPatchRadius_textEdited ( const QString & text )
{
  this->Settings.PatchRadius = text.toUInt();
}

void PatchBasedInpaintingGUI::on_txtNumberOfTopPatchesToSave_textEdited ( const QString & text )
{
  this->Settings.NumberOfTopPatchesToSave = text.toUInt();
}

void PatchBasedInpaintingGUI::on_txtNumberOfForwardLook_textEdited ( const QString & text )
{
  this->Settings.NumberOfForwardLook = text.toUInt();
}

void PatchBasedInpaintingGUI::on_txtNumberOfTopPatchesToDisplay_textEdited ( const QString & text )
{
  EnterFunction("on_txtNumberOfTopPatchesToDisplay_textEdited()");
  this->Settings.NumberOfTopPatchesToDisplay = text.toUInt();
  this->TopPatchesModel->SetNumberOfTopPatchesToDisplay(this->Settings.NumberOfTopPatchesToDisplay);
  this->TopPatchesModel->Refresh();
  //HighlightSourcePatches();
  LeaveFunction("on_txtNumberOfTopPatchesToDisplay_textEdited()");
}

void PatchBasedInpaintingGUI::on_cmbPriority_activated(int value)
{
  SetupSaveModel();
}

void PatchBasedInpaintingGUI::slot_ChangeDisplayedImages(QModelIndex)
{
  for(unsigned int imageId = 0; imageId < this->IterationRecords[this->RecordDisplayState.Iteration].GetNumberOfImages(); ++imageId)
    {
//     Helpers::ITKScalarImageToScaledVTKImage<UnsignedCharScalarImageType>
//     (dynamic_cast<UnsignedCharScalarImageType*>(this->IterationRecords[this->IterationToDisplay].Images[imageId].Image.GetPointer()), this->BoundaryLayer.ImageData);

    }
  this->qvtkWidget->GetRenderWindow()->Render();
}
