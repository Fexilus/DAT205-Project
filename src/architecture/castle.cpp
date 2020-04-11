#include "castle.h"

namespace architecture
{
	void Shape::init()
	{
		// Create a handle for the vertex array object
		glGenVertexArrays(1, &vao);
		// Set it as current, i.e., related calls will affect this object
		glBindVertexArray(vao);


		// Define vertecies of bounding box
		// TODO: Do not redo this all the time
		const float positions[] = {
			// X		     Y             Z
			bounds[0][0], bounds[1][0], bounds[2][0],
			bounds[0][0], bounds[1][0], bounds[2][1],
			bounds[0][0], bounds[1][1], bounds[2][0],
			bounds[0][0], bounds[1][1], bounds[2][1],

			bounds[0][1], bounds[1][0], bounds[2][0],
			bounds[0][1], bounds[1][1], bounds[2][0],
			bounds[0][1], bounds[1][0], bounds[2][1],
			bounds[0][1], bounds[1][1], bounds[2][1],

			bounds[0][0], bounds[1][0], bounds[2][0],
			bounds[0][1], bounds[1][0], bounds[2][0],
			bounds[0][0], bounds[1][0], bounds[2][1],
			bounds[0][1], bounds[1][0], bounds[2][1],

			bounds[0][0], bounds[1][1], bounds[2][0],
			bounds[0][0], bounds[1][1], bounds[2][1],
			bounds[0][1], bounds[1][1], bounds[2][0],
			bounds[0][1], bounds[1][1], bounds[2][1],

			bounds[0][0], bounds[1][0], bounds[2][0],
			bounds[0][0], bounds[1][1], bounds[2][0],
			bounds[0][1], bounds[1][0], bounds[2][0],
			bounds[0][1], bounds[1][1], bounds[2][0],

			bounds[0][0], bounds[1][0], bounds[2][1],
			bounds[0][1], bounds[1][0], bounds[2][1],
			bounds[0][0], bounds[1][1], bounds[2][1],
			bounds[0][1], bounds[1][1], bounds[2][1],
		};

		// Define normals
		const float normals[] = {
			-1,  0,  0,
			-1,  0,  0,
			-1,  0,  0,
			-1,  0,  0,

			 1,  0,  0,
			 1,  0,  0,
			 1,  0,  0,
			 1,  0,  0,

			 0, -1,  0,
			 0, -1,  0,
			 0, -1,  0,
			 0, -1,  0,

			 0,  1,  0,
			 0,  1,  0,
			 0,  1,  0,
			 0,  1,  0,

			 0,  0, -1,
			 0,  0, -1,
			 0,  0, -1,
			 0,  0, -1,

			 0,  0,  1,
			 0,  0,  1,
			 0,  0,  1,
			 0,  0,  1,
		};

		//Define the corresponding indicies
		const int indices[] = {
			 0,  1,  2,
			 3,  2,  1, // Side 1
			 4,  5,  6,
			 7,  6,  5, // Side 2
			 8,  9, 10,
			11, 10,  9, // Side 3
			12, 13, 14,
			15, 14, 13, // Side 4
			16, 17, 18,
			19, 18, 17, // Side 5
			20, 21, 22,
			23, 22, 21  // Side 6
		};

		GLuint positionBuffer;
		// Create a handle for the vertex position buffer
		glGenBuffers(1, &positionBuffer);
		// Set the newly created buffer as the current one
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		// Send the vetex position data to the current buffer
		glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, false /*normalized*/, 0 /*stride*/, 0 /*offset*/);
		// Enable the attribute
		glEnableVertexAttribArray(0);

		GLuint normalBuffer;
		// Create a handle for the vertex position buffer
		glGenBuffers(1, &normalBuffer);
		// Set the newly created buffer as the current one
		glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
		// Send the vetex position data to the current buffer
		glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, false /*normalized*/, 0 /*stride*/, 0 /*offset*/);
		// Enable the attribute
		glEnableVertexAttribArray(1);

		GLuint indexBuffer;
		glGenBuffers(1, &indexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	}

	void Shape::render()
	{
		glBindVertexArray(vao);
		GLint current_program = 0;

		glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);
		glm::vec3 m_color(0.61, 0.56, 0.52);
		float m_fresnel = 0.5;
		glUniform3fv(glGetUniformLocation(current_program, "material_color"), 1, &m_color.x);
		glUniform1fv(glGetUniformLocation(current_program, "material_fresnel"), 1, &m_fresnel);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	}
}