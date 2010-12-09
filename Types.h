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

#ifndef Types_H
#define Types_H

#include "itkImage.h"
#include "itkCovariantVector.h"

typedef itk::Image<float,2> FloatImageType;
typedef itk::Image<unsigned char,2> UnsignedCharImageType;
typedef itk::Image<itk::CovariantVector<unsigned char, 3> ,2> ColorImageType;

#endif