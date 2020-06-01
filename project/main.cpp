
#ifdef _WIN32
extern "C" _declspec(dllexport) unsigned int NvOptimusEnablement = 0x00000001;
#endif

#include <GL/glew.h>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <chrono>
#include <unordered_map>

#include <labhelper.h>
#include <imgui.h>
#include <imgui_impl_sdl_gl3.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
using namespace glm;

#include <Model.h>
#include "hdr.h"
#include "fbo.h"

#include <castle.h>
#include <picking.h>

using std::min;
using std::max;

///////////////////////////////////////////////////////////////////////////////
// Various globals
///////////////////////////////////////////////////////////////////////////////
SDL_Window* g_window = nullptr;
float currentTime = 0.0f;
float previousTime = 0.0f;
float deltaTime = 0.0f;
bool showUI = false;
int windowWidth, windowHeight;

// Mouse input
ivec2 g_prevMouseCoords = { -1, -1 };
bool g_isMouseDragging = false;
bool g_isMouseDraggingLeft = false;
bool g_isMouseDraggingRight = false;

///////////////////////////////////////////////////////////////////////////////
// Shader programs
///////////////////////////////////////////////////////////////////////////////
GLuint shaderProgram;       // Shader for rendering the final image
GLuint simpleShaderProgram; // Shader used to draw the shadow map
GLuint backgroundProgram;
GLuint ssaoInputProgram; // Shader that calculates normals as color
GLuint ssaoOutputProgram; // Shader that calculates the screen space ambient occlusion
GLuint ssaoBlurProgram; // Shader that blurs the screen space ambient occlusion

///////////////////////////////////////////////////////////////////////////////
// Environment
///////////////////////////////////////////////////////////////////////////////
float environment_multiplier = 1.5f;
GLuint environmentMap, irradianceMap, reflectionMap;
const std::string envmap_base_name = "001";

///////////////////////////////////////////////////////////////////////////////
// Light source
///////////////////////////////////////////////////////////////////////////////
vec3 lightPosition;
vec3 point_light_color = vec3(1.f, 1.f, 1.f);

float point_light_intensity_multiplier = 10000.0f;



///////////////////////////////////////////////////////////////////////////////
// Camera parameters.
///////////////////////////////////////////////////////////////////////////////
vec3 cameraPosition(-70.0f, 50.0f, 70.0f);
vec3 cameraDirection = normalize(vec3(0.0f) - cameraPosition);
float cameraSpeed = 45.f;
float cameraRotationSpeed = 0.14f;

vec3 worldUp(0.0f, 1.0f, 0.0f);

///////////////////////////////////////////////////////////////////////////////
// Models
///////////////////////////////////////////////////////////////////////////////
labhelper::Model* fighterModel = nullptr;
labhelper::Model* landingpadModel = nullptr;
labhelper::Model* sphereModel = nullptr;

mat4 roomModelMatrix;
mat4 landingPadModelMatrix;
mat4 fighterModelMatrix;

///////////////////////////////////////////////////////////////////////////////
// SSAO parameters.
///////////////////////////////////////////////////////////////////////////////
FboInfo ssaoInputFB;
FboInfo ssaoOutputFB;
FboInfo ssaoBlurFB;
std::vector<vec3> ssaoHemisphereSamples;
GLuint ssaoRotationTexture;

int numberOfSsaoSamples = 32;
const int ssaoRotationTextureSize = 4;
bool useSsaoRotation = true;
bool useSsaoBlur = true;

bool drawSsao = false;
bool useSsao = true;
float ssaoRadius = 3.0f;

///////////////////////////////////////////////////////////////////////////////
// Procedural generation
///////////////////////////////////////////////////////////////////////////////
std::unordered_map<uint, architecture::CastlePart*> proceduralObjects;
uint proceduralFreeId = 1;

///////////////////////////////////////////////////////////////////////////////
// Mouse picking
///////////////////////////////////////////////////////////////////////////////
bool drawIds = false;
int hoverID = 0;
float hoverDepth = 0;
int pickedID = 0;
float pickedDepth = 0;
vec3 pickedModelCoord;
int mouseX, mouseY = 0;
FboInfo finalFB;

vec3 pickedMovement(0);

///////////////////////////////////////////////////////////////////////////////
// Boolean operations
///////////////////////////////////////////////////////////////////////////////
bool drawWireframe = false;
architecture::Shape* intersectShape;
architecture::Shape* isectRect1;
architecture::Shape* isectRect2;
architecture::Shape* isectRects;
uint booleanTestId;
mat4 booleanTestModelMatrix(1);

architecture::CastleHeightMixin* pickedObjectHeightable = 0;
architecture::CastleRadiusMixin* pickedObjectRadiusable = 0;

void loadShaders(bool is_reload)
{
	GLuint shader = labhelper::loadShaderProgram("../project/simple.vert", "../project/simple.frag", is_reload);
	if(shader != 0) simpleShaderProgram = shader;
	shader = labhelper::loadShaderProgram("../project/background.vert", "../project/background.frag", is_reload);
	if(shader != 0) backgroundProgram = shader;
	shader = labhelper::loadShaderProgram("../project/ssaoInput.vert", "../project/ssaoInput.frag", is_reload);
	if (shader != 0) ssaoInputProgram = shader;
	shader = labhelper::loadShaderProgram("../project/ssaoOutput.vert", "../project/ssaoOutput.frag", is_reload);
	if (shader != 0) ssaoOutputProgram = shader;
	shader = labhelper::loadShaderProgram("../project/ssaoOutput.vert", "../project/ssaoBlur.frag", is_reload);
	if (shader != 0) ssaoBlurProgram = shader;

	std::vector<std::string> mainFragmentShaders;
	mainFragmentShaders.push_back("../project/shading.frag");
	mainFragmentShaders.push_back("../src/mousepicking/picking.frag");
	shader = labhelper::loadMultiShaderProgram("../project/shading.vert", mainFragmentShaders, is_reload);
	if (shader != 0) shaderProgram = shader;
}

void initSsaoSamples()
{
	ssaoHemisphereSamples.resize(numberOfSsaoSamples);
	for (int i = 0; i < numberOfSsaoSamples; i++) {
		ssaoHemisphereSamples[i] = labhelper::cosineSampleHemisphere();
		ssaoHemisphereSamples[i] *= labhelper::randf();
	}
}

void generateGeometry()
{
	// Main castle walls
	vec3 wallNodes[] = { vec3(-80,0,0), vec3(130,0,30), vec3(200,0,70), vec3(240,0,0), vec3(350,0,-60), vec3(480,0,60), vec3(555,0,0), vec3(670,0,-30), vec3(790,0,150), vec3(800,0,250), vec3(790,0,365) };
	for (auto& object : architecture::makeWalls(wallNodes, 11))
	{
		proceduralObjects[proceduralFreeId++] = object;
	}

	// Tower test
	proceduralObjects[proceduralFreeId++] = new architecture::CastleTower(vec3(0,0,50));

	// Wall test
	//proceduralObjects[proceduralFreeId++] = architecture::makeWall(vec3(0, 0, 90), vec3(80, 0, 100));

	// Init geometry
	for (auto& object : proceduralObjects)
	{
		object.second->init();
	}

	// Generate test objects
	/*
	architecture::CoordSys isectCoordSys = { architecture::CoordSysType::cylindrical, vec3(-60,0,50), { glm::vec3(0,1,0), glm::vec3(0,0,1), glm::vec3(1,0,0) } };

	glm::vec2 isectBounds[3] = { glm::vec2(0, 10),
								 glm::vec2(0, 2 * glm::pi<float>() - 0.0001),
								 glm::vec2(0, 15) };

	intersectShape = new architecture::Shape(isectCoordSys, isectBounds);
	intersectShape->repeat(0, "Intersect", architecture::SizePolicy::relative, 0.5);
	intersectShape->children.at("Intersect")->at(0)->coordSys.origin += vec3(1, 3, 0);
	intersectShape->childChildOp = architecture::Shape::ChildChildOperator::intersect;
	intersectShape->init();
	*/
	architecture::CoordSys isectRect1CoordSys = { architecture::CoordSysType::cartesian, vec3(-60,0,50), { glm::vec3(0,0,1), glm::vec3(1,0,0), glm::vec3(0,1,0) } };
	architecture::CoordSys isectRect2CoordSys = { architecture::CoordSysType::cartesian, vec3(-73,-5,55), { normalize(glm::vec3(1,0,1)), cross(glm::vec3(0,1,0), normalize(glm::vec3(1,0,1))), glm::vec3(0,1,0) } };

	glm::vec2 isectRectBounds[3] = { glm::vec2(0, 10),
									 glm::vec2(0, 10),
									 glm::vec2(0, 15) };
	isectRect1 = new architecture::Shape(isectRect1CoordSys, isectRectBounds);
	isectRect2 = new architecture::Shape(isectRect2CoordSys, isectRectBounds);
	//isectRect1->init();
	//isectRect2->init();
	isectRects = new architecture::Shape(isectRect1CoordSys, isectRectBounds);
	isectRects->children["Isects"] = new std::vector<architecture::Shape*>();
	isectRects->children["Isects"]->push_back(isectRect1);
	isectRects->children["Isects"]->push_back(isectRect2);
	isectRects->childChildOp = architecture::Shape::ChildChildOperator::intersect;
	isectRects->init();
	booleanTestId = proceduralFreeId++;
}

void initGL()
{
	///////////////////////////////////////////////////////////////////////
	// Load Shaders
	///////////////////////////////////////////////////////////////////////
	loadShaders(false);

	///////////////////////////////////////////////////////////////////////
	// Load SSAO components
	///////////////////////////////////////////////////////////////////////
	initSsaoSamples();

	// Create rotation texture
	float rotations[ssaoRotationTextureSize * ssaoRotationTextureSize * 3];
	for (int i = 0; i < ssaoRotationTextureSize * ssaoRotationTextureSize * 3; i += 3) {
		rotations[i] = rotations[i + 1] = rotations[i + 2] = labhelper::randf();
	}

	glGenTextures(1, &ssaoRotationTexture);
	glBindTexture(GL_TEXTURE_2D, ssaoRotationTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ssaoRotationTextureSize, ssaoRotationTextureSize, 0, GL_RGB, GL_FLOAT, rotations);

	// Set up ssao framebuffers
	SDL_GetWindowSize(g_window, &windowWidth, &windowHeight);
	ssaoInputFB.resize(windowWidth, windowHeight);
	ssaoOutputFB.resize(windowWidth, windowHeight);
	ssaoBlurFB.resize(windowWidth, windowHeight);
	GLint finalTextureFormats[] = { GL_RGBA16F, GL_R32UI };
	finalFB = FboInfo(2, finalTextureFormats);
	finalFB.resize(windowWidth, windowHeight);

	///////////////////////////////////////////////////////////////////////
	// Load models and set up model matrices
	///////////////////////////////////////////////////////////////////////
	//fighterModel = labhelper::loadModelFromOBJ("../scenes/NewShip.obj");
	//landingpadModel = labhelper::loadModelFromOBJ("../scenes/landingpad.obj");
	//sphereModel = labhelper::loadModelFromOBJ("../scenes/sphere.obj");
	//
	//roomModelMatrix = mat4(1.0f);
	//fighterModelMatrix = translate(15.0f * worldUp);
	//landingPadModelMatrix = mat4(1.0f);

	///////////////////////////////////////////////////////////////////////
	// Load environment map
	///////////////////////////////////////////////////////////////////////
	const int roughnesses = 8;
	std::vector<std::string> filenames;
	for(int i = 0; i < roughnesses; i++)
		filenames.push_back("../scenes/envmaps/" + envmap_base_name + "_dl_" + std::to_string(i) + ".hdr");

	reflectionMap = labhelper::loadHdrMipmapTexture(filenames);
	environmentMap = labhelper::loadHdrTexture("../scenes/envmaps/" + envmap_base_name + ".hdr");
	irradianceMap = labhelper::loadHdrTexture("../scenes/envmaps/" + envmap_base_name + "_irradiance.hdr");


	glEnable(GL_DEPTH_TEST); // enable Z-buffering
	glEnable(GL_CULL_FACE);  // enables backface culling

	///////////////////////////////////////////////////////////////////////
	// Set up procedural generation
	///////////////////////////////////////////////////////////////////////
	generateGeometry();
}

void debugDrawLight(const glm::mat4& viewMatrix,
                    const glm::mat4& projectionMatrix,
                    const glm::vec3& worldSpaceLightPos)
{
	mat4 modelMatrix = glm::translate(worldSpaceLightPos);
	glUseProgram(shaderProgram);
	labhelper::setUniformSlow(shaderProgram, "modelViewProjectionMatrix",
	                          projectionMatrix * viewMatrix * modelMatrix);
	labhelper::render(sphereModel);
	labhelper::setUniformSlow(shaderProgram, "modelViewProjectionMatrix", projectionMatrix * viewMatrix);
	labhelper::debugDrawLine(viewMatrix, projectionMatrix, worldSpaceLightPos);
}


void drawBackground(const mat4& viewMatrix, const mat4& projectionMatrix)
{
	glUseProgram(backgroundProgram);
	labhelper::setUniformSlow(backgroundProgram, "environment_multiplier", environment_multiplier);
	labhelper::setUniformSlow(backgroundProgram, "inv_PV", inverse(projectionMatrix * viewMatrix));
	labhelper::setUniformSlow(backgroundProgram, "camera_pos", cameraPosition);
	labhelper::drawFullScreenQuad();
}

void drawScene(GLuint currentShaderProgram,
               const mat4& viewMatrix,
               const mat4& projectionMatrix,
               const mat4& lightViewMatrix,
               const mat4& lightProjectionMatrix)
{
	glUseProgram(currentShaderProgram);

	// Light source
	vec4 viewSpaceLightPosition = viewMatrix * vec4(lightPosition, 1.0f);
	labhelper::setUniformSlow(currentShaderProgram, "point_light_color", point_light_color);
	labhelper::setUniformSlow(currentShaderProgram, "point_light_intensity_multiplier",
	                          point_light_intensity_multiplier);
	labhelper::setUniformSlow(currentShaderProgram, "viewSpaceLightPosition", vec3(viewSpaceLightPosition));
	labhelper::setUniformSlow(currentShaderProgram, "viewSpaceLightDir",
	                          normalize(vec3(viewMatrix * vec4(-lightPosition, 0.0f))));


	// Environment
	labhelper::setUniformSlow(currentShaderProgram, "environment_multiplier", environment_multiplier);

	// camera
	labhelper::setUniformSlow(currentShaderProgram, "viewInverse", inverse(viewMatrix));

	/*
	// landing pad
	labhelper::setUniformSlow(currentShaderProgram, "modelViewProjectionMatrix",
	                          projectionMatrix * viewMatrix * landingPadModelMatrix);
	labhelper::setUniformSlow(currentShaderProgram, "modelViewMatrix", viewMatrix * landingPadModelMatrix);
	labhelper::setUniformSlow(currentShaderProgram, "normalMatrix",
	                          inverse(transpose(viewMatrix * landingPadModelMatrix)));

	labhelper::render(landingpadModel);

	// Fighter
	labhelper::setUniformSlow(currentShaderProgram, "modelViewProjectionMatrix",
	                          projectionMatrix * viewMatrix * fighterModelMatrix);
	labhelper::setUniformSlow(currentShaderProgram, "modelViewMatrix", viewMatrix * fighterModelMatrix);
	labhelper::setUniformSlow(currentShaderProgram, "normalMatrix",
	                          inverse(transpose(viewMatrix * fighterModelMatrix)));

	labhelper::render(fighterModel);
	*/

	// Castle

	for (auto& object : proceduralObjects)
	{
		labhelper::setUniformSlow(currentShaderProgram, "objectId", object.first);

		mat4 modelMatrix = object.first == pickedID && g_isMouseDraggingLeft ? translate(pickedMovement) : mat4(1.0f);
		labhelper::setUniformSlow(currentShaderProgram, "modelViewProjectionMatrix",
			projectionMatrix * viewMatrix * modelMatrix);
		labhelper::setUniformSlow(currentShaderProgram, "modelViewMatrix",
			viewMatrix * modelMatrix);
		labhelper::setUniformSlow(currentShaderProgram, "normalMatrix",
			inverse(transpose(viewMatrix * modelMatrix)));

		object.second->render();
	}

	// Booleantest
	//intersectShape->render();
	//isectRect1->render();
	//isectRect2->render();
	labhelper::setUniformSlow(currentShaderProgram, "objectId", booleanTestId);
	mat4 tempModelMatrix = booleanTestId == pickedID && g_isMouseDraggingLeft ? translate(pickedMovement) : mat4(1.0f);
	labhelper::setUniformSlow(currentShaderProgram, "modelViewProjectionMatrix",
		projectionMatrix * viewMatrix * booleanTestModelMatrix * tempModelMatrix);
	labhelper::setUniformSlow(currentShaderProgram, "modelViewMatrix",
		viewMatrix * booleanTestModelMatrix * tempModelMatrix);
	labhelper::setUniformSlow(currentShaderProgram, "normalMatrix",
		inverse(transpose(viewMatrix * booleanTestModelMatrix * tempModelMatrix)));
	isectRects->render();
}


void display(void)
{
	///////////////////////////////////////////////////////////////////////////
	// Check if window size has changed and resize buffers as needed
	///////////////////////////////////////////////////////////////////////////
	{
		int w, h;
		SDL_GetWindowSize(g_window, &w, &h);
		if(w != windowWidth || h != windowHeight)
		{
			windowWidth = w;
			windowHeight = h;
			ssaoInputFB.resize(windowWidth, windowHeight);
			ssaoOutputFB.resize(windowWidth, windowHeight);
			ssaoBlurFB.resize(windowWidth, windowHeight);
			finalFB.resize(windowWidth, windowHeight);
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// setup matrices
	///////////////////////////////////////////////////////////////////////////
	mat4 projMatrix = perspective(radians(45.0f), float(windowWidth) / float(windowHeight), 5.0f, 2000.0f);
	mat4 inverseProjMatrix = inverse(projMatrix);
	mat4 viewMatrix = lookAt(cameraPosition, cameraPosition + cameraDirection, worldUp);

	vec4 lightStartPosition = vec4(40.0f, 40.0f, 0.0f, 1.0f);
	lightPosition = vec3(rotate(currentTime, worldUp) * lightStartPosition);
	mat4 lightViewMatrix = lookAt(lightPosition, vec3(0.0f), worldUp);
	mat4 lightProjMatrix = perspective(radians(45.0f), 1.0f, 25.0f, 100.0f);

	///////////////////////////////////////////////////////////////////////////
	// Bind the environment map(s) to unused texture units
	///////////////////////////////////////////////////////////////////////////
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, environmentMap);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, irradianceMap);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, reflectionMap);
	glActiveTexture(GL_TEXTURE0);

	if (useSsao | drawSsao)
	{
		///////////////////////////////////////////////////////////////////////////
		// First SSAO render
		///////////////////////////////////////////////////////////////////////////
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoInputFB.framebufferId);
		glViewport(0, 0, windowWidth, windowHeight);
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDepthRangef(0, 1);

		// Draw scene to the pre-process framebuffer
		drawScene(ssaoInputProgram, viewMatrix, projMatrix, lightViewMatrix, lightProjMatrix);

		///////////////////////////////////////////////////////////////////////////
		// Second SSAO render
		///////////////////////////////////////////////////////////////////////////
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoOutputFB.framebufferId);
		glViewport(0, 0, windowWidth, windowHeight);
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(ssaoOutputProgram);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ssaoInputFB.colorTextureTargets[0]);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, ssaoInputFB.depthBuffer);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, ssaoRotationTexture);

		glUniform3fv(glGetUniformLocation(ssaoOutputProgram, "hemisphereSamples"), numberOfSsaoSamples, &ssaoHemisphereSamples[0].x);
		glUniform1i(glGetUniformLocation(ssaoOutputProgram, "numberOfSamples"), numberOfSsaoSamples); //Removed as Shader Storage Buffer Object is needed
		glUniform1f(glGetUniformLocation(ssaoOutputProgram, "ssaoRadius"), ssaoRadius);
		glUniform1i(glGetUniformLocation(ssaoOutputProgram, "useRotation"), useSsaoRotation);

		glUniformMatrix4fv(glGetUniformLocation(ssaoOutputProgram, "projectionMatrix"), 1, false, &projMatrix[0].x);
		glUniformMatrix4fv(glGetUniformLocation(ssaoOutputProgram, "inverseProjectionMatrix"), 1, false, &inverseProjMatrix[0].x);

		labhelper::drawFullScreenQuad();

		///////////////////////////////////////////////////////////////////////////
		// SSAO blur
		///////////////////////////////////////////////////////////////////////////
		if (useSsaoBlur)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFB.framebufferId);
			glViewport(0, 0, windowWidth, windowHeight);
			glClearColor(0.0, 0.0, 0.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(ssaoBlurProgram);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ssaoOutputFB.colorTextureTargets[0]);

			glUniform1i(glGetUniformLocation(ssaoBlurProgram, "rotationTextureSize"), ssaoRotationTextureSize);

			labhelper::drawFullScreenQuad();
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// Draw from camera
	///////////////////////////////////////////////////////////////////////////

	if (useSsao | drawSsao)
	{
		// Bind the ssao buffer
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, useSsaoBlur ? ssaoBlurFB.colorTextureTargets[0] : ssaoOutputFB.colorTextureTargets[0]);
	}

	glUseProgram(shaderProgram);
	glUniform1i(glGetUniformLocation(shaderProgram, "drawSsao"), drawSsao);
	glUniform1i(glGetUniformLocation(shaderProgram, "useSsao"), useSsao);
	glUniform1i(glGetUniformLocation(shaderProgram, "drawId"), drawIds);
	glUniform1ui(glGetUniformLocation(shaderProgram, "hoverId"), hoverID);
	glUniform1ui(glGetUniformLocation(shaderProgram, "pickedId"), pickedID);

	glBindFramebuffer(GL_FRAMEBUFFER, finalFB.framebufferId);
	glViewport(0, 0, windowWidth, windowHeight);
	glClearColor(0.2, 0.2, 0.8, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//drawBackground(viewMatrix, projMatrix);
	drawScene(shaderProgram, viewMatrix, projMatrix, lightViewMatrix, lightProjMatrix);
	//debugDrawLight(viewMatrix, projMatrix, vec3(lightPosition));

	// Blit final screen to default frame buffer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, 0, windowWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
}

void rotateCamera(int deltaX, int deltaY)
{
	mat4 yaw = rotate(cameraRotationSpeed * deltaTime * -deltaX, worldUp);
	mat4 pitch = rotate(cameraRotationSpeed * deltaTime * -deltaY,
		normalize(cross(cameraDirection, worldUp)));
	cameraDirection = vec3(pitch * yaw * vec4(cameraDirection, 0.0f));
}

bool handleEvents(void)
{
	SDL_GetMouseState(&mouseX, &mouseY);

	mat4 projMatrix = perspective(radians(45.0f), float(windowWidth) / float(windowHeight), 5.0f, 2000.0f);
	mat4 viewMatrix = lookAt(cameraPosition, cameraPosition + cameraDirection, worldUp);
	vec4 viewport(0.0f, 0.0f, windowWidth, windowHeight);

	// check events (keyboard among other)
	SDL_Event event;
	bool quitEvent = false;
	while(SDL_PollEvent(&event))
	{
		if(event.type == SDL_QUIT || (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE))
		{
			quitEvent = true;
		}
		if(event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_g)
		{
			showUI = !showUI;
		}
		if(event.type == SDL_MOUSEBUTTONDOWN && 
		   (event.button.button == SDL_BUTTON_RIGHT || event.button.button == SDL_BUTTON_LEFT)
		   && (!showUI || !ImGui::GetIO().WantCaptureMouse))
		{
			g_isMouseDragging = true;
			g_prevMouseCoords.x = mouseX;
			g_prevMouseCoords.y = mouseY;

			if (event.button.button == SDL_BUTTON_RIGHT) g_isMouseDraggingRight = true;
			if (event.button.button == SDL_BUTTON_LEFT)
			{
				pickedID = hoverID;
				pickedDepth = hoverDepth;
				vec2 pickedScreenCoords = vec2(g_prevMouseCoords.x, windowHeight - 1.0f - g_prevMouseCoords.y);
				pickedModelCoord = glm::unProject(vec3(pickedScreenCoords, pickedDepth), viewMatrix, projMatrix, viewport);
				g_isMouseDraggingLeft = true;

				// Handle variables for gui

				auto& pickedObject = proceduralObjects.find(pickedID);
				if (pickedObject != proceduralObjects.end())
				{
					pickedObjectHeightable = dynamic_cast<architecture::CastleHeightMixin*>(pickedObject->second);
					pickedObjectRadiusable = dynamic_cast<architecture::CastleRadiusMixin*>(pickedObject->second);
				}
				else
				{
					pickedObjectHeightable = 0;
					pickedObjectRadiusable = 0;
				}
			}
		}

		if(!(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT)))
		{
			g_isMouseDraggingRight = false;
		}
		if (!(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) && g_isMouseDraggingLeft)
		{
			g_isMouseDraggingLeft = false;

			auto& pickedObject = proceduralObjects.find(pickedID);
			if (pickedObject != proceduralObjects.end())
			{
				pickedObject->second->move(pickedMovement);
			}
			else if (pickedID == booleanTestId)
			{
				booleanTestModelMatrix *= translate(pickedMovement);
			}

			// Reset to avoid weird movement previews
			pickedMovement = vec3(0);
		}
		if (!(g_isMouseDraggingRight || g_isMouseDraggingLeft))
		{
			g_isMouseDragging = false;
		}

		if(event.type == SDL_MOUSEMOTION && g_isMouseDragging)
		{
			// More info at https://wiki.libsdl.org/SDL_MouseMotionEvent
			int delta_x = event.motion.x - g_prevMouseCoords.x;
			int delta_y = event.motion.y - g_prevMouseCoords.y;
			if (g_isMouseDraggingRight) rotateCamera(delta_x, delta_y);
			if (g_isMouseDraggingLeft)
			{
				vec2 newScreenCoords = vec2(event.motion.x, windowHeight - 1.0f - event.motion.y);

				vec3 modelSpaceMovement = mousepicking::moveAlongPlane(pickedModelCoord, newScreenCoords, viewMatrix, projMatrix, viewport, worldUp);
				pickedMovement =  modelSpaceMovement;
			}
			g_prevMouseCoords.x = event.motion.x;
			g_prevMouseCoords.y = event.motion.y;
		}
	}

	// check keyboard state (which keys are still pressed)
	const uint8_t* state = SDL_GetKeyboardState(nullptr);
	vec3 cameraRight = cross(cameraDirection, worldUp);

	if(state[SDL_SCANCODE_W])
	{
		cameraPosition += cameraSpeed * deltaTime * cameraDirection;
	}
	if(state[SDL_SCANCODE_S])
	{
		cameraPosition -= cameraSpeed * deltaTime * cameraDirection;
	}
	if(state[SDL_SCANCODE_A])
	{
		cameraPosition -= cameraSpeed * deltaTime * cameraRight;
	}
	if(state[SDL_SCANCODE_D])
	{
		cameraPosition += cameraSpeed * deltaTime * cameraRight;
	}
	if(state[SDL_SCANCODE_LCTRL])
	{
		cameraPosition -= cameraSpeed * deltaTime * worldUp;
	}
	if(state[SDL_SCANCODE_SPACE])
	{
		cameraPosition += cameraSpeed * deltaTime * worldUp;
	}

	/*if (state[SDL_SCANCODE_DELETE])
	{
		auto& pickedObject = proceduralObjects.find(pickedID);
		if (pickedObject != proceduralObjects.end())
		{
			delete pickedObject->second;
			proceduralObjects.erase(pickedObject);
		}
	}*/

	// Update picking data
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, finalFB.framebufferId);
		{
			glReadBuffer(GL_COLOR_ATTACHMENT1);

			uint id;
			glReadPixels(mouseX, windowHeight - mouseY, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &id);

			hoverID = id;

			float depth;
			glReadPixels(mouseX, windowHeight - mouseY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

			hoverDepth = depth;
		}
		// Reset stuff
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	return quitEvent;
}

void gui()
{
	// Inform imgui of new frame
	ImGui_ImplSdlGL3_NewFrame(g_window);

	// SSAO options
	if (ImGui::CollapsingHeader("SSAO", ImGuiTreeNodeFlags_Framed))
	{
		ImGui::Checkbox("Use SSAO", &useSsao);
		ImGui::Checkbox("Draw SSAO", &drawSsao);
		ImGui::SliderFloat("Radius", &ssaoRadius, 0, 10);
		if (ImGui::SliderInt("Number of samples", &numberOfSsaoSamples, 1, 64))
			initSsaoSamples();
		ImGui::Checkbox("Use rotation texture", &useSsaoRotation);
		ImGui::Checkbox("Use blur pass", &useSsaoBlur);
	}

	// Picking options
	if (ImGui::CollapsingHeader("Mouse Picking", ImGuiTreeNodeFlags_Framed))
	{
		ImGui::Checkbox("Draw object ids", &drawIds);
		ImGui::Text("Hover position X: %i, Y: %i", mouseX, mouseY);
		ImGui::Text("Hover id: %i", hoverID);
		ImGui::Text("Hover depth: %f", hoverDepth);
		ImGui::Text("Picked position: (%f, %f, %f)", pickedModelCoord.x, pickedModelCoord.y, pickedModelCoord.z);
		ImGui::Text("Picked id: %i", pickedID);
		ImGui::Text("Picked depth: %f", pickedDepth);
		ImGui::Text("Picked movement: (%f, %f, %f)", pickedMovement.x, pickedMovement.y, pickedMovement.z);
	}

	if (ImGui::CollapsingHeader("Boolean operations", ImGuiTreeNodeFlags_Framed))
	{
		if (ImGui::Checkbox("Draw wireframe", &drawWireframe))
		{
			if (drawWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		
	}

	if (ImGui::CollapsingHeader("Live editing", ImGuiTreeNodeFlags_Framed + ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (pickedObjectHeightable)
		{
			float height = pickedObjectHeightable->height();
			if (ImGui::SliderFloat("Height", &height, 20, 100))
			{
				pickedObjectHeightable->set_height(height);
			}
		}
		if (pickedObjectRadiusable)
		{
			float radius = pickedObjectRadiusable->radius();
			if (ImGui::SliderFloat("Radius", &radius, 20, 70))
			{
				pickedObjectRadiusable->set_radius(radius);
			}
		}
	}

	// Reload shaders
	if (ImGui::Button("Reload Shaders")) {
		loadShaders(true);
	}

	// ----------------- Set variables --------------------------
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
		ImGui::GetIO().Framerate);
	// ----------------------------------------------------------

	// Render the GUI.
	ImGui::Render();
}

int main(int argc, char* argv[])

{
	g_window = labhelper::init_window_SDL("OpenGL Project");

	initGL();

	bool stopRendering = false;
	auto startTime = std::chrono::system_clock::now();

	while(!stopRendering)
	{
		//update currentTime
		std::chrono::duration<float> timeSinceStart = std::chrono::system_clock::now() - startTime;
		previousTime = currentTime;
		currentTime = timeSinceStart.count();
		deltaTime = currentTime - previousTime;
		// render to window
		display();

		// Render overlay GUI.
		if(showUI)
		{
			gui();
		}

		// Swap front and back buffer. This frame will now been displayed.
		SDL_GL_SwapWindow(g_window);

		// check events (keyboard among other)
		stopRendering = handleEvents();
	}
	// Free Models
	labhelper::freeModel(fighterModel);
	labhelper::freeModel(landingpadModel);
	labhelper::freeModel(sphereModel);

	// Shut down everything. This includes the window and all other subsystems.
	labhelper::shutDown(g_window);
	return 0;
}
