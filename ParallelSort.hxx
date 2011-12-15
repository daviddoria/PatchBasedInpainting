#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

namespace ParallelSort
{

template <typename T>
bool operator<(IndexedValue<T> element1, IndexedValue<T> element2)
{
  return element1.value < element2.value;
}

template <typename T>
std::vector<IndexedValue<T> > ParallelSort(const std::vector<T>& v, const SORTORDER ordering)
{
  std::vector<IndexedValue<int> > pairs(v.size());
  for(unsigned int i = 0; i < v.size(); i++)
    {
    pairs[i].index = i;
    pairs[i].value = v[i];
    }
  if(ordering == ASCENDING)
    {
    std::sort(pairs.begin(), pairs.end());
    }
  else // DESCENDING
    {
    std::sort(pairs.rbegin(), pairs.rend());
    }


  return pairs;
}

template <typename T>
void OutputIndexedValueVector(const std::vector<T>& v)
{
  for(unsigned int i = 0; i < v.size(); i++)
    {
    std::cout << v[i].index << " " << v[i].value << std::endl;
    }
}

template <typename T>
void OutputVector(const std::vector<T>& v)
{
  for(unsigned int i = 0; i < v.size(); i++)
    {
    std::cout << v[i] << std::endl;
    }
}

} // end namespace
