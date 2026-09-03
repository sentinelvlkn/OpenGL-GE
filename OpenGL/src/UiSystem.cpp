#include "include/UiSystem.h"

#include <string>

#include "include/DebugLog.h"
#include "include/FileSystem.h"

bool initializeUi(UiState& uiState) {

	std::string fontPath =
		FileSystem::pathFromRoot("src/ui/fonts/LektonNerdFontMono-Regular.ttf");

	bool loaded = uiState.textRenderer.loadFont(fontPath, 24);

	if (!loaded) {
		DebugLog::log("[TEXT ERROR] Failed to load the font: " + fontPath);
		return false;
	}
	
	DebugLog::log("[BOOT] UI initialized");

	return true;
}