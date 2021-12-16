#include "SpellCraftGame.h"

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
	GameMemory->DebugLog("TestMessage");

	GameMemory->TestChar = 'C';
}
