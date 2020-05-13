#include "castle.h"

#include <vector>
#include <stdexcept>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace architecture
{
	Shape* makeTower(glm::vec3 origin)
	{
		float height = 40;
		float radius = 20;
		float wallThickness = 3;
		float baseHeight = 5;
		float ceilingThickness = 3;

		// Create the tower structure
		architecture::CoordSys cylinderCoordSys = { architecture::CoordSysType::cylindrical, origin, { glm::vec3(0,0,1), glm::vec3(1,0,0), glm::vec3(0,1,0) } };

		glm::vec2 cylinderBounds[3] = { glm::vec2(0, radius),
									    glm::vec2(0, 2 * glm::pi<float>() - 0.0001),
									    glm::vec2(0, height) };

		Shape* towerStructure = new architecture::Shape(cylinderCoordSys, cylinderBounds);

		// Create inner room and walls
		std::string strucureNames[] = { "Room", "Wall" };
		SizePolicy structurePolicies[] = { SizePolicy::relative,
									  SizePolicy::absoluteTrue };
		float strucureSizes[] = { 1, wallThickness };

		towerStructure->subdivide(0, strucureNames, structurePolicies, strucureSizes, 2);

		// Adjust inner room
		std::string roomNames[] = { "Floor", "EMPTY", "Ceiling" };
		SizePolicy roomPolicies[] = { SizePolicy::absoluteTrue,
									  SizePolicy::relative,
									  SizePolicy::absoluteTrue };
		float roomSizes[] = { baseHeight, 1, ceilingThickness };
		int roomMask[] = { 1, 0, 1 };

		for (auto& room : *towerStructure->children["Room"])
		{
			room->subdivide(2, roomNames, roomPolicies, roomSizes, 3, roomMask);
		}

		// Ornate tower wall
		for (auto& wall : *towerStructure->children["Wall"]) castleOuterWall(wall);

		return towerStructure;
	}

	Shape* makeTower(glm::vec3 origin, glm::vec3 connectorDirs[], float connectorWidths[], size_t numConnectors)
	{
		float height = 40;
		float radius = 20;
		float wallThickness = 3;
		float baseHeight = 5;
		float ceilingThickness = 3;

		glm::vec3 upDir(0, 1, 0);

		std::vector<float> connectorAngleWidths;
		for (int i = 0; i < numConnectors; ++i)
		{
			connectorAngleWidths.push_back(2 * asin(connectorWidths[i] / (2 * radius)));
		}

		std::vector<float> angleWidths;
		if (numConnectors > 1)
		{
			for (int i = 0; i < numConnectors; ++i)
			{
				float wallAngle = glm::angle(glm::normalize(connectorDirs[i]), glm::normalize(connectorDirs[(i + 1) % numConnectors]));
				if (glm::dot(glm::cross(glm::normalize(connectorDirs[i]), glm::normalize(connectorDirs[(i + 1) % numConnectors])), upDir) < 0)
				{
					wallAngle = 2 * glm::pi<float>() - wallAngle;
				}

				wallAngle -= (connectorAngleWidths[i] + connectorAngleWidths[(i + 1) % numConnectors]) / 2.0;

				// TODO: Ineffective to have connector values in two parts of memory
				angleWidths.push_back(connectorAngleWidths[i]);
				angleWidths.push_back(wallAngle);
			}
		}
		else
		{
			angleWidths.push_back(connectorAngleWidths[0]);
			angleWidths.push_back(2 * glm::pi<float>() - connectorAngleWidths[0]);
		}

		glm::vec3 rDir = glm::rotate(glm::normalize(connectorDirs[0]), -connectorAngleWidths[0] / 2.0f, upDir);
		glm::vec3 phiDir = glm::cross(upDir, rDir);

		// Create the tower structure
		architecture::CoordSys cylinderCoordSys = { architecture::CoordSysType::cylindrical, origin, { rDir, phiDir, upDir } };

		glm::vec2 cylinderBounds[3] = { glm::vec2(0, radius),
										glm::vec2(0, 2 * glm::pi<float>() - 0.0001),
										glm::vec2(0, height) };

		Shape* towerStructure = new architecture::Shape(cylinderCoordSys, cylinderBounds);

		// Create inner room and walls
		std::string strucureNames[] = { "Room", "Wall" };
		SizePolicy structurePolicies[] = { SizePolicy::relative,
									  SizePolicy::absoluteTrue };
		float strucureSizes[] = { 1, wallThickness };

		towerStructure->subdivide(0, strucureNames, structurePolicies, strucureSizes, 2);

		// Adjust inner room
		std::string roomNames[] = { "Floor", "EMPTY", "Ceiling" };
		SizePolicy roomPolicies[] = { SizePolicy::absoluteTrue,
									  SizePolicy::relative,
									  SizePolicy::absoluteTrue };
		float roomSizes[] = { baseHeight, 1, ceilingThickness };
		int roomMask[] = { 1, 0, 1 };

		for (auto& room : *towerStructure->children["Room"])
		{
			room->subdivide(2, roomNames, roomPolicies, roomSizes, 3, roomMask);
		}

		// Split wall for connectors
		std::vector<std::string> connectorSplitNames(2 * numConnectors, "Wall Part");
		std::vector<SizePolicy> connectorSplitPolicies(2 * numConnectors, SizePolicy::absoluteTrue);
		std::vector<int> splitMask(2 * numConnectors, 1);
		for (size_t i = 0; i < numConnectors; ++i)
		{
			splitMask[2*i] = 0;
		}

		for (auto& wall : *towerStructure->children["Wall"])
		{
			wall->subdivide(1, connectorSplitNames.data(), connectorSplitPolicies.data(), angleWidths.data(), 2 * numConnectors, splitMask.data());

			for (Shape* wallPart : *wall->children["Wall Part"])
			{
				castleOuterWall(wallPart);
			}
		}

		return towerStructure;
	}

	Shape* makeWall(glm::vec3 start, glm::vec3 end)
	{
		float wallHeight = 40;
		float wallDepth = 10;
		float wallThickness = 3;
		float baseHeight = 5;
		float ceilingThickness = 3;

		glm::vec3 upDir(0, 1, 0);

		glm::vec3 yDir = glm::normalize(end - start);
		glm::vec3 xDir = glm::cross(yDir, upDir);

		CoordSys blockCoordSys = { CoordSysType::cartesian, start, { xDir, yDir, upDir } };

		glm::vec2 blockBounds[3] = { glm::vec2(-wallDepth, wallDepth),
									 glm::vec2(0, glm::length(end - start)),
									 glm::vec2(0, wallHeight) };

		Shape* wallStructure = new architecture::Shape(blockCoordSys, blockBounds);

		// Create inner room and walls
		std::string structureNames[] = { "Reverse Wall", "Room", "Wall" };
		SizePolicy structurePolicies[] = { SizePolicy::absoluteTrue,
									       SizePolicy::relative,
									       SizePolicy::absoluteTrue };
		float structureSizes[] = { wallThickness, 1, wallThickness };

		wallStructure->subdivide(0, structureNames, structurePolicies, structureSizes, 3);

		// Rotate coordinate system of back wall so that x points outwards
		// TODO: Separate into separate system
		for (auto& reverseWall : *wallStructure->children["Reverse Wall"])
		{
			reverseWall->coordSys.bases[0] *= -1;
			reverseWall->coordSys.bases[1] *= -1;
			glm::vec2 oldXBounds = reverseWall->bounds[0];
			reverseWall->bounds[0][0] = -1 * oldXBounds[1];
			reverseWall->bounds[0][1] = -1 * oldXBounds[0];
			glm::vec2 oldYBounds = reverseWall->bounds[1];
			reverseWall->bounds[1][0] = -1 * oldYBounds[1];
			reverseWall->bounds[1][1] = -1 * oldYBounds[0];
		}

		// Adjust inner room
		std::string roomNames[] = { "Floor", "EMPTY", "Ceiling" };
		SizePolicy roomPolicies[] = { SizePolicy::absoluteTrue,
									  SizePolicy::relative,
									  SizePolicy::absoluteTrue };
		float roomSizes[] = { baseHeight, 1, ceilingThickness };
		int roomMask[] = { 1, 0, 1 };

		for (auto& room : *wallStructure->children["Room"])
		{
			room->subdivide(2, roomNames, roomPolicies, roomSizes, 3, roomMask);
		}

		// Ornate walls
		for (architecture::Shape* wall : *wallStructure->children["Reverse Wall"])
		{
			architecture::castleOuterWall(wall);
		}
		for (architecture::Shape* wall : *wallStructure->children["Wall"])
		{
			architecture::castleOuterWall(wall);
		}

		return wallStructure;
	}

	std::vector<Shape*> makeWalls(glm::vec3 nodes[], size_t numNodes)
	{
		float wallHeight = 50;
		float wallDepth = 10;
		float wallThickness = 3;
		float towerRadius = 20;

		glm::vec3 upDir(0, 1, 0);

		std::vector<Shape*> structures;
		std::vector<Shape*> walls;

		// First tower and wall
		glm::vec3 startConnector[] = { nodes[1] - nodes[0] };
		float startConnectorWidth[] = { 2 * wallDepth };
		structures.push_back(makeTower(nodes[0], startConnector, startConnectorWidth, 1));

		float wallBuffer = sqrt(towerRadius * towerRadius - wallDepth * wallDepth);
		structures.push_back(makeWall(nodes[0] + wallBuffer * glm::normalize(nodes[1] - nodes[0]), nodes[1] - wallBuffer * glm::normalize(nodes[1] - nodes[0])));

		for (int i = 1; i < numNodes - 1; ++i)
		{
			// Place a tower
			glm::vec3 connector[] = { nodes[i + 1] - nodes[i], nodes[i - 1] - nodes[i] };
			float connectorWidth[] = { 2 * wallDepth, 2 * wallDepth };
			structures.push_back(makeTower(nodes[i], connector, connectorWidth, 2));

			// Place straight wall
			float wallBuffer = sqrt(towerRadius * towerRadius - wallDepth * wallDepth);
			structures.push_back(makeWall(nodes[i] + wallBuffer * glm::normalize(nodes[i + 1] - nodes[i]), nodes[i+1] - wallBuffer * glm::normalize(nodes[i + 1] - nodes[i])));
		}

		// Last tower
		glm::vec3 endConnector[] = { nodes[numNodes - 2] - nodes[numNodes - 1] };
		float endConnectorWidth[] = { 2 * wallDepth };
		structures.push_back(makeTower(nodes[numNodes - 1], endConnector, endConnectorWidth, 1));

		return(structures);
	}

	void castleWindows(Shape* wall)
	{
		std::vector<Shape*> segments;

		wall->repeat(1, "Native Segment", SizePolicy::absoluteOuter, 18, false);

		if (wall->coordSys.type == CoordSysType::cartesian)
		{
			segments = *wall->children["Native Segment"];
		}
		else if (wall->coordSys.type == CoordSysType::cylindrical)
		{
			for (architecture::Shape* cylinderSegment : *wall->children["Native Segment"])
			{
				cylinderSegment->wrapCartesianOverCylindrical("Cartesian Segment");
				for (auto& segment : *cylinderSegment->children["Cartesian Segment"]) segments.push_back(segment);
			}
		}
		else
		{
			throw std::invalid_argument("Invalid coordinate system type");
		}

		SizePolicy splitPolicies[] = { SizePolicy::relative,
									   SizePolicy::absoluteTrue,
									   SizePolicy::relative };

		std::string splitOuterNames[] = { "EMPTY", "Window", "EMPTY" };
		float splitSizesOuterWidth[] = { 1, 4, 1 };
		float splitSizesOuterHeight[] = { 1.5, 10, 1 };
		glm::vec2 frameExpansion[3] = { glm::vec2(1), glm::vec2(0), glm::vec2(0) };
		std::string splitInnerNames[] = { "Frame", "Space", "Frame" };
		float splitSizesInnerWidth[] = { 1, 2, 1 };
		float splitSizesInnerHeight[] = { 1.5, 8, 1 };
		int windowOuterMask[] = { 0, 1, 0 };
		int windowInnerMask[] = { 1, 0, 1 };
		for (architecture::Shape* segment : segments)
		{
			segment->subdivide(1, splitOuterNames, splitPolicies, splitSizesOuterWidth, 3, windowOuterMask);
			for (auto& wWindow : *segment->children["Window"])
			{
				wWindow->subdivide(2, splitOuterNames, splitPolicies, splitSizesOuterHeight, 3, windowOuterMask);
				for (auto& window : *wWindow->children["Window"])
				{ 
					window->boundsExpand(frameExpansion);
					window->subdivide(1, splitInnerNames, splitPolicies, splitSizesInnerWidth, 3);
					for (auto& space : *window->children["Space"])
					{
						space->subdivide(2, splitInnerNames, splitPolicies, splitSizesInnerHeight, 3, windowInnerMask);
					}
				}
			}
		}
		
		wall->parentChildOp = Shape::ParentChildOperator::unite;
	}

	void castleBattlement(Shape* wall)
	{
		// Parameters for the two sides of the wall and the walkway between
		float railingSize = 2.0f;
		std::string overhangNames[] = { "Floor", "Railing" };
		SizePolicy overhangPolicies[] = { SizePolicy::relative,
										  SizePolicy::absoluteTrue };
		float overhang[] = { 1, railingSize };
		glm::vec2 railingExpansion[] = { glm::vec2(0), glm::vec2(0), glm::vec2(0, 8) };

		// Parameters for the repeating sections of the wall
		float sectionLength = 12.0f;

		std::string sectionProportionNames[] = { "Portion", "Embrasure", "Portion" };
		SizePolicy sectionProportionPolicies[] = { SizePolicy::relative,
									               SizePolicy::absoluteOuter,
									               SizePolicy::relative };
		float sectionProportions[] = { 1, 2, 1 };
		glm::vec2 embrasureExpansion[] = { glm::vec2(0), glm::vec2(0), glm::vec2(0,-4) };

		wall->subdivide(0, overhangNames, overhangPolicies, overhang, 2);
		for (auto& railing : *wall->children["Railing"])
		{
			railing->boundsExpand(railingExpansion);

			railing->repeat(1, "Section", SizePolicy::absoluteOuter, sectionLength);

			for (architecture::Shape* section : *railing->children["Section"])
			{
				section->subdivide(1, sectionProportionNames, sectionProportionPolicies, sectionProportions, 3);
				for (auto& embrasure : * section->children["Embrasure"]) embrasure->boundsExpand(embrasureExpansion);
			}
		}
	}

	void castleOuterWall(Shape* wall)
	{
		std::string splitNames[] = { "Base", "Wall", "Battlement" };
		SizePolicy splitPolicies[] = { SizePolicy::absoluteTrue,
									   SizePolicy::relative,
									   SizePolicy::absoluteTrue };
		float splitSizes[] = { 5, 1, 5 };
		wall->subdivide(2, splitNames, splitPolicies, splitSizes, 3);

		glm::vec2 baseExpansion[] = { glm::vec2(0, 2), glm::vec2(0), glm::vec2(0) };
		for (auto& base : *wall->children["Base"]) base->boundsExpand(baseExpansion);

		for (auto& vertWall : *wall->children["Wall"]) castleWindows(vertWall);

		for (auto& battlement : *wall->children["Battlement"])
		{
			battlement->boundsExpand(baseExpansion);
			castleBattlement(battlement);
		}
	}
}