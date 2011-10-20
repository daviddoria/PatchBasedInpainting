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

/*
 * This class is a subclass of itkImage that provides the concept of "valid" pixels
 * and "hole" pixels. Pixels that are any other value are never used in computations.
 */

#ifndef MASK_H
#define MASK_H

#include "itkImage.h"
#include "itkImageRegionIterator.h"

#include <QColor>

#include <vtkImageData.h>

class Mask : public itk::Image< unsigned char, 2>
{
  public:
  /** Standard typedefs. */
  typedef Mask                       Self;
  typedef itk::Image< unsigned char, 2> Superclass;
  typedef itk::SmartPointer< Self >              Pointer;
  typedef itk::SmartPointer< const Self >        ConstPointer;
  typedef itk::WeakPointer< const Self >         ConstWeakPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(Mask, Image);

  /** Dimension of the image. */
  itkStaticConstMacro(ImageDimension, unsigned int,
                      Superclass::ImageDimension);

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
  
  bool IsHole(const itk::Index<2>& index) const;
    
  bool IsValid(const itk::ImageRegion<2>& region) const;

  bool IsValid(const itk::Index<2>& index) const;
  
  void Invert();

  void Cleanup();
  
  void ExpandHole();
  
  void SetHoleValue(const unsigned char value);

  void SetValidValue(const unsigned char value);

  unsigned char GetHoleValue() const;
  
  unsigned char GetValidValue() const;
  
  void OutputMembers() const;
  
  void DeepCopyFrom(const Mask::Pointer inputMask);
  
  template<typename TImage>
  void ApplyToImage(const typename TImage::Pointer image, const QColor& color);
  
  void MakeVTKImage(vtkImageData* image, const QColor& validColor, const QColor& holeColor, const bool holeTransparent, const bool validTransparent);
  
protected:
  Mask();
  
  unsigned char HoleValue; // Pixels with this value will be filled.
  unsigned char ValidValue; // Pixels with this value will not be filled - they are the source region.
  
private:

  Mask(const Self &);    //purposely not implemented
  void operator=(const Self &); //purposely not implemented
};

#include "Mask.hxx"

#endif
