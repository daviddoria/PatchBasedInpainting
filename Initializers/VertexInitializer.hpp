#ifndef VERTEXINITIALIZER_HPP
#define VERTEXINITIALIZER_HPP

void InitializeVertex(VertexDescriptorType v) const
{
  //std::cout << "Initializing " << v[0] << " " << v[1] << std::endl;
  // Create the patch object and associate with the node
  itk::Index<2> index = ITKHelpers::CreateIndex(v);

  // It is ok if this region is partially outside the image - we do the cropping in later functions.
  // We cannot crop here because then we lose the relative position of the remaining piece of the target
  // patch relative to full valid patches.
  itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(index, this->HalfWidth);

  DescriptorType descriptor(this->Image, this->MaskImage, region);
  descriptor.SetVertex(v);
  put(this->DescriptorMap, v, descriptor);

  // This is done in the descriptor constructor (above)
//     if(mask->IsValid(region))
//       {
//       descriptor.SetStatus(DescriptorType::SOURCE_NODE);
//       }
}

#endif // VERTEXINITIALIZER_HPP
