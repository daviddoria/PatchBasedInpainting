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
#include <QMetaType>
class QGraphicsView;
class QTableWidget;

// ITK
#include "itkImageRegion.h"

// Custom
#include "DisplayStyle.h"
#include "ImageProcessing/Mask.h"

Q_DECLARE_METATYPE(itk::ImageRegion<2>)

namespace HelpersQt
{
/** Convert a QColor to an unsigned char[3] */
void QColorToUCharColor(const QColor& color, unsigned char outputColor[3]);

/** Get a columns location in the table based on its header string */
bool GetColumnIdByHeader(const QTableWidget* table, const std::string& header, int& columnId);

/** Scale an image so that it fits in a QGraphicsView */
QImage FitToGraphicsView(const QImage qimage, const QGraphicsView* gfx);

/** Change the center pixel (in-place) to the specified 'color' */
void HighlightCenterPixel(QImage& qimage, const QColor& color);

////////////////////////////////////
///////// Function templates (defined in HelpersQt.hxx) /////////
////////////////////////////////////
template <typename TImage>
QImage GetQImage(const TImage* const image, const itk::ImageRegion<2>& region, const DisplayStyle& style);

template <typename TImage>
QImage GetQImageColor(const TImage* const image, const itk::ImageRegion<2>& region);

template <typename TImage>
QImage GetQImageMagnitude(const TImage* const image, const itk::ImageRegion<2>& region);

template <typename TImage>
QImage GetQImageScalar(const TImage* const image, const itk::ImageRegion<2>& region);

template <typename TImage>
QImage GetQImageChannel(const TImage* const image, const itk::ImageRegion<2>& region, const unsigned int channel);

/** Convert an image to a QImage, but changed the corresponding masked pixels to the specified 'color'.*/
template <typename TImage>
QImage GetQImageMasked(const TImage* const image, const Mask* const mask,
                       const itk::ImageRegion<2>& region, const QColor& color = QColor(0, 255, 0));

/** Convert an image to a QImage, but changed the pixels from 'image' in 'imageRegion' to 'color' if the corresponding mask pixels in "maskRegion" are masked.*/
template <typename TImage>
QImage GetQImageMasked(const TImage* const image, const itk::ImageRegion<2>& imageRegion, const Mask* const mask, const itk::ImageRegion<2>& maskRegion, const QColor& color = QColor(0, 255, 0));

} // end namespace

#include "HelpersQt.hxx"

#endif
