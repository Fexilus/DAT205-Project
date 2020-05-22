#pragma once

#include <vector>

#include <shape.h>

namespace architecture
{
	class CastlePart
	{
	protected:
		Shape* shape = nullptr;
	public:
		virtual void move(glm::vec3 movement) = 0;
		virtual void init() = 0;
		void render();
	};

	class CastleTower;
	class ConnectingCastleWall;

	class CastleTower : public CastlePart
	{
	private:
		float height = 40;
		float radius_ = 20;
		float wallThickness = 3;
		float baseHeight = 5;
		float ceilingThickness = 3;
	public:
		float radius() const { return radius_; }

		glm::vec3 origin;

		struct Connector
		{
			ConnectingCastleWall* wall;
			CastleTower* tower;
		};
		std::vector<Connector> connectors;

		CastleTower(glm::vec3 origin);

		void init();

		void move(glm::vec3 movement);
	};

	class ConnectingCastleWall : public CastlePart
	{
	private:
		float height = 40;
		float width_ = 20;
	public:
		float width() const { return width_; }

		CastleTower* node1;
		CastleTower* node2;

		ConnectingCastleWall(CastleTower* node1, CastleTower* node2);

		void init();

		void move(glm::vec3 movement);
	};

	// Rules on allignment elements
	Shape* makeTower(glm::vec3 origin);
	Shape* makeTower(glm::vec3 origin, glm::vec3 connectorDirs[], float connectorWidths[], size_t numConnectors);
	Shape* makeWall(glm::vec3 start, glm::vec3 end);
	std::vector<CastlePart*> makeWalls(glm::vec3 nodes[], size_t numNodes);

	// Rules on shapes
	void castleWindows(Shape* wall);
	void castleBattlement(Shape* wall);
	void castleOuterWall(Shape* wall);
}