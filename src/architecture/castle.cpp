#include "castle.h"

#include <vector>

#include <glm/glm.hpp>

namespace architecture
{
	std::vector<Shape*> makeWalls(glm::vec3 nodes[], size_t numNodes)
	{
		// Make walls from a series of nodes.

		float wallHeight = 50;
		float wallDepth = 20;

		glm::vec3 upDir(0, 1, 0);

		std::vector<Shape*> walls;

		for (int i = 0; i < numNodes - 1; ++i)
		{
			glm::vec3 yDir = glm::normalize(nodes[i + 1] - nodes[i]);
			glm::vec3 xDir = glm::cross(yDir, upDir);

			CoordSys wallCoordSys = { CoordSysType::cartesian, nodes[i], { xDir, yDir, upDir } };

			glm::vec2 wallBounds[3] = { glm::vec2(-wallDepth / 2.0f, wallDepth / 2.0f),
										glm::vec2(0, glm::length(nodes[i + 1] - nodes[i])), 
				                        glm::vec2(0, wallHeight) };
			walls.push_back(new architecture::Shape(wallCoordSys, wallBounds));
		}

		return(walls);
	}

	void castleWindows(Shape* wall)
	{
		wall->repeat(1, SizePolicy::absolute, 18);

		SizePolicy splitPolicies[] = { SizePolicy::relative,
									   SizePolicy::absolute,
									   SizePolicy::relative };
		float splitSizesOuterWidth[] = { 1, 4, 1 };
		float splitSizesOuterHeight[] = { 1.5, 10, 1 };
		glm::vec2 frameExpansion[3] = { glm::vec2(2), glm::vec2(0), glm::vec2(0) };
		float splitSizesInnerWidth[] = { 1, 2, 1 };
		float splitSizesInnerHeight[] = { 1.5, 8, 1 };
		glm::vec2 openingExpansion[3] = { glm::vec2(-(wall->bounds[0][1] - wall->bounds[0][0]) / 2.0f), glm::vec2(0), glm::vec2(0) };
		for (architecture::Shape* child : wall->children)
		{
			child->subdivide(1, splitPolicies, splitSizesOuterWidth, 3);
			child->children[1]->subdivide(2, splitPolicies, splitSizesOuterHeight, 3);
			child->children[1]->children[1]->boundsExpand(frameExpansion);
			child->children[1]->children[1]->subdivide(1, splitPolicies, splitSizesInnerWidth, 3);
			child->children[1]->children[1]->children[1]->subdivide(2, splitPolicies, splitSizesInnerHeight, 3);
			child->children[1]->children[1]->children[1]->children[1]->boundsExpand(openingExpansion);
		}
	}

	void castleBattlement(Shape* wall)
	{
		// Parameters for the two sides of the wall and the walkway between
		float railingSize = 2.0f;
		SizePolicy sidesProportionPolicies[] = { SizePolicy::absolute,
												 SizePolicy::relative,
												 SizePolicy::absolute };
		float sidesProportiopns[] = { railingSize, 1, railingSize };
		glm::vec2 walkwayExpansion[] = { glm::vec2(0), glm::vec2(0), glm::vec2(0,-8) };

		// Parameters for the repeating sections of the wall
		float sectionLength = 12.0f;
		SizePolicy sectionProportionPolicies[] = { SizePolicy::relative,
									               SizePolicy::absolute,
									               SizePolicy::relative };
		float sectionProportions[] = { 1, 2, 1 };
		glm::vec2 embrasureExpansion[] = { glm::vec2(0), glm::vec2(0), glm::vec2(0,-4) };

		wall->subdivide(0, sidesProportionPolicies, sidesProportiopns, 3);
		wall->children[1]->boundsExpand(walkwayExpansion);

		wall->children[0]->repeat(1, SizePolicy::absolute, sectionLength);

		for (architecture::Shape* section : wall->children[0]->children)
		{
			section->subdivide(1, sectionProportionPolicies, sectionProportions, 3);
			section->children[1]->boundsExpand(embrasureExpansion);
		}

		wall->children[2]->repeat(1, SizePolicy::absolute, sectionLength);

		for (architecture::Shape* section : wall->children[2]->children)
		{
			section->subdivide(1, sectionProportionPolicies, sectionProportions, 3);
			section->children[1]->boundsExpand(embrasureExpansion);
		}
	}

	void castleOuterWall(Shape* wall)
	{
		SizePolicy splitPolicies[] = { SizePolicy::absolute,
									   SizePolicy::relative,
									   SizePolicy::absolute };
		float splitSizes[] = { 5, 1, 15 };
		wall->subdivide(2, splitPolicies, splitSizes, 3);

		glm::vec2 baseExpansion[3] = { glm::vec2(2), glm::vec2(0), glm::vec2(0) };
		wall->children[0]->boundsExpand(baseExpansion);

		castleWindows(wall->children[1]);

		wall->children[2]->boundsExpand(baseExpansion);
		castleBattlement(wall->children[2]);
	}
}