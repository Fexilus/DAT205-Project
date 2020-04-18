#pragma once

#include <vector>
//#include <unordered_map>

#include <GL/glew.h>
#include <glm/glm.hpp>

namespace architecture
{
	enum class CoordSysType 
	{ 
		cartesian,  // x,   y, z
		cylindrical // r, phi, z (phi in radians, will be interpreted as values between 0 and 2pi)
	};

	struct CoordSys
	{
		CoordSysType type;
		glm::vec3 origin;
		glm::vec3 bases[3];
	};

	enum class SizePolicy { absolute, relative };

	// Main shape definition
	class Shape
	{
	public:
		// The basis vectors of the shapes coordinate system in model space
		CoordSys coordSys;
		// The min and max bounds of the shape in the shape's coordinate system
		glm::vec2 bounds[3];
		// Child shapes
		std::vector<Shape*> children;
		// Geometry resolution
		// First this operation is done across all children
		enum class ChildChildOperator { unite, intersect } childChildOp;
		// Then this operation is done with the resulting geometry and the owner. None ignore owner geometry)
		enum class ParentChildOperator { none, unite, intersect, subtract } parentChildOp;

	private:
		// The vertex array object
		GLuint vao;
		// The number of nodes to draw in the
		int numNodes = 0;

	public:

		Shape(CoordSys coordSys, glm::vec2 bounds[3]);

		void init();
		void render();

		// Operators
		void subdivide(int axis, SizePolicy policies[], float sizeVals[], size_t numSubEl);
		void subdivide(int axis, SizePolicy policies[], float sizeVals[], size_t numSubEl, int mask[]);
		void repeat(int axis, SizePolicy policy, float sizeVal);
		void boundsExpand(glm::vec2 boundExpansions[3]);
		void wrapCartesianOverCylindrical();

	private:
		// Utility functions
		void adjustPhiBounds();
	};
}