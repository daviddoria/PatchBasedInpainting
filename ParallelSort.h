#ifndef PARALLELSORT_H
#define PARALLELSORT_H

#include <vector>

namespace ParallelSort
{
enum SORTORDER{ASCENDING, DESCENDING};

template <typename T>
void OutputIndexedValueVector(const std::vector<T>& v);

template <typename T>
void OutputVector(const std::vector<T>& v);

template <typename T>
struct IndexedValue
{
  unsigned int index;
  T value;
};

template <typename T>
bool operator<(IndexedValue<T> element1, IndexedValue<T> element2);

template <typename T>
std::vector<IndexedValue<T> > ParallelSort(const std::vector<T>& v, const SORTORDER ordering);

} // end namespace

#include "ParallelSort.hxx"

#endif
