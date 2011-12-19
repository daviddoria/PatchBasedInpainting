#include "ListModelDisplay.h"

#include <iostream>

QVariant ListModelDisplay::data (const QModelIndex  &index , int role ) const
{
  if (index.row() < 0 || index.row() >= rowCount() || !index.isValid())
    {
    return QVariant();
    }

  if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
    std::cout << "ListModelDisplay Name " << index.row() << " " << this->Items->at(index.row()).Name.toStdString() << std::endl;
    return this->Items->at(index.row()).Name;
    }

  if(role == Qt::CheckStateRole)
    {
    return this->Items->at(index.row()).Display;
    }

  return QVariant();
}

bool ListModelDisplay::setData (const QModelIndex &index, const QVariant &value, int role)
{
  if (index.row() < 0 || index.row() >= rowCount() || !index.isValid())
    {
    return false;
    }

  if(role == Qt::CheckStateRole)
    {
    (*this->Items)[index.row()].Display = static_cast<Qt::CheckState>(value.toUInt());
    }

  emit dataChanged(index, index);
  return true;
}
