#ifndef ListModelSave_H
#define ListModelSave_H

#include "ListModelImageInput.h"

class ListModelSave : public ListModelImageInput
{
public:

  QVariant data(const QModelIndex& index, int role) const;

  bool setData (const QModelIndex &index, const QVariant &value, int role);

};

#endif
