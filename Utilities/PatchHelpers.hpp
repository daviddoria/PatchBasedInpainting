#ifndef PatchHelpers_HPP
#define PatchHelpers_HPP

#include "PatchHelpers.h"

// Submodules
#include <ITKHelpers/ITKHelpers.h>

// STL
#include <cassert>
#include <stdexcept>

// Qt
#include <QColor>

namespace PatchHelpers
{

template <typename TNodeQueue, typename TPropertyMap>
void WriteValidQueueNodesLocationsImage(TNodeQueue nodeQueue, const TPropertyMap propertyMap,
                                       const itk::ImageRegion<2>& fullRegion, const std::string& fileName)
{
  typedef itk::Image<unsigned char, 2> ImageType;
  ImageType::Pointer image = ImageType::New();
  image->SetRegions(fullRegion);
  image->Allocate();
  image->FillBuffer(0);

  while(!nodeQueue.empty())
  {
    typename TNodeQueue::value_type queuedNode = nodeQueue.top();

    bool valid = get(propertyMap, queuedNode);
    if(valid)
    {
      itk::Index<2> index = Helpers::ConvertFrom<itk::Index<2>, typename TNodeQueue::value_type>(queuedNode);
      image->SetPixel(index, 255);
    }

    nodeQueue.pop();
  }

  ITKHelpers::WriteImage(image.GetPointer(), fileName);
}

template <typename TNodeQueue, typename TPropertyMap>
void WriteValidQueueNodesPrioritiesImage(TNodeQueue nodeQueue, const TPropertyMap propertyMap,
                                         const itk::ImageRegion<2>& fullRegion, const std::string& fileName)
{
  typedef itk::Image<float, 2> ImageType;
  ImageType::Pointer image = ImageType::New();
  image->SetRegions(fullRegion);
  image->Allocate();
  image->FillBuffer(0);

  while(!nodeQueue.empty())
  {
    typename TNodeQueue::value_type queuedNode = nodeQueue.top();

    bool valid = get(propertyMap, queuedNode);
    if(valid)
    {
      itk::Index<2> index = Helpers::ConvertFrom<itk::Index<2>, typename TNodeQueue::value_type>(queuedNode);
      image->SetPixel(index, get(nodeQueue.keys(), queuedNode));
    }

    nodeQueue.pop();
  }

  ITKHelpers::WriteImage(image.GetPointer(), fileName);
}

template <class TPriorityQueue>
void WritePriorityQueue(TPriorityQueue q, const std::string& fileName)
{
  std::ofstream fout(fileName.c_str());

  while(!q.empty())
  {
    float priority = get(q.keys(), q.top());
    fout << priority << std::endl;
    q.pop();
  }
}

template <typename TImage>
QImage GetQImageCombinedPatch(const TImage* const image, const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion, const Mask* const mask)
{
  assert(sourceRegion.GetSize() == targetRegion.GetSize());

  QImage qimage(sourceRegion.GetSize()[0], sourceRegion.GetSize()[1], QImage::Format_RGB888);

  typedef itk::RegionOfInterestImageFilter<TImage, TImage> RegionOfInterestImageFilterType;

  typename RegionOfInterestImageFilterType::Pointer sourcePatchExtractor = RegionOfInterestImageFilterType::New();
  sourcePatchExtractor->SetRegionOfInterest(sourceRegion);
  sourcePatchExtractor->SetInput(image);
  sourcePatchExtractor->Update();

  typename RegionOfInterestImageFilterType::Pointer targetPatchExtractor = RegionOfInterestImageFilterType::New();
  targetPatchExtractor->SetRegionOfInterest(targetRegion);
  targetPatchExtractor->SetInput(image);
  targetPatchExtractor->Update();

  typedef itk::RegionOfInterestImageFilter<Mask, Mask> RegionOfInterestMaskFilterType;
  typename RegionOfInterestMaskFilterType::Pointer regionOfInterestMaskFilter = RegionOfInterestMaskFilterType::New();
  regionOfInterestMaskFilter->SetRegionOfInterest(targetRegion);
  regionOfInterestMaskFilter->SetInput(mask);
  regionOfInterestMaskFilter->Update();

  itk::ImageRegionIterator<TImage> sourcePatchIterator(sourcePatchExtractor->GetOutput(),
                                                       sourcePatchExtractor->GetOutput()->GetLargestPossibleRegion());

  itk::ImageRegionIterator<TImage> targetPatchIterator(targetPatchExtractor->GetOutput(),
                                                       targetPatchExtractor->GetOutput()->GetLargestPossibleRegion());

  while(!sourcePatchIterator.IsAtEnd())
    {
    itk::Index<2> index = targetPatchIterator.GetIndex();

    typename TImage::PixelType pixel;
    if(regionOfInterestMaskFilter->GetOutput()->IsHole(index))
      {
      pixel = sourcePatchIterator.Get();
      }
    else
      {
      pixel = targetPatchIterator.Get();
      }

    QColor pixelColor(static_cast<int>(pixel[0]), static_cast<int>(pixel[1]), static_cast<int>(pixel[2]));
    qimage.setPixel(index[0], index[1], pixelColor.rgb());

    ++targetPatchIterator;
    ++sourcePatchIterator;
    }

  // std::cout << "There were " << numberOfHolePixels << " hole pixels." << std::endl;

  //return qimage; // The actual image region
  return qimage.mirrored(false, true); // The flipped image region
}

template <class TImage>
void CopyRegion(const TImage* sourceImage, TImage* targetImage,
               const itk::Index<2>& sourcePosition, const itk::Index<2>& targetPosition, const unsigned int radius)
{
  // Copy a patch of radius 'radius' centered at 'sourcePosition' from 'sourceImage' to 'targetImage' centered at 'targetPosition'
  typedef itk::RegionOfInterestImageFilter<TImage, TImage> ExtractFilterType;

  typename ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
  extractFilter->SetRegionOfInterest(ITKHelpers::GetRegionInRadiusAroundPixel(sourcePosition, radius));
  extractFilter->SetInput(sourceImage);
  extractFilter->Update();

  CopyPatchIntoImage<TImage>(extractFilter->GetOutput(), targetImage, targetPosition);
}


template <class TImage>
void CopyPatchIntoImage(const TImage* const patch, TImage* const image, const Mask* const mask,
                        const itk::Index<2>& position)
{
  // This function copies 'patch' into 'image' centered at 'position' only where the 'mask' is non-zero

  // 'Mask' must be the same size as 'image'
  if(mask->GetLargestPossibleRegion().GetSize() != image->GetLargestPossibleRegion().GetSize())
    {
    throw std::runtime_error("mask and image must be the same size!");
    }

  // The PasteFilter expects the lower left corner of the destination position, but we have passed the center pixel.
  position[0] -= patch->GetLargestPossibleRegion().GetSize()[0]/2;
  position[1] -= patch->GetLargestPossibleRegion().GetSize()[1]/2;

  itk::ImageRegion<2> region = GetRegionInRadiusAroundPixel(position,
                                                            patch->GetLargestPossibleRegion().GetSize()[0]/2);

  itk::ImageRegionConstIterator<TImage> patchIterator(patch,patch->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator<Mask> maskIterator(mask,region);
  itk::ImageRegionIterator<TImage> imageIterator(image, region);

  while(!patchIterator.IsAtEnd())
    {
    if(mask->IsHole(maskIterator.GetIndex())) // we are in the target region
      {
      imageIterator.Set(patchIterator.Get());
      }
    ++imageIterator;
    ++maskIterator;
    ++patchIterator;
    }
}


template <class TImage>
void CopyPatchIntoImage(const TImage* patch, TImage* const image, const itk::Index<2>& centerPixel)
{
  // This function copies 'patch' into 'image' centered at 'position'.

  // The PasteFilter expects the lower left corner of the destination position, but we have passed the center pixel.
  itk::Index<2> cornerPixel;
  cornerPixel[0] = centerPixel[0] - patch->GetLargestPossibleRegion().GetSize()[0]/2;
  cornerPixel[1] = centerPixel[1] - patch->GetLargestPossibleRegion().GetSize()[1]/2;

  typedef itk::PasteImageFilter <TImage, TImage> PasteImageFilterType;

  typename PasteImageFilterType::Pointer pasteFilter = PasteImageFilterType::New();
  pasteFilter->SetInput(0, image);
  pasteFilter->SetInput(1, patch);
  pasteFilter->SetSourceRegion(patch->GetLargestPossibleRegion());
  pasteFilter->SetDestinationIndex(cornerPixel);
  pasteFilter->InPlaceOn();
  pasteFilter->Update();

  image->Graft(pasteFilter->GetOutput());

}

} // end namespace

#endif
