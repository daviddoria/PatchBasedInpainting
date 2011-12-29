#include "ListModelSave.h"

#include <iostream>


ListModelSave::ListModelSave( QObject * parent) : QAbstractListModel(parent)
{

}

void ListModelSave::Clear()
{
  emit beginResetModel();
  this->Saved.clear();
  this->Names.clear();
  emit endResetModel();
}

void ListModelSave::Add(const QString& name, const Qt::CheckState saved)
{
  emit beginResetModel();
  this->Saved.push_back(saved);
  this->Names.push_back(name);
  emit endResetModel();
}

void ListModelSave::Refresh()
{
  emit beginResetModel();
  emit endResetModel();
}

Qt::ItemFlags ListModelSave::flags (const QModelIndex  &index ) const
{
  if (index.row() < 0 || index.row() >= rowCount() || !index.isValid())
    {
    return Qt::NoItemFlags;
    }
  //return Qt::ItemIsUserCheckable | Qt::ItemIsEditable | Qt::ItemIsEnabled; // ItemIsEditable will let you change the text. You can still change the check boxes even without ItemIsEditable.
  return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
}

int ListModelSave::rowCount(const QModelIndex& parent) const
{
  return this->Names.size();
}


QVariant ListModelSave::data (const QModelIndex  &index , int role ) const
{
  if (index.row() < 0 || index.row() >= rowCount() || !index.isValid())
    {
    return QVariant();
    }

  if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
    //std::cout << "ListModelSave Name " << index.row() << " " << this->Items->at(index.row()).Name.toStdString() << std::endl;
    return this->Names[index.row()];
    }

  if(role == Qt::CheckStateRole)
    {
    return this->Saved[index.row()];
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
    this->Saved[index.row()] = static_cast<Qt::CheckState>(value.toUInt());
    }

  emit dataChanged(index, index);
  return true;
}
