#include "include/ModelSystem.h"


#include "include/DebugLog.h"
#include "include/FileSystem.h"

ModelState::ModelState() :
	playerModel(
		FileSystem::pathFromRoot("src/models/test_character/test_character.obj").c_str()
	)
{}



void initializeModels(ModelState& modelState) {
	DebugLog::log("[BOOT] Models initialized.");
}