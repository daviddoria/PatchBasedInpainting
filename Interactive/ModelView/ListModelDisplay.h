#ifndef ListModelDisplay_H
#define ListModelDisplay_H

#include "ListModelImageInput.h"

#include <QVariant>

class ListModelDisplay : public ListModelImageInput
{
public:

  QVariant data(const QModelIndex& index, int role) const;

  bool setData (const QModelIndex &index, const QVariant &value, int role);

};

#endif
