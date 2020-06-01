#include "castle.h"

#include <vector>
#include <stdexcept>

#include <range.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

using util::lang::indices;

namespace architecture
{
	Shape* makeTower(glm::vec3 origin, float height /*= 40*/, float radius /*= 20*/)
	{
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

	Shape* makeTower(glm::vec3 origin, glm::vec3 connectorDirs[], float connectorWidths[], size_t numConnectors, float height /*= 40*/, float radius /*= 20*/)
	{
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
		std::vector<std::string> connectorSplitNames;
		std::vector<SizePolicy> connectorSplitPolicies;
		std::vector<int> splitMask;
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
				if (wallAngle >= 0)
				{
					angleWidths.push_back(connectorAngleWidths[i]);
					connectorSplitNames.push_back("Connector");
					connectorSplitPolicies.push_back(SizePolicy::absoluteTrue);
					splitMask.push_back(0);

					angleWidths.push_back(wallAngle);
					connectorSplitNames.push_back("Wall Part");
					connectorSplitPolicies.push_back(SizePolicy::absoluteTrue);
					splitMask.push_back(1);
				}
				else
				{
					angleWidths.push_back(connectorAngleWidths[i] - wallAngle);
					connectorSplitNames.push_back("Connector");
					connectorSplitPolicies.push_back(SizePolicy::absoluteTrue);
					splitMask.push_back(0);
				}
			}
		}
		else
		{
			angleWidths.push_back(connectorAngleWidths[0]);
			connectorSplitNames.push_back("Connector");
			connectorSplitPolicies.push_back(SizePolicy::absoluteTrue);
			splitMask.push_back(0);

			angleWidths.push_back(2 * glm::pi<float>() - connectorAngleWidths[0]);
			connectorSplitNames.push_back("Wall Part");
			connectorSplitPolicies.push_back(SizePolicy::absoluteTrue);
			splitMask.push_back(1);
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
		for (auto& wall : *towerStructure->children["Wall"])
		{
			wall->subdivide(1, connectorSplitNames.data(), connectorSplitPolicies.data(), angleWidths.data(), angleWidths.size(), splitMask.data());

			for (Shape* wallPart : *wall->children["Wall Part"])
			{
				castleOuterWall(wallPart);
			}
		}

		return towerStructure;
	}

	Shape* makeWall(glm::vec3 start, glm::vec3 end, float wallHeight /*= 40*/)
	{
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

	std::vector<CastlePart*> makeWalls(glm::vec3 nodes[], size_t numNodes)
	{
		//float wallHeight = 50;
		//float wallDepth = 10;
		//float wallThickness = 3;
		//float towerRadius = 20;

		//glm::vec3 upDir(0, 1, 0);

		std::vector<CastlePart*> structures(2 * numNodes - 1);

		// First tower and wall
		structures[0] = new CastleTower(nodes[0]);

		for (int i = 1; i < numNodes; ++i)
		{
			// Place a tower
			structures[i] = new CastleTower(nodes[i]);

			// Place straight wall
			structures[numNodes - 1 + i] = new ConnectingCastleWall((CastleTower*)structures[i-1], (CastleTower*)structures[i]);

			CastleTower::Connector backConnector = { (ConnectingCastleWall*)structures[numNodes - 1 + i], (CastleTower*)structures[i] };
			((CastleTower*)structures[i - 1])->connectors.push_back(backConnector);
			CastleTower::Connector forwardConnector = { (ConnectingCastleWall*)structures[numNodes - 1 + i], (CastleTower*)structures[i - 1] };
			((CastleTower*)structures[i])->connectors.push_back(forwardConnector);
		}

		return(structures);
	}

	void castleWindows(Shape* wall)
	{
		wall->repeat(2, "Level", SizePolicy::absoluteOuter, 28, 1, PaddingType::high);
		for (auto& level : *wall->children["Level"])
		{
			std::vector<Shape*> segments;

			level->repeat(1, "Native Segment", SizePolicy::absoluteOuter, 18, false);

			if (level->coordSys.type == CoordSysType::cartesian)
			{
				segments = *level->children["Native Segment"];
			}
			else if (level->coordSys.type == CoordSysType::cylindrical)
			{
				for (architecture::Shape* cylinderSegment : *level->children["Native Segment"])
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

			level->parentChildOp = Shape::ParentChildOperator::unite;
		}
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
	
	void CastlePart::render()
	{
		shape->render();
	}

	CastleTower::CastleTower(glm::vec3 origin) :
		origin(origin)
	{
	}

	void CastleTower::init()
	{
		if (shape != nullptr) delete shape;

		if (connectors.size() == 0) shape = makeTower(origin, height(), radius());
		else
		{
			std::vector<glm::vec3> connectorDirs(connectors.size());
			std::vector<float> connectorWidths(connectors.size());
			for (int i : indices(connectors))
			{
				connectorDirs[i] = connectors[i].tower->origin - origin;
				connectorWidths[i] = connectors[i].wall->width();
			}


			shape = makeTower(origin, connectorDirs.data(), connectorWidths.data(), connectors.size(), height(), radius());
		}

		shape->init();
	}

	void CastleTower::set_height(float newHeight)
	{
		height_ = newHeight;
		init();
		for (auto& connector : connectors)
		{
			if (connector.wall->height() > newHeight)
			{
				connector.wall->set_height(newHeight);
				connector.wall->init();
			}
		}
	}

	void CastleTower::set_radius(float newRadius)
	{
		radius_ = newRadius;
		init();
		for (auto& connector : connectors)
		{
			connector.wall->init();
		}
	}

	void CastleTower::move(glm::vec3 movement)
	{
		origin += movement;
		init();

		for (auto& connector : connectors)
		{
			connector.wall->init();
			connector.tower->init();
		}
	}

	ConnectingCastleWall::ConnectingCastleWall(CastleTower* node1, CastleTower* node2) :
		node1(node1),
		node2(node2)
	{
	}

	void ConnectingCastleWall::set_height(float newHeight)
	{
		height_ = newHeight;
		init();
		if (node1->height() < newHeight)
		{
			node1->set_height(newHeight);
			node1->init();
		}
		if (node2->height() < newHeight)
		{
			node2->set_height(newHeight);
			node2->init();
		}
	}

	void ConnectingCastleWall::init()
	{
		if (shape != nullptr) delete shape;

		float wallBuffer1 = sqrt(node1->radius() * node1->radius() - width() * width() / 4);
		float wallBuffer2 = sqrt(node2->radius() * node2->radius() - width() * width() / 4);
		shape = makeWall(node1->origin + wallBuffer1 * glm::normalize(node2->origin - node1->origin), node2->origin - wallBuffer2 * glm::normalize(node2->origin - node1->origin), height());

		shape->init();
	}

	void ConnectingCastleWall::move(glm::vec3 movement)
	{
	}
}