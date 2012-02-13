
add_executable(TestNodeConcept TestNodeConcept.cpp)
target_link_libraries(TestNodeConcept ${ITK_LIBRARIES})
add_test(TestNodeConcept TestNodeConcept)


add_executable(TestPriorityConcept TestPriorityConcept.cpp)
target_link_libraries(TestPriorityConcept ${ITK_LIBRARIES})
add_test(TestPriorityConcept TestPriorityConcept)

