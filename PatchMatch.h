#ifndef __PatchMatch_h
#define __PatchMatch_h

#include "itkImageToImageFilter.h"

namespace itk
{
template< typename TImage, typename TMask>
class PatchMatch : public ImageToImageFilter< TImage, TImage >
{
public:
  /** Standard class typedefs. */
  typedef PatchMatch             Self;
  typedef ImageToImageFilter< TImage, TImage > Superclass;
  typedef SmartPointer< Self >        Pointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(PatchMatch, ImageToImageFilter);

  /** The image in which to find the best matching patch to 'Patch'.*/
  void SetInputImage(const TImage* image);

  /** The patch to be matched.*/
  void SetInputPatch(const TImage* patch);
  
  /** The mask to be inpainted. White pixels will be inpainted, black pixels will be passed through to the output.*/
  void SetInputMask(const TMask* mask);

protected:
  PatchMatch();
  ~PatchMatch(){}

  typename TImage::ConstPointer GetInputImage();
  typename TMask::ConstPointer GetInputMask();
  typename TImage::ConstPointer GetInputPatch();

  /** Does the real work. */
  virtual void GenerateData();

private:
  PatchMatch(const Self &); //purposely not implemented
  void operator=(const Self &);  //purposely not implemented

};
} //namespace ITK


#ifndef ITK_MANUAL_INSTANTIATION
#include "PatchMatch.txx"
#endif


#endif // __PatchMatch_h