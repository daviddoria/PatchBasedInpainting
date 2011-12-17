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

// ITK
#include "itkImage.h"

// Qt
//#include <QColor>

// VTK
//#include <vtkImageData.h>
class vtkImageData;

// Custom
#include "DebugOutputs.h"
#include "Types.h"

class Mask : public itk::Image< unsigned char, 2>, public DebugOutputs
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

  // Determine if a pixel is a hole pixel.
  bool IsHole(const itk::Index<2>& index) const;

  // Determine if an entire region is valid.
  bool IsValid(const itk::ImageRegion<2>& region) const;

  // Determine if a pixel is valid.
  bool IsValid(const itk::Index<2>& index) const;

  // Determine if any of a pixels 8 neighbors are holes.
  bool HasHoleNeighbor(const itk::Index<2>& pixel);

  // Look from a pixel across the hole in a specified direction and return the pixel that exists on the other side of the hole.
  itk::Index<2> FindPixelAcrossHole(const itk::Index<2>& queryPixel, const FloatVector2Type& direction) const;

  // Invert the mask by switching the hole and valid pixel values.
  void Invert();

  // Snap the pixel values to either 'hole' or 'valid'.
  void Cleanup();

  // Slightly dilate the hole.
  void ExpandHole();

  // Specify which value should be considered a hole.
  void SetHoleValue(const unsigned char value);

  // Specify which value should be considered valid.
  void SetValidValue(const unsigned char value);

  // Get the value that is considered a hole.
  unsigned char GetHoleValue() const;

  // Get the value that is considered valid.
  unsigned char GetValidValue() const;

  // Print information about the Mask.
  void OutputMembers() const;

  // Copy a mask.
  void DeepCopyFrom(const Mask* inputMask);

  // Find the boundary of the Mask.
  void FindBoundary(UnsignedCharScalarImageType* boundary) const;

  // Recolor the hole pixels in 'image' a specified 'color'.
  template<typename TImage, typename TColor>
  void ApplyColorToImage(const typename TImage::Pointer image, const TColor& color);

  // Change the hole pixels in 'image' to a specified 'holeValue'.
  template<typename TImage>
  void ApplyToImage(TImage* image, const typename TImage::PixelType& holeValue);

  // Recolor the hole pixels in 'image' a specified 'color'.
  template<typename TImage, typename TColor>
  void ApplyToVectorImage(TImage* image, const TColor& color);

  template<typename TColor>
  void MakeVTKImage(vtkImageData* image, const TColor& validColor, const TColor& holeColor, const bool holeTransparent, const bool validTransparent);

  // Create a mask from a mask image.
  template<typename TImage>
  void CreateFromImage(const TImage* image, const typename TImage::PixelType& holeColor);

  std::vector<itk::Index<2> > GetValidPixelsInRegion(const itk::ImageRegion<2>& region);
  std::vector<itk::Index<2> > GetHolePixelsInRegion(const itk::ImageRegion<2>& region);

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
