#ifndef ForwardLookTableModel_H
#define ForwardLookTableModel_H

// Qt
#include <QAbstractTableModel>
#include <QItemSelection>

// STL
#include <vector>

// Custom
#include "CandidatePairs.h"

class ForwardLookTableModel : public QAbstractTableModel
{
public:
  ForwardLookTableModel(std::vector<std::vector<CandidatePairs> >& allCandidatePairs);
  
  int rowCount(const QModelIndex& parent) const;
  int columnCount(const QModelIndex& parent) const;
  QVariant data(const QModelIndex& index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  
  Qt::ItemFlags flags(const QModelIndex& index) const;
  void SetImage(FloatVectorImageType::Pointer image);
  void SetIterationToDisplay(const unsigned int);
  void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
  
  void Refresh();
  
protected:
  
  std::vector<std::vector<CandidatePairs> >& AllCandidatePairs;
  
  FloatVectorImageType::Pointer Image;
  unsigned int IterationToDisplay;
};

#endif
