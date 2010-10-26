#ifndef PriorityFunctor_h
#define PriorityFunctor_h

#include "itkImage.h"
#include "itkCovariantVector.h"
#include "itkRigid2DTransform.h"

  ImageType

class PriorityFunctor
{
  // Operates on an unsigned char image and produces a float pixel
public:
  PriorityFunctor() {};
  ~PriorityFunctor() {};
  bool operator!=( const PriorityFunctor & ) const
    {
    return false;
    }
  bool operator==( const PriorityFunctor & other ) const
    {
    return !(*this != other);
    }
  inline itk::Image<float, 2>::PixelType operator()( const itk::Image<unsigned char, 2>::PixelType & A ) const
    {

      return transformedVector;
    }
};

/*
class PriorityFunctor
{
public:
  typedef itk::Image< itk::CovariantVector<float, 2>, 2 > VectorImageType;
  typedef itk::Image< unsigned char, 2 > ImageType;

  PriorityFunctor() {};

  // Priorities will only be computed at pixels which are non zero in the mask
  void SetMask(ImageType::Pointer mask) {this->Mask = mask;}

  void Compute();

private:
  ImageType::Pointer Mask;

  double ComputeDataTerm(ImageType::IndexType pixel);

  double ComputeConfidenceTerm(ImageType::IndexType pixel);

  double ComputePriority(ImageType::IndexType pixel);

};



double Priorities::ComputeDataTerm(ImageType::IndexType pixel)
{
  double alpha = 255; // for grey scale images
  // D(p) = |dot(isophote direction at p, normal of the front at p)|/alpha
}

double Priorities::ComputeConfidenceTerm(ImageType::IndexType pixel)
{
// sum of the confidences of patch pixels in the source region / area of the patch
}

double Priorities::ComputePriority(int x, int y)
{
  return ComputeConfidenceTerm(x, y) + ComputeDataTerm(x, y);
}

*/

#endif