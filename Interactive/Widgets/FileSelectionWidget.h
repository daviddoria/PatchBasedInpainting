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

#ifndef FileSelectionWidget_H
#define FileSelectionWidget_H

#include "ui_FileSelectionWidget.h"

#include <QMainWindow>

class QFileSystemModel;

class FileSelectionWidget : public QWidget, private Ui::FileSelectionWidget
{
  Q_OBJECT

public:
  FileSelectionWidget(QWidget *parent = 0);

  bool IsValid();

  void setModel(QAbstractItemModel* model);
  void setCurrentIndex(const QModelIndex& index);

  QModelIndex currentIndex() const;

public slots:
  void on_listView_doubleClicked(const QModelIndex & index);
  void on_listView_clicked(const QModelIndex & index);
  void on_btnUp_clicked();

signals:
  void selectionChanged();

protected:
  QFileSystemModel *model;

  bool Valid;
  std::string FileName;
};

#endif
