#include "ListModelDisplay.h"

#include "InpaintingIterationRecord.h"

#include <iostream>

ListModelDisplay::ListModelDisplay(QObject * parent) : QAbstractListModel(parent), IterationRecord(NULL)
{
  
}

void ListModelDisplay::SetIterationRecord(InpaintingIterationRecord* const iterationRecord)
{
  emit beginResetModel();
  this->IterationRecord = iterationRecord;
  emit endResetModel();
}

void ListModelDisplay::Refresh()
{
  emit beginResetModel();
  emit endResetModel();
}

Qt::ItemFlags ListModelDisplay::flags (const QModelIndex  &index ) const
{
  if (index.row() < 0 || index.row() >= rowCount() || !index.isValid())
    {
    return Qt::NoItemFlags;
    }
  //return Qt::ItemIsUserCheckable | Qt::ItemIsEditable | Qt::ItemIsEnabled; // ItemIsEditable will let you change the text. You can still change the check boxes even without ItemIsEditable.
  return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
}

int ListModelDisplay::rowCount(const QModelIndex& parent) const
{
  if(!this->IterationRecord)
    {
    return 0;
    }

  return this->IterationRecord->GetNumberOfImages();
}

QVariant ListModelDisplay::data (const QModelIndex  &index , int role ) const
{
  if (index.row() < 0 || index.row() >= rowCount() || !index.isValid() || !this->IterationRecord)
    {
    return QVariant();
    }

  if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
    //std::cout << "ListModelDisplay Name " << index.row() << " " << this->Items->at(index.row()).Name.toStdString() << std::endl;
    return this->IterationRecord->GetImage(index.row()).Name.c_str();
    }

  if(role == Qt::CheckStateRole)
    {
    return this->IterationRecord->IsDisplayed(index.row());
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
    this->IterationRecord->SetDisplayed(index.row(), static_cast<Qt::CheckState>(value.toUInt()));
    }

  emit dataChanged(index, index);
  return true;
}
