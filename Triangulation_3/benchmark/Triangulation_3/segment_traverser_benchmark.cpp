// Copyright (c) 2019  GeometryFactory (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
// You can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
//
//

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Triangulation_simplex_3.h>

#include <iostream>
#include <vector>

#include <CGAL/Random.h>
#include <CGAL/Timer.h>

// Define the kernel.
using Epeck = CGAL::Exact_predicates_exact_constructions_kernel;
using Epick = CGAL::Exact_predicates_inexact_constructions_kernel;


template<typename Kernel>
void bench_segment_traverser(const int nb_queries,
                             const int nbv,
                             const double rad,
                             CGAL::Random& rng)
{
  using DT = CGAL::Delaunay_triangulation_3<Kernel>;
  using Tds = typename DT::Triangulation_data_structure;
  using Point_3 = typename DT::Point_3;
  using Cell_handle = typename DT::Cell_handle;
  using Simplex_3 = CGAL::Triangulation_simplex_3<Tds>;

  std::cout << "\nBench :\t " << nb_queries << " queries," << std::endl
    << "\t in triangulation of size " << nbv << std::endl
    << "\t lying in the sphere (-" << rad << "; " << rad << ")" << std::endl
    << "\t and Kernel : " << typeid(Kernel()).name() << std::endl;

  // Collect random points to build a triangulation
  std::vector<Point_3> points(nbv);
  int i = 0;
  while (i < nbv)
  {
    points[i++] = Point_3(rng.get_double(-rad, rad),
                          rng.get_double(-rad, rad),
                          rng.get_double(-rad, rad));
  }

  // Construct the Delaunay triangulation.
  DT dt(points.begin(), points.end());
  assert(dt.is_valid());

  // Collect random segments for queries in [-2*rad, 2*rad]
  std::vector<Point_3> segments(2 * nb_queries);
  i = 0;
  while (i < 2 * nb_queries)
  {
    segments[i++] = Point_3(rng.get_double(-2 * rad, 2 * rad),
                            rng.get_double(-2 * rad, 2 * rad),
                            rng.get_double(-2 * rad, 2 * rad));
  }

  CGAL::Timer timer_st, timer_ct;
  timer_st.reset();
  timer_ct.reset();

  for (i = 0; i < nb_queries; ++i)
  {
    //Simplex traverser
    timer_st.start();

    // Count the number of finite cells traversed.
    unsigned int inf = 0, fin = 0;
    for (Simplex_3 st : dt.segment_traverser_simplices(segments[2 * i], segments[2 * i + 1]))
    {
      Cell_handle c = st.incident_cell();
      if (dt.is_infinite(c)) ++inf;
      else                   ++fin;
    }
    timer_st.stop();

    //Cell traverser
    timer_ct.start();

    // Count the number of finite cells traversed.
    inf = 0, fin = 0;
    for (Cell_handle c : dt.segment_traverser_cell_handles(segments[2 * i], segments[2 * i + 1]))
    {
      if (dt.is_infinite(c)) ++inf;
      else                   ++fin;
    }
    timer_ct.stop();
  }

  std::cout << "Simplex traverser took " << timer_st.time() << " seconds." << std::endl;
  std::cout << "Cell traverser took " << timer_ct.time() << " seconds." << std::endl;
}

int main(int argc, char* argv[])
{
  CGAL::Random rng;
  std::cout << "CGAL::Random seed is " << rng.get_seed() << std::endl;

  //nb of segments tested
  const int nb_queries = (argc > 1) ? atoi(argv[1]) : 100;
  //nb of vertices in triangulation
  const int nbv = (argc > 2) ? atoi(argv[2]) : 1000;
  //radius of sphere including the triangulation
  const double rad = (argc > 3) ? atof(argv[3]) : 10.;

//  bench_segment_traverser<Epeck>(nb_queries, nbv, rad, rng);
  bench_segment_traverser<Epick>(nb_queries, nbv, rad, rng);

  return EXIT_SUCCESS;
}
