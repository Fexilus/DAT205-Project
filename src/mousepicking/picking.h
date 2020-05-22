#include <glm/glm.hpp>

namespace mousepicking
{
	glm::vec3 moveAlongPlane(glm::vec3 oldModelCoord, glm::vec2 newWinCoord, glm::mat4 modelViewMatrix, glm::mat4 projMatrix, glm::vec4 viewport, glm::vec3 modelSpacePlaneNormal);
}