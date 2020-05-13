#pragma once

#include <shape.h>

namespace architecture
{
	// Rules on allignment elements
	Shape* makeTower(glm::vec3 origin);
	Shape* makeTower(glm::vec3 origin, glm::vec3 connectorDirs[], float connectorWidths[], size_t numConnectors);
	Shape* makeWall(glm::vec3 start, glm::vec3 end);
	std::vector<Shape*> makeWalls(glm::vec3 nodes[], size_t numNodes);

	// Rules on shapes
	void castleWindows(Shape* wall);
	void castleBattlement(Shape* wall);
	void castleOuterWall(Shape* wall);
}