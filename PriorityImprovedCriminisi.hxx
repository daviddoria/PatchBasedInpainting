
template <typename TImage>
float PriorityCriminisi<TImage>::ComputeDataTerm(const itk::Index<2>& queryPixel)
{
  // The difference between this funciton and Criminisi's original data term computation (ComputeDataTermCriminisi)
  // is that we claim there is no reason to penalize the priority of linear structures that don't have a perpendicular incident
  // angle with the boundary. Of course, we don't want to continue structures that are almost parallel with the boundary, but above
  // a threshold, the strength of the isophote should be more important than the angle of incidence.

  FloatVector2Type isophote = this->IsophoteImage->GetPixel(queryPixel);
  FloatVector2Type boundaryNormal = this->BoundaryNormalsImage->GetPixel(queryPixel);

  DebugMessage<FloatVector2Type>("Isophote: ", isophote);
  DebugMessage<FloatVector2Type>("Boundary normal: ", boundaryNormal);
  // D(p) = |dot(isophote at p, normalized normal of the front at p)|/alpha

  vnl_double_2 vnlIsophote(isophote[0], isophote[1]);

  vnl_double_2 vnlNormal(boundaryNormal[0], boundaryNormal[1]);

  float dataTerm = 0.0f;

  float angleBetween = Helpers::AngleBetween(isophote, boundaryNormal);
  if(angleBetween < 20)
    {
    float projectionMagnitude = isophote.GetNorm() * cos(angleBetween);

    dataTerm = projectionMagnitude;
    }
  else
  {
    dataTerm = isophote.GetNorm();
  }

  return dataTerm;

}
