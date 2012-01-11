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

#ifndef Image_H
#define Image_H

// ITK
#include "itkImage.h"

class vtkImageData;

// Custom
#include "Types.h"

/**
\class ImageParent
\brief This class provides a way to store heterogeneous images in a container.
*/
class ImageParent
{
public:
  std::string GetName();

private:
  /** The name of the image. */
  std::string Name;
};

template <typename TPixel>
class Image : public itk::VectorImage<TPixel, 2>, public ImageParent
{
public:

  /** Standard typedefs. */
  typedef Image                                  Self;
  typedef itk::VectorImage<TPixel, 2>            Superclass;
  typedef itk::SmartPointer< Self >              Pointer;
  typedef itk::SmartPointer< const Self >        ConstPointer;
  typedef itk::WeakPointer< const Self >         ConstWeakPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(Image, VectorImage);

  /** Types derived from the Superclass */
  typedef typename Superclass::IndexType IndexType;

  typedef typename Superclass::IOPixelType IOPixelType;

  /** Tyepdef for the functor used to access a neighborhood of pixel
  * pointers. */
  typedef itk::NeighborhoodAccessorFunctor< Self >
  NeighborhoodAccessorFunctorType;

  /** Return the NeighborhoodAccessor functor. This method is called by the
   * neighborhood iterators. */
  NeighborhoodAccessorFunctorType GetNeighborhoodAccessor()
  { return NeighborhoodAccessorFunctorType(); }

  /** Return the NeighborhoodAccessor functor. This method is called by the
   * neighborhood iterators. */
  const NeighborhoodAccessorFunctorType GetNeighborhoodAccessor() const
  { return NeighborhoodAccessorFunctorType(); }


private:

  Image(const Self &);    //purposely not implemented
  void operator=(const Self &); //purposely not implemented

  Image(){}

};

#include "Image.hxx"

#endif
