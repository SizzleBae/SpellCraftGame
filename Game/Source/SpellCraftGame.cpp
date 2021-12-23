#include "SpellCraftGame.h"

void FillRectangle(screen_buffer* ScreenBuffer, int32 MinX, int32 MinY, int32 MaxX, int32 MaxY, uint32 RGB) 
{
	for (int32 Y = MinY; Y < MaxY; Y++) 
	{
		for (int32 X = MinX; X < MaxX; X++) 
		{
			if (X >= 0 && X < ScreenBuffer->Width && 
				Y >= 0 && Y < ScreenBuffer->Height) 
			{
				ScreenBuffer->XRGB[Y * ScreenBuffer->Width + X] = RGB;
			}
		}
	}
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
	Assert(sizeof(game_state) <= GameMemory->PermanentStorageSize);

	game_state *GameState = static_cast<game_state*>(GameMemory->PermanentStorage);

	GameState->TestChar = 'C';

	GameState->X += GameInput->Right - GameInput->Left;
	GameState->Y += GameInput->Down - GameInput->Up;

	FillRectangle(OutScreenBuffer, -10, -10, 500, 500, 0xFF0000);
	FillRectangle(OutScreenBuffer, 100, 100, 500, 150, 0x00FF00);
	FillRectangle(OutScreenBuffer, 250, 270, 500, 500, 0x0000FF);

	int32 PlayerHalfSize = 10;
	FillRectangle(OutScreenBuffer, 
		int32(GameState->X)-PlayerHalfSize, int32(GameState->Y)-PlayerHalfSize,
		int32(GameState->X)+PlayerHalfSize, int32(GameState->Y)+PlayerHalfSize, 0xFF00FF);
}

extern "C" GAME_OUTPUT_SOUND(GameOutputSound)
{
	GameMemory->DebugLog("HELLO FROM GAME SOUND!");
}