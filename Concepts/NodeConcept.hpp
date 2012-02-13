#ifndef NodeConcept_HPP
#define NodeConcept_HPP

#include <boost/concept_check.hpp>

/**
 * This concept-check class defines the functions that must be available for an object to be interpreted as a node.
 * 
 * Valid Expressions:
 * 
 *  node[component];  Access a component via operator[]()
 *
 * \tparam TNode The node type.
 */
template <typename TNode>
struct NodeConcept 
{
  TNode node;

  BOOST_CONCEPT_USAGE(NodeConcept ) 
  {
    node[0];
  };

};

#endif
