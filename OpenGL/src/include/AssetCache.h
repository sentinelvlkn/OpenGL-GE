#pragma once

#include <string>

#include "Model.h"
#include "Room.h"

Model &getRoomModel(const Room &room);
Model &getCachedModel(const std::string &modelPath);