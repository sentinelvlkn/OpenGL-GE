#include "include/DebugLog.h"

#include <iostream>

namespace {
	std::vector<std::string> gLines;
	const int MAX_LINES = 200;
} 

namespace DebugLog {

void log(const std::string& text) {
	
	gLines.push_back(text);

	if ((int)gLines.size() > MAX_LINES)
		gLines.erase(gLines.begin());

	std::cout << text << "\n";
}

void log(const char *text) { 
	log(std::string(text)); 
}

const std::vector<std::string>& getLines() {
	return gLines;
}

void clear() {
	gLines.clear();
}

}
