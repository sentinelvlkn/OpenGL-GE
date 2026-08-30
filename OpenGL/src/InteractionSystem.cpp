#include "include/InteractionSystem.h"

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