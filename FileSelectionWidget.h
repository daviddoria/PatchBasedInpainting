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
