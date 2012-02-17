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

#ifndef TopPatchesWidget_H
#define TopPatchesWidget_H

#include "ui_TopPatchesWidget.h"

// ITK
#include "itkImage.h"

// Qt
#include <QMainWindow>
#include <QGraphicsScene>

// Custom
#include "ImageCamera.h"
#include "ImageProcessing/Mask.h"
#include "Interactive/ModelView/ListModelPatches.h"

/** This class is necessary because a class template cannot have the Q_OBJECT macro directly. */
class TopPatchesWidgetParent : public QMainWindow, public Ui::TopPatchesWidget
{
Q_OBJECT

public slots:

  //virtual void slot_UpdateImage() = 0;

};

/** This class displays a set of patches in an ordered list. */
template <typename TImage>
class TopPatchesWidget : public TopPatchesWidgetParent
{
private:
  /** The image that will be displayed, and the from which the patches will be extracted before being displayed. */
  TImage* Image;

public:
  // Constructor
  TopPatchesWidget(TImage* const image);

private:

  void SetupScenes();

  QGraphicsScene* SourcePatchScene;

  // The color to use as the background of the QGraphicsScenes
  QColor SceneBackground;

  // Connect all signals and slots.
  void SetupConnections();

  ListModelPatches<TImage>* PatchesModel;
};

#include "TopPatchesWidget.hpp"

#endif // TopPatchesWidget_H
