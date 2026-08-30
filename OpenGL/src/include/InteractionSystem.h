#pragma once

#include <string>

#include <glm/glm.hpp>

#include "Interaction.h"
#include "Room.h"

struct NearbyAction {
	const DoorTrigger *door = nullptr;
	Interaction *interaction = nullptr;
	std::string promptText;

	bool hasAction() const { return door != nullptr || interaction != nullptr; }
};

std::string getInteractionPromptText(const Interaction &interaction);
const DoorTrigger *getDoorInRange(const Room &room, const glm::vec3 &playerPos);
NearbyAction getNearbyAction(Room &room, const glm::vec3 &playerPos);
Interaction *getInteractionInRange(Room &room, const glm::vec3 &playerPos);