#pragma once

#include "Platform.h"

struct game_state {
	real32 X, Y;
	uint8 TestChar;
};

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender);