#include "castle.h"

#include <glm/gtc/constants.hpp>

namespace architecture
{
	Shape::Shape(CoordSys coordSys, glm::vec2 bounds_[3]) : 
		coordSys(coordSys),
		parentChildOp(ParentChildOperator::none),
		childChildOp(ChildChildOperator::unite)
	{
		bounds[0] = bounds_[0];
		bounds[1] = bounds_[1];
		bounds[2] = bounds_[2];
	}

	Shape::~Shape()
	{
		for (auto& childVec : children)
		{
			for (auto& child : *childVec.second)
			{
				delete child;
			}

			delete childVec.second;
		}

		if (vao != 0) glDeleteVertexArrays(1, &vao);
		if (positionBuffer != 0) glDeleteBuffers(1, &positionBuffer);
		if (normalBuffer != 0) glDeleteBuffers(1, &normalBuffer);
		if (indexBuffer != 0) glDeleteBuffers(1, &indexBuffer);
	}

	void Shape::init()
	{
		for (auto& childCollection : children)
		{
			for (Shape* child : *childCollection.second)
			{
				child->init();
			}
		}
		if ((children.size() == 0) | (parentChildOp != ParentChildOperator::none) | (childChildOp == ChildChildOperator::intersect))
		{
			// Create a handle for the vertex array object
			if (vao == 0) glGenVertexArrays(1, &vao);
			// Set it as current, i.e., related calls will affect this object
			glBindVertexArray(vao);

			if (childChildOp == ChildChildOperator::intersect)
			{
				bool firstMesh = true;
				boolean3d::Mesh resultMesh;

				for (auto& childCollection : children)
				{
					for (Shape* child : *childCollection.second)
					{
						if (firstMesh)
						{
							resultMesh = boolean3d::toMesh(child->soup);
							firstMesh = false;
						}
						else
						{
							boolean3d::Mesh curMesh = boolean3d::toMesh(child->soup);
							resultMesh = boolean3d::intersect(curMesh, resultMesh);
						}
					}
				}

				soup = boolean3d::fromMesh(resultMesh);
			}
			else
			{
				if (coordSys.type == CoordSysType::cartesian)
				{
					glm::mat3 coordMatrix(coordSys.bases[0], coordSys.bases[1], coordSys.bases[2]);

					// Define vertices of bounding box
					soup.positions = {
						// X		     Y             Z
						coordSys.origin + coordMatrix * glm::vec3(bounds[0][0], bounds[1][0], bounds[2][0]),
						coordSys.origin + coordMatrix * glm::vec3(bounds[0][0], bounds[1][0], bounds[2][1]),
						coordSys.origin + coordMatrix * glm::vec3(bounds[0][0], bounds[1][1], bounds[2][0]),
						coordSys.origin + coordMatrix * glm::vec3(bounds[0][0], bounds[1][1], bounds[2][1]),

						coordSys.origin + coordMatrix * glm::vec3(bounds[0][1], bounds[1][0], bounds[2][0]),
						coordSys.origin + coordMatrix * glm::vec3(bounds[0][1], bounds[1][1], bounds[2][0]),
						coordSys.origin + coordMatrix * glm::vec3(bounds[0][1], bounds[1][0], bounds[2][1]),
						coordSys.origin + coordMatrix * glm::vec3(bounds[0][1], bounds[1][1], bounds[2][1]),

						coordSys.origin + coordMatrix * glm::vec3(bounds[0][0], bounds[1][0], bounds[2][0]),
						coordSys.origin + coordMatrix * glm::vec3(bounds[0][1], bounds[1][0], bounds[2][0]),
						coordSys.origin + coordMatrix * glm::vec3(bounds[0][0], bounds[1][0], bounds[2][1]),
						coordSys.origin + coordMatrix * glm::vec3(bounds[0][1], bounds[1][0], bounds[2][1]),

						coordSys.origin + coordMatrix * glm::vec3(bounds[0][0], bounds[1][1], bounds[2][0]),
						coordSys.origin + coordMatrix * glm::vec3(bounds[0][0], bounds[1][1], bounds[2][1]),
						coordSys.origin + coordMatrix * glm::vec3(bounds[0][1], bounds[1][1], bounds[2][0]),
						coordSys.origin + coordMatrix * glm::vec3(bounds[0][1], bounds[1][1], bounds[2][1]),

						coordSys.origin + coordMatrix * glm::vec3(bounds[0][0], bounds[1][0], bounds[2][0]),
						coordSys.origin + coordMatrix * glm::vec3(bounds[0][0], bounds[1][1], bounds[2][0]),
						coordSys.origin + coordMatrix * glm::vec3(bounds[0][1], bounds[1][0], bounds[2][0]),
						coordSys.origin + coordMatrix * glm::vec3(bounds[0][1], bounds[1][1], bounds[2][0]),

						coordSys.origin + coordMatrix * glm::vec3(bounds[0][0], bounds[1][0], bounds[2][1]),
						coordSys.origin + coordMatrix * glm::vec3(bounds[0][1], bounds[1][0], bounds[2][1]),
						coordSys.origin + coordMatrix * glm::vec3(bounds[0][0], bounds[1][1], bounds[2][1]),
						coordSys.origin + coordMatrix * glm::vec3(bounds[0][1], bounds[1][1], bounds[2][1]),
					};

					// Define mesh.normals
					soup.normals = {
						coordMatrix * glm::vec3(-1,  0,  0),
						coordMatrix * glm::vec3(-1,  0,  0),
						coordMatrix * glm::vec3(-1,  0,  0),
						coordMatrix * glm::vec3(-1,  0,  0),

						coordMatrix * glm::vec3(1,  0,  0),
						coordMatrix * glm::vec3(1,  0,  0),
						coordMatrix * glm::vec3(1,  0,  0),
						coordMatrix * glm::vec3(1,  0,  0),

						coordMatrix * glm::vec3(0, -1,  0),
						coordMatrix * glm::vec3(0, -1,  0),
						coordMatrix * glm::vec3(0, -1,  0),
						coordMatrix * glm::vec3(0, -1,  0),

						coordMatrix * glm::vec3(0,  1,  0),
						coordMatrix * glm::vec3(0,  1,  0),
						coordMatrix * glm::vec3(0,  1,  0),
						coordMatrix * glm::vec3(0,  1,  0),

						coordMatrix * glm::vec3(0,  0, -1),
						coordMatrix * glm::vec3(0,  0, -1),
						coordMatrix * glm::vec3(0,  0, -1),
						coordMatrix * glm::vec3(0,  0, -1),

						coordMatrix * glm::vec3(0,  0,  1),
						coordMatrix * glm::vec3(0,  0,  1),
						coordMatrix * glm::vec3(0,  0,  1),
						coordMatrix * glm::vec3(0,  0,  1),
					};

					//Define the corresponding indicies
					soup.indices = {
						glm::ivec3(0,  1,  2),
						glm::ivec3(3,  2,  1), // Side 1
						glm::ivec3(4,  5,  6),
						glm::ivec3(7,  6,  5), // Side 2
						glm::ivec3(8,  9, 10),
						glm::ivec3(11, 10,  9), // Side 3
						glm::ivec3(12, 13, 14),
						glm::ivec3(15, 14, 13), // Side 4
						glm::ivec3(16, 17, 18),
						glm::ivec3(19, 18, 17), // Side 5
						glm::ivec3(20, 21, 22),
						glm::ivec3(23, 22, 21)  // Side 6
					};
				}
				else if (coordSys.type == CoordSysType::cylindrical)
				{
					int resolution = 20;

					adjustPhiBounds();
					//float circleFrac = (bounds[1][1] - bounds[1][0]) / (2 * glm::pi<float>());

					// TODO: Make complete circle at when end point is almost at begining point
					int startNode = ceil(bounds[1][0] / (2 * glm::pi<float>()) * resolution);
					int endNode = floor(bounds[1][1] / (2 * glm::pi<float>()) * resolution);
					int numCircNodes = endNode - startNode + 3;
					std::vector<glm::vec3> circNodes(numCircNodes);

					float phiStep = 2 * glm::pi<float>() / resolution;

					for (size_t i = 0; i < numCircNodes - 2; ++i)
					{
						circNodes[i + 1] = cosf((startNode + i) * phiStep) * coordSys.bases[0] + sinf((startNode + i) * phiStep) * coordSys.bases[1];
					}

					circNodes[0] = cosf(bounds[1][0]) * coordSys.bases[0] + sinf(bounds[1][0]) * coordSys.bases[1];
					circNodes[numCircNodes - 1] = cosf(bounds[1][1]) * coordSys.bases[0] + sinf(bounds[1][1]) * coordSys.bases[1];

					// Nodes for wedge opening side
					soup.positions.push_back(coordSys.origin + bounds[0][1] * circNodes[0] + bounds[2][0] * coordSys.bases[2]);
					soup.positions.push_back(coordSys.origin + bounds[0][1] * circNodes[0] + bounds[2][1] * coordSys.bases[2]);
					soup.positions.push_back(coordSys.origin + bounds[0][0] * circNodes[0] + bounds[2][1] * coordSys.bases[2]);
					soup.positions.push_back(coordSys.origin + bounds[0][0] * circNodes[0] + bounds[2][0] * coordSys.bases[2]);

					soup.normals.push_back(-glm::cross(coordSys.bases[2], circNodes[0]));
					soup.normals.push_back(-glm::cross(coordSys.bases[2], circNodes[0]));
					soup.normals.push_back(-glm::cross(coordSys.bases[2], circNodes[0]));
					soup.normals.push_back(-glm::cross(coordSys.bases[2], circNodes[0]));

					soup.indices.push_back(glm::ivec3(0, 1, 2));
					soup.indices.push_back(glm::ivec3(2, 3, 0));

					// Initial circular nodes
					soup.positions.push_back(coordSys.origin + bounds[0][1] * circNodes[0] + bounds[2][0] * coordSys.bases[2]);
					soup.positions.push_back(coordSys.origin + bounds[0][1] * circNodes[0] + bounds[2][1] * coordSys.bases[2]);
					soup.positions.push_back(coordSys.origin + bounds[0][0] * circNodes[0] + bounds[2][1] * coordSys.bases[2]);
					soup.positions.push_back(coordSys.origin + bounds[0][0] * circNodes[0] + bounds[2][0] * coordSys.bases[2]);

					soup.normals.push_back(circNodes[0]);
					soup.normals.push_back(circNodes[0]);
					soup.normals.push_back(-circNodes[0]);
					soup.normals.push_back(-circNodes[0]);

					soup.positions.push_back(coordSys.origin + bounds[0][1] * circNodes[0] + bounds[2][0] * coordSys.bases[2]);
					soup.positions.push_back(coordSys.origin + bounds[0][1] * circNodes[0] + bounds[2][1] * coordSys.bases[2]);
					soup.positions.push_back(coordSys.origin + bounds[0][0] * circNodes[0] + bounds[2][1] * coordSys.bases[2]);
					soup.positions.push_back(coordSys.origin + bounds[0][0] * circNodes[0] + bounds[2][0] * coordSys.bases[2]);

					soup.normals.push_back(-coordSys.bases[2]);
					soup.normals.push_back(coordSys.bases[2]);
					soup.normals.push_back(coordSys.bases[2]);
					soup.normals.push_back(-coordSys.bases[2]);

					for (int i = 1; i < numCircNodes; ++i)
					{
						// The outer and inner part of the cylinder
						soup.positions.push_back(coordSys.origin + bounds[0][1] * circNodes[i] + bounds[2][0] * coordSys.bases[2]);
						soup.positions.push_back(coordSys.origin + bounds[0][1] * circNodes[i] + bounds[2][1] * coordSys.bases[2]);
						soup.positions.push_back(coordSys.origin + bounds[0][0] * circNodes[i] + bounds[2][1] * coordSys.bases[2]);
						soup.positions.push_back(coordSys.origin + bounds[0][0] * circNodes[i] + bounds[2][0] * coordSys.bases[2]);

						soup.normals.push_back(circNodes[i]);
						soup.normals.push_back(circNodes[i]);
						soup.normals.push_back(-circNodes[i]);
						soup.normals.push_back(-circNodes[i]);

						soup.indices.push_back(4 + glm::ivec3(8 * (i - 1), 8 * i, 8 * i + 1));
						soup.indices.push_back(4 + glm::ivec3(8 * i + 1, 8 * (i - 1) + 1, 8 * (i - 1)));
						soup.indices.push_back(4 + glm::ivec3(8 * (i - 1) + 3, 8 * (i - 1) + 2, 8 * i + 2));
						soup.indices.push_back(4 + glm::ivec3(8 * i + 2, 8 * i + 3, 8 * (i - 1) + 3));

						// The top and bottom part of the cylinder
						soup.positions.push_back(coordSys.origin + bounds[0][1] * circNodes[i] + bounds[2][0] * coordSys.bases[2]);
						soup.positions.push_back(coordSys.origin + bounds[0][1] * circNodes[i] + bounds[2][1] * coordSys.bases[2]);
						soup.positions.push_back(coordSys.origin + bounds[0][0] * circNodes[i] + bounds[2][1] * coordSys.bases[2]);
						soup.positions.push_back(coordSys.origin + bounds[0][0] * circNodes[i] + bounds[2][0] * coordSys.bases[2]);

						soup.normals.push_back(-coordSys.bases[2]);
						soup.normals.push_back(coordSys.bases[2]);
						soup.normals.push_back(coordSys.bases[2]);
						soup.normals.push_back(-coordSys.bases[2]);

						soup.indices.push_back(8 + glm::ivec3(8 * (i - 1), 8 * (i - 1) + 3, 8 * i + 3));
						soup.indices.push_back(8 + glm::ivec3(8 * i + 3, 8 * i, 8 * (i - 1)));
						soup.indices.push_back(8 + glm::ivec3(8 * (i - 1) + 1, 8 * i + 1, 8 * i + 2));
						soup.indices.push_back(8 + glm::ivec3(8 * i + 2, 8 * (i - 1) + 2, 8 * (i - 1) + 1));
					}

					// Nodes for wedge closing side
					soup.positions.push_back(coordSys.origin + bounds[0][1] * circNodes[numCircNodes - 1] + bounds[2][0] * coordSys.bases[2]);
					soup.positions.push_back(coordSys.origin + bounds[0][1] * circNodes[numCircNodes - 1] + bounds[2][1] * coordSys.bases[2]);
					soup.positions.push_back(coordSys.origin + bounds[0][0] * circNodes[numCircNodes - 1] + bounds[2][1] * coordSys.bases[2]);
					soup.positions.push_back(coordSys.origin + bounds[0][0] * circNodes[numCircNodes - 1] + bounds[2][0] * coordSys.bases[2]);

					soup.normals.push_back(glm::cross(coordSys.bases[2], circNodes[numCircNodes - 1]));
					soup.normals.push_back(glm::cross(coordSys.bases[2], circNodes[numCircNodes - 1]));
					soup.normals.push_back(glm::cross(coordSys.bases[2], circNodes[numCircNodes - 1]));
					soup.normals.push_back(glm::cross(coordSys.bases[2], circNodes[numCircNodes - 1]));

					soup.indices.push_back(4 + 8 * numCircNodes + glm::ivec3(0, 3, 2));
					soup.indices.push_back(4 + 8 * numCircNodes + glm::ivec3(2, 1, 0));
				}
			}

			// Create a handle for the vertex position buffer
			if(positionBuffer == 0) glGenBuffers(1, &positionBuffer);
			// Set the newly created buffer as the current one
			glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
			// Send the vetex position data to the current buffer
			glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * soup.positions.size(), soup.positions.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, false /*normalized*/, 0 /*stride*/, 0 /*offset*/);
			// Enable the attribute
			glEnableVertexAttribArray(0);

			// Create a handle for the vertex position buffer
			if (normalBuffer == 0) glGenBuffers(1, &normalBuffer);
			// Set the newly created buffer as the current one
			glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
			// Send the vetex position data to the current buffer
			glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * soup.normals.size(), soup.normals.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(1, 3, GL_FLOAT, false /*normalized*/, 0 /*stride*/, 0 /*offset*/);
			// Enable the attribute
			glEnableVertexAttribArray(1);

			if (indexBuffer == 0) glGenBuffers(1, &indexBuffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::ivec3) * soup.indices.size(), soup.indices.data(), GL_STATIC_DRAW);

			numNodes = soup.indices.size() * 3;
		}
	}

	void Shape::render()
	{
		if (childChildOp != ChildChildOperator::intersect)
		{
			for (auto& childCollection : children)
			{
				for (Shape* child : *childCollection.second)
				{
					child->render();
				}
			}
		}
		if ((children.size() == 0) | (parentChildOp != ParentChildOperator::none) | (childChildOp == ChildChildOperator::intersect))
		{
			if (childChildOp == ChildChildOperator::intersect)
			{
				GLint current_program = 1;
			}
			glBindVertexArray(vao);
			GLint current_program = 0;

			glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);
			glm::vec3 m_color(0.61, 0.56, 0.52);
			float m_fresnel = 0.5;
			glUniform3fv(glGetUniformLocation(current_program, "material_color"), 1, &m_color.x);
			glUniform1fv(glGetUniformLocation(current_program, "material_fresnel"), 1, &m_fresnel);
			glDrawElements(GL_TRIANGLES, numNodes, GL_UNSIGNED_INT, 0);
		}
	}

	void Shape::subdivide(int axis, std::string names[], SizePolicy policies[], float sizeVals[], size_t numSubEl)
	{
		std::vector<int> mask(numSubEl, 1);
		subdivide(axis, names, policies, sizeVals, numSubEl, mask.data());
	}

	// Subdivision where subshapes can me masked away
	void Shape::subdivide(int axis, std::string names[], SizePolicy policies[], float sizeVals[], size_t numSubEl, int mask[])
	{
		glm::vec2 newBounds[3];
		newBounds[0] = bounds[0];
		newBounds[1] = bounds[1];
		newBounds[2] = bounds[2];
		float parentSize = bounds[axis][1] - bounds[axis][0];

		// Add children lists
		for (size_t i = 0; i < numSubEl; ++i)
		{
			if (children.find(names[i]) == children.end()) children[names[i]] = new std::vector<Shape*>();
		}

		float absSum = 0;
		float relSum = 0;
		for (int i = 0; i < numSubEl; i++)
		{
			if (policies[i] == SizePolicy::absoluteTrue ||
				policies[i] == SizePolicy::absoluteInner ||
				policies[i] == SizePolicy::absoluteOuter)
			{
				// Scale the measure depending on what type of value is given
				float scale = absoluteRescaling(axis, policies[i]);
				absSum += scale * sizeVals[i];
			}
			else if (policies[i] == SizePolicy::relative) relSum += sizeVals[i];
		}

		// Scale factor for the relative size values
		float relScale = (parentSize - absSum) / relSum;

		newBounds[axis][1] = newBounds[axis][0];
		for (int i = 0; i < numSubEl; i++)
		{
			if (policies[i] == SizePolicy::absoluteTrue ||
				policies[i] == SizePolicy::absoluteInner ||
				policies[i] == SizePolicy::absoluteOuter)
			{
				// Scale the measure depending on what type of value is given
				float scale = absoluteRescaling(axis, policies[i]);
				newBounds[axis] = glm::vec2(newBounds[axis][1], newBounds[axis][1] + scale * sizeVals[i]);
			}
			else if (policies[i] == SizePolicy::relative)
			{
				newBounds[axis] = glm::vec2(newBounds[axis][1], newBounds[axis][1] + sizeVals[i] * relScale);
			}

			if (mask[i]) children[names[i]]->push_back(new Shape(coordSys, newBounds));
		}
	}

	void Shape::repeat(int axis, std::string name, SizePolicy policy, float sizeVal, int paddingMask /*= 1*/, PaddingType paddingType /*= PaddingType::balance*/)
	{
		glm::vec2 newBounds[3];
		newBounds[0] = bounds[0];
		newBounds[1] = bounds[1];
		newBounds[2] = bounds[2];
		float parentSize = bounds[axis][1] - bounds[axis][0];

		// Add children list
		if (children.find(name) == children.end()) children[name] = new std::vector<Shape*>();
		if (children.find("Padding") == children.end()) children["Padding"] = new std::vector<Shape*>();

		size_t numSubEl;

		if (policy == SizePolicy::absoluteTrue ||
			policy == SizePolicy::absoluteInner ||
			policy == SizePolicy::absoluteOuter)
		{
			// Scale the measure depending on what type of value is given
			float scale = absoluteRescaling(axis, policy);

			float nativePaddingVal;
			if (scale * sizeVal > parentSize) nativePaddingVal = parentSize / 2.0f;
			else nativePaddingVal = fmod(parentSize, scale * sizeVal) / 2.0f;

			numSubEl = (int)floor(parentSize / (scale * sizeVal));

			switch (paddingType)
			{
				case (PaddingType::low):
					newBounds[axis][1] = newBounds[axis][0] + 2 * nativePaddingVal;
					break;
				case (PaddingType::high):
					newBounds[axis][1] = newBounds[axis][0];
					break;
				case (PaddingType::balance):
					newBounds[axis][1] = newBounds[axis][0] + nativePaddingVal;
					break;
				default:
					newBounds[axis][1] = newBounds[axis][0] + nativePaddingVal;
			}
			for (int i = 0; i < numSubEl; i++)
			{
				newBounds[axis] = glm::vec2(newBounds[axis][1], newBounds[axis][1] + (scale * sizeVal));

				children[name]->push_back(new Shape(coordSys, newBounds));
			}

			if (scale * nativePaddingVal > 0.0001 && paddingMask)
			{
				if (paddingType == PaddingType::low || paddingType == PaddingType::balance)
				{
					glm::vec2 lowerPaddingBounds[3];
					lowerPaddingBounds[0] = bounds[0];
					lowerPaddingBounds[1] = bounds[1];
					lowerPaddingBounds[2] = bounds[2];

					switch (paddingType)
					{
						case (PaddingType::low):
							lowerPaddingBounds[axis][1] = lowerPaddingBounds[axis][0] + 2 * nativePaddingVal;
							break;
						case (PaddingType::balance):
							lowerPaddingBounds[axis][1] = lowerPaddingBounds[axis][0] + nativePaddingVal;
							break;
					}

					children["Padding"]->push_back(new Shape(coordSys, lowerPaddingBounds));
				}

				if (paddingType == PaddingType::high || paddingType == PaddingType::balance)
				{
					glm::vec2 upperPaddingBounds[3];
					upperPaddingBounds[0] = bounds[0];
					upperPaddingBounds[1] = bounds[1];
					upperPaddingBounds[2] = bounds[2];

					switch (paddingType)
					{
						case (PaddingType::high):
							upperPaddingBounds[axis][0] = upperPaddingBounds[axis][1] - 2 * nativePaddingVal;
							break;
						case (PaddingType::balance):
							upperPaddingBounds[axis][0] = upperPaddingBounds[axis][1] - nativePaddingVal;
							break;
					}

					children["Padding"]->push_back(new Shape(coordSys, upperPaddingBounds));
				}
			}
		}
		else if (policy == SizePolicy::relative)
		{
			float paddingVal = fmod(1, sizeVal);

			numSubEl = (int)floor(1.0f / sizeVal);
			newBounds[axis][1] = newBounds[axis][0];
			for (int i = 0; i < numSubEl; i++)
			{
				newBounds[axis] = glm::vec2(newBounds[axis][1], newBounds[axis][1] + sizeVal * parentSize);

				children[name]->push_back(new Shape(coordSys, newBounds));
			}

			if (paddingVal * parentSize > 0.0001)
			{
				glm::vec2 lowerPaddingBounds[3];
				lowerPaddingBounds[0] = bounds[0];
				lowerPaddingBounds[1] = bounds[1];
				lowerPaddingBounds[2] = bounds[2];

				lowerPaddingBounds[axis][1] = lowerPaddingBounds[axis][0] + paddingVal / 2.0f * parentSize;

				children["Padding"]->push_back(new Shape(coordSys, lowerPaddingBounds));

				glm::vec2 upperPaddingBounds[3];
				upperPaddingBounds[0] = bounds[0];
				upperPaddingBounds[1] = bounds[1];
				upperPaddingBounds[2] = bounds[2];

				upperPaddingBounds[axis][0] = upperPaddingBounds[axis][1] - paddingVal / 2.0f * parentSize;

				children["Padding"]->push_back(new Shape(coordSys, upperPaddingBounds));
			}
		}
	}

	void Shape::boundsExpand(glm::vec2 boundExpansions[3])
	{
		bounds[0][0] -= boundExpansions[0][0];
		bounds[0][1] += boundExpansions[0][1];
		bounds[1][0] -= boundExpansions[1][0];
		bounds[1][1] += boundExpansions[1][1];
		bounds[2][0] -= boundExpansions[2][0];
		bounds[2][1] += boundExpansions[2][1];
	}

	void Shape::wrapCartesianOverCylindrical(std::string name)
	{
		// Add children list
		if (children.find(name) == children.end()) children[name] = new std::vector<Shape*>();

		adjustPhiBounds();
		float halfwayAngle = (bounds[1][1] + bounds[1][0]) / 2;
		glm::vec3 halfwayAngleDir = cosf(halfwayAngle) * coordSys.bases[0] + sinf(halfwayAngle) * coordSys.bases[1];

		CoordSys wrapSys = { CoordSysType::cartesian, coordSys.origin, { halfwayAngleDir, glm::normalize(glm::cross(coordSys.bases[2], halfwayAngleDir)), coordSys.bases[2] } };

		float oneSideYBounds = sinf(halfwayAngle - bounds[1][0]) * bounds[0][1];

		glm::vec2 wrapBounds[3] = { glm::vec2(bounds[0][0], bounds[0][1]),
									glm::vec2(-oneSideYBounds, oneSideYBounds),
									bounds[2] };

		children[name]->push_back(new Shape(wrapSys, wrapBounds));
	}

	void Shape::adjustPhiBounds()
	{
		glm::vec2 phiBounds = glm::mod(bounds[1], 2 * glm::pi<float>());
		if (phiBounds[1] < phiBounds[0]) phiBounds[1] += 2 * glm::pi<float>();
		bounds[1] = phiBounds;
	}

	// Rescale non-"true absolute" size to "true unit"
	float Shape::absoluteRescaling(int axis, SizePolicy policy)
	{
		float scale;

		if (policy == SizePolicy::absoluteInner &&
			coordSys.type == CoordSysType::cylindrical &&
			axis == 1)
		{
			// Rescale inner arc size to radians
			scale = 1.0 / bounds[0][0];
		}
		if (policy == SizePolicy::absoluteOuter &&
			coordSys.type == CoordSysType::cylindrical &&
			axis == 1)
		{
			// Rescale outer arc size to radians
			scale = 1.0 / bounds[0][1];
		}
		else
		{
			scale = 1.0;
		}

		return scale;
	}
}