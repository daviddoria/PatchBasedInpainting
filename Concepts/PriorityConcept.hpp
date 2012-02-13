#ifndef PriorityConcept_HPP
#define PriorityConcept_HPP

#include <boost/concept_check.hpp>

/**
 * This concept-check class defines the functions that must be available for an object to be interpreted as a node.
 * 
 * Valid Expressions:
 * 
 *  node[component];  Access a component via operator[]()
 *
 * \tparam TPriority The node type.
 */
template <typename TPriority>
struct PriorityConcept 
{
  TPriority priority;

  struct Node { void operator[](int) {return 0;} };

  Node node;
  
  BOOST_CONCEPT_USAGE(PriorityConcept ) 
  {
    priority.ComputePriority(node);
    priority.Update(node, node); // Many of the priority functions need to copy data from the source region to the target region, so these functions must take both the source and target node location.
  };

};

#endif
