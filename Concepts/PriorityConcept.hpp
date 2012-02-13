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

  typename TPriority::NodeType node;

  BOOST_CONCEPT_USAGE(PriorityConcept ) 
  {
    priority.ComputePriority(node);
    priority.Update(node);
  };

};

#endif
