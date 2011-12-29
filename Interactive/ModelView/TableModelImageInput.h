#ifndef TableModelImageInput_H
#define TableModelImageInput_H

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QVariant>
#include <QVector>

#include "ImageInput.h"

class TableModelImageInput : public QAbstractTableModel
{
public:
  TableModelImageInput(QObject * parent = 0);

  enum ColumnNames {NAME_COLUMN=0, FILENAME_COLUMN, DISPLAY_COLUMN, SAVE_COLUMN};

  QVariant data(const QModelIndex& index, int role) const;

  bool setData (const QModelIndex &index, const QVariant &value, int role);

  int rowCount(const QModelIndex  &parent=QModelIndex() ) const;

  int columnCount(const QModelIndex  &parent=QModelIndex() ) const;

  void setItems(QVector<ImageInput>* const items);

  Qt::ItemFlags flags (const QModelIndex &index ) const;

  void Refresh();

  QVariant headerData (int section , Qt::Orientation orientation, int role=Qt::DisplayRole )const;

protected:
  QVector<ImageInput>* Items;

};

#endif
