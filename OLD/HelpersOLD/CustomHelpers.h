#ifndef CustomHelpers_HPP
#define CustomHelpers_HPP

namespace CustomHelpers
{
  template <typename TPropertyMap, typename TImage>
  void WritePropertyMapAsImage(const TPropertyMap propertyMap, TImage* const image, const std::string& fileName);

  template <typename TNodeQueue, typename TPropertyMap, typename TImage>
  void WriteValidQueueNodesAsImage(TNodeQueue nodeQueue, const TPropertyMap propertyMap, TImage* const image, const std::string& fileName);

  template <typename TNodeQueue, typename TImage>
  void WriteAllQueueNodesAsImage(TNodeQueue nodeQueue, TImage* const image, const std::string& fileName);
}

#endif
