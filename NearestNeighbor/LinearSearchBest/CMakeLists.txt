# if(BuildTests)
#   add_subdirectory(Tests)
# endif(BuildTests)

add_custom_target(LinearSearchBest SOURCES
AdaptiveDualHistogramDifference.hpp
AdaptiveDualQuadrantHistogramDifference.hpp
DualHistogramDifference.hpp
HistogramCorrelation.hpp
HistogramDifference.hpp
HistogramNewColors.hpp
HistogramParent.hpp
HoleHistogramDifference.hpp
IntroducedEnergy.hpp
Property.hpp
First.hpp
FirstAndWrite.hpp
LinearSearchBestParent.hpp
StrategySelection.hpp
QuadrantHistogramDifference.hpp
Texture.hpp
)
