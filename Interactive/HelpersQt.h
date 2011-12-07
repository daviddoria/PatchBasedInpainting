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

#ifndef HELPERS_QT_H
#define HELPERS_QT_H

// Qt
#include <QImage>
class QGraphicsView;
class QTableWidget;

// ITK
#include "itkImageRegion.h"

// Custom
#include "DisplayStyle.h"
#include "Mask.h"

namespace HelpersQt
{
// Convert a QColor to an unsigned char[3]
void QColorToUCharColor(const QColor& color, unsigned char outputColor[3]);

// Get a columns location in the table based on its header string
bool GetColumnIdByHeader(const QTableWidget* table, const std::string& header, int& columnId);

// Scale an image so that it fits in a QGraphicsView
QImage FitToGraphicsView(const QImage qimage, const QGraphicsView* gfx);

////////////////////////////////////
///////// Function templates (defined in HelpersQt.hxx) /////////
////////////////////////////////////
template <typename TImage>
QImage GetQImage(const TImage* image, const itk::ImageRegion<2>& region, const DisplayStyle& style);

template <typename TImage>
QImage GetQImageColor(const TImage* image, const itk::ImageRegion<2>& region);

template <typename TImage>
QImage GetQImageMagnitude(const TImage* image, const itk::ImageRegion<2>& region);

template <typename TImage>
QImage GetQImageScalar(const TImage* image, const itk::ImageRegion<2>& region);

template <typename TImage>
QImage GetQImageMasked(const TImage* image, const Mask::Pointer mask, const itk::ImageRegion<2>& region);

} // end namespace

#include "HelpersQt.hxx"

#endif
