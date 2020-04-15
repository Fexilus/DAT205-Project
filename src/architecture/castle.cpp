#include "castle.h"

#include <vector>

#include <glm/glm.hpp>

namespace architecture
{
	Shape* makeWalls(glm::vec3 nodes[], size_t numNodes)
	{
		/* Make walls from a series of nodes. Only generates one wall at the moment */
		// TODO: Handle multiple walls
		// TODO: Handle translation of wall
		float wallHeight = 30;
		float wallDepth = 10;

		glm::vec3 upDir(0, 1, 0);

		std::vector<Shape*> walls;

		for (int i = 0; i < numNodes - 1; ++i)
		{
			glm::vec3 xDir = glm::normalize(nodes[i + 1] - nodes[i]);
			glm::vec3 zDir = glm::cross(xDir, upDir);

			glm::vec3 wallCoordSys[3] = { xDir, upDir, zDir };

			glm::vec2 wallBounds[3] = { glm::vec2(0, glm::length(nodes[i + 1] - nodes[i])), 
				                        glm::vec2(0, wallHeight),
										glm::vec2(-wallDepth / 2.0f, wallDepth / 2.0f) };
			walls.push_back(new architecture::Shape(wallCoordSys, wallBounds));
		}

		return(walls[0]);
	}

	void castleWindows(Shape* wall)
	{
		wall->repeat(0, SizePolicy::absolute, 10);

		SizePolicy splitPolicies[] = { SizePolicy::relative,
									   SizePolicy::absolute,
									   SizePolicy::relative };
		glm::vec2 expansion[3] = { glm::vec2(0), glm::vec2(0), glm::vec2(2) };
		float splitSizes1[] = { 1, 8, 1 };
		float splitSizes2[] = { 1, 15, 1 };
		for (architecture::Shape* child : wall->children)
		{
			child->subdivide(0, splitPolicies, splitSizes1, 3);
			child->children[1]->subdivide(1, splitPolicies, splitSizes2, 3);
			child->children[1]->children[1]->boundsExpand(expansion);
		}
	}
}