#ifndef ListModelSave_H
#define ListModelSave_H

#include "ListModelImageInput.h"

#include <QVariant>

class ListModelSave : public QAbstractListModel
{
public:

  ListModelSave(QObject * parent = 0);

  // QAbstractListModel interface
  QVariant data(const QModelIndex& index, int role) const;

  bool setData (const QModelIndex &index, const QVariant &value, int role);

  int rowCount(const QModelIndex  &parent=QModelIndex() ) const;

  void setItems(QVector<ImageInput>* const items);

  Qt::ItemFlags flags (const QModelIndex &index ) const;

  // Display model custom functions
  void Refresh();

  void Clear();

  void Add(const QString& name, const Qt::CheckState saved);

protected:
  QVector<Qt::CheckState> Saved;
  QVector<QString> Names;
};

#endif
