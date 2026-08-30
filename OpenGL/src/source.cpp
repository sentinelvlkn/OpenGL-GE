
#include <cstddef>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <include/Input.h>
#include "include/Shader.h"
#include "include/PlayerMovement.h"
#include "include/Camera.h"
#include "include/Model.h"
#include "include/Room.h"
#include "include/Player.h"
#include "include/Interaction.h"
#include "include/InteractionSystem.h"
#include "stb_image.h"
#include "include/FileSystem.h"
#include "include/TextRenderer.h"
#include "include/UiRenderer.h"
#include "include/DebugLog.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <vector>
#include <sstream>
#include <algorithm>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <memory>
#include <unordered_map>
#include "include/test_vertices.h"

static void updateFade(float deltaTime);
static void updateRoomCamera(Room& room, const Player& player, Camera& camera);
static void renderSceneToFBO(unsigned int framebuffer, Shader& shader, Model& roomModel, Model& playerModel, const Player& player, const Room& room, Camera& camera);
static void renderScreen(unsigned int quadVAO, Shader& screenShader, unsigned int textureColorBuffer, float time);
static void runInteraction(Interaction& interaction);
static unsigned int loadTexture2D(const char* path);
static Model& getRoomModel(const Room& room);
static Model& getCachedModel(const std::string &modelPath);
static float hashFloat(float x);
static float noise1D(float x);
static float calculateFlickerMultiplier(const PointLight& light, float time, int lightIndex);
static void updateLightningDebugInput(GLFWwindow *window, float deltaTime);
static void resizeFramebufferAttachments(unsigned int fbo, unsigned int textureColorBuffer,
										 unsigned int rbo);
static std::vector<std::string> wrapTextWidth(const std::string& text, TextRenderer& textRenderer, float scale, float maxWidth);
static void drawDebugTerminal(UiRenderer &uiRenderer, Shader &uiRectangleShader,
							  TextRenderer &textRenderer, Shader &textShader,
							  int screenWidth, int screenHeight);


int WIDTH = 800;
int HEIGHT = 600;

static int gFramebufferWidth = WIDTH;
static int gFramebufferHeight = HEIGHT;
static bool gFramebufferResized = false;

void glfwErrorCallback(int error, const char* description);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
unsigned int loadTexture(const char* path);
unsigned int loadCubemap(std::vector<std::string> faces);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = (float)gFramebufferWidth / 2.0;
float lastY = (float)gFramebufferHeight / 2.0;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

static bool debugTerminalOpen = false;

//light debug variables
static float debugForcedLightningTimer = 0.0f;
static bool debugLightningPreview = false; //keeps spotlight visible
static bool debugDisableFlicker = false;  
static bool debugPrintLightInformation = false;
static bool debugSpotAsPoint = false; // turn spotlight to point light

//for scene fade in and fade out
float fade = 1.0f;
bool fadeIn = false; //into black
bool fadeOut = true; //out of black
bool transitioning = false;

static bool messageOpen = false;
static std::string activeMessage;

static bool noteOpen = false;
static std::string activeNoteImagePath;
static unsigned int activeNoteTexture = 0;

static std::unordered_map<std::string, std::unique_ptr<Model>> roomModelCache;
static std::unordered_map<std::string, std::unique_ptr<Model>> modelCache;

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	//window create
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "OPENGL", NULL, NULL);
	if(window == NULL){
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	
	glfwMakeContextCurrent(window);
	glfwSetErrorCallback(glfwErrorCallback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwGetFramebufferSize(window, &gFramebufferWidth, &gFramebufferHeight);
	//glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	//load glad
	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		std::cout << "Failed to load GLAD" << std::endl;
		return -1;
	}
	
	std::cout << "GLAD load: " << (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) ? "OK" : "FAIL")<< "\n";
	std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << "\n";


	unsigned int cubemapTexture = loadCubemap(faces);
	stbi_set_flip_vertically_on_load(true);
	
	glEnable(GL_DEPTH_TEST);
		
	Shader shader( 
		FileSystem::pathFromRoot("src/shaders/vertex_shader.vert").c_str(),
		FileSystem::pathFromRoot("src/shaders/fragment_shader.frag").c_str()
	);

	Shader skybShader( 
		FileSystem::pathFromRoot("src/shaders/skyb_vertex_shader.vert").c_str(), 
		FileSystem::pathFromRoot("src/shaders/skyb_fragment_shader.frag").c_str()
	);

	Shader screenShader(
		FileSystem::pathFromRoot("src/shaders/fbuffer_vertex_shader.vert").c_str(),
		FileSystem::pathFromRoot("src/shaders/fbuffer_fragment_shader.frag").c_str()
	);

	Shader uiShader(
		FileSystem::pathFromRoot("src/shaders/note_quad.vert").c_str(),
		FileSystem::pathFromRoot("src/shaders/note_quad.frag").c_str()
	);

	Shader textShader(
		FileSystem::pathFromRoot("src/shaders/text.vert").c_str(),
		FileSystem::pathFromRoot("src/shaders/text.frag").c_str()
	);

	Shader uiRectangleShader(
		FileSystem::pathFromRoot("src/shaders/ui_rectangle.vert").c_str(),
		FileSystem::pathFromRoot("src/shaders/ui_rectangle.frag").c_str()
	);

	DebugLog::log("Debug terminal ready. Press F1 to toggle. Press F2 to print player state.");

	/*Model backpackModel(
		FileSystem::pathFromRoot("src/models/backpack/backpack.obj").c_str()
	);*/
	
	stbi_set_flip_vertically_on_load(false);
	Room testRoom = loadRoomFromJson(
		FileSystem::pathFromRoot("src/models/test_room/test_room.json").c_str()
	);

	for (int i = 0; i < testRoom.cameras.size(); ++i) {
    		auto& c = testRoom.cameras[i];
			{
				std::ostringstream ss;

				ss << "Cam " << i << " min(" << c.aabbMin.x << ","
				   << c.aabbMin.y << "," << c.aabbMin.z << ") "
				   << "max(" << c.aabbMax.x << "," << c.aabbMax.y << ","
				   << c.aabbMax.z << ")";

				DebugLog::log(ss.str());
			}
			
	}



	/*std::unique_ptr<Model> testRoomModel = std::make_unique<Model>(
		FileSystem::pathFromRoot(testRoom.modelPath).c_str()
	);*/
	Model* testRoomModel = nullptr;
	testRoomModel = &getRoomModel(testRoom);

	Model playerModel(
		FileSystem::pathFromRoot("src/models/test_character/test_character.obj").c_str()
	);

	Player player;
	player.position = testRoom.playerSpawn;

	
	std::string pendingRoomJson;
	glm::vec3 pendingSpawn(0.0f);

	TextRenderer textRenderer;
	std::string fontPath = FileSystem::pathFromRoot("src/ui/fonts/LektonNerdFontMono-Regular.ttf");

	if (!textRenderer.loadFont(fontPath, 24))
		DebugLog::log("[TEXT ERROR] Failed to load the font: " + fontPath);


	//skybox
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
	
	//unsigned int cubeTexture = loadTexture("const char *path")
	
	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER,quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
	

	shader.use();	

	skybShader.use();
    skybShader.setInt("skybox", 0);
	
	screenShader.use();
	//screenShader.setInt("screenTexture", 0);
	screenShader.setFloat("vignetteStrength", 0.5f);
	screenShader.setFloat("grainStrength", 0.02f);
	screenShader.setFloat("saturation", 0.7f);
	screenShader.setFloat("contrast", 1.2f);

	uiShader.use();
	uiShader.setInt("uiTexture", 0);
	
	//framebuffer
	unsigned int framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	
	//color attachment texture
	unsigned int textureColorBuffer;
	glGenTextures(1, &textureColorBuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, gFramebufferWidth, gFramebufferHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);
	
	//renderbuffer obj
	unsigned int RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, gFramebufferWidth, gFramebufferHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
	
	//check if framebuffer was crated
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		DebugLog::log("[FRAMEBUFFER ERROR] Framebuffer is not complete! Kaput!!!!");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	lastFrame = static_cast<float>(glfwGetTime());

	float promptScale = 0.75f;

	UiRenderer uiRenderer;

	DebugLog::log("[BOOT] Debug terminal initialized.");
	DebugLog::log("[BOOT] Press F1 to toggle terminal.");
	DebugLog::log("[BOOT] Press F2 to print player state.");

	//main game loop
	while (!glfwWindowShouldClose(window)) {
    	float currentFrame = static_cast<float>(glfwGetTime());
    	deltaTime = currentFrame - lastFrame;
    	lastFrame = currentFrame;

		float time = static_cast<float>(glfwGetTime());

    	deltaTime = glm::min(deltaTime, 1.0f/30.0f);

		std::string promptText;
		

		if (gFramebufferResized)
			resizeFramebufferAttachments(framebuffer ,textureColorBuffer, RBO);

    	processInput(window, fade, fadeIn, fadeOut);
		updateLightningDebugInput(window, deltaTime);

		if (keyPressedOnce(window, GLFW_KEY_F1))
			debugTerminalOpen = !debugTerminalOpen;

		if (keyPressedOnce(window, GLFW_KEY_F2)) {
			std::ostringstream ss;

			ss << "Room: " << testRoom.name
			   << " | Player: " << player.position.x << ", "
			   << player.position.y << ", " << player.position.z
			   << " | Camera: " << testRoom.activeCamera
			   << " | Props: " << testRoom.props.size()
			   << " | Lights: " << testRoom.lights.size();

			DebugLog::log(ss.str());
		}

    	updateFade(deltaTime);

    	// If fade-out finished, swap rooms
    	if (transitioning && !fadeIn && fade >= 1.0f) {
       		try {
				testRoom = loadRoomFromJson(FileSystem::pathFromRoot(pendingRoomJson).c_str());
				testRoomModel = &getRoomModel(testRoom);
				/*testRoomModel = std::make_unique<Model>(
					FileSystem::pathFromRoot(testRoom.modelPath).c_str()
				);*/
				
				player.position = pendingSpawn;
				roomCameraSwitch(testRoom, player.position);
        		applyRoomCamera(testRoom, camera);

				fade = 1.0f;
				fadeIn = false;
				fadeOut = true;

				transitioning = false;
				pendingRoomJson.clear();
				
			} catch (const std::exception& e) {

				DebugLog::log(std::string("[ROOM ERROR] Failed to load room: ") + e.what());
				fade = 1.0f;
				fadeIn = false;
				fadeOut = true;

				transitioning = false;
				pendingRoomJson.clear();

			}
   		}

    	// Gameplay only when not transitioning
    	if (!transitioning) {
			bool pressedE = keyPressedOnce(window, GLFW_KEY_E);

			if (messageOpen) {
				if (pressedE){
					messageOpen = false;
					activeMessage.clear();

					noteOpen = false;
					activeNoteImagePath.clear();

					if (activeNoteTexture != 0) {
						glDeleteTextures(1, &activeNoteTexture);
						activeNoteTexture = 0;
					}

					DebugLog::log("[MESSAGE CLOSED]");
				}
			}
			else {
	        	updatePlayerMovement(window, player, camera, testRoom, deltaTime);

        		NearbyAction nearbyAction = getNearbyAction(testRoom, player.position);

				promptText = nearbyAction.promptText;

				if (nearbyAction.door && pressedE) {
					
					fade = 0.0f;
					fadeOut = false;
					fadeIn = true;

					transitioning = true;
					pendingRoomJson = nearbyAction.door->targetRoomJson;
					pendingSpawn = nearbyAction.door->targetSpawn;

				} else if (nearbyAction.interaction && pressedE) {
					
					runInteraction(*nearbyAction.interaction);

				}

			}
        		updateRoomCamera(testRoom, player, camera);
    	}

    
   		static int lastCam = -1;
		if (testRoom.activeCamera != lastCam) {
			DebugLog::log("[CAMERA] Active camera: " + std::to_string(testRoom.activeCamera));
        	lastCam = testRoom.activeCamera;
		}

		renderSceneToFBO(framebuffer, shader, *testRoomModel, playerModel, player, testRoom, camera);
		renderScreen(quadVAO, screenShader, textureColorBuffer, time);
		

		if (!messageOpen && !transitioning && !promptText.empty()) {
			
			float promptWidth = textRenderer.measureTextWidth(promptText, promptScale);
			float promptX = (gFramebufferWidth * 0.5f) - (promptWidth * 0.5f);

			float promptY = 80.0f;

			float paddingX = 18.0f;
			float rectangleX = promptX - paddingX;
			float rectangleY = promptY - 12.0f;
			float rectangleW = promptWidth + paddingX * 2.0f;
			float rectangleH = 36.0f;

			uiRenderer.drawRectangle(
				uiRectangleShader,
				rectangleX,
				rectangleY,
				rectangleW,
				rectangleH,
				glm::vec3(0.0f, 0.0f, 0.0f),
				0.55f,
				gFramebufferWidth,
				gFramebufferHeight
			);

			textRenderer.renderText(
				textShader,
				promptText,
				promptX,
				80.0f,
				promptScale,
				glm::vec3(1.0f, 0.9f, 0.65f),
				gFramebufferWidth,
				gFramebufferHeight
			);
		}

		if (noteOpen && activeNoteTexture != 0) {
			glDisable(GL_DEPTH_TEST);

			uiShader.use();

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, activeNoteTexture);

			glBindVertexArray(quadVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			glBindVertexArray(0);

			glEnable(GL_DEPTH_TEST);
		}

		drawDebugTerminal(	uiRenderer, 
							uiRectangleShader,
							textRenderer, 
							textShader,
							gFramebufferWidth, 
							gFramebufferHeight);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}	
	
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &skyboxVBO);
	glDeleteBuffers(1, &quadVBO);
	glDeleteRenderbuffers(1, &RBO);
    glDeleteFramebuffers(1, &framebuffer);
		
	glfwTerminate();
    return 0;
}


void glfwErrorCallback(int error, const char* description){
	std::cerr << "GLFW error " << error << ": " << description << std::endl;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	if (width <= 0 || height <= 0)
		return;

	gFramebufferWidth = width;
	gFramebufferHeight = height;
	gFramebufferResized = true;
}


unsigned int loadTexture(const char* path){ //load 2D texture from file
	unsigned int textureID;
	glGenTextures(1, &textureID);
	
	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if(data){
		GLenum format = GL_RGB;
		switch(nrComponents){
			case 1: format = GL_RED; break;
			case 3: format = GL_RGB; break;
			case 4: format = GL_RGBA; break;
			default: break; 
		}
		
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		stbi_image_free(data);
	}
	else {
		DebugLog::log(std::string("[TEXTURE LOAD ERROR] Texture failed to load at path: ") + path);
		stbi_image_free(data);
	}
	
	return textureID;
}

 //loads a cubemap texture from 6 individual texture faces in order:
 //+X (right) -X (left)
 //+Y (top) -Y (bottom)
 //+Z (front) -Z (back)
unsigned int loadCubemap(std::vector<std::string> faces){
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	int width, height, nrChannels;
	for(unsigned int i=0; i<faces.size(); i++){
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if(data){
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else{
			DebugLog::log("[CUBEMAP ERROR] Failed to load texture: " + faces[i]);
			stbi_image_free(data);
		}
	}
	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	
	return textureID;
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn){
	float xpos = static_cast<float>(xposIn);
	    float ypos = static_cast<float>(yposIn);
	    if (firstMouse){
	        lastX = xpos;
	        lastY = ypos;
	        firstMouse = false;
	    }

	    float xoffset = xpos - lastX;
	    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	    lastX = xpos;
	    lastY = ypos;

	    camera.processMouseMovement(xoffset, yoffset);
	
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
    camera.processMouseScroll(static_cast<float>(yoffset));
}

static void updateFade(float deltaTime) {
	if(fadeIn){
		fade += deltaTime;
		if(fade >= 1.0f){
			fade = 1.0f;
			fadeIn = false;
		}
	} else if (fadeOut) {
		fade -= deltaTime;
		if(fade <= 0.0f){
			fade = 0.0f;
			fadeOut = false;
		}
	}

	fade = glm::clamp(fade, 0.0f, 1.0f);
}

	
static void updateRoomCamera(Room& room, const Player& player, Camera& camera){
	roomCameraSwitch(room, player.position);
	applyRoomCamera(room, camera);
}

static void renderSceneToFBO(unsigned int framebuffer, Shader& shader, Model& roomModel, Model& playerModel, const Player& player, const Room& room, Camera& camera){
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glViewport(0, 0, gFramebufferWidth, gFramebufferHeight);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader.use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	shader.setInt("texture_diffuse1", 0);

	//glm;:vec3 ambientColor = glm::vec3(0.10f, 0.10f, 0.12f); 
	//glm::vec3 ambientColor = glm::vec3(0.18f, 0.18f, 0.20f); 
	//glm::vec3 ambientColor = glm::vec3(0.25f, 0.25f, 0.28f); 

	shader.setVec3("ambientColor", room.ambientColor);
	
	const int MAX_POINT_LIGHTS = 8;
	const int MAX_SPOT_LIGHTS = 4;

	int numPointLights = 0;
	int numSpotLights = 0;
	
	bool shouldPrintLightDebug = debugPrintLightInformation;

	if (!room.lights.empty()) {

		for (int i=0; i < (int)room.lights.size(); ++i) {
			const PointLight& light = room.lights[i];

			float t = static_cast<float>(glfwGetTime());

			float multiplier = calculateFlickerMultiplier(light, t, i);
			float intensity = light.intensity * multiplier;

			if (shouldPrintLightDebug) {
				std::ostringstream ss;

				ss << "[LIGHT DEBUG] room=" << room.name << " index=" << i
				   << " type=" << light.type
				   << " flickerType=" << light.flickerType
				   << " baseIntensity=" << light.intensity
				   << " multiplier=" << multiplier
				   << " uploadedIntensity=" << intensity;

				DebugLog::log(ss.str());
			}

			if (light.type == "spot" && !debugSpotAsPoint) {
				if (numSpotLights >= MAX_SPOT_LIGHTS) {
					DebugLog::log("[LIGHT WARNING] Maximum number of spot lights reached. Ignoring additional spot light.");
					continue;
				}

				glm::vec3 direction = light.target - light.position;

				if (shouldPrintLightDebug) {
					std::ostringstream ss;

					ss << "[SPOT DEBUG] position=" << light.position.x << ", "
					   << light.position.y << ", " << light.position.z
					   << " target=" << light.target.x << ", " << light.target.y
					   << ", " << light.target.z << " direction=" << direction.x
					   << ", " << direction.y << ", " << direction.z
					   << " inner=" << light.innerCutoff
					   << " outer=" << light.outerCutoff;

					DebugLog::log(ss.str());
				}

				if (glm::length(direction) < 0.001f)
					direction = glm::vec3(0.0f, -1.0f, 0.0f);
				else
					direction = glm::normalize(direction);

				std::string base = "spotLights[" + std::to_string(numSpotLights) +"]";

				shader.setVec3(base + ".position", light.position);
				shader.setVec3(base + ".direction", direction);
				shader.setVec3(base + ".color", light.color);
				shader.setFloat(base + ".intensity", intensity);
				shader.setFloat(base + ".radius", light.radius);
				shader.setFloat(base + ".innerCutoff", light.innerCutoff);
				shader.setFloat(base + ".outerCutoff", light.outerCutoff);

				numSpotLights++;

				if (shouldPrintLightDebug) {
					std::ostringstream ss;

					ss << "[LIGHT DEBUG] numPointLights=" << numPointLights << " numSpotLights=" << numSpotLights;

					DebugLog::log(ss.str());
				}

			} else {

				if (numPointLights >= MAX_POINT_LIGHTS) {
					DebugLog::log("[LIGHT WARNING] Maximum number of point lights reached. Ignoring additional point light.");
					continue;
				}
				std::string base = "pointLights[" + std::to_string(numPointLights) + "]";
				shader.setVec3(base + ".position", light.position);
				shader.setVec3(base + ".color", light.color);
				shader.setFloat(base + ".intensity", intensity);
				shader.setFloat(base + ".radius", light.radius);

				numPointLights++;

			}

			if (debugPrintLightInformation)
				debugPrintLightInformation = false;
		}
	}
	else {
		//if there are no lights in the room - a light will be fixed to the player
		numPointLights = 1;
		
		std::string base = "pointLights[0]";

		shader.setVec3(base + ".position", player.position + glm::vec3(0.0f, 2.0f, 0.0f));
		shader.setVec3(base + ".color", glm::vec3(1.0f, 0.75f, 0.45f));
		shader.setFloat(base + ".intensity", 3.0f);
		shader.setFloat(base + ".radius", 10.0f);

	}
		
	shader.setInt("numPointLights", numPointLights);
	shader.setInt("numSpotLights", numSpotLights);

	float aspect = (float)gFramebufferWidth / (float)gFramebufferHeight;

	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspect, 0.1f, 100.0f);
	glm::mat4 view = camera.getViewMatrix();
	shader.setMat4("projection", projection);
	shader.setMat4("view", view);

	glActiveTexture(GL_TEXTURE0);
	shader.setInt("texture_diffuse1", 0);
	
	shader.setVec3("emissiveColor", glm::vec3(0.0f));
	shader.setFloat("emissiveStrength", 0.0f);

	glm::mat4 model = glm::mat4(1.0f);
        //model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0,1,0));
	shader.setMat4("model", model);
	roomModel.Draw(shader);

	glActiveTexture(GL_TEXTURE0);
	shader.setInt("texture_diffuse1", 0);

	for (const Prop& prop : room.props) {

		if (prop.modelPath.empty()) {
			DebugLog::log(std::string("[PROP WARNING] Prop has empty model path: ") + prop.id);
			continue;
		}

		Model& propModel = getCachedModel(prop.modelPath);

		glm::mat4 propModelMatrix = glm::mat4(1.0f);

		propModelMatrix = glm::translate(propModelMatrix, prop.position);

		propModelMatrix = glm::rotate(
				propModelMatrix, 
				glm::radians(prop.rotation.x),
				glm::vec3(1.0f, 0.0f, 0.0f)
		);

		propModelMatrix = glm::rotate(
				propModelMatrix, 
				glm::radians(prop.rotation.y),
				glm::vec3(0.0f, 1.0f, 0.0f)
		);
		
		propModelMatrix = glm::rotate(
				propModelMatrix, 
				glm::radians(prop.rotation.z),
				glm::vec3(0.0f, 0.0f, 1.0f)
		);

		if (prop.spin) {
			float spinAngle = static_cast<float>(glfwGetTime()) * prop.spinSpeed;

			propModelMatrix = glm::rotate(
				propModelMatrix, 
				glm::radians(spinAngle),
				prop.spinAxis
			);

		}

		propModelMatrix = glm::scale(propModelMatrix, prop.scale);

		shader.setMat4("model", propModelMatrix);

		if (prop.glow) {
			shader.setVec3("emissiveColor", prop.glowColor);
			shader.setFloat("emissiveStrength", prop.glowStrength);
		} else {
			shader.setVec3("emissiveColor", glm::vec3(0.0f));
			shader.setFloat("emissiveStrength", 0.0f);
		}

		propModel.Draw(shader);

	}

	shader.setVec3("emissiveColor", glm::vec3(0.0f));
	shader.setFloat("emissiveStrength", 0.0f);
	
	glm::mat4 playerMo = glm::mat4(1.0f);
	playerMo = glm::translate(playerMo, player.position);
	shader.setMat4("model", playerMo);
	playerModel.Draw(shader);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void renderScreen(unsigned int quadVAO, Shader& screenShader, unsigned int textureColorBuffer, float time) {

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, gFramebufferWidth, gFramebufferHeight);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glClearColor(1,1,1,1);
	glClear(GL_COLOR_BUFFER_BIT);

	screenShader.use();

	screenShader.setInt("screenTexture", 0);

	screenShader.setFloat("fade" , fade);
	screenShader.setFloat("time", time);

	screenShader.setFloat("vignetteStrength", 0.0f);
	screenShader.setFloat("grainStrength", 0.0f);
	screenShader.setFloat("saturation", 1.0f);
	screenShader.setFloat("contrast", 1.0f);
	screenShader.setFloat("brightness", 0.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureColorBuffer);

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}

static void runInteraction(Interaction& interaction){
	switch (interaction.type){
		case InteractionType::Message:
			activeMessage = interaction.message;
			messageOpen = true;

			DebugLog::log("[MESSAGE] " + interaction.message);
			DebugLog::log("[MESSAGE] Press E to close.");

			break;

		case InteractionType::Note:
			noteOpen = true;
			messageOpen = true;
			
			activeNoteImagePath = interaction.assetPath;

			if (activeNoteTexture != 0) {
				glDeleteTextures(1, &activeNoteTexture);
				activeNoteTexture = 0;
			}

			activeNoteTexture = loadTexture2D(
				FileSystem::pathFromRoot(activeNoteImagePath).c_str()
			);

			DebugLog::log("[NOTE] Opened: " + activeNoteImagePath);
			DebugLog::log("[NOTE] Press E to close.");

			break;

		case InteractionType::Pickup:
			activeMessage = "Picked up: " + interaction.targetId;
			messageOpen = true;

			DebugLog::log("[PICKUP] Picked up: " + interaction.targetId);
			DebugLog::log("[PICKUP] Press E to close.");

			break;

		case InteractionType::Switch:
			activeMessage = "Triggered: " + interaction.targetId;
			messageOpen = true;

			DebugLog::log("[SWITCH] Triggered: " + interaction.targetId);
			DebugLog::log("[SWITCH] Press E to close.");

			break;

		default:
			activeMessage = "Unknown interaction: " + interaction.id;
			messageOpen = true;

			DebugLog::log("[INTERACTION] Unknown interaction: " + interaction.id);
			DebugLog::log("[INTERACTION] Press E to close.");

			break;
	}

	interaction.used = true;
}

//intial idea for this function is to load notes on screen
//related to Interactions xoxoxox
static unsigned int loadTexture2D(const char* path) {

	unsigned int textureID;
	glGenTextures(1, &textureID);
	
	int width, height, nrComponents;
	stbi_set_flip_vertically_on_load(true);

	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);

	if (data) {
		GLenum format = GL_RGB;
		
		switch (nrComponents) {
			case 1: format = GL_RED; break;

			case 3: format = GL_RGB; break;
		
			case 4: format = GL_RGBA; break;

			default: format = GL_RGB; break;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
		stbi_image_free(data);
	}
	else {
		DebugLog::log(std::string("[UI TEXTURE ERROR] Failed to load: ") + path);
		stbi_image_free(data);
	}
	
	return textureID;
}

static Model& getRoomModel(const Room& room){
	std::string path = FileSystem::pathFromRoot(room.modelPath);

	auto it = roomModelCache.find(path);
	if (it == roomModelCache.end()) {
		roomModelCache[path] = std::make_unique<Model>(path.c_str());
		return *roomModelCache[path];
	}

	return *it->second;
}

static float hashFloat(float x) {
	return glm::fract(sin(x * 12.9898f) * 43758.5453f);
}

static float noise1D(float x) {
	float i = floor(x);
	float f = glm::fract(x);

	float a = hashFloat(i);
	float b = hashFloat(i + 1.0f);

	float u = f * f * (3.0f - 2.0f * f);

	return glm::mix(a, b, u);
}

static float calculateFlickerMultiplier(const PointLight& light, float time, int lightIndex) {
	if (debugDisableFlicker)
		return 1.0f;

	if (light.flickerType == "lightning") {
		if (debugForcedLightningTimer > 0.0f)
			return light.flickerStrength * 4.0f;

		if (debugLightningPreview)
			return 1.0f;
	}
	
	if (!light.flicker)
		return 1.0f;

	float freq = glm::max(light.flickerFrequency, 0.001f);
	float strength = glm::clamp(light.flickerStrength, 0.0f, 1.0f);
	float randomness = glm::clamp(light.flickerRandomness, 0.0f, 1.0f);

	float phase = lightIndex * 17.31f;
	float x = time * freq + phase;

	if (light.flickerType == "fluorescent") { //fast small pulse like worn out halogen tubes
		float pulse = 0.5f + 0.5f * sin(x * 6.28318f);

		float noise = noise1D(x * 2.7f) * 2.0f - 1.0f;

		float variation = (pulse - 0.5f) * strength + noise * strength * randomness;

		return glm::clamp(1.0f + variation, 0.15f, 1.35f);
	}

	if (light.flickerType == "tungsten") { //irregular voltage incadescnet
		float slow = sin(x * 6.283f) * 0.5f;

		float n = noise1D(x * 1.35f) * 2.0f - 1.0f;

		float variation = slow * strength * 0.45f + n * strength * randomness;

		return glm::clamp(1.0f + variation, 0.25f, 1.25f);
	}

	if (light.flickerType == "lightning") { //thunderstorm, occasional flash
		float n = noise1D(x);

		if (n < light.flickerThreshold)
			return 0.0f;

		float flashAmount = (n - light.flickerThreshold) / glm::max(1.0f - light.flickerThreshold, 0.001f);

		float strobe = 0.65f + 0.35f * sin(time * 80.0f + phase); // rapid pulse after initial lightning to simulate
										// several flashes

		return glm::clamp(flashAmount * strobe * strength * 4.0f, 0.0f, 4.0f);
	}
	
	//default sine flicker
	float simple = 0.85f + 0.15f * sin(time * freq + phase);
	return glm::mix(1.0f, simple, strength);
}

static void updateLightningDebugInput(GLFWwindow* window, float deltatime) {
	if (keyPressedOnce(window, GLFW_KEY_F5)) {
		debugForcedLightningTimer = 0.25f;
		DebugLog::log("[LIGHT DEBUG] Forced lightning flash triggered");
	}

	if (keyPressedOnce(window, GLFW_KEY_F6)) {
		debugLightningPreview = !debugLightningPreview;
		DebugLog::log(std::string("[LIGHT DEBUG] Lightning preview ") + (debugLightningPreview ? "ON" : "OFF") );
	}

	if (keyPressedOnce(window, GLFW_KEY_F7)) {
		debugDisableFlicker = !debugDisableFlicker;
		DebugLog::log(std::string("[LIGHT DEBUG] Disable flicker: ") + (debugDisableFlicker ? "ON" : "OFF"));
	}
	
	if (keyPressedOnce(window, GLFW_KEY_F8)) {
		debugPrintLightInformation = !debugPrintLightInformation;
		DebugLog::log(std::string("[LIGHT DEBUG] Print light information: ") + (debugPrintLightInformation ? "ON" : "OFF"));
	}

	if (keyPressedOnce(window, GLFW_KEY_F9)) {
		debugSpotAsPoint = !debugSpotAsPoint;
		DebugLog::log(std::string("[LIGHT DEBUG] Spot as point: ") + (debugSpotAsPoint ? "ON" : "OFF"));
	}

	if (debugForcedLightningTimer > 0.0f) {
		debugForcedLightningTimer -= deltatime;
		if (debugForcedLightningTimer <= 0.0f) {
			debugForcedLightningTimer = 0.0f;
			DebugLog::log("[LIGHT DEBUG] Forced lightning flash ended");
		}
	}
}

static void resizeFramebufferAttachments(unsigned int fbo, unsigned int textureColorBuffer, unsigned int rbo) {
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glBindTexture(GL_TEXTURE_2D, textureColorBuffer);

	glTexImage2D(
		GL_TEXTURE_2D,
		0, 
		GL_RGB,
		gFramebufferWidth,
		gFramebufferHeight,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		nullptr
	);

	glBindTexture(GL_TEXTURE_2D, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, rbo);

	glRenderbufferStorage(
		GL_RENDERBUFFER,
		GL_DEPTH24_STENCIL8,
		gFramebufferWidth,
		gFramebufferHeight
	);

	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		DebugLog::log("[FRAMEBUFFER ERROR] Framebuffer incomplete after resize");
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	gFramebufferResized = false;

	DebugLog::log("[FBO RESIZE] Framebuffer: " + std::to_string(gFramebufferWidth) + "x" + std::to_string(gFramebufferHeight));
}

static Model& getCachedModel(const std::string& modelPath) {

	if (modelPath.empty()) {
		throw std::runtime_error("[CACHE MODEL ERROR] getCachedModel called with empty modelPath");
	}

	std::string path = FileSystem::pathFromRoot(modelPath);

	auto it = modelCache.find(path);

	if (it == modelCache.end()) {
		DebugLog::log("[MODEL CACHE] Loading: " + path);

		auto model = std::make_unique<Model>(path.c_str());
		auto result = modelCache.emplace(path, std::move(model));

		it = result.first;
	}

	if (!it->second) {
		throw std::runtime_error("[CACHE MODEL ERROR] Cached model is null: " + path);
	}

	return *it->second;
}

//wraps by words first and then breaks long words if necessary
static std::vector<std::string> wrapTextWidth(
												const std::string& text,
												TextRenderer& textRenderer,
												float scale, 
												float maxWidth) {
	std::vector<std::string> result;

	if (text.empty()) {
		result.push_back("");
		return result;
	}

	std::istringstream stream(text);
	std::string word;
	std::string currentLine;

	while (stream >> word) {
		std::string candidate;

		if (currentLine.empty())
			candidate = word;
		else
			candidate = currentLine + " " + word;

		if (textRenderer.measureTextWidth(candidate, scale) <= maxWidth)
			currentLine = candidate;
		
		else {
		
			if (!currentLine.empty()) {
				result.push_back(currentLine);
				currentLine.clear();
			}

			if (textRenderer.measureTextWidth(word, scale) <= maxWidth)
				currentLine = word;
			else {
				std::string chunk;

				for (char c : word) {
					std::string chunkCandidate = chunk + c;

					if (textRenderer.measureTextWidth(chunkCandidate, scale) > maxWidth && !chunk.empty()) {
						result.push_back(chunk);
						chunk = std::string(1, c);
					} else
						chunk = chunkCandidate;
				}
				
				currentLine = chunk;
			}
		}
	}

	if (!currentLine.empty())
		result.push_back(currentLine);

	return result;

}

static void drawDebugTerminal(UiRenderer &uiRenderer, Shader &uiRectangleShader,
							  TextRenderer &textRenderer, Shader &textShader,
							  int screenWidth, int screenHeight) {

	if (!debugTerminalOpen)
		return;

	float sw = static_cast<float>(screenWidth);
	float sh = static_cast<float>(screenHeight);

	float panelMargin = 20.0f;

	float panelX = panelMargin;
	float panelW = sw - panelMargin * 2.0f;

	if (panelW < 240.0f) panelW = 240.0f;

	float panelH = sh * 0.38f;

	if (panelH < 140.0f) panelH = 140.0f;
	if (panelH > 360.0f) panelH = 360.0f;

	float panelY = sh - panelH - panelMargin;

	uiRenderer.drawRectangle(	uiRectangleShader, 
								panelX,	
								panelY, 
								panelW, 
								panelH, 
								glm::vec3(0.0f, 0.0f, 0.0f), 
								0.78f, 
								screenWidth,
								screenHeight
	);

	float textScale = 0.65f;
	float padding = 14.0f;
	float lineHeight = 17.0f;

	textRenderer.renderText(	textShader, 
								"DEBUG TERMINAL [F1]", 
								panelX + padding,
								panelY + panelH - 28.0f, 
								textScale,
								glm::vec3(0.75f, 1.0f, 0.75f), 
								screenWidth,
								screenHeight
	);

	const std::vector<std::string> &lines = DebugLog::getLines();

	if (lines.empty()) {

		textRenderer.renderText(	textShader, 
									"No debug messages yet.",
									panelX + padding, 
									panelY + panelH - 55.0f,
									textScale, 
									glm::vec3(0.85f, 0.9f, 0.85f),
									screenWidth, 
									screenHeight
		);

		return;
	}

	float maxTextWidth = panelW - padding * 2.0f;
	float startY = panelY + panelH - 55.0f;
	float bottomY = panelY + padding;

	std::vector<std::string> wrappedLines;

	for (const std::string &line : lines) {
		std::vector<std::string> wrapped = wrapTextWidth(line, textRenderer, textScale, maxTextWidth);
		wrappedLines.insert(wrappedLines.end(), wrapped.begin(), wrapped.end());
	}

	int maxVisibleLines = static_cast<int>((startY - bottomY) / lineHeight);
	if (maxVisibleLines <= 0)
		return;

	int startIndex = glm::max(0, static_cast<int>(wrappedLines.size()) - maxVisibleLines);

	float y = startY;

	for (int i = startIndex; i < (int)wrappedLines.size(); ++i) {
		textRenderer.renderText(	
									textShader, 
									wrappedLines[i], 
									panelX + padding, 
									y, 
									textScale,
									glm::vec3(0.85f, 0.9f, 0.85f), 
									screenWidth, 
									screenHeight
		);

		y -= lineHeight;

		if (y < bottomY)
			break;
	}
}
