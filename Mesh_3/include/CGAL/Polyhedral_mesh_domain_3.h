// Copyright (c) 2009 INRIA Sophia-Antipolis (France).
// Copyright (c) 2011 GeometryFactory Sarl (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Stéphane Tayeb
//
//******************************************************************************
// File Description :
//
//******************************************************************************

#ifndef CGAL_POLYHEDRAL_MESH_DOMAIN_3_H
#define CGAL_POLYHEDRAL_MESH_DOMAIN_3_H

#include <CGAL/license/Mesh_3.h>

#include <CGAL/disable_warnings.h>

#include <CGAL/Mesh_3/Robust_intersection_traits_3.h>
#include <CGAL/Mesh_3/Profile_counter.h>

#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_face_graph_triangle_primitive.h>
#include <CGAL/boost/graph/helpers.h>
#include <CGAL/boost/graph/properties.h>
#include <CGAL/Default.h>
#include <CGAL/point_generators_3.h>
#include <CGAL/Random.h>
#include <CGAL/Side_of_triangle_mesh.h>
#include <CGAL/tuple.h>

#include <optional>
#include <boost/none.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/mpl/or.hpp>
#include <boost/format.hpp>
#include <variant>
#include <boost/math/special_functions/round.hpp>

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <type_traits>

#ifdef CGAL_LINKED_WITH_TBB
# include <tbb/enumerable_thread_specific.h>
#endif

// To handle I/O for Surface_patch_index if that is a pair of `int` (the
// default)
#include <CGAL/SMDS_3/internal/Handle_IO_for_pair_of_int.h>

#include <CGAL/SMDS_3/internal/indices_management.h>

namespace CGAL {

namespace Mesh_3 {

namespace details {

inline
double
max_length(const Bbox_3& b)
{
  return (std::max)(b.xmax()-b.xmin(),
                    (std::max)(b.ymax()-b.ymin(),b.zmax()-b.zmin()) );
}

// -----------------------------------
// Geometric traits generator
// -----------------------------------
template < typename GT,
           typename Use_exact_intersection_construction_tag >
struct IGT_generator {};

template < typename GT >
struct IGT_generator<GT,CGAL::Tag_true>
{
#ifdef CGAL_MESH_3_NEW_ROBUST_INTERSECTION_TRAITS
  typedef CGAL::Mesh_3::Robust_intersection_traits_3_new<GT> type;
#else // NOT CGAL_MESH_3_NEW_ROBUST_INTERSECTION_TRAITS
  typedef CGAL::Mesh_3::Robust_intersection_traits_3<GT> type;
#endif // NOT CGAL_MESH_3_NEW_ROBUST_INTERSECTION_TRAITS
  typedef type Type;
};

template < typename GT >
struct IGT_generator<GT,CGAL::Tag_false>
{
  typedef GT type;
  typedef type Type;
};

}  // end namespace details
}  // end namespace Mesh_3


/*!
\ingroup PkgMesh3Domains

The class `Polyhedral_mesh_domain_3` implements
a domain defined by a simplicial polyhedral surface.

The input polyhedral surface must be free of intersections.

It must include (at least) one closed connected component
that defines the boundary of the domain to be meshed.
Inside this bounding component,
the input polyhedral surface may contain
several other components (closed or not)
that will be represented in the final mesh.
This class is a model of the concept `MeshDomain_3`.

\note When the given surface(s) are not closed, the surface only will be meshed.
It is then recommended to use the parameter `parameters::surface_only()` to speedup
the meshing process.

\tparam Polyhedron stands for the type of the input polyhedral surface(s),
model of `FaceListGraph`.

\tparam IGT stands for a geometric traits class
providing the types and functors required to implement
the intersection tests and intersection computations
for polyhedral boundary surfaces. This parameter has to be instantiated
with a model of the concept `IntersectionGeometricTraits_3`.

\cgalModels{MeshDomain_3}

\sa `IntersectionGeometricTraits_3`
\sa `CGAL::make_mesh_3()`.
*/
#ifdef DOXYGEN_RUNNING
template<class Polyhedron /*FaceGraph*/
         ,class IGT>
#else // DOXYGEN_RUNNING
template<class Polyhedron, /*FaceGraph*/
         class IGT_,
         class TriangleAccessor = CGAL::Default,
         class Patch_id_ = void,
         class Use_exact_intersection_construction_tag = CGAL::Tag_true>
#endif // DOXYGEN_RUNNING
class Polyhedral_mesh_domain_3
{
public:
  typedef typename Mesh_3::details::IGT_generator<
    IGT_,Use_exact_intersection_construction_tag>::type IGT;


  typedef Patch_id_ Patch_id;

  // Geometric object types
  typedef typename IGT::Point_3    Point_3;
  typedef typename IGT::Segment_3  Segment_3;
  typedef typename IGT::Ray_3      Ray_3;
  typedef typename IGT::Line_3     Line_3;
  typedef typename IGT::Vector_3   Vector_3;
  typedef typename IGT::Sphere_3   Sphere_3;

  //-------------------------------------------------------
  // Index Types
  //-------------------------------------------------------
  // Type of indexes for cells of the input complex
  typedef int Subdomain_index;
  typedef std::optional<Subdomain_index> Subdomain;

  // Type of indexes for surface patch of the input complex
  typedef typename boost::property_map<Polyhedron,
                                       face_patch_id_t<Patch_id>
                                       >::type            Face_patch_id_pmap;
  typedef typename boost::property_traits<
    Face_patch_id_pmap>::value_type                       Surface_patch_index;
  typedef std::optional<Surface_patch_index>            Surface_patch;

  // Type of indexes to characterize the lowest dimensional face of the input
  // complex on which a vertex lie
  typedef typename Mesh_3::internal::Index_generator<
    Subdomain_index, Surface_patch_index>::type           Index;

  typedef std::tuple<Point_3,Index,int> Intersection;

  typedef typename IGT::FT         FT;

  // Kernel_traits compatibility
  typedef IGT R;

  BOOST_MPL_HAS_XXX_TRAIT_DEF(HalfedgeDS)

  template <typename P>
  struct Primitive_type {
      //setting OneFaceGraphPerTree to false transforms the id type into
      //std::pair<FD, const FaceGraph*>.
    typedef AABB_face_graph_triangle_primitive<P, typename boost::property_map<P,vertex_point_t>::const_type, CGAL::Tag_false> type;

    static
    typename IGT_::Triangle_3 datum(const typename type::Id primitive_id) {
      CGAL::Triangle_from_face_descriptor_map<P> pmap(primitive_id.second);
      return get(pmap, primitive_id.first);
    }

    static Surface_patch_index get_index(const typename type::Id primitive_id) {
      return get(get(face_patch_id_t<Patch_id>(),
                     *primitive_id.second),
                 primitive_id.first);
    }
  }; // Primitive_type (for non-Polyhedron_3)


public:
  typedef typename Primitive_type<Polyhedron>::type       Ins_fctor_primitive;
  typedef CGAL::AABB_traits_3<IGT, Ins_fctor_primitive>   Ins_fctor_traits;
  typedef CGAL::AABB_tree<Ins_fctor_traits>               Ins_fctor_AABB_tree;

  typedef Side_of_triangle_mesh<Polyhedron,
                                IGT,
                                Default,
                                Ins_fctor_AABB_tree>      Inside_functor;
  typedef typename Inside_functor::AABB_tree              AABB_tree_;
  BOOST_STATIC_ASSERT((std::is_same<AABB_tree_, Ins_fctor_AABB_tree>::value));
  typedef typename AABB_tree_::AABB_traits                AABB_traits;
  typedef typename AABB_tree_::Primitive                  AABB_primitive;
  typedef typename AABB_tree_::Primitive_id               AABB_primitive_id;
  typedef typename AABB_traits::Bounding_box              Bounding_box;

public:
  Polyhedral_mesh_domain_3(CGAL::Random* p_rng = nullptr)
    : tree_()
    , bounding_tree_(&tree_)
    , p_rng_(p_rng)
  {
  }

  /// \name Creation
  /// @{

  /*!
    Construction from a polyhedral surface which must be free of intersections.
    If `polyhedron` is closed, its inside will be meshed,
    otherwise there will be no interior and only the surface will be meshed.
  */
  Polyhedral_mesh_domain_3(const Polyhedron& polyhedron
#ifndef DOXYGEN_RUNNING
                           , CGAL::Random* p_rng = nullptr
#endif
                           )
    : tree_()
    , bounding_tree_(&tree_) // the bounding tree is tree_
    , p_rng_(p_rng)
  {
    this->add_primitives(polyhedron);
    if(!is_triangle_mesh(polyhedron)) {
      std::cerr << "Your input polyhedron must be triangulated!\n";
      CGAL_error_msg("Your input polyhedron must be triangulated!");
    }
    this->build();

    if(!is_closed(polyhedron))
      set_surface_only();
  }

  /*!
    Construction from a polyhedral surface, and a bounding polyhedral surface.
    The first polyhedron must be entirely included inside `bounding_polyhedron`, which must be closed
    and free of intersections.
    Using this constructor enables to mesh a polyhedral surface which is not closed, or has holes.
    The inside of `bounding_polyhedron` will be meshed.
  */
  Polyhedral_mesh_domain_3(const Polyhedron& p,
                           const Polyhedron& bounding_polyhedron
#ifndef DOXYGEN_RUNNING
                           , CGAL::Random* p_rng = nullptr
#endif
                           )
    : tree_()
    , bounding_tree_(new AABB_tree_)
    , p_rng_(p_rng)
  {
    this->add_primitives(p);
    this->add_primitives(bounding_polyhedron);
    if(!bounding_polyhedron.empty()) {
      this->add_primitives_to_bounding_tree(bounding_polyhedron);
    } else {
      this->set_surface_only();
    }
    this->build();
  }

  /*!
   * Constructor from a sequence of polyhedral surfaces, and a bounding
   * polyhedral surface.
   *
   * @tparam InputPolyhedraPtrIterator must be a model of
   * `ForwardIterator` and value type `Polyhedron*`
   *
   * @param begin iterator for a sequence of pointers to polyhedra
   * @param end iterator for a sequence of pointers to polyhedra
   * @param bounding_polyhedron the bounding surface
   */
  template <typename InputPolyhedraPtrIterator>
  Polyhedral_mesh_domain_3(InputPolyhedraPtrIterator begin,
                           InputPolyhedraPtrIterator end,
                           const Polyhedron& bounding_polyhedron
#ifndef DOXYGEN_RUNNING
                           , CGAL::Random* p_rng = nullptr
#endif
                           )
    : p_rng_(p_rng)
    , delete_rng_(false)
  {
    if(begin != end) {
      for(; begin != end; ++begin) {
        this->add_primitives(**begin);
      }
      this->add_primitives(bounding_polyhedron);
    }
    if(!bounding_polyhedron.empty()) {
      this->add_primitives_to_bounding_tree(bounding_polyhedron);
    } else {
      this->set_surface_only();
    }
    this->build();
  }

  /*!
   * Constructor from a sequence of polyhedral surfaces, without a bounding
   * surface. The domain will always answer `false` to `is_in_domain()`
   * queries.
   *
   * @tparam InputPolyhedraPtrIterator must be a model of
   * `ForwardIterator` and value type `Polyhedron*`
   *
   * @param begin iterator for a sequence of pointers to polyhedra
   * @param end iterator for a sequence of pointers to polyhedra
   */
  template <typename InputPolyhedraPtrIterator>
  Polyhedral_mesh_domain_3(InputPolyhedraPtrIterator begin
                           , InputPolyhedraPtrIterator end
#ifndef DOXYGEN_RUNNING
                           , CGAL::Random* p_rng = nullptr
#endif
                           )
    : p_rng_(p_rng)
  {
    if(begin != end) {
      for(; begin != end; ++begin) {
        this->add_primitives(**begin);
      }
      tree_.build();
    }
    set_surface_only();
  }

  /// @}

  // Destructor
  ~Polyhedral_mesh_domain_3() {
    if(bounding_tree_ != 0 && bounding_tree_ != &tree_) {
      delete bounding_tree_;
    }
  }

  void set_surface_only() {
    bounding_tree_ = 0;
  }

  /**
   * constructs a set of `n` points on the surface, and output them to
   *  the output iterator `pts` whose value type is required to be
   *  `std::pair<Points_3, Index>`.
   */
  struct Construct_initial_points
  {
    Construct_initial_points(const Polyhedral_mesh_domain_3& domain)
      : r_domain_(domain) {}

    template<class OutputIterator>
    OutputIterator operator()(OutputIterator pts, const int n = 8) const;

  private:
    const Polyhedral_mesh_domain_3& r_domain_;
  };

  Construct_initial_points construct_initial_points_object() const
  {
    return Construct_initial_points(*this);
  }


  /**
   * Returns a bounding box of the domain
   */
  Bbox_3 bbox() const {
    return tree_.bbox();
  }


  /**
   * Returns true if point `p` is in the domain. If `p` is in the
   *  domain, the parameter index is set to the index of the subdomain
   *  including `p`. It is set to the default value otherwise.
   */
  struct Is_in_domain
  {
    Is_in_domain(const Polyhedral_mesh_domain_3& domain)
      : r_domain_(domain) {}

    Subdomain operator()(const Point_3& p) const;
  private:
    const Polyhedral_mesh_domain_3& r_domain_;
  };

  Is_in_domain is_in_domain_object() const { return Is_in_domain(*this); }

  Point_3 project_on_surface(const Point_3& p) const
  {
    return tree_.closest_point(p);
  }

  // Allowed query types
  typedef boost::mpl::vector<Segment_3, Ray_3, Line_3> Allowed_query_types;

  /**
   * Returns `true` if the element `type` intersects properly any of the
   * surface patches describing either the domain boundary or some
   * subdomain boundary.
   * `Type` is either `Segment_3`, `Ray_3` or `Line_3`.
   * Parameter index is set to the index of the intersected surface patch
   * if `true` is returned and to the default `Surface_patch_index`
   * value otherwise.
   */
  struct Do_intersect_surface
  {
    Do_intersect_surface(const Polyhedral_mesh_domain_3& domain)
      : r_domain_(domain) {}

    template <typename Query>
    typename std::enable_if_t<boost::mpl::contains<Allowed_query_types,
                                                   Query>::value,
                              Surface_patch>
    operator()(const Query& q) const
    {
      CGAL_MESH_3_PROFILER(std::string("Mesh_3 profiler: ") + std::string(CGAL_PRETTY_FUNCTION));

      std::optional<AABB_primitive_id> primitive_id = r_domain_.tree_.any_intersected_primitive(q);
      if ( primitive_id )
      {
        r_domain_.cache_primitive(q, *primitive_id);
        return Surface_patch(r_domain_.make_surface_index(*primitive_id));
      } else {
        return Surface_patch();
      }
    }

  private:
    const Polyhedral_mesh_domain_3& r_domain_;
  };

  Do_intersect_surface do_intersect_surface_object() const
  {
    return Do_intersect_surface(*this);
  }

  /**
   * Returns a point in the intersection of the primitive `type`
   * with some boundary surface.
   * `Type1` is either `Segment_3`, `Ray_3` or `Line_3`.
   * The integer `dimension` is set to the dimension of the lowest
   * dimensional face in the input complex containing the returned point, and
   * `index` is set to the index to be stored at a mesh vertex lying
   * on this face.
   */
  struct Construct_intersection
  {
    Construct_intersection(const Polyhedral_mesh_domain_3& domain)
      : r_domain_(domain) {}

    template <typename Query>
    typename std::enable_if_t<boost::mpl::contains<Allowed_query_types,
                                                   Query>::value,
                              Intersection>
    operator()(const Query& q) const
    {
      CGAL_MESH_3_PROFILER(std::string("Mesh_3 profiler: ") + std::string(CGAL_PRETTY_FUNCTION));
      typedef typename AABB_tree_::template Intersection_and_primitive_id<Query>::Type
        Intersection_and_primitive_id;
      typedef std::optional<Intersection_and_primitive_id> AABB_intersection;
      typedef Point_3 Bare_point;

      AABB_intersection intersection;
#ifndef CGAL_MESH_3_NO_LONGER_CALLS_DO_INTERSECT_3
      if(r_domain_.query_is_cached(q))
      {
        const AABB_primitive_id primitive_id = r_domain_.cached_primitive_id();
        const auto o = IGT().intersect_3_object()(Primitive(primitive_id).datum(),q);
        intersection = o ?
          Intersection_and_primitive_id(*o, primitive_id) :
          AABB_intersection();
      } else
#endif // not CGAL_MESH_3_NO_LONGER_CALLS_DO_INTERSECT_3
      {
#ifndef CGAL_MESH_3_NO_LONGER_CALLS_DO_INTERSECT_3
        CGAL_precondition(r_domain_.do_intersect_surface_object()(q)
                          != std::nullopt);
#endif // NOT CGAL_MESH_3_NO_LONGER_CALLS_DO_INTERSECT_3

        intersection = r_domain_.tree_.any_intersection(q);
      }
      if ( intersection )
      {
        // Get primitive
        AABB_primitive_id primitive_id = intersection->second;

        // intersection may be either a point or a segment
        if ( const Bare_point* p_intersect_pt =
             std::get_if<Bare_point>( &(intersection->first) ) )
        {
          return Intersection(*p_intersect_pt,
                              r_domain_.index_from_surface_patch_index(
                                r_domain_.make_surface_index(primitive_id)),
                              2);
        }
        else if ( const Segment_3* p_intersect_seg =
                  std::get_if<Segment_3>(&(intersection->first)))
        {
          CGAL_MESH_3_PROFILER("Mesh_3 profiler: Intersection is a segment");

          return Intersection(p_intersect_seg->source(),
                              r_domain_.index_from_surface_patch_index(
                                r_domain_.make_surface_index(primitive_id)),
                              2);
        }
        else {
#ifndef CGAL_MESH_3_NO_LONGER_CALLS_DO_INTERSECT_3
          std::stringstream stream;
          stream.precision(17);
          set_pretty_mode(stream);
          stream <<
            "Mesh_3 error : AABB_tree any_intersection result is "
            "not a point nor a segment\n";
          if(intersection->first.empty()) {
            stream <<  "The intersection is empty!";
          } else {
            stream <<  "The intersection typeinfo name is ";
            stream <<  intersection->first.type().name();
          }
          stream << "\nThe query was: ";
          stream << q << std::endl;
          stream << "The intersecting primitive in the AABB tree was: "
                 << AABB_primitive(intersection->second).datum() << std::endl;
          CGAL_error_msg(stream.str().c_str());
#endif // not CGAL_MESH_3_NO_LONGER_CALLS_DO_INTERSECT_3
        }
      }

      // Should not happen
      // unless CGAL_MESH_3_NO_LONGER_CALLS_DO_INTERSECT_3 is defined
      return Intersection();
    }

  private:
    const Polyhedral_mesh_domain_3& r_domain_;
  };

  Construct_intersection construct_intersection_object() const
  {
    return Construct_intersection(*this);
  }


  /**
   * Returns the index to be stored in a vertex lying on the surface identified
   * by `index`.
   */
  Index index_from_surface_patch_index(const Surface_patch_index& index) const
  { return Index(index); }

  /**
   * Returns the index to be stored in a vertex lying in the subdomain
   * identified by `index`.
   */
  Index index_from_subdomain_index(const Subdomain_index& index) const
  { return Index(index); }

  /**
   * Returns the `Surface_patch_index` of the surface patch
   * where lies a vertex with dimension 2 and index `index`.
   */
  Surface_patch_index surface_patch_index(const Index& index) const
  { return std::get<Surface_patch_index>(index); }

  /**
   * Returns the index of the subdomain containing a vertex
   *  with dimension 3 and index `index`.
   */
  Subdomain_index subdomain_index(const Index& index) const
  { return std::get<Subdomain_index>(index); }

  // -----------------------------------
  // Backward Compatibility
  // -----------------------------------
#ifndef CGAL_MESH_3_NO_DEPRECATED_SURFACE_INDEX
  typedef Surface_patch_index   Surface_index;

  Index index_from_surface_index(const Surface_index& index) const
  { return index_from_surface_patch_index(index); }

  Surface_index surface_index(const Index& index) const
  { return surface_patch_index(index); }
#endif // CGAL_MESH_3_NO_DEPRECATED_SURFACE_INDEX
  // -----------------------------------
  // End backward Compatibility
  // -----------------------------------

public:
  Surface_patch_index make_surface_index(
    const AABB_primitive_id& primitive_id = AABB_primitive_id() ) const
  {
    return Primitive_type<Polyhedron>::get_index(primitive_id);
  }

  // Undocumented function, used to implement a sizing field that
  // computes lfs using this AABB tree. That avoids to rebuild the same
  // tree.
  typedef AABB_tree_ AABB_tree;
  const AABB_tree& aabb_tree() const {
    return tree_;
  }

  const AABB_tree* bounding_aabb_tree_ptr() const {
    return bounding_tree_;
  }

protected:
  void add_primitives(const Polyhedron& p)
  {
    tree_.insert(faces(p).begin(), faces(p).end(), p);
  }

  void add_primitives_to_bounding_tree(const Polyhedron& p)
  {
    if(bounding_tree_ == &tree_ || bounding_tree_ == 0) {
      bounding_tree_ = new AABB_tree_;
    }
    bounding_tree_->insert(faces(p).begin(), faces(p).end(), p);
  }

  void build() {
    tree_.build();
    CGAL_assertion(!tree_.empty());
    if(bounding_tree_ != &tree_ && bounding_tree_ != 0) {
      bounding_tree_->build();
      CGAL_assertion(!bounding_tree_->empty());
    }
  }

private:
  /// The AABB tree: intersection detection and more
  AABB_tree_ tree_;

  AABB_tree_* bounding_tree_;

  // cache queries and intersected primitive
  typedef std::variant<Segment_3, Ray_3, Line_3> Cached_query;
  struct Query_cache
  {
    Query_cache() : has_cache(false) {}
    bool has_cache;
    Cached_query cached_query;
    AABB_primitive_id cached_primitive_id;
  };
#ifdef CGAL_LINKED_WITH_TBB
  mutable tbb::enumerable_thread_specific<Query_cache> query_cache;
#else
  mutable Query_cache query_cache;
#endif

  //random number generator for Construct_initial_points
  CGAL::Random* p_rng_;
  bool delete_rng_;

public:

  template <typename Query>
  void cache_primitive(const Query& q,
                       const AABB_primitive_id id) const
  {
#ifdef CGAL_LINKED_WITH_TBB
    Query_cache &qc = query_cache.local();
    qc.cached_query = Cached_query(q);
    qc.has_cache = true;
    qc.cached_primitive_id = id;
#else
    query_cache.cached_query = Cached_query(q);
    query_cache.has_cache = true;
    query_cache.cached_primitive_id = id;
#endif
  }

  template <typename Query>
  bool query_is_cached(const Query& q) const {
#ifdef CGAL_LINKED_WITH_TBB
    Query_cache &qc = query_cache.local();
    return qc.has_cache && (qc.cached_query == Cached_query(q));
#else
    return query_cache.has_cache
      && (query_cache.cached_query == Cached_query(q));
#endif
  }

  AABB_primitive_id cached_primitive_id() const {
#ifdef CGAL_LINKED_WITH_TBB
    return query_cache.local().cached_primitive_id;
#else
    return query_cache.cached_primitive_id;
#endif
  }

  void set_random_generator(CGAL::Random* p_rng)
  {
    p_rng_ = p_rng;
  }

private:
  // Disabled copy constructor & assignment operator
  typedef Polyhedral_mesh_domain_3 Self;
  Polyhedral_mesh_domain_3(const Self& src);
  Self& operator=(const Self& src);

};  // end class Polyhedral_mesh_domain_3





template<typename P_, typename IGT_, typename TA,
         typename Tag, typename E_tag_>
template<class OutputIterator>
OutputIterator
Polyhedral_mesh_domain_3<P_,IGT_,TA,Tag,E_tag_>::
Construct_initial_points::operator()(OutputIterator pts,
                                     const int n) const
{
  typename IGT::Construct_ray_3 ray = IGT().construct_ray_3_object();
  typename IGT::Construct_vector_3 vector = IGT().construct_vector_3_object();

  const Bounding_box bbox = r_domain_.tree_.bbox();
  Point_3 center( FT( (bbox.xmin() + bbox.xmax()) / 2),
                  FT( (bbox.ymin() + bbox.ymax()) / 2),
                  FT( (bbox.zmin() + bbox.zmax()) / 2) );

  CGAL::Random& rng = *(r_domain_.p_rng_ != 0 ?
                        r_domain_.p_rng_ :
                        new Random(0));

  Random_points_on_sphere_3<Point_3> random_point(1., rng);

  int i = n;
# ifdef CGAL_MESH_3_VERBOSE
  std::cerr << "construct initial points:" << std::endl;
# endif
  // Point construction by ray shooting from the center of the enclosing bbox
  while ( i > 0 )
  {
    const Ray_3 ray_shot = ray(center, vector(CGAL::ORIGIN,*random_point));

#ifdef CGAL_MESH_3_NO_LONGER_CALLS_DO_INTERSECT_3
    Intersection intersection = r_domain_.construct_intersection_object()(ray_shot);
    if(std::get<2>(intersection) != 0) {
#else
    if(r_domain_.do_intersect_surface_object()(ray_shot)) {
      Intersection intersection = r_domain_.construct_intersection_object()(ray_shot);
#endif
      *pts++ = std::make_pair(std::get<0>(intersection),
                              std::get<1>(intersection));

      --i;

#ifdef CGAL_MESH_3_VERBOSE
      std::cerr << boost::format("\r             \r"
                                 "%1%/%2% initial point(s) found...")
        % (n - i)
        % n;
# endif

      // If the source of the ray is on the surface, every ray will return its source
      // so change the source to a random point in the bounding box
      if(std::get<0>(intersection) == ray_shot.source())
      {
        center = Point_3(rng.get_double(bbox.xmin(), bbox.xmax()),
                         rng.get_double(bbox.ymin(), bbox.ymax()),
                         rng.get_double(bbox.zmin(), bbox.zmax()));
      }
    }
    ++random_point;
  }

#ifdef CGAL_MESH_3_VERBOSE
  std::cerr << std::endl;
#endif
  if(r_domain_.p_rng_ == 0) delete &rng;
  return pts;
}


template<typename P_, typename IGT_, typename TA,
         typename Tag, typename E_tag_>
typename Polyhedral_mesh_domain_3<P_,IGT_,TA,Tag,E_tag_>::Subdomain
Polyhedral_mesh_domain_3<P_,IGT_,TA,Tag,E_tag_>::
Is_in_domain::operator()(const Point_3& p) const
{
  if(r_domain_.bounding_tree_ == 0) return Subdomain();

  Inside_functor inside_functor(*(r_domain_.bounding_tree_));
  Bounded_side side = inside_functor(p);

  if(side == CGAL::ON_UNBOUNDED_SIDE) { return Subdomain(); }
  else { return Subdomain(Subdomain_index(1)); } // case ON_BOUNDARY && ON_BOUNDED_SIDE
}

}  // end namespace CGAL

#include <CGAL/enable_warnings.h>

#endif // POLYHEDRAL_MESH_TRAITS_3_H_
