#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

namespace architecture
{
	class Shape
	{
	public:
		// The bounds of the shape in the 3 (for now unmodifiable) coordinates
		glm::vec2 bounds[3];
		// The vertex array object
		GLuint vao;
		// Child shapes
		std::vector<Shape*> children;

		Shape(glm::vec2 bounds_[3]);

		void init();
		void render();

		// Operators
		void subdivide(int axis, float splits[], size_t num_splits);
	};
}