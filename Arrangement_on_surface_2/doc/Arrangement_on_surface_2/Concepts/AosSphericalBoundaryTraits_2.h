/*! \ingroup PkgArrangementOnSurface2ConceptsTraits
 * \cgalConcept
 *
 * Models of the concept `AosSphericalBoundaryTraits_2` handle curves on
 * a sphere or a surface that is topological equivalent to a sphere. The sphere
 * is oriented in such a way that the boundary of the rectangular parameter
 * space, the sphere is the mapping of which, is identified on the left and
 * right sides and contracted at the top and bottom sides.
 *
 * \cgalRefines{AosBasicTraits_2,AosIdentifiedVerticalTraits_2,
 *   AosContractedBottomTraits_2,AosContractedTopTraits_2}
 *
 * \cgalHasModelsBegin
 * \cgalHasModels{CGAL::Arr_geodesic_arc_on_sphere_traits_2<Kernel, X, Y>}
 * \cgalHasModelsEnd
 *
 * \sa `AosOpenBoundaryTraits_2`
 * \sa `AosBasicTraits_2`
 * \sa `AosIdentifiedVerticalTraits_2`
 * \sa `AosContractedBottomTraits_2`
 * \sa `AosContractedTopTraits_2`
 * \sa `AosHorizontalSideTraits_2`
 * \sa `AosVerticalSideTraits_2`
 */
class AosSphericalBoundaryTraits_2 {
public:
  /// \name Categories
  /// @{

  /// Must be convertible to `CGAL::Arr_identified_side_tag`.
  typedef unspecified_type Left_side_category;

  /// Must be convertible to `CGAL::Arr_identified_side_tag`.
  typedef unspecified_type Bottom_side_category;

  /// Must be convertible to `CGAL::Arr_contracted_side_tag`.
  typedef unspecified_type Top_side_category;

  /// Must be convertible to `CGAL::Arr_contracted_side_tag`.
  typedef unspecified_type Right_side_category;

  /// @}

  /// \name Functor Types
  /// @{

  /// models the concept `AosTraits::ParameterSpaceInX_2`.
  typedef unspecified_type Parameter_space_in_x_2;

  /// models the concept `AosTraits::CompareXOnBoundaryOfCurveEnd_2`.
  typedef unspecified_type Compare_x_on_boundary_2;

  /// models the concept `AosTraits::CompareXNearBoundary_2`.
  typedef unspecified_type Compare_x_near_boundary_2;

  /// models the concept `AosTraits::ParameterSpaceInY_2`.
  typedef unspecified_type Parameter_space_in_y_2;

  /// models the concept `AosTraits::CompareYOnBoundary_2`.
  typedef unspecified_type Compare_y_on_boundary_2;

  /// models the concept `AosTraits::CompareYNearBoundary_2`.
  typedef unspecified_type Compare_y_near_boundary_2;

  /// models the concept `AosTraits::IsOnYIdentification_2`.
  typedef unspecified_type  Is_on_y_identification_2;

  /// @}

  /// \name Accessing Functor Objects
  /// @{

  ///
  Parameter_space_in_x_2 parameter_space_in_x_2_object() const;

  ///
  Compare_y_on_boundary_2 compare_y_on_boundary_2_object() const;

  ///
  Compare_y_near_boundary_2 compare_y_near_boundary_2_object() const;

  ///
  Parameter_space_in_y_2 parameter_space_in_y_2_object() const;

  ///
  Compare_x_on_boundary_2 compare_x_on_boundary_2_object() const;

  ///
  Compare_x_near_boundary_2 compare_x_near_boundary_2_object() const;

  ///
  Is_on_y_identification_2 is_on_y_identification_2_object() const;

  /// @}
}; /* end AosSphericalBoundaryTraits_2 */
