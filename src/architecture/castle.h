#pragma once

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

		void init();
		void render();
	};
}