#include "castle.h"

#include <vector>
#include <stdexcept>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace architecture
{
	std::vector<Shape*> makeWalls(glm::vec3 nodes[], size_t numNodes)
	{
		float wallHeight = 50;
		float wallDepth = 20;

		glm::vec3 upDir(0, 1, 0);

		std::vector<Shape*> walls;

		for (int i = 0; i < numNodes - 1; ++i)
		{
			// Place a tower
			architecture::CoordSys cylinderCoordSys = { architecture::CoordSysType::cylindrical, nodes[i], { glm::vec3(0,0,1), glm::vec3(1,0,0), glm::vec3(0,1,0) } };

			glm::vec2 cylinderBounds[3] = { glm::vec2(0, 20),
										glm::vec2(0, 2 * glm::pi<float>() - 0.0001),
										glm::vec2(0, wallHeight) };
			walls.push_back(new architecture::Shape(cylinderCoordSys, cylinderBounds));

			// Place straight wall
			glm::vec3 yDir = glm::normalize(nodes[i + 1] - nodes[i]);
			glm::vec3 xDir = glm::cross(yDir, upDir);

			CoordSys blockCoordSys = { CoordSysType::cartesian, nodes[i], { xDir, yDir, upDir } };

			glm::vec2 blockBounds[3] = { glm::vec2(-wallDepth / 2.0f, wallDepth / 2.0f),
										glm::vec2(0, glm::length(nodes[i + 1] - nodes[i])), 
				                        glm::vec2(0, wallHeight) };
			walls.push_back(new architecture::Shape(blockCoordSys, blockBounds));
		}

		// Place a final tower
		architecture::CoordSys cylinderCoordSys = { architecture::CoordSysType::cylindrical, nodes[numNodes - 1], { glm::vec3(0,0,1), glm::vec3(1,0,0), glm::vec3(0,1,0) } };

		glm::vec2 cylinderBounds[3] = { glm::vec2(0, 20),
									glm::vec2(0, 2 * glm::pi<float>() - 0.0001),
									glm::vec2(0, wallHeight) };
		walls.push_back(new architecture::Shape(cylinderCoordSys, cylinderBounds));

		return(walls);
	}

	void castleWindows(Shape* wall)
	{
		std::vector<Shape*> segments;

		if (wall->coordSys.type == CoordSysType::cartesian)
		{
			wall->repeat(1, SizePolicy::absolute, 18);
			segments = wall->children;
		}
		else if (wall->coordSys.type == CoordSysType::cylindrical)
		{
			wall->repeat(1, architecture::SizePolicy::relative, 0.2); // TODO: Temporary splitting length

			for (architecture::Shape* side : wall->children)
			{
				side->wrapCartesianOverCylindrical();
				segments.push_back(side->children[0]);
			}
		}
		else
		{
			throw std::invalid_argument("Invalid coordinate system type");
		}

		SizePolicy splitPolicies[] = { SizePolicy::relative,
									   SizePolicy::absolute,
									   SizePolicy::relative };
		float splitSizesOuterWidth[] = { 1, 4, 1 };
		float splitSizesOuterHeight[] = { 1.5, 10, 1 };
		glm::vec2 frameExpansion[3] = { glm::vec2(1), glm::vec2(0), glm::vec2(0) };
		float splitSizesInnerWidth[] = { 1, 2, 1 };
		float splitSizesInnerHeight[] = { 1.5, 8, 1 };
		int windowOuterMask[] = { 0, 1, 0 };
		int windowInnerMask[] = { 1, 0, 1 };
		for (architecture::Shape* child : segments)
		{
			child->subdivide(1, splitPolicies, splitSizesOuterWidth, 3, windowOuterMask);
			child->children[0]->subdivide(2, splitPolicies, splitSizesOuterHeight, 3, windowOuterMask);
			child->children[0]->children[0]->boundsExpand(frameExpansion);
			child->children[0]->children[0]->subdivide(1, splitPolicies, splitSizesInnerWidth, 3);
			child->children[0]->children[0]->children[1]->subdivide(2, splitPolicies, splitSizesInnerHeight, 3, windowInnerMask);
		}
		
		wall->parentChildOp = Shape::ParentChildOperator::unite;
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