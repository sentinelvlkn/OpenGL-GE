
#include <cstddef>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "include/Shader.h"
#include "include/Camera.h"
#include "include/Model.h"
#include "include/Room.h"
#include "include/Player.h"
#include "include/Interaction.h"
#include "stb_image.h"
#include "include/FileSystem.h"
#include "include/TextRenderer.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <memory>
#include <unordered_map>
#include "include/test_vertices.h"

static bool circleAABBIntersectXZ(const glm::vec3& pos, float radius, const AABB& box);
static glm::vec3 getMoveDirInput(GLFWwindow* window, const glm::vec3& camForward, const glm::vec3& camRight);
static void updatePlayerMovement(GLFWwindow* window, Player& player, const Camera& camera, const Room& room, float deltaTime);
static void updateFade(float deltaTime);
static void updateRoomCamera(Room& room, const Player& player, Camera& camera);
static void renderSceneToFBO(unsigned int framebuffer, Shader& shader, Model& roomModel, Model& playerModel, const Player& player, const Room& room, Camera& camera);
static void renderScreen(unsigned int quadVAO, Shader& screenShader, unsigned int textureColorBuffer, float time);
static bool keyPressedOnce(GLFWwindow* window, int key);
static const DoorTrigger* getDoorInRange(const Room& room, const glm::vec3& playerPos);
static Interaction* getInteractionInRange(Room& room, const glm::vec3& playerPos);
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


int WIDTH = 800;
int HEIGHT = 600;

static int gFramebufferWidth = WIDTH;
static int gFramebufferHeight = HEIGHT;
static bool gFramebufferResized = false;

void glfwErrorCallback(int error, const char* description);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
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

struct NearbyAction {
	const DoorTrigger *door = nullptr;
	Interaction *interaction = nullptr;
	std::string promptText;

	bool hasAction() const { return door != nullptr || interaction != nullptr; }
};
static std::string getInteractionPromptText(const Interaction &interaction);
static NearbyAction getNearbyAction(Room &room, const glm::vec3 &playerPos);

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

	/*Model backpackModel(
		FileSystem::pathFromRoot("src/models/backpack/backpack.obj").c_str()
	);*/
	
	stbi_set_flip_vertically_on_load(false);
	Room testRoom = loadRoomFromJson(
		FileSystem::pathFromRoot("src/models/test_room/test_room.json").c_str()
	);

	for (int i = 0; i < testRoom.cameras.size(); ++i) {
    		auto& c = testRoom.cameras[i];
	    	std::cout << "Cam " << i
        	      << " min(" << c.aabbMin.x << "," << c.aabbMin.y << "," << c.aabbMin.z << ") "
              	<< "max(" << c.aabbMax.x << "," << c.aabbMax.y << "," << c.aabbMax.z << ")\n";
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
	std::string fontPath =
		FileSystem::pathFromRoot("src/ui/fonts/LektonNerdFontMono-Regular.ttf");

	if (!textRenderer.loadFont(fontPath, 24)) 
		std::cerr << "Failed to load font\n";


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
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete! Kaput!!!!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	lastFrame = static_cast<float>(glfwGetTime());

	float promptScale = 0.75f;

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

    	processInput(window);
		updateLightningDebugInput(window, deltaTime);

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

				std::cerr << "Room failed to load: " << e.what() << "\n";
				
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

					std::cout << "[MESSAGE CLOSED]\n";
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
        		std::cout << "Active camera: " << testRoom.activeCamera << "\n";
        		lastCam = testRoom.activeCamera;
		}

		renderSceneToFBO(framebuffer, shader, *testRoomModel, playerModel, player, testRoom, camera);
		renderScreen(quadVAO, screenShader, textureColorBuffer, time);
		
		float promptWidth = textRenderer.measureTextWidth(promptText, promptScale);
		float promptX = (gFramebufferWidth * 0.5f) - (promptWidth * 0.5f);

		if (!messageOpen && !transitioning && !promptText.empty()) {
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

void processInput(GLFWwindow* window){

	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	//if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	// 	camera.processKeyboard(FORWARD, deltaTime);

	//if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	//	camera.processKeyboard(BACKWARD, deltaTime);

	//if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	//	camera.processKeyboard(LEFT, deltaTime);

	//if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	//	camera.processKeyboard(RIGHT, deltaTime);
	//fadeIn sets to true
	if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
		fade = 0.0f;
		fadeIn = true;
	}
	// fadeOut sets to true	
	if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
		fade = 1.0f;
		fadeOut = true;
	}

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
		std::cout << "Texture failed to load at path: " << path << std::endl;
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
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
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

static bool circleAABBIntersectXZ(const glm::vec3& pos, float radius, const AABB& box){
	float closestX = glm::clamp(pos.x, box.min.x, box.max.x);
	float closestZ = glm::clamp(pos.z, box.min.z, box.max.z);

	float dx = pos.x - closestX;
	float dz = pos.z - closestZ;

	return (dx * dx + dz * dz) < (radius * radius);
}

static glm::vec3 getMoveDirInput(GLFWwindow* window, const glm::vec3& camForward, const glm::vec3& camRight){
	glm::vec3 moveDir(0.0f);
	
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		moveDir += camForward;
	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		moveDir -= camForward;
	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		moveDir -= camRight;
	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		moveDir += camRight;
	
	if(glm::length(moveDir) > 0.0f)
		moveDir = glm::normalize(moveDir);

	return moveDir;
}


static void updatePlayerMovement(GLFWwindow* window,
                                 Player& player,
                                 const Camera& camera,
                                 const Room& room,
                                 float deltaTime)
{
    // clamp dt to reduce tunneling on frame spikes
    deltaTime = glm::min(deltaTime, 1.0f / 30.0f);

    // camera-relative basis (flattened)
    glm::vec3 camForward = camera.Front;
    camForward.y = 0.0f;
    if (glm::length(camForward) > 0.0001f) camForward = glm::normalize(camForward);
    else camForward = glm::vec3(0, 0, -1);

    glm::vec3 camRight = camera.Right;
    camRight.y = 0.0f;
    if (glm::length(camRight) > 0.0001f) camRight = glm::normalize(camRight);
    else camRight = glm::vec3(1, 0, 0);

    glm::vec3 moveDir = getMoveDirInput(window, camForward, camRight);
    if (glm::length(moveDir) < 0.0001f)
        return;

    const float playerRadius = 0.40f;

    // substeps
    const int steps = 4;
    const float dt = deltaTime / (float)steps;

    static int lastHit = -1;

    for (int s = 0; s < steps; ++s) {
        glm::vec3 nextPos = player.position + moveDir * player.speed * dt;

        bool blocked = false;
        int hitIndex = -1;

        for (int i = 0; i < (int)room.colliders.size(); ++i) {
            if (circleAABBIntersectXZ(nextPos, playerRadius, room.colliders[i])) {
                blocked = true;
                hitIndex = i;
                break;
            }
        }

        if (blocked) {
            if (hitIndex != lastHit) {
                std::cout << "Blocked by collider #" << hitIndex << "\n";
                lastHit = hitIndex;
            }
            return; // stop moving further this frame
        }

        // not blocked -> commit this substep
        player.position = nextPos;
        lastHit = -1;
    }
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
				std::cout << "[LIGHT DEBUG] room=" << room.name
						  << " index=" << i << " type=" << light.type
						  << " flickerType=" << light.flickerType
						  << " baseIntensity=" << light.intensity
						  << " multiplier=" << multiplier
						  << " uploadedIntensity=" << intensity << "\n";
			}

			if (light.type == "spot" && !debugSpotAsPoint) {
				if (numSpotLights >= MAX_SPOT_LIGHTS) {
					std::cerr << "Warning: Maximum number of spot lights "
								 "reached. Ignoring additional spot light.\n";
					continue;
				}

				glm::vec3 direction = light.target - light.position;

				if (shouldPrintLightDebug) {
					std::cout
						<< "[SPOT DEBUG] position=" << light.position.x << ", "
						<< light.position.y << ", " << light.position.z
						<< " target=" << light.target.x << ", "
						<< light.target.y << ", " << light.target.z
						<< " direction=" << direction.x << ", " << direction.y
						<< ", " << direction.z << " inner=" << light.innerCutoff
						<< " outer=" << light.outerCutoff << "\n";
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
					std::cout
						<< "[LIGHT DEBUG] numPointLights=" << numPointLights
						<< " numSpotLights=" << numSpotLights << "\n";
				}

			} else {

				if (numPointLights >= MAX_POINT_LIGHTS) {
					std::cerr << "Warning: Maximum number of point lights "
								 "reached. Ignoring additional point light.\n";
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
			std::cerr << "WARNING: prop " << prop.id
					  << " has empty model path\n";
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
				glm::radians(prop.rotation.y),
				glm::vec3(0.0f, 0.0f, 1.0f)
		);

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

static bool keyPressedOnce(GLFWwindow* window, int key) {

    static bool wasDown[GLFW_KEY_LAST + 1] = { false };
    bool down = glfwGetKey(window, key) == GLFW_PRESS;
    bool pressed = down && !wasDown[key];
    wasDown[key] = down;
    return pressed;
}

static const DoorTrigger* getDoorInRange(const Room& room, const glm::vec3& playerPos) {
    for (const auto& d : room.doors) {
        if (pointInsideAabb(playerPos, d.aabb.min, d.aabb.max))
            return &d;
    }
    return nullptr;
}

static Interaction* getInteractionInRange(Room& room, const glm::vec3& playerPos){
	for (auto& interaction : room.interactions) {
		if (!interaction.repeatable && interaction.used) 
			continue;

		if (pointInsideAabb(playerPos, interaction.trigger))
			return &interaction;
	}

	return nullptr;
}

static void runInteraction(Interaction& interaction){
	switch (interaction.type){
		case InteractionType::Message:
			activeMessage = interaction.message;
			messageOpen = true;

			std::cout << "[MESSAGE] " << interaction.message << "\n";
			std::cout << "Press E to close" << "\n";

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

			std::cout << "[NOTE OPENED] " << activeNoteImagePath << "\n";
	
			std::cout << "Press E to close" << "\n";
			break;

		case InteractionType::Pickup:
			std::cout << "[PICKUP] Picked up: " << interaction.targetId << "\n";
			messageOpen = true;
			std::cout << "Press E to close" << "\n";
			break;

		case InteractionType::Switch:
			std::cout << "[SWICTH] Triggered: " << interaction.targetId << "\n";
			messageOpen = true;
			std::cout << "Press E to close" << "\n";
			break;

		default:
			std::cout << "[INTERACTION] Unknown interaction: " << interaction.id << "\n";
			messageOpen = true;
			std::cout << "Press E to close" << "\n";
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
		std::cerr << "Failed to load UI texture: " << path << "\n";
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
		std::cout << "[LIGHT DEBUG] Forced lightning flash triggered\n";
	}

	if (keyPressedOnce(window, GLFW_KEY_F6)) {
		debugLightningPreview = !debugLightningPreview;
		std::cout << "[LIGHT DEBUG] Lightning preview "
				  << (debugLightningPreview ? "ON" : "OFF") << "\n";
	}

	if (keyPressedOnce(window, GLFW_KEY_F7)) {
		debugDisableFlicker = !debugDisableFlicker;
		std::cout << "[LIGHT DEBUG] Disable flicker: "
				  << (debugDisableFlicker ? "ON" : "OFF") << "\n";
	}
	
	if (keyPressedOnce(window, GLFW_KEY_F8)) {
		debugPrintLightInformation = !debugPrintLightInformation;
		std::cout << "[LIGHT DEBUG] Print light information: "
				  << (debugPrintLightInformation ? "ON" : "OFF") << "\n";
	}

	if (keyPressedOnce(window, GLFW_KEY_F9)) {
		debugSpotAsPoint = !debugSpotAsPoint;
		std::cout << "[LIGHT DEBUG] Spot as point: "
				  << (debugSpotAsPoint ? "ON" : "OFF") << "\n";
	}

	if (debugForcedLightningTimer > 0.0f) {
		debugForcedLightningTimer -= deltatime;
		if (debugForcedLightningTimer <= 0.0f) {
			debugForcedLightningTimer = 0.0f;
			std::cout << "[LIGHT DEBUG] Forced lightning flash ended\n";
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
		std::cout
			<< "ERROR::FRAMEBUFFER:: framebuffer incomplete after resize\n";
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	gFramebufferResized = false;

	std::cout << "[RESIZE] framebuffer: " << gFramebufferWidth << "x"
			  << gFramebufferHeight << "\n";
}

static Model& getCachedModel(const std::string& modelPath) {

	if (modelPath.empty()) {
		throw std::runtime_error("getCachedModel called with empty modelPath");
	}

	std::string path = FileSystem::pathFromRoot(modelPath);

	auto it = modelCache.find(path);

	if (it == modelCache.end()) {
		std::cout << "[MODEL CACHE] Loading: " << path << "\n";

		auto model = std::make_unique<Model>(path.c_str());
		auto result = modelCache.emplace(path, std::move(model));

		it = result.first;
	}

	if (!it->second) {
		throw std::runtime_error("Cached model is null: " + path);
	}

	return *it->second;
}

static std::string getInteractionPromptText(const Interaction &interaction) {
	switch (interaction.type) {
	case InteractionType::Note:
		return "Press E to read";

	case InteractionType::Pickup:
		return "Press E to pick up";

	case InteractionType::Switch:
		return "Press E to use";

	case InteractionType::Message:
		return "Press E to inspect";

	default:
		return "Press E";
	}
}

static NearbyAction getNearbyAction(Room &room, const glm::vec3 &playerPos) {
	NearbyAction action;

	const DoorTrigger *door = getDoorInRange(room, playerPos);

	if (door) {
		action.door = door;
		action.promptText = "Press E to enter";
		return action;
	}

	Interaction *interaction = getInteractionInRange(room, playerPos);

	if (interaction) {
		action.interaction = interaction;
		action.promptText = getInteractionPromptText(*interaction);
		return action;
	}

	return action;
}