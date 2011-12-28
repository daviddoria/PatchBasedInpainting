#ifndef ListModelImageInput_H
#define ListModelImageInput_H

#include <QAbstractListModel>
#include <QModelIndex>
#include <QVariant>
#include <QVector>

#include "ImageInput.h"

class ListModelImageInput : public QAbstractListModel
{
public:
  // QAbstractListModel interface
  virtual QVariant data(const QModelIndex& index, int role) const = 0;

  virtual bool setData (const QModelIndex &index, const QVariant &value, int role) = 0;

  int rowCount(const QModelIndex  &parent=QModelIndex() ) const;

  Qt::ItemFlags flags (const QModelIndex &index ) const;

  // Custom functions
  void setItems(QVector<ImageInput>* const items);

  void Refresh();

protected:
  QVector<ImageInput>* Items;

};

#endif
