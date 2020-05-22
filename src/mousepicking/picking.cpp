#include "picking.h"

#include <glm/gtc/matrix_transform.hpp>

// Returns a movement vector in model space
glm::vec3 mousepicking::moveAlongPlane(glm::vec3 oldModelCoord, glm::vec2 newWinCoord, glm::mat4 modelViewMatrix, glm::mat4 projMatrix, glm::vec4 viewport, glm::vec3 modelSpacePlaneNormal)
{
	float oldRelativePlaneHeight = glm::dot(oldModelCoord, glm::normalize(modelSpacePlaneNormal));

	glm::vec3 modelSpaceNewMin = glm::unProject(glm::vec3(newWinCoord, 0), modelViewMatrix, projMatrix, viewport);
	glm::vec3 modelSpaceNewMax = glm::unProject(glm::vec3(newWinCoord, 1), modelViewMatrix, projMatrix, viewport);

	float newMinRelativePlaneHeight = glm::dot(modelSpaceNewMin, glm::normalize(modelSpacePlaneNormal));
	float newMaxRelativePlaneHeight = glm::dot(modelSpaceNewMax, glm::normalize(modelSpacePlaneNormal));
	float newMixVal = (oldRelativePlaneHeight - newMinRelativePlaneHeight) / (newMaxRelativePlaneHeight - newMinRelativePlaneHeight);

	glm::vec3 modelSpaceNewPoint = (1 - newMixVal) * modelSpaceNewMin + newMixVal * modelSpaceNewMax;
	return modelSpaceNewPoint - oldModelCoord;
}
