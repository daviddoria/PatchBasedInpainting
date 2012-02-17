/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "PixmapDelegate.h"

#include <QPainter>

#include <iostream>


PixmapDelegate::PixmapDelegate() : Padding(4)
{

}

void PixmapDelegate::SetPadding(const unsigned int padding)
{
  this->Padding = padding;
}

void PixmapDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  QStyledItemDelegate::paint(painter, option, index);

  QPixmap pixmap = index.data(Qt::DisplayRole).value<QPixmap>();

  QRect rect = option.rect;

  unsigned int originalWidth = rect.width();
  unsigned int originalHeight = rect.height();

  int minSize = std::min(rect.width(), rect.height()) - Padding*2; // We have to double the padding because we want it taken off from both sides.

  // These setLeft and setTop calls must come before setHeight and setWidth
  rect.setLeft(originalWidth/2 - minSize/2 + Padding);
  rect.setTop(rect.top() + originalHeight/2 - minSize/2 + Padding);

  rect.setHeight(minSize);
  rect.setWidth(minSize);

  painter->drawPixmap(rect, pixmap, pixmap.rect());
}
