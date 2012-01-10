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

#ifndef ImageInput_H
#define ImageInput_H

#include <QString>
#include <QVector>
#include <iostream>

// This class stores the name of an image and the filename that it was loaded from.
class ImageInput
{
public:
  ImageInput(const QString& name = QString(), const QString& fileName = QString());

//private:

  //friend class TableModelImageInput;

  QString Name;
  QString FileName;

  static bool ImageExists(const QVector<ImageInput>&, const QString& name);
};

#endif
