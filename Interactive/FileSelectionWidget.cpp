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

#include <QDir>
#include <QFileSystemModel>
#include <QWidget>

#include <iostream>

#include "FileSelectionWidget.h"

FileSelectionWidget::FileSelectionWidget(QWidget *parent)
    : QWidget(parent)
{
  std::cout << "Current path: " << QDir::currentPath().toStdString() << std::endl;
  std::cout << "Root path: " << QDir::rootPath().toStdString() << std::endl;
  setupUi(this); // Otherwise 'listView' seems to be undefined

  this->model = new QFileSystemModel;
  this->model->setRootPath(QDir::rootPath());


  this->listView->setModel(model);
  this->listView->setRootIndex(model->index(QDir::currentPath()));

  this->lblPath->setText(QDir::currentPath());
  this->lblFile->setText(listView->currentIndex().data(QFileSystemModel::FilePathRole).toString());
  //this->listView->setCurrentIndex(model->index(QDir::currentPath()));
  //this->listView->setCurrentIndex(model->index(QDir::currentPath()));
  this->listView->setCurrentIndex(this->listView->rootIndex());

  this->Valid = false;
}

void FileSelectionWidget::setModel(QAbstractItemModel* model)
{
  this->listView->setModel(model);
}

void FileSelectionWidget::setCurrentIndex(const QModelIndex& index)
{
  this->listView->setCurrentIndex(index);
}

bool FileSelectionWidget::IsValid()
{
  return this->Valid;
}

void FileSelectionWidget::on_listView_doubleClicked(const QModelIndex & index)
{
  std::cout << listView->currentIndex().data(QFileSystemModel::FilePathRole).toString().toStdString() << std::endl;
  //std::cout << listView->currentIndex().data(QFileSystemModel::FileNameRole).toString().toStdString() << std::endl;

  if(this->model->isDir(this->listView->currentIndex())) // A directory was selected
    {
    this->listView->setRootIndex(model->index(listView->currentIndex().data(QFileSystemModel::FilePathRole).toString()));

    this->lblPath->setText(listView->currentIndex().data(QFileSystemModel::FilePathRole).toString());
    }
  else // A file was selected
    {
    on_listView_clicked(index);
    }
}

QModelIndex FileSelectionWidget::currentIndex() const
{
  return this->listView->currentIndex();
}

void FileSelectionWidget::on_listView_clicked(const QModelIndex & index)
{
  this->FileName = listView->currentIndex().data(QFileSystemModel::FilePathRole).toString().toStdString();
  this->lblFile->setText(this->FileName.c_str());

  if(!this->model->isDir(this->listView->currentIndex()))
    {
    this->Valid = true;

    // Emit the signal
    emit selectionChanged();
    }
  else
    {
    this->Valid = false;
    }

}

void FileSelectionWidget::on_btnUp_clicked()
{
  this->listView->setRootIndex(model->index(listView->rootIndex().parent().data(QFileSystemModel::FilePathRole).toString()));
  this->listView->setCurrentIndex(this->listView->rootIndex());

  this->lblPath->setText(listView->currentIndex().data(QFileSystemModel::FilePathRole).toString());
}
