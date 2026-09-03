#include "include/SceneRenderer.h"
#include "include/FramebufferSystem.h"

#include <glad/glad.h>

#include <sstream>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "include/AppState.h"
#include "include/GameState.h"
#include "include/Shader.h"
#include "include/Model.h"
#include "include/Room.h"
#include "include/Player.h"
#include "include/Camera.h"
#include "include/AssetCache.h"
#include "include/DebugLog.h"
#include "include/LightingSystem.h"
#include "include/Light.h"
#include "include/Prop.h"

namespace {
	constexpr int MAX_POINT_LIGHTS = 8;
	constexpr int MAX_SPOT_LIGHTS = 4;
} // namespace


static void uploadRoomLights(Shader &shader, const Room &room,
							 const Player &player, float time,
							 LightingDebugState &lightingDebugState) {

	int numPointLights = 0;
	int numSpotLights = 0;

	bool shouldPrintLightDebug = lightingDebugState.printLightInformation;

	if (!room.lights.empty()) {
		for (int i = 0; i < static_cast<int>(room.lights.size()); ++i) {
			const PointLight &light = room.lights[i];

			float multiplier =
				calculateFlickerMultiplier(light, time, i, lightingDebugState);

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

			if (light.type == "spot" && !lightingDebugState.spotAsPoint) {
				if (numSpotLights >= MAX_SPOT_LIGHTS) {
					DebugLog::log(
						"[LIGHT WARNING] Maximum number of spot lights "
						"reached. Ignoring additional spot lights.");
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

				std::string base =
					"spotLights[" + std::to_string(numSpotLights) + "]";

				shader.setVec3(base + ".position", light.position);
				shader.setVec3(base + ".direction", direction);
				shader.setVec3(base + ".color", light.color);
				shader.setFloat(base + ".intensity", intensity);
				shader.setFloat(base + ".radius", light.radius);
				shader.setFloat(base + ".innerCutoff", light.innerCutoff);
				shader.setFloat(base + ".outerCutoff", light.outerCutoff);

				numSpotLights++;

			} else {

				if (numPointLights >= MAX_POINT_LIGHTS) {
					DebugLog::log(
						"[LIGHT WARNING] Maximum number of point lights "
						"reached. Ignoring additional point lights.");
					continue;
				}
				std::string base =
					"pointLights[" + std::to_string(numPointLights) + "]";
				shader.setVec3(base + ".position", light.position);
				shader.setVec3(base + ".color", light.color);
				shader.setFloat(base + ".intensity", intensity);
				shader.setFloat(base + ".radius", light.radius);

				numPointLights++;
			}
		}
	} else {
		// if there are no lights in the room - a light will be fixed to the
		// player
		numPointLights = 1;

		std::string base = "pointLights[0]";

		shader.setVec3(base + ".position",
					   player.position + glm::vec3(0.0f, 2.0f, 0.0f));
		shader.setVec3(base + ".color", glm::vec3(1.0f, 0.75f, 0.45f));
		shader.setFloat(base + ".intensity", 3.0f);
		shader.setFloat(base + ".radius", 10.0f);
	}

	shader.setInt("numPointLights", numPointLights);
	shader.setInt("numSpotLights", numSpotLights);

	if (shouldPrintLightDebug) {
		std::ostringstream ss;

		ss << "[LIGHT DEBUG] numPointLights=" << numPointLights
		   << " numSpotLights=" << numSpotLights;

		DebugLog::log(ss.str());

		lightingDebugState.printLightInformation = false;
	}
}

static void drawRoom(Shader &shader, Model &roomModel) {

	shader.setVec3("emissiveColor", glm::vec3(0.0f));
	shader.setFloat("emissiveStrength", 0.0f);

	glm::mat4 model = glm::mat4(1.0f);

	shader.setMat4("model", model);
	roomModel.Draw(shader);
}

static void drawProps(Shader &shader, const Room &room, float time) {

	glActiveTexture(GL_TEXTURE0);
	shader.setInt("texture_diffuse1", 0);

	for (const Prop &prop : room.props) {

		if (prop.modelPath.empty()) {
			DebugLog::log(
				std::string("[PROP WARNING] Prop has empty model path: ") +
				prop.id);
			continue;
		}

		Model &propModel = getCachedModel(prop.modelPath);

		glm::mat4 propModelMatrix = glm::mat4(1.0f);

		propModelMatrix = glm::translate(propModelMatrix, prop.position);

		propModelMatrix =
			glm::rotate(propModelMatrix, glm::radians(prop.rotation.x),
						glm::vec3(1.0f, 0.0f, 0.0f));

		propModelMatrix =
			glm::rotate(propModelMatrix, glm::radians(prop.rotation.y),
						glm::vec3(0.0f, 1.0f, 0.0f));

		propModelMatrix =
			glm::rotate(propModelMatrix, glm::radians(prop.rotation.z),
						glm::vec3(0.0f, 0.0f, 1.0f));

		if (prop.spin) {
			float spinAngle = time * prop.spinSpeed;

			propModelMatrix = glm::rotate(
				propModelMatrix, glm::radians(spinAngle), prop.spinAxis);
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
}

static void drawPlayer(Shader &shader, Model &playerModel,
					   const Player &player) {

	shader.setVec3("emissiveColor", glm::vec3(0.0f));
	shader.setFloat("emissiveStrength", 0.0f);

	glm::mat4 playerMo = glm::mat4(1.0f);
	playerMo = glm::translate(playerMo, player.position);

	shader.setMat4("model", playerMo);
	playerModel.Draw(shader);
}

void renderSceneToFBO(	const FramebufferState& fboState, 
						Shader &shader,
						Model &playerModel, 
						const AppState &appState,
						GameState &gameState
) {

	if (appState.framebufferWidth <= 0 || appState.framebufferHeight <= 0)
		return;

	if (gameState.roomModel == nullptr) {
		DebugLog::log("[RENDER ERROR] No active room model");
		return;
	}

	float time = appState.timing.currentTime;

	glBindFramebuffer(GL_FRAMEBUFFER, fboState.framebuffer);

	glViewport(0, 0, appState.framebufferWidth, appState.framebufferHeight);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader.use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	shader.setInt("texture_diffuse1", 0);

	shader.setVec3("ambientColor", gameState.room.ambientColor);

	uploadRoomLights(shader, gameState.room, gameState.player, time,
					 gameState.lightingDebug);

	float aspect = static_cast<float>(appState.framebufferWidth) /
				   static_cast<float>(appState.framebufferHeight);

	glm::mat4 projection = glm::perspective(glm::radians(gameState.camera.Zoom),
											aspect, 0.1f, 100.0f);

	glm::mat4 view = gameState.camera.getViewMatrix();

	shader.setMat4("projection", projection);
	shader.setMat4("view", view);

	glActiveTexture(GL_TEXTURE0);
	shader.setInt("texture_diffuse1", 0);

	drawRoom(shader, *gameState.roomModel);
	drawProps(shader, gameState.room, time);
	drawPlayer(shader, playerModel, gameState.player);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}