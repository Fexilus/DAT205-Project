#include <boolean3d.h>

#include <unordered_map>

#include <CGAL/Plane_3.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>

#include <CGAL/Projection_traits_xy_3.h>
#include <CGAL/Projection_traits_yz_3.h>
#include <CGAL/Projection_traits_xz_3.h>

#include <CGAL/internal/Projection_traits_3.h>

namespace boolean3d
{
	typedef CGAL::Arr_segment_traits_2<Kernel> SegmentTraits;
	typedef CGAL::Arrangement_2<SegmentTraits> Arrangement_2;


	typedef CGAL::Triangulation_vertex_base_2<Kernel> Vb;
	typedef CGAL::Triangulation_face_base_2<Kernel> Fbb;
	typedef CGAL::Constrained_triangulation_face_base_2<Kernel, Fbb> Fb;
	typedef CGAL::Triangulation_data_structure_2<Vb, Fb> TDS;
	typedef CGAL::Exact_intersections_tag Itag;

	typedef CGAL::Constrained_Delaunay_triangulation_2<Kernel, TDS, Itag> CDT;

	Kernel::Point_2 pointProj(Kernel::Triangle_3* tri, const Kernel::Point_3* p3d)
	{
		Kernel::Plane_3 triPlane = tri->supporting_plane();
		Kernel::Point_2 p2d;
		if (abs(triPlane.a()) > abs(triPlane.b()))
		{
			if (abs(triPlane.a()) > abs(triPlane.c()))
			{
				p2d = { p3d->y(), p3d->z() };
			}
			else
			{
				p2d = { p3d->x(), p3d->y() };
			}
		}
		else if (abs(triPlane.b()) > abs(triPlane.c()))
		{
			p2d = { p3d->z(), p3d->x() };
		}
		else
		{
			p2d = { p3d->x(), p3d->y() };
		}

		return p2d;
	}

	Kernel::Point_3 pointUnproj(Kernel::Triangle_3* tri, const Kernel::Point_2* p2d)
	{
		Kernel::Plane_3 triPlane = tri->supporting_plane();
		Kernel::Point_3 p3d;
		if (abs(triPlane.a()) > abs(triPlane.b()))
		{
			if (abs(triPlane.a()) > abs(triPlane.c()))
			{
				p3d = { (-triPlane.d() - triPlane.b() * p2d->x() - triPlane.c() * p2d->y()) / triPlane.a(), p2d->x(), p2d->y() };
			}
			else
			{
				p3d = { p2d->x(), p2d->y(), (-triPlane.d() - triPlane.a() * p2d->x() - triPlane.b() * p2d->y()) / triPlane.c() };
			}
		}
		else if (abs(triPlane.b()) > abs(triPlane.c()))
		{
			p3d = { p2d->y(), (-triPlane.d() - triPlane.c() * p2d->x() - triPlane.a() * p2d->y()) / triPlane.b(), p2d->x() };
		}
		else
		{
			p3d = { p2d->x(), p2d->y(), (-triPlane.d() - triPlane.a() * p2d->x() - triPlane.b() * p2d->y()) / triPlane.c(), };
		}

		return p3d;
	}

	Kernel::Triangle_3 triangleUnproj(Kernel::Triangle_3* tri, const Kernel::Triangle_2* t2d)
	{
		Kernel::Plane_3 triPlane = tri->supporting_plane();
		Kernel::Point_3 p3d0, p3d1, p3d2;
		Kernel::Triangle_3 t3d;
		if (abs(triPlane.a()) > abs(triPlane.b()))
		{
			if (abs(triPlane.a()) > abs(triPlane.c()))
			{
				p3d0 = { (-triPlane.d() - triPlane.b() * t2d->vertex(0).x() - triPlane.c() * t2d->vertex(0).y()) / triPlane.a(), t2d->vertex(0).x(), t2d->vertex(0).y() };
				p3d1 = { (-triPlane.d() - triPlane.b() * t2d->vertex(1).x() - triPlane.c() * t2d->vertex(1).y()) / triPlane.a(), t2d->vertex(1).x(), t2d->vertex(1).y() };
				p3d2 = { (-triPlane.d() - triPlane.b() * t2d->vertex(2).x() - triPlane.c() * t2d->vertex(2).y()) / triPlane.a(), t2d->vertex(2).x(), t2d->vertex(2).y() };
				if (triPlane.a() > 0) t3d = { p3d0, p3d1, p3d2 };
				else t3d = { p3d0, p3d2, p3d1 };
			}
			else
			{
				p3d0 = { t2d->vertex(0).x(), t2d->vertex(0).y(), (-triPlane.d() - triPlane.a() * t2d->vertex(0).x() - triPlane.b() * t2d->vertex(0).y()) / triPlane.c() };
				p3d1 = { t2d->vertex(1).x(), t2d->vertex(1).y(), (-triPlane.d() - triPlane.a() * t2d->vertex(1).x() - triPlane.b() * t2d->vertex(1).y()) / triPlane.c() };
				p3d2 = { t2d->vertex(2).x(), t2d->vertex(2).y(), (-triPlane.d() - triPlane.a() * t2d->vertex(2).x() - triPlane.b() * t2d->vertex(2).y()) / triPlane.c() };
				if (triPlane.c() > 0) t3d = { p3d0, p3d1, p3d2 };
				else t3d = { p3d0, p3d2, p3d1 };
			}
		}
		else if (abs(triPlane.b()) > abs(triPlane.c()))
		{
			p3d0 = { t2d->vertex(0).y(), (-triPlane.d() - triPlane.c() * t2d->vertex(0).x() - triPlane.a() * t2d->vertex(0).y()) / triPlane.b(), t2d->vertex(0).x() };
			p3d1 = { t2d->vertex(1).y(), (-triPlane.d() - triPlane.c() * t2d->vertex(1).x() - triPlane.a() * t2d->vertex(1).y()) / triPlane.b(), t2d->vertex(1).x() };
			p3d2 = { t2d->vertex(2).y(), (-triPlane.d() - triPlane.c() * t2d->vertex(2).x() - triPlane.a() * t2d->vertex(2).y()) / triPlane.b(), t2d->vertex(2).x() };
			if (triPlane.b() > 0) t3d = { p3d0, p3d1, p3d2 };
			else t3d = { p3d0, p3d2, p3d1 };
		}
		else
		{
			p3d0 = { t2d->vertex(0).x(), t2d->vertex(0).y(), (-triPlane.d() - triPlane.a() * t2d->vertex(0).x() - triPlane.b() * t2d->vertex(0).y()) / triPlane.c() };
			p3d1 = { t2d->vertex(1).x(), t2d->vertex(1).y(), (-triPlane.d() - triPlane.a() * t2d->vertex(1).x() - triPlane.b() * t2d->vertex(1).y()) / triPlane.c() };
			p3d2 = { t2d->vertex(2).x(), t2d->vertex(2).y(), (-triPlane.d() - triPlane.a() * t2d->vertex(2).x() - triPlane.b() * t2d->vertex(2).y()) / triPlane.c() };
			if (triPlane.c() > 0) t3d = { p3d0, p3d1, p3d2 };
			else t3d = { p3d0, p3d2, p3d1 };
		}

		return t3d;
	}

	Mesh intersect(Mesh mesh1, Mesh mesh2)
	{
		Mesh mesh;

		std::unordered_map<CompleteTriangle*, Arrangement_2*> arrangements1;
		std::unordered_map<CompleteTriangle*, Arrangement_2*> arrangements2;

		for (auto& triangle1 : mesh1)
		{
			if (!triangle1.positions.is_degenerate())
			{
				if (arrangements1.find(&triangle1) == arrangements1.end())
				{
					arrangements1.try_emplace(&triangle1, new Arrangement_2());
				}

				for (auto& triangle2 : mesh2)
				{
					if (!triangle2.positions.is_degenerate())
					{
						if (arrangements2.find(&triangle2) == arrangements2.end())
						{
							arrangements2.try_emplace(&triangle2, new Arrangement_2());
						}

						Intersection_3 intsect = intersection(triangle1.positions, triangle2.positions);
						if (intsect)
						{
							auto& arr1 = arrangements1[&triangle1];
							auto& arr2 = arrangements2[&triangle2];
							// Point_3, or Segment_3, or Triangle_3, or std::vector < Point_3 >
							if (const Kernel::Point_3* p = boost::get<Kernel::Point_3>(&*intsect))
							{
								CGAL::insert_point(*arr1, pointProj(&triangle1.positions, p));
								CGAL::insert_point(*arr2, pointProj(&triangle2.positions, p));
							}
							else if (const Kernel::Segment_3* s = boost::get<Kernel::Segment_3>(&*intsect))
							{
								Kernel::Segment_2 s1(pointProj(&triangle1.positions, &s->vertex(0)), pointProj(&triangle1.positions, &s->vertex(1)));
								CGAL::insert(*arr1, s1);
								Kernel::Segment_2 s2(pointProj(&triangle2.positions, &s->vertex(0)), pointProj(&triangle2.positions, &s->vertex(1)));
								CGAL::insert(*arr2, s2);
							}
							//else if (const Kernel::Triangle_3* t = boost::get<Kernel::Triangle_3>(&*intsect))
							//{
							//	Kernel::Triangle_2 t1(plane1.to_2d(t->vertex(0)), plane1.to_2d(t->vertex(1)), plane1.to_2d(t->vertex(2)));
							//}


						}
					}
				}
			}
		}

		
		for (auto& arrPair : arrangements1)
		{
			if (!arrPair.first->positions.is_degenerate())
			{
				Kernel::Point_2 trip0 = pointProj(&arrPair.first->positions, &arrPair.first->positions.vertex(0));
				Kernel::Point_2 trip1 = pointProj(&arrPair.first->positions, &arrPair.first->positions.vertex(1));
				Kernel::Point_2 trip2 = pointProj(&arrPair.first->positions, &arrPair.first->positions.vertex(2));
				Kernel::Segment_2 tris0(trip0, trip1);
				Kernel::Segment_2 tris1(trip1, trip2);
				Kernel::Segment_2 tris2(trip2, trip0);
				CGAL::insert(*arrPair.second, tris0);
				CGAL::insert(*arrPair.second, tris1);
				CGAL::insert(*arrPair.second, tris2);

				CDT triangulation;
				Arrangement_2::Vertex_const_iterator vit;
				for (vit = arrPair.second->vertices_begin(); vit != arrPair.second->vertices_end(); ++vit)
				{
					triangulation.insert(vit->point());
				}
				Arrangement_2::Edge_const_iterator eit;
				for (eit = arrPair.second->edges_begin(); eit != arrPair.second->edges_end(); ++eit)
				{
					triangulation.insert_constraint(eit->source()->point(), eit->target()->point());
				}

				//if (triangulation.number_of_faces() > 1)
				{
					for (auto& face : triangulation.finite_face_handles())
					{
						auto subtriangle2d = triangulation.triangle(face);
						Kernel::Triangle_3 subtriangle3d(triangleUnproj(&arrPair.first->positions, &subtriangle2d));
						CompleteTriangle subCompleteTriangle = { subtriangle3d, arrPair.first->normals };
						mesh.push_back(subCompleteTriangle);
					}
				}
				//else
				{
				//	mesh.push_back(*arrPair.first);
				}
			}
		}
		for (auto& arrPair : arrangements2)
		{
			if (!arrPair.first->positions.is_degenerate())
			{
				Kernel::Point_2 tri1p0 = pointProj(&arrPair.first->positions, &arrPair.first->positions.vertex(0));
				Kernel::Point_2 tri1p1 = pointProj(&arrPair.first->positions, &arrPair.first->positions.vertex(1));
				Kernel::Point_2 tri1p2 = pointProj(&arrPair.first->positions, &arrPair.first->positions.vertex(2));
				Kernel::Segment_2 tri1s1(tri1p0, tri1p1);
				Kernel::Segment_2 tri1s2(tri1p1, tri1p2);
				Kernel::Segment_2 tri1s3(tri1p2, tri1p0);
				CGAL::insert(*arrPair.second, tri1s1);
				CGAL::insert(*arrPair.second, tri1s2);
				CGAL::insert(*arrPair.second, tri1s3);

				CDT triangulation1;
				Arrangement_2::Vertex_const_iterator vit;
				for (vit = arrPair.second->vertices_begin(); vit != arrPair.second->vertices_end(); ++vit)
				{
					triangulation1.insert(vit->point());
				}
				Arrangement_2::Edge_const_iterator eit;
				for (eit = arrPair.second->edges_begin(); eit != arrPair.second->edges_end(); ++eit)
				{
					triangulation1.insert_constraint(eit->source()->point(), eit->target()->point());
				}

				if (triangulation1.number_of_faces() > 1)
				{
					for (auto& face : triangulation1.finite_face_handles())
					{
						auto subtriangle2d = triangulation1.triangle(face);
						Kernel::Triangle_3 subtriangle3d(triangleUnproj(&arrPair.first->positions, &subtriangle2d));
						CompleteTriangle subCompleteTriangle = { subtriangle3d, arrPair.first->normals };
						mesh.push_back(subCompleteTriangle);
					}
				}
				else
				{
					mesh.push_back(*arrPair.first);
				}
			}
		}

		return mesh;
	}

	Mesh toMesh(PolygonSoup soup)
	{
		Mesh mesh;
		for (auto& triIndex : soup.indices)
		{
			Kernel::Point_3 position0(soup.positions.at(triIndex.x).x, soup.positions.at(triIndex.x).y, soup.positions.at(triIndex.x).z);
			Kernel::Point_3 position1(soup.positions.at(triIndex.y).x, soup.positions.at(triIndex.y).y, soup.positions.at(triIndex.y).z);
			Kernel::Point_3 position2(soup.positions.at(triIndex.z).x, soup.positions.at(triIndex.z).y, soup.positions.at(triIndex.z).z);
			Kernel::Kernel::Triangle_3 positions(position0, position1, position2);

			Kernel::Point_3 normal0(soup.normals.at(triIndex.x).x, soup.normals.at(triIndex.x).y, soup.normals.at(triIndex.x).z);
			Kernel::Point_3 normal1(soup.normals.at(triIndex.y).x, soup.normals.at(triIndex.y).y, soup.normals.at(triIndex.y).z);
			Kernel::Point_3 normal2(soup.normals.at(triIndex.z).x, soup.normals.at(triIndex.z).y, soup.normals.at(triIndex.z).z);
			Kernel::Kernel::Triangle_3 normals(normal0, normal1, normal2);

			CompleteTriangle vertex = { positions, normals };
			mesh.push_back(vertex);
		}

		return mesh;
	}

	PolygonSoup fromMesh(Mesh mesh)
	{
		PolygonSoup soup;
		int nextIndex = 0;
		for (auto& triangle : mesh)
		{
			glm::vec3 position0(triangle.positions.vertex(0).x().to_double(), triangle.positions.vertex(0).y().to_double(), triangle.positions.vertex(0).z().to_double());
			glm::vec3 position1(triangle.positions.vertex(1).x().to_double(), triangle.positions.vertex(1).y().to_double(), triangle.positions.vertex(1).z().to_double());
			glm::vec3 position2(triangle.positions.vertex(2).x().to_double(), triangle.positions.vertex(2).y().to_double(), triangle.positions.vertex(2).z().to_double());
			soup.positions.push_back(position0);
			soup.positions.push_back(position1);
			soup.positions.push_back(position2);

			glm::vec3 normals0(triangle.normals.vertex(0).x().to_double(), triangle.normals.vertex(0).y().to_double(), triangle.normals.vertex(0).z().to_double());
			glm::vec3 normals1(triangle.normals.vertex(1).x().to_double(), triangle.normals.vertex(1).y().to_double(), triangle.normals.vertex(1).z().to_double());
			glm::vec3 normals2(triangle.normals.vertex(2).x().to_double(), triangle.normals.vertex(2).y().to_double(), triangle.normals.vertex(2).z().to_double());
			soup.normals.push_back(normals0);
			soup.normals.push_back(normals1);
			soup.normals.push_back(normals2);

			int index0 = nextIndex++;
			int index1 = nextIndex++;
			int index2 = nextIndex++;
			soup.indices.push_back(glm::ivec3(index0, index1, index2));
		}

		return soup;
	}
}