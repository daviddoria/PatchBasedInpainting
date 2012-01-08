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

#include "ui_FileSelector.h"

// Custom
#include "FileSelector.h"
#include "Widgets/FileSelectionWidget.h"
#include "Helpers/Helpers.h"
#include "HelpersQt.h"
#include "Mask.h"
#include "Types.h"

// Qt
#include <QFileSystemModel>
#include <QWidget>

// STL
#include <iostream>

// ITK
#include "itkImageFileReader.h"

// Constructor
FileSelector::FileSelector()
{
  // Create the custom widgets.
  this->FileSelectionWidgetImage = new FileSelectionWidget(this);
  this->FileSelectionWidgetMask = new FileSelectionWidget(this);

  this->setupUi(this);

  this->FileSelectionLayout->addWidget(this->FileSelectionWidgetImage);
  this->FileSelectionLayout->addWidget(this->FileSelectionWidgetMask);

  this->ImageGraphicsScene = new QGraphicsScene(this);
  this->graphicsViewImage->setScene(this->ImageGraphicsScene);

  this->MaskGraphicsScene = new QGraphicsScene(this);
  this->graphicsViewMask->setScene(this->MaskGraphicsScene);

  connect(this->FileSelectionWidgetImage, SIGNAL(selectionChanged()), this, SLOT(LoadAndDisplayImage()));
  connect(this->FileSelectionWidgetMask, SIGNAL(selectionChanged()), this, SLOT(LoadAndDisplayMask()));
};

void FileSelector::LoadAndDisplayImage()
{
  this->ImageFileName = this->FileSelectionWidgetImage->currentIndex().data(QFileSystemModel::FilePathRole).toString().toStdString();

  typedef itk::ImageFileReader<FloatVectorImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(this->ImageFileName);
  reader->Update();

  this->Image = reader->GetOutput();

  QImage image = HelpersQt::GetQImageColor<FloatVectorImageType>(this->Image, this->Image->GetLargestPossibleRegion());
  image = HelpersQt::FitToGraphicsView(image, this->graphicsViewImage);

  this->ImageGraphicsScene->clear();
  this->ImageGraphicsScene->addPixmap(QPixmap::fromImage(image));

  Verify();
}

void FileSelector::LoadAndDisplayMask()
{
  this->MaskFileName = this->FileSelectionWidgetMask->currentIndex().data(QFileSystemModel::FilePathRole).toString().toStdString();

  typedef itk::ImageFileReader<Mask> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(this->MaskFileName);
  reader->Update();

  this->ImageMask = reader->GetOutput();

  QImage mask = HelpersQt::GetQImageScalar<Mask>(this->ImageMask, this->ImageMask->GetLargestPossibleRegion());
  mask = HelpersQt::FitToGraphicsView(mask, this->graphicsViewMask);

  this->MaskGraphicsScene->clear();
  this->MaskGraphicsScene->addPixmap(QPixmap::fromImage(mask));

  Verify();
}

void FileSelector::Verify()
{
  bool bothLoaded = this->FileSelectionWidgetImage->IsValid() && this->FileSelectionWidgetMask->IsValid();
  if(bothLoaded)
    {
    if(this->Image->GetLargestPossibleRegion() == this->ImageMask->GetLargestPossibleRegion())
      {
      this->buttonBox->setEnabled(true);
      return;
      }
    }
  this->buttonBox->setEnabled(false);
}

std::string FileSelector::GetImageFileName()
{
  return this->ImageFileName;
}

std::string FileSelector::GetMaskFileName()
{
  return this->MaskFileName;
}

bool FileSelector::IsMaskInverted()
{
  return this->MaskInverted;
}

void FileSelector::on_buttonBox_accepted()
{
  // QFileDialog::getOpenFileName(this, "Open File", ".", "Image Files (*.jpg *.jpeg *.bmp *.png *.mha);;PNG Files (*.png)");
  // QString fileName = QFileDialog::getOpenFileName(this, "Open File", ".", "Image Files (*.png *.bmp)");
  //this->ImageFileName = this->treeImage->currentIndex().data(QFileSystemModel::FilePathRole).toString().toStdString() + "/" +
    //                    this->treeImage->currentIndex().data(QFileSystemModel::FileNameRole).toString().toStdString();


  this->MaskInverted = this->chkInvertMask->isChecked();

  // Set the working directory if both files came from the same directory
  if( this->FileSelectionWidgetImage->currentIndex().data(QFileSystemModel::FilePathRole).toString().toStdString().compare(
      this->FileSelectionWidgetMask->currentIndex().data(QFileSystemModel::FilePathRole).toString().toStdString()) == 0)
    {
    std::string workingDirectory = this->FileSelectionWidgetImage->currentIndex().data(QFileSystemModel::FilePathRole).toString().toStdString() + "/";

    QDir::setCurrent(QString(workingDirectory.c_str()));
    std::cout << "Working directory set to: " << workingDirectory << std::endl;
    }

  std::cout << "FileSelector got image file name: " << this->ImageFileName
            << " and mask file name: " << this->MaskFileName << std::endl;

  this->setResult(QDialog::Accepted);

  this->accept();
}

void FileSelector::on_buttonBox_rejected()
{
  this->setResult(QDialog::Rejected);
  this->reject();
}
