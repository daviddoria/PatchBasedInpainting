# if(BuildTests)
#   add_subdirectory(Tests)
# endif(BuildTests)

add_custom_target(LinearSearchBest SOURCES
AdaptiveDualHistogramDifference.hpp
AdaptiveDualQuadrantHistogramDifference.hpp
ColorTexture.hpp
DualHistogramDifference.hpp
First.hpp
FirstAndWrite.hpp
HistogramCorrelation.hpp
HistogramDifference.hpp
HistogramNewColors.hpp
HistogramParent.hpp
HoleHistogramDifference.hpp
IntroducedEnergy.hpp
LidarHSVTextureGradient.hpp
LidarHSVTextureGradientWithSort.hpp
LidarRGBTextureGradient.hpp
LidarTextureDerivatives.hpp
LinearSearchBestParent.hpp
Property.hpp
PropertyNoCheck.hpp
QuadrantHistogramDifference.hpp
StrategySelection.hpp
Texture.hpp
)
