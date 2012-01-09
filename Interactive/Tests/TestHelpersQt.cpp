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

#include "HelpersQt.h"

#include "Testing.h"

static void TestQColorToUCharColor();
static void TestGetColumnIdByHeader();
static void TestFitToGraphicsView();
static void TestGetQImage();
static void TestGetQImageColor();
static void TestGetQImageMagnitude();
static void TestGetQImageScalar();
static void TestGetQImageMasked();

int main(int argc, char*argv[])
{
  TestQColorToUCharColor();
  TestGetColumnIdByHeader();
  TestFitToGraphicsView();
  TestGetQImage();
  TestGetQImageColor();
  TestGetQImageMagnitude();
  TestGetQImageScalar();
  TestGetQImageMasked();

  return EXIT_SUCCESS;
}

void TestQColorToUCharColor()
{
  QColor qcolor;
  qcolor.setRed(1);
  qcolor.setGreen(2);
  qcolor.setBlue(3);

  unsigned char correctColorArray[3] = {1,2,3};
  unsigned char retrievedColorArray[3];

  HelpersQt::QColorToUCharColor(qcolor, retrievedColorArray);

  if(!Testing::ArraysEqual(correctColorArray, retrievedColorArray, 3))
    {
    
    }
}

void TestGetColumnIdByHeader()
{


// // Get a columns location in the table based on its header string
// bool GetColumnIdByHeader(const QTableWidget* table, const std::string& header, int& columnId);
//
}

void TestFitToGraphicsView()
{

// // Scale an image so that it fits in a QGraphicsView
// QImage FitToGraphicsView(const QImage qimage, const QGraphicsView* gfx);
//
}

void TestGetQImage()
{

// ////////////////////////////////////
// ///////// Function templates (defined in HelpersQt.hxx) /////////
// ////////////////////////////////////
// template <typename TImage>
// QImage GetQImage(const TImage* image, const itk::ImageRegion<2>& region, const DisplayStyle& style);

}

void TestGetQImageColor()
{
//
// template <typename TImage>
// QImage GetQImageColor(const TImage* image, const itk::ImageRegion<2>& region);

}

void TestGetQImageMagnitude()
{
//
// template <typename TImage>
// QImage GetQImageMagnitude(const TImage* image, const itk::ImageRegion<2>& region);

}

void TestGetQImageScalar()
{
//
// template <typename TImage>
// QImage GetQImageScalar(const TImage* image, const itk::ImageRegion<2>& region);

}

void TestGetQImageMasked()
{
//
// template <typename TImage>
// QImage GetQImageMasked(const TImage* image, const Mask::Pointer mask, const itk::ImageRegion<2>& region);

}
