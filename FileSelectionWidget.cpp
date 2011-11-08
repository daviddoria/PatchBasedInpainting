#include <QDir>
#include <QFileSystemModel>
#include <QWidget>

#include <iostream>

#include "FileSelectionWidget.h"

FileSelectionWidget::FileSelectionWidget(QWidget *parent)
    : QWidget(parent)
{
  std::cout << "Current path: " << QDir::currentPath().toStdString() << std::endl;
  setupUi(this); // Otherwise 'listView' seems to be undefined
    
  this->model = new QFileSystemModel;
  this->model->setRootPath(QDir::currentPath());
  //this->model->setCurrentIndex(imageModel->index(QDir::currentPath()));
  
  this->listView->setModel(model);
  this->listView->setRootIndex(model->index(QDir::currentPath()));

  this->lblPath->setText(QDir::currentPath());
  this->lblFile->setText(listView->currentIndex().data(QFileSystemModel::FilePathRole).toString());
  this->listView->setCurrentIndex(model->index(QDir::currentPath()));

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

  this->listView->setRootIndex(model->index(listView->currentIndex().parent().data(QFileSystemModel::FilePathRole).toString()));

  this->lblPath->setText(listView->currentIndex().data(QFileSystemModel::FilePathRole).toString());
}
