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

#ifndef RotateVectors_h
#define RotateVectors_h

#include "itkImage.h"
#include "itkCovariantVector.h"
#include "itkRigid2DTransform.h"

template< class TInput, class TOutput>
class RotateVectors
{
public:
  RotateVectors() {};
  ~RotateVectors() {};
  bool operator!=( const RotateVectors & ) const
    {
    return false;
    }
  bool operator==( const RotateVectors & other ) const
    {
    return !(*this != other);
    }
  inline TOutput operator()( const TInput & A ) const
    {
      typedef itk::Vector<double, 2> VectorType;
      VectorType v;
      v[0] = A[0];
      v[1] = A[1];

      typedef itk::Rigid2DTransform< float > TransformType;

      TransformType::Pointer transform = TransformType::New();
      transform->SetAngle(M_PI/2.0);

      VectorType outputV = transform->TransformVector(v);
      TOutput transformedVector;
      transformedVector[0] = outputV[0];
      transformedVector[1] = outputV[1];

      return transformedVector;
    }
};

#endif