#include "ListModelImageInput.h"

#include <iostream>

void ListModelImageInput::setItems(QVector<ImageInput>* const items)
{
  emit beginResetModel();
  this->Items = items;
  emit endResetModel();
}

void ListModelImageInput::Refresh()
{
  emit beginResetModel();
  emit endResetModel();
}

Qt::ItemFlags ListModelImageInput::flags (const QModelIndex  &index ) const
{
  if (index.row() < 0 || index.row() >= rowCount() || !index.isValid())
    {
    return Qt::NoItemFlags;
    }
  //return Qt::ItemIsUserCheckable | Qt::ItemIsEditable | Qt::ItemIsEnabled; // ItemIsEditable will let you change the text. You can still change the check boxes even without ItemIsEditable.
  return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
}

int ListModelImageInput::rowCount(const QModelIndex& parent) const
{
  return this->Items->size();
}
