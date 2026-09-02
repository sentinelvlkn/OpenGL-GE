#pragma once

#include <string>
#include <vector>

struct Room;
struct Player;

namespace DebugLog {
	void log(const std::string &text);
	void log(const char *text);

	const std::vector<std::string> &getLines();

	void clear();

} 

