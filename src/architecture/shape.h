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

	class Shape
	{
	public:
		// The basis vectors of the shapes coordinate system in model space
		CoordSys coordSys;
		// The min and max bounds of the shape in the shape's coordinate system
		glm::vec2 bounds[3];
		// Child shapes
		std::vector<Shape*> children;

	private:
		// The vertex array object
		GLuint vao;
		int numNodes;

	public:

		Shape(CoordSys coordSys, glm::vec2 bounds[3]);

		void init();
		void render();

		// Operators
		void subdivide(int axis, SizePolicy policies[], float sizeVals[], size_t numSubEl);
		void subdivide(int axis, SizePolicy policies[], float sizeVals[], size_t numSubEl, int mask[]);
		void repeat(int axis, SizePolicy policy, float sizeVal);
		void boundsExpand(glm::vec2 boundExpansions[3]);
	};
}