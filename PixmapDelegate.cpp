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

void PixmapDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  QPixmap pixmap = index.data(Qt::DisplayRole).value<QPixmap>();
  //std::cout << pixmap.width() << " " << pixmap.height() << std::endl;
  //std::cout << "rect: " << option.rect.width() << " " << option.rect.height() << std::endl;

  QRect rect = option.rect;
  //rect.adjust(rect.width()/3, 0, -rect.width()/3, 0);
  painter->drawPixmap(rect, pixmap, pixmap.rect());
}
