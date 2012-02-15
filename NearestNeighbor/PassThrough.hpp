#ifndef PassThrough_HPP
#define PassThrough_HPP

class PassThrough
{
public:

  template <typename ForwardIteratorType, typename OutputContainerType>
  void operator()(ForwardIteratorType first, ForwardIteratorType last, typename ForwardIteratorType::value_type queryNode, OutputContainerType& output)
  {
    output.clear();
    std::copy(first, last, output.begin());
  }

};

#endif
