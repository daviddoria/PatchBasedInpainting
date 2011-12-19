#include "ListModelSave.h"

#include <iostream>

QVariant ListModelSave::data (const QModelIndex  &index , int role ) const
{
  if (index.row() < 0 || index.row() >= rowCount() || !index.isValid())
    {
    return QVariant();
    }

  if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
    std::cout << "ListModelSave Name " << index.row() << " " << this->Items->at(index.row()).Name.toStdString() << std::endl;
    return this->Items->at(index.row()).Name;
    }

  if(role == Qt::CheckStateRole)
    {
    return this->Items->at(index.row()).Save;
    }

  return QVariant();
}

bool ListModelSave::setData (const QModelIndex &index, const QVariant &value, int role)
{
  if (index.row() < 0 || index.row() >= rowCount() || !index.isValid())
    {
    return false;
    }

  if(role == Qt::CheckStateRole)
    {
    (*this->Items)[index.row()].Save = static_cast<Qt::CheckState>(value.toUInt());
    }

  emit dataChanged(index, index);
  return true;
}
