#ifndef PIXMAPELEGATE_H
#define PIXMAPDELEGATE_H

#include <QStyledItemDelegate>

class PixmapDelegate : public QStyledItemDelegate
{
   Q_OBJECT
public:
  PixmapDelegate(){}
  
  void paint(QPainter *painter, const QStyleOptionViewItem &option,
                const QModelIndex &index) const;
};

#endif
