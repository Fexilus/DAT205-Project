#include <GL/glew.h>
#include <vector>

class FboInfo
{
public:
	GLuint framebufferId;
	std::vector<GLuint> colorTextureTargets;
	std::vector<GLint> colorTextureTargetFormats;
	GLuint depthBuffer;
	int width;
	int height;
	bool isComplete;

	FboInfo(int numberOfColorBuffers = 1, GLint colorBufferFormat = GL_RGBA16F);
	FboInfo(int numberOfColorBuffers, GLint colorBufferFormats[]);

	void resize(int w, int h);
	bool checkFramebufferComplete(void);
};
