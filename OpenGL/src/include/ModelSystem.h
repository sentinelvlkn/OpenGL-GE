#pragma once

#include "Model.h"

struct ModelState {
	Model playerModel;

	ModelState();
};

void initializeModels(ModelState &modelState);