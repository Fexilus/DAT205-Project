#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

namespace architecture
{
	enum class SizePolicy { absolute, relative };

	class Shape
	{
	public:
		// The bounds of the shape in the 3 (for now unmodifiable) coordinates
		glm::vec2 bounds[3];
		// Child shapes
		std::vector<Shape*> children;

	private:
		// The vertex array object
		GLuint vao;

	public:

		Shape(glm::vec2 bounds_[3]);

		void init();
		void render();

		// Operators
		void subdivide(int axis, SizePolicy policy[], float size[], size_t numSubEl);
	};
}