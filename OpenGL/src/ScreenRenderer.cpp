#include "include/ScreenRenderer.h"

#include <glad/glad.h>
#include <sstream>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "include/AssetCache.h"
#include "include/DebugLog.h"
#include "include/Light.h"
#include "include/Prop.h"

namespace {
	constexpr int MAX_POINT_LIGHTS = 8;
	constexpr int MAX_SPOT_LIGHTS = 4;
}


void renderScreen(
					unsigned int quadVAO, 
					Shader& screenShader,
					unsigned int textureColorBuffer, 
					float time, 
					float fadeValue,
					int width, 
					int height
) {
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	screenShader.use();

	screenShader.setInt("screenTexture", 0);

	screenShader.setFloat("fade", fadeValue);
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

static void uploadRoomLights(
	Shader& shader,
	const Room& room,
	const Player& player,
	float time,
	LightingDebugState& lightingDebugState) {

	int numPointLights = 0;
	int numSpotLights = 0;

	bool shouldPrintLightDebug = lightingDebugState.printLightInformation;

	if (!room.lights.empty()) {
		for (int i = 0; i < static_cast<int>(room.lights.size()); ++i) {
			const PointLight &light = room.lights[i];

			float multiplier = calculateFlickerMultiplier(light, time, i, lightingDebugState);

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
						"reached. Ignoring additional spot lights."
					);
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

				std::string base = "spotLights[" + std::to_string(numSpotLights) + "]";

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
						"reached. Ignoring additional point lights."
					);
					continue;
				}
				std::string base = "pointLights[" + std::to_string(numPointLights) + "]";
				shader.setVec3(base + ".position", light.position);
				shader.setVec3(base + ".color", light.color);
				shader.setFloat(base + ".intensity", intensity);
				shader.setFloat(base + ".radius", light.radius);

				numPointLights++;
			}

			if (lightingDebugState.printLightInformation)
				lightingDebugState.printLightInformation = false;
		}
	} else {
		// if there are no lights in the room - a light will be fixed to the player
		numPointLights = 1;

		std::string base = "pointLights[0]";

		shader.setVec3(base + ".position", player.position + glm::vec3(0.0f, 2.0f, 0.0f));
		shader.setVec3(base + ".color", glm::vec3(1.0f, 0.75f, 0.45f));
		shader.setFloat(base + ".intensity", 3.0f);
		shader.setFloat(base + ".radius", 10.0f);
	}

	shader.setInt("numPointLights", numPointLights);
	shader.setInt("numSpotLights", numSpotLights);

	if (shouldPrintLightDebug) {
		std::ostringstream ss;

		ss << "[LIGHT DEBUG] numPointLights=" << numPointLights << " numSpotLights=" << numSpotLights;

		DebugLog::log(ss.str());

		lightingDebugState.printLightInformation = false;
	}

}

static void drawRoom(Shader& shader, Model& roomModel) {

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
				prop.id
			);
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
			float spinAngle = time * prop.spinSpeed;

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
}

static void drawPlayer(Shader& shader, Model& playerModel, const Player& player) {
	
	shader.setVec3("emissiveColor", glm::vec3(0.0f));
	shader.setFloat("emissiveStrength", 0.0f);

	glm::mat4 playerMo = glm::mat4(1.0f);
	playerMo = glm::translate(playerMo, player.position);

	shader.setMat4("model", playerMo);
	playerModel.Draw(shader);
}

void renderSceneToFBO(
							unsigned int framebuffer, 
							Shader& shader,
							Model& roomModel, 
							Model& playerModel,
							const Player& player, 
							const Room& room,
							Camera& camera,
							int fboWidth,
							int fboHeight,
							float time,
							LightingDebugState& lightingDebugState 
) {

	if (fboWidth <= 0 || fboHeight <= 0)
		return;

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glViewport(0, 0, fboWidth, fboHeight);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader.use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	shader.setInt("texture_diffuse1", 0);

	shader.setVec3("ambientColor", room.ambientColor);

	uploadRoomLights(
		shader,
		room,
		player,
		time,
		lightingDebugState
	);

	float aspect = static_cast<float>(fboWidth) / static_cast<float>(fboHeight);

	glm::mat4 projection = glm::perspective(
		glm::radians(camera.Zoom), 
		aspect, 
		0.1f, 
		100.0f
	);

	glm::mat4 view = camera.getViewMatrix();

	shader.setMat4("projection", projection);
	shader.setMat4("view", view);

	glActiveTexture(GL_TEXTURE0);
	shader.setInt("texture_diffuse1", 0);
	
	drawRoom(shader, roomModel);
	drawProps(shader, room, time);
	drawPlayer(shader, playerModel, player);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}