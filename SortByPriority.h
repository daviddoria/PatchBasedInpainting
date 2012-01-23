#ifndef SortByPriority_h
#define SortByPriority_h

template <typename TPair>
class SortByPriority
{
public:

  bool operator()(const TPair& item1, const TPair& item2) const
  {
    if(item1.second() < item2.second())
    {
      return true;
    }
    else
    {
      return false;
    }
  }

};

#endif
