#pragma once

#include "Platform.h"

struct game_state {
	uint8 TestChar;

	real32 X, Y;
};

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender);