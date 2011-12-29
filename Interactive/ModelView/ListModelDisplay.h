#ifndef ListModelDisplay_H
#define ListModelDisplay_H

#include "ListModelImageInput.h"

#include <QVariant>

class InpaintingIterationRecord;

// This model is driven by an InpaintingIterationRecord data.

class ListModelDisplay : public QAbstractListModel
{
public:

  ListModelDisplay(QObject * parent = 0);

  // QAbstractListModel interface
  QVariant data(const QModelIndex& index, int role) const;

  bool setData (const QModelIndex &index, const QVariant &value, int role);

  int rowCount(const QModelIndex  &parent=QModelIndex() ) const;

  void setItems(QVector<ImageInput>* const items);

  Qt::ItemFlags flags (const QModelIndex &index ) const;

  // Display model custom functions
  void Refresh();
  void SetIterationRecord(InpaintingIterationRecord* const);

protected:
  InpaintingIterationRecord* IterationRecord;
};

#endif
