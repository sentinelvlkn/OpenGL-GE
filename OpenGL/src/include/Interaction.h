#pragma once

#include <string>
#include "AABB.h"

enum class InteractionType {
	Unknown = 0,
	Message = 1,
	Note = 2,
	Pickup = 3,
	Switch = 4
};

struct Interaction {
	std::string id;

	InteractionType type = InteractionType::Unknown;

	AABB trigger;

	std::string message;
	std::string assetPath;
	std::string targetId;

	bool repeatable = true;
	bool used = false;
};

inline InteractionType interactionTypeFromString(const std::string& s){
	if (s == "message") return InteractionType::Message;
	if (s == "note") return InteractionType::Note;
	if (s == "pickup") return InteractionType::Pickup;
	if (s == "switch") return InteractionType::Switch;

	return InteractionType::Unknown;
}
