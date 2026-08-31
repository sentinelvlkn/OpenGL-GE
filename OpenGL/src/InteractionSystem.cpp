#include <glad/glad.h>

#include "include/DebugLog.h"
#include "include/FileSystem.h"
#include "include/InteractionSystem.h"
#include "include/TextureUtilities.h"

std::string getInteractionPromptText(const Interaction &interaction) {
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

const DoorTrigger *getDoorInRange(const Room &room,
								  const glm::vec3 &playerPos) {
	for (const auto &d : room.doors) {
		if (pointInsideAabb(playerPos, d.aabb.min, d.aabb.max))
			return &d;
	}
	return nullptr;
}

NearbyAction getNearbyAction(Room &room, const glm::vec3 &playerPos) {
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

Interaction *getInteractionInRange(Room &room, const glm::vec3 &playerPos) {
	for (auto &interaction : room.interactions) {
		if (!interaction.repeatable && interaction.used)
			continue;

		if (pointInsideAabb(playerPos, interaction.trigger))
			return &interaction;
	}

	return nullptr;
}

void closeActiveInteraction(InteractionRuntimeState& interactionState) {
	interactionState.messageOpen = false;
	interactionState.activeMessage.clear();

	interactionState.noteOpen = false;
	interactionState.activeNoteImagePath.clear();

	if (interactionState.activeNoteTexture != 0) {
		glDeleteTextures(1, &interactionState.activeNoteTexture);
		interactionState.activeNoteTexture = 0;
	}

	DebugLog::log("[MESSAGE CLOSED]");
}

void runInteraction(Interaction &interaction, InteractionRuntimeState& interactionState) {
	
	switch (interaction.type) {
	case InteractionType::Message:
		interactionState.activeMessage = interaction.message;
		interactionState.messageOpen = true;

		DebugLog::log("[MESSAGE] " + interaction.message);
		DebugLog::log("[MESSAGE] Press E to close.");

		break;

	case InteractionType::Note:
		interactionState.noteOpen = true;
		interactionState.messageOpen = true;

		interactionState.activeNoteImagePath = interaction.assetPath;

		if (interactionState.activeNoteTexture != 0) {
			glDeleteTextures(1, &interactionState.activeNoteTexture);
			interactionState.activeNoteTexture = 0;
		}

		interactionState.activeNoteTexture = loadTexture2D(
			FileSystem::pathFromRoot(interactionState.activeNoteImagePath).c_str()
		);

		DebugLog::log("[NOTE] Opened: " + interactionState.activeNoteImagePath);
		DebugLog::log("[NOTE] Press E to close.");

		break;

	case InteractionType::Pickup:
		interactionState.activeMessage = "Picked up: " + interaction.targetId;
		interactionState.messageOpen = true;

		DebugLog::log("[PICKUP] Picked up: " + interaction.targetId);
		DebugLog::log("[PICKUP] Press E to close.");

		break;

	case InteractionType::Switch:
		interactionState.activeMessage = "Triggered: " + interaction.targetId;
		interactionState.messageOpen = true;

		DebugLog::log("[SWITCH] Triggered: " + interaction.targetId);
		DebugLog::log("[SWITCH] Press E to close.");

		break;

	default:
		interactionState.activeMessage = "Unknown interaction: " + interaction.id;
		interactionState.messageOpen = true;

		DebugLog::log("[INTERACTION] Unknown interaction: " + interaction.id);
		DebugLog::log("[INTERACTION] Press E to close.");

		break;
	}

	interaction.used = true;
}