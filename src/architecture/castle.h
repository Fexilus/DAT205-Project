#pragma once

#include <shape.h>

namespace architecture
{
	// Rules on allignment elements
	std::vector<Shape*> makeWalls(glm::vec3 nodes[], size_t numNodes);

	// Rules on shapes
	void castleWindows(Shape* wall);
}