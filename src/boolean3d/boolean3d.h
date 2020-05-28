#pragma once

#include <CGAL/Triangle_3.h>
#include <CGAL/Cartesian.h>
#include <CGAL/Gmpq.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/intersections.h>

#include <glm/glm.hpp>

namespace boolean3d
{
	typedef CGAL::Cartesian<CGAL::Gmpq> Kernel;
	typedef CGAL::cpp11::result_of<Kernel::Intersect_2(Kernel::Triangle_3, Kernel::Triangle_3)>::type Intersection_3;
	typedef CGAL::cpp11::result_of<Kernel::Intersect_2(Kernel::Triangle_2, Kernel::Triangle_2)>::type Intersection_2;
	struct CompleteTriangle { Kernel::Kernel::Triangle_3 positions; Kernel::Kernel::Triangle_3 normals; };
	typedef std::vector<CompleteTriangle> Mesh;
	struct PolygonSoup
	{
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::ivec3> indices;
	};

	Mesh intersect(Mesh mesh1, Mesh mesh2);

	Mesh toMesh(PolygonSoup soup);

	PolygonSoup fromMesh(Mesh mesh);
}