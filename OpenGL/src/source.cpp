
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
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <memory>
#include "include/test_vertices.h"

static bool circleAABBIntersectXZ(const glm::vec3& pos, float radius, const AABB& box);
static glm::vec3 getMoveDirInput(GLFWwindow* window, const glm::vec3& camForward, const glm::vec3& camRight);
static void updatePlayerMovement(GLFWwindow* window, Player& player, const Camera& camera, const Room& room, float deltaTime);
static void updateFade(float deltaTime);
static void updateRoomCamera(Room& room, const Player& player, Camera& camera);
static void renderSceneToFBO(unsigned int framebuffer, Shader& shader, Model& roomModel, Model& playerModel, const Player& player, Camera& camera);
static void renderScreen(unsigned int quadVAO, Shader& screenShader, unsigned int textureColorBuffer);
static bool keyPressedOnce(GLFWwindow* window, int key);
static const DoorTrigger* getDoorInRange(const Room& room, const glm::vec3& playerPos);
static Interaction* getInteractionInRange(Room& room, const glm::vec3& playerPos);
static void runInteraction(Interaction& interaction);


int WIDTH = 800;
int HEIGHT = 600;

void glfwErrorCallback(int error, const char* description);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
unsigned int loadCubemap(std::vector<std::string> faces);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);


Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = (float)WIDTH / 2.0;
float lastY = (float)HEIGHT / 2.0;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

//for scene fade in and fade out
float fade = 0.0f;
bool fadeIn = true; 
bool fadeOut = false;

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	
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



	std::unique_ptr<Model> testRoomModel = std::make_unique<Model>(
		FileSystem::pathFromRoot(testRoom.modelPath).c_str()
	);

	Model playerModel(
		FileSystem::pathFromRoot("src/models/test_character/test_character.obj").c_str()
	);

	Player player;
	player.position = testRoom.playerSpawn;

	bool transitioning = false;
	std::string pendingRoomJson;
	glm::vec3 pendingSpawn(0.0f);
	

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
	screenShader.setInt("screenTexture", 0);
	
	//framebuffer
	unsigned int framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	
	//color attachment texture
	unsigned int textureColorBuffer;
	glGenTextures(1, &textureColorBuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);
	
	//renderbuffer obj
	unsigned int RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
	
	//check if framebuffer was crated
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete! Kaput!!!!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	lastFrame = static_cast<float>(glfwGetTime());

	//main game loop
	while (!glfwWindowShouldClose(window)) {
    		float currentFrame = static_cast<float>(glfwGetTime());
    		deltaTime = currentFrame - lastFrame;
    		lastFrame = currentFrame;

    		deltaTime = glm::min(deltaTime, 1.0f/30.0f);

    		processInput(window);

    		updateFade(deltaTime);

    		// If fade-out finished, swap rooms
    		if (transitioning && !fadeOut && fade <= 0.0f) {
       			try {
				testRoom = loadRoomFromJson(FileSystem::pathFromRoot(pendingRoomJson).c_str());
			
				testRoomModel = std::make_unique<Model>(
					FileSystem::pathFromRoot(testRoom.modelPath).c_str()
				);
				
				player.position = pendingSpawn;
				roomCameraSwitch(testRoom, player.position);
        			applyRoomCamera(testRoom, camera);

				fade = 0.0f;
				fadeIn = true;
				fadeOut = false;

				transitioning = false;
				pendingRoomJson.clear();
				
			} catch (const std::exception& e) {
				std::cerr << "Room failed to load: " << e.what() << "\n";
				transitioning = false;
				pendingRoomJson.clear();

				fade = 0.0f;
				fadeIn = true;
				fadeOut = false;
			}
        		
   	 	}

    		// Gameplay only when not transitioning
    		if (!transitioning) {
        		updatePlayerMovement(window, player, camera, testRoom, deltaTime);

        		const DoorTrigger* door = getDoorInRange(testRoom, player.position);
        		if (door && keyPressedOnce(window, GLFW_KEY_E)) {
           			transitioning = true;
            			pendingRoomJson = door->targetRoomJson;
            			pendingSpawn = door->targetSpawn;

            			fade = 1.0f;
            			fadeOut = true;
            			fadeIn = false;
        		}
			else if (keyPressedOnce(window, GLFW_KEY_E)) {
				Interaction* interaction = getInteractionInRange(testRoom, player.position);
				if (interaction) 
					runInteraction(*interaction);
			}

        		updateRoomCamera(testRoom, player, camera);
    		}

    
   		static int lastCam = -1;
		if (testRoom.activeCamera != lastCam) {
        		std::cout << "Active camera: " << testRoom.activeCamera << "\n";
        		lastCam = testRoom.activeCamera;
		}

		renderSceneToFBO(framebuffer, shader, *testRoomModel, playerModel, player, camera);
		renderScreen(quadVAO, screenShader, textureColorBuffer);

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
	glViewport(0,0,width,height);
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

static void renderSceneToFBO(unsigned int framebuffer, Shader& shader, Model& roomModel, Model& playerModel, const Player& player, Camera& camera){
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader.use();
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH/(float)HEIGHT, 0.1f, 100.0f);
	glm::mat4 view = camera.getViewMatrix();
	shader.setMat4("projection", projection);
	shader.setMat4("view", view);

	glm::mat4 model = glm::mat4(1.0f);
        //model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0,1,0));
	shader.setMat4("model", model);
	roomModel.Draw(shader);
	
	glm::mat4 playerMo = glm::mat4(1.0f);
	playerMo = glm::translate(playerMo, player.position);
	shader.setMat4("model", playerMo);
	playerModel.Draw(shader);
}

static void renderScreen(unsigned int quadVAO, Shader& screenShader, unsigned int textureColorBuffer){
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, WIDTH, HEIGHT);
	glDisable(GL_DEPTH_TEST);

	glClearColor(1,1,1,1);
	glClear(GL_COLOR_BUFFER_BIT);

	screenShader.use();
	screenShader.setFloat("fade" , fade);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureColorBuffer);

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
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
			std::cout << "[MESSAGE] " << interaction.message << "\n";
			break;

		case InteractionType::Note:
			std::cout << "[NOTE] Open note asset: " << interaction.assetPath << "\n";
			break;

		case InteractionType::Pickup:
			std::cout << "[PICKUP] Picked up: " << interaction.targetId << "\n";
			break;

		case InteractionType::Switch:
			std::cout << "[SWICTH] Triggered: " << interaction.targetId << "\n";
			break;

		default:
			std::cout << "[INTERACTION] Unknown interaction: " << interaction.id << "\n";
			break;
	}

	interaction.used = true;
}





