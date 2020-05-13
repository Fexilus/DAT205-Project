#pragma once

#include <vector>
#include <unordered_map>

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

	// Types of size values that can be given to shape splitting
	enum class SizePolicy { 
		absoluteTrue,  // Size with the native unit of the axis to split
		relative,      // Relative size to all other relative sizes
		absoluteInner, // Size with the cartesian unit, measured on the shortest part of the geometry
		absoluteOuter  // Size with the cartesian unit, measured on the longest part of the geometry
	};

	// Main shape definition
	class Shape
	{
	public:
		// The basis vectors of the shapes coordinate system in model space
		CoordSys coordSys;
		// The min and max bounds of the shape in the shape's coordinate system
		glm::vec2 bounds[3];
		// Child shapes
		std::unordered_map<std::string, std::vector<Shape*>*> children;
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
		void subdivide(int axis, std::string names[], SizePolicy policies[], float sizeVals[], size_t numSubEl);
		void subdivide(int axis, std::string names[], SizePolicy policies[], float sizeVals[], size_t numSubEl, int mask[]);
		void repeat(int axis, std::string name, SizePolicy policy, float sizeVal, int paddingMask = true);
		void boundsExpand(glm::vec2 boundExpansions[3]);
		void wrapCartesianOverCylindrical(std::string name);

	private:
		// Utility functions
		void adjustPhiBounds();
		float absoluteRescaling(int axis, SizePolicy policy);
	};
}