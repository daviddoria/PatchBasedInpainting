#include "TableModelImageInput.h"

#include <iostream>

TableModelImageInput::TableModelImageInput(QObject * parent) : QAbstractTableModel(parent)
{
}

QVariant TableModelImageInput::headerData (int section , Qt::Orientation orientation, int role) const
{
  if(role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
    if(section == NAME_COLUMN)
      {
      return QString("Name");
      }
    else if(section == FILENAME_COLUMN)
      {
      return QString("FileName");
      }
    }
  return QVariant();
}

void TableModelImageInput::setItems(QVector<ImageInput>* const items)
{
  emit beginResetModel();
  this->Items = items;
  emit endResetModel();
}

void TableModelImageInput::Refresh()
{
  emit beginResetModel();
  emit endResetModel();
}

Qt::ItemFlags TableModelImageInput::flags (const QModelIndex  &index ) const
{
  if(!index.isValid())
    {
    return Qt::NoItemFlags;
    }
    
  if(index.column() == DISPLAY_COLUMN || index.column() == SAVE_COLUMN)
    {
    return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    }

  return Qt::ItemIsEnabled;
}

int TableModelImageInput::rowCount(const QModelIndex& parent) const
{
  return this->Items->size();
}

int TableModelImageInput::columnCount(const QModelIndex& parent) const
{
  return 2;
}

QVariant TableModelImageInput::data (const QModelIndex  &index , int role ) const
{
  if (index.row() < 0 || index.row() >= rowCount() || !index.isValid())
    {
    return QVariant();
    }

  if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
    //std::cout << "TableModelImageInput Name: " << this->Items->at(index.row()).Name.toStdString() << " FileName: " << this->Items->at(index.row()).FileName.toStdString() << std::endl;
    if(index.column() == NAME_COLUMN)
      {
      return this->Items->at(index.row()).Name;
      }
    else if(index.column() == FILENAME_COLUMN)
      {
      return this->Items->at(index.row()).FileName;
      }
//     else if(index.column() == DISPLAY_COLUMN)
//       {
//       return this->Items->at(index.row()).Display;
//       }
//     else if(index.column() == SAVE_COLUMN)
//       {
//       return this->Items->at(index.row()).Save;
//       }
    }

  return QVariant();
}

bool TableModelImageInput::setData (const QModelIndex &index, const QVariant &value, int role)
{
  if (index.row() < 0 || index.row() >= rowCount() || !index.isValid())
    {
    return false;
    }

  if (role == Qt::EditRole && index.column() == FILENAME_COLUMN)
    {
    (*this->Items)[index.row()].FileName = value.toString();
    }

  emit dataChanged(index, index);
  return true;
}
