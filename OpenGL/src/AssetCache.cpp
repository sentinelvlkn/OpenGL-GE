#include "include/AssetCache.h"

#include <memory>
#include <unordered_map>

#include "include/FileSystem.h"
#include "include/DebugLog.h"

static std::unordered_map<std::string, std::unique_ptr<Model>> roomModelCache;
static std::unordered_map<std::string, std::unique_ptr<Model>> modelCache;

Model &getRoomModel(const Room &room) {
	std::string path = FileSystem::pathFromRoot(room.modelPath);

	auto it = roomModelCache.find(path);
	if (it == roomModelCache.end()) {
		roomModelCache[path] = std::make_unique<Model>(path.c_str());
		return *roomModelCache[path];
	}

	return *it->second;
}

Model &getCachedModel(const std::string &modelPath) {

	if (modelPath.empty()) {
		throw std::runtime_error(
			"[CACHE MODEL ERROR] getCachedModel called with empty modelPath");
	}

	std::string path = FileSystem::pathFromRoot(modelPath);

	auto it = modelCache.find(path);

	if (it == modelCache.end()) {
		DebugLog::log("[MODEL CACHE] Loading: " + path);

		auto model = std::make_unique<Model>(path.c_str());
		auto result = modelCache.emplace(path, std::move(model));

		it = result.first;
	}

	if (!it->second) {
		throw std::runtime_error("[CACHE MODEL ERROR] Cached model is null: " +
								 path);
	}

	return *it->second;
}