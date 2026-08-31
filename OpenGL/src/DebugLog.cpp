#include "include/DebugLog.h"
#include "include/Player.h"
#include "include/Room.h"
#include <iostream>
#include <sstream>

namespace {
	std::vector<std::string> gLines;
	const int MAX_LINES = 200;
} 

namespace DebugLog {

void log(const std::string &text) {

	gLines.push_back(text);

	if ((int)gLines.size() > MAX_LINES)
		gLines.erase(gLines.begin());

	std::cout << text << "\n";
}

void log(const char *text) { log(std::string(text)); }

const std::vector<std::string> &getLines() { return gLines; }

void clear() { gLines.clear(); }

void F2DebugPrint(const Room& testRoom, const Player& player) {
	std::ostringstream ss;

	ss << "Room: " << testRoom.name << " | Player: " << player.position.x
	   << ", " << player.position.y << ", " << player.position.z
	   << " | Camera: " << testRoom.activeCamera
	   << " | Props: " << testRoom.props.size()
	   << " | Lights: " << testRoom.lights.size();

	DebugLog::log(ss.str());
}

} // namespace DebugLog
