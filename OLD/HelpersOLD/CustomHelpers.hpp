#ifndef CustomHelpers_HPP
#define CustomHelpers_HPP

namespace CustomHelpers
{
  template <typename TPropertyMap, typename TImage>
  void WritePropertyMapAsImage(const TPropertyMap propertyMap, TImage* const image, const std::string& fileName)
  {
    typename itk::ImageRegionIterator<TImage> imageIterator(image, image->GetLargestPossibleRegion());

    while(!imageIterator.IsAtEnd())
      {
      typename TPropertyMap::key_type v =
          Helpers::ConvertFrom<typename TPropertyMap::key_type, itk::Index<2> >(imageIterator.GetIndex());
      int value = 0;
      if(get(propertyMap, v))
        {
          value = 255;
        }
      imageIterator.Set(value);
      ++imageIterator;
      }
    typedef typename itk::ImageFileWriter<TImage> WriterType;
    typename WriterType::Pointer writer = WriterType::New();
    writer->SetInput(image);
    writer->SetFileName(fileName);
    writer->Write();
  }


  template <typename TNodeQueue, typename TImage>
  void WriteAllQueueNodesAsImage(TNodeQueue nodeQueue, TImage* const image, const std::string& fileName)
  {
    typename TNodeQueue::key_map propertyMap = nodeQueue.keys();

    itk::Index<2> zeroIndex = {{0,0}};
    typename TImage::PixelType zeroPixel = image->GetPixel(zeroIndex);
    ITKHelpers::SetObjectToZero(zeroPixel);
    ITKHelpers::SetImageToConstant(image, zeroPixel);

    // Read out the queue, saving the objects in the order they were in the queue
    while(!nodeQueue.empty())
    {
      typename TNodeQueue::value_type queuedObject = nodeQueue.top();
      //std::cout << "queuedObject: " << queuedObject << " ";

      itk::Index<2> index = Helpers::ConvertFrom<itk::Index<2>, typename TNodeQueue::value_type>(queuedObject);
      image->SetPixel(index, 255);
      //std::cout << " value: " << value << std::endl;
      nodeQueue.pop();
    }

    typedef typename itk::ImageFileWriter<TImage> WriterType;
    typename WriterType::Pointer writer = WriterType::New();
    writer->SetInput(image);
    writer->SetFileName(fileName);
    writer->Write();
  }

} // end namespace

#endif
