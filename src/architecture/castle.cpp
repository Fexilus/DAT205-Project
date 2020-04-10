#include "castle.h"

#include <GL/glew.h>

namespace architecture
{
	void Shape::render()
	{
		GLuint vao;
		// Create a handle for the vertex array object
		glGenVertexArrays(1, &vao);
		// Set it as current, i.e., related calls will affect this object
		glBindVertexArray(vao);


		// Define vertecies of bounding box
		// TODO: Do not redo this all the time
		const float positions[] = {
			// X		     Y             Z
			bounds[0][0], bounds[1][0], bounds[2][0],  // v0
			bounds[0][1], bounds[1][0], bounds[2][0],  // v1
			bounds[0][0], bounds[1][1], bounds[2][0],  // v2
			bounds[0][1], bounds[1][1], bounds[2][0],  // v3
			bounds[0][0], bounds[1][0], bounds[2][1],  // v4
			bounds[0][1], bounds[1][0], bounds[2][1],  // v5
			bounds[0][0], bounds[1][1], bounds[2][1],  // v6
			bounds[0][1], bounds[1][1], bounds[2][1],  // v7
		};
		//Define the corresponding indicies
		const int indices[] = {
			1, 0, 2,
			2, 3, 1, // Side 1
			4, 0, 1,
			1, 5, 4, // Side 2
			2, 0, 4,
			4, 6, 2, // Side 3
			5, 7, 6,
			6, 4, 5, // Side 4
			6, 7, 3,
			3, 2, 6, // Side 5
			3, 7, 5,
			5, 1, 3  // Side 6
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

		GLuint indexBuffer;
		glGenBuffers(1, &indexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	}
}