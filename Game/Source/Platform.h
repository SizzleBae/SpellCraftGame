#pragma once


#define DEBUG_LOG(name) void name(const char *Message)
typedef DEBUG_LOG(debug_log);

struct game_memory {
	debug_log* DebugLog;
};


#define GAME_UPDATE_AND_RENDER(name) void name(game_memory *GameMemory)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);