/*
Copyright (C) 2010 David Doria, daviddoria@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * This class is responsible for the user interaction with the input image.
 * A vtkImageTracerWidget does most of the work, but this class appends and maintains
 * the selections.
*/

#ifndef InteractorStyleImageNoLevel_H
#define InteractorStyleImageNoLevel_H

#include <vtkInteractorStyleImage.h> // superclass

class InteractorStyleImageNoLevel : public vtkInteractorStyleImage
{
public:
  static InteractorStyleImageNoLevel* New();
  vtkTypeMacro(InteractorStyleImageNoLevel, vtkInteractorStyleImage);

  //InteractorStyleImageNoLevel();

  void OnLeftButtonDown();
  void OnLeftButtonUp();
};

#endif