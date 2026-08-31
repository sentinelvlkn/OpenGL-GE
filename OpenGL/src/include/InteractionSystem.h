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

struct InteractionRuntimeState {
	bool messageOpen = false;
	std::string activeMessage;

	bool noteOpen = false;
	std::string activeNoteImagePath;
	unsigned int activeNoteTexture = 0;
};

std::string getInteractionPromptText(const Interaction &interaction);

const DoorTrigger *getDoorInRange(const Room &room, const glm::vec3 &playerPos);

NearbyAction getNearbyAction(Room &room, const glm::vec3 &playerPos);

Interaction *getInteractionInRange(Room &room, const glm::vec3 &playerPos);

void runInteraction(Interaction &interaction, InteractionRuntimeState& interactionState);

void closeActiveInteraction(InteractionRuntimeState &interactionState);