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

/**
\class Mask
\brief This class is a subclass of itkImage that provides the concept of "valid" pixels
       and hole pixels. Pixels that are any other value are never used in computations.
       Using itkImageFileReader, the first channel of any input image will be attempted
       to be converted to a Mask. NOTE: If the image is a 4 channel image where the 4th
       channel represents alpha, the reader sometimes produces a blank image. Ideally
       a 3 channel grayscale (all channels are the same) or 1 channel image is used
       as input.
*/

#ifndef MASK_H
#define MASK_H

// ITK
#include "itkImage.h"

class vtkImageData;

// Custom
#include "ImageTypes.h"

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

  /** Determine if a pixel is a hole pixel.*/
  bool IsHole(const itk::Index<2>& index) const;

  /** Determine if an entire region consists of hole pixels.*/
  bool IsHole(const itk::ImageRegion<2>& region) const;
  
  /** Determine if an entire region is valid.*/
  bool IsValid(const itk::ImageRegion<2>& region) const;

  /** Determine if a pixel is valid.*/
  bool IsValid(const itk::Index<2>& index) const;

  /** Invert the mask by switching the hole and valid pixel values.*/
  void Invert();

  /** Snap the pixel values to either 'hole' or 'valid'.*/
  void Cleanup();

  /** Slightly dilate the hole.*/
  void ExpandHole();

  /** Specify which value should be considered a hole.*/
  void SetHoleValue(const unsigned char value);

  /** Specify which value should be considered valid.*/
  void SetValidValue(const unsigned char value);

  /** Get the value that is considered a hole.*/
  unsigned char GetHoleValue() const;

  /** Get the value that is considered valid.*/
  unsigned char GetValidValue() const;

  /** Print information about the Mask.*/
  void OutputMembers() const;

  /** Copy a mask.*/
  void DeepCopyFrom(const Mask* inputMask);

  /** Find the boundary of the Mask.*/
  typedef UnsignedCharScalarImageType BoundaryImageType;
  void FindBoundary(BoundaryImageType* const boundary) const;
  void FindBoundaryInRegion(const itk::ImageRegion<2>& region, BoundaryImageType* const boundary) const;

  /** Recolor the hole pixels in 'image' a specified 'color'.*/
  template<typename TImage, typename TColor>
  void ApplyColorToImage(const TImage* const image, const TColor& color) const;

  /** Change the hole pixels in 'image' to a specified 'holeValue'.*/
  template<typename TImage>
  void ApplyToImage(TImage* const image, const typename TImage::PixelType& holeValue) const;

  /** Recolor the hole pixels in 'image' a specified 'color'.*/
  template<typename TImage, typename TColor>
  void ApplyToVectorImage(TImage* const image, const TColor& color)const ;

  /** Create a VTK image from the mask.*/
  template<typename TColor>
  void MakeVTKImage(vtkImageData* const image, const TColor& validColor, const TColor& holeColor, const bool holeTransparent, const bool validTransparent) const;

  /** Create a mask from a mask image.*/
  template<typename TImage>
  void CreateFromImage(const TImage* const image, const typename TImage::PixelType& holeColor);

  /** Get a list of the valid neighbors of a pixel.*/
  std::vector<itk::Index<2> > GetValidNeighbors(const itk::Index<2>& pixel) const;

  /** Determine if a pixel has at least 1 hole neighbor.*/
  bool HasHoleNeighbor(const itk::Index<2>& pixel) const;
  
  /** Get a list of the hole neighbors of a pixel.*/
  std::vector<itk::Index<2> > GetHoleNeighbors(const itk::Index<2>& pixel) const;

  /** Get a list of the offsets of the valid neighbors of a pixel.*/
  std::vector<itk::Offset<2> > GetValidNeighborOffsets(const itk::Index<2>& pixel) const;

  /** Get a list of the offsets of the hole neighbors of a pixel.*/
  std::vector<itk::Offset<2> > GetHoleNeighborOffsets(const itk::Index<2>& pixel) const;
  
  /** Get a list of the valid pixels in a region.*/
  std::vector<itk::Index<2> > GetValidPixelsInRegion(itk::ImageRegion<2> region) const;
  
  /** Get a list of the hole pixels in a region.*/
  std::vector<itk::Index<2> > GetHolePixelsInRegion(itk::ImageRegion<2> region) const;

  /** Get a list of the offsets of the valid pixels in a region.*/
  std::vector<itk::Offset<2> > GetValidOffsetsInRegion(itk::ImageRegion<2> region) const;

  /** Get a list of the offsets of the hole pixels in a region.*/
  std::vector<itk::Offset<2> > GetHoleOffsetsInRegion(itk::ImageRegion<2> region) const;
  
  /** Count hole pixels in a region.*/
  unsigned int CountHolePixels(const itk::ImageRegion<2>& region) const;
  
  /** Count hole pixels in the whole mask.*/
  unsigned int CountHolePixels() const;

  /** Count valid pixels in a region.*/
  unsigned int CountValidPixels(const itk::ImageRegion<2>& region) const;
  
  /** Count valid pixels in the whole mask.*/
  unsigned int CountValidPixels() const;

  /** Read the mask from a file.*/
  void Read(const std::string& filename);

  /** Mark the pixel as a hole.*/
  void MarkAsHole(const itk::Index<2>& pixel);

  /** Mark the pixel as a valid pixel.*/
  void MarkAsValid(const itk::Index<2>& pixel);

private:

  Mask(const Self &);    //purposely not implemented
  void operator=(const Self &); //purposely not implemented

  Mask();

  unsigned char HoleValue; // Pixels with this value will be filled.
  unsigned char ValidValue; // Pixels with this value will not be filled - they are the source region.

};

#include "Mask.hxx"

#endif
