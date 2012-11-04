/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
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

#ifndef ManualPatchSelectionDialogParent_H
#define ManualPatchSelectionDialogParent_H

#include "ui_ManualPatchSelectionDialog.h"

// ITK
#include "itkImage.h"

// Qt
#include <QDialog>

class ManualPatchSelectionDialogParent : public QDialog, public Ui::ManualPatchSelectionDialog
{
Q_OBJECT

public slots:

  //virtual void slot_UpdateImage() = 0;
  virtual void slot_UpdateSource(const itk::ImageRegion<2>& region,
                                 const itk::ImageRegion<2>& targetregion) = 0;
  virtual void slot_UpdateTarget(const itk::ImageRegion<2>& region) = 0;
  virtual void slot_UpdateResult(const itk::ImageRegion<2>& sourceRegion,
                                 const itk::ImageRegion<2>& targetRegion) = 0;
  virtual void on_btnAccept_clicked() = 0;

  virtual void slot_PatchMoved() = 0;
};

#endif // ManualPatchSelectionDialogParent_H
