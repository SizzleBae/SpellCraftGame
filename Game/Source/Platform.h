#pragma once

#include <stdint.h>
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef size_t memory_index;

typedef float real32;
typedef double real64;

#define internal static
#define local_persist static
#define global_variable static

#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}

constexpr float Pi32 = 3.14159265359f;

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#include <string>

// LOGGING
struct log_category
{
	const char* Name;
};

struct log_verbosity
{
	const char* Name;
	uint32 Level;
};

global_variable log_verbosity Verbose{ .Name = "All", .Level = 0 };
global_variable log_verbosity Warning{ .Name = "Warning", .Level = 1 };
global_variable log_verbosity Error{ .Name = "Error", .Level = 2 };

#define LOG(name) void name(const log_category &Category, const log_verbosity &Verbosity, const std::string &Message)
typedef LOG(engine_log);

struct screen_buffer {
	uint32 *XRGB;
	int32 Width;
	int32 Height;
};

struct game_input {
	real32 Up;
	real32 Down;
	real32 Left;
	real32 Right;
};

struct game_memory {
	uint64 PermanentStorageSize;
	void *PermanentStorage;

	uint64 TransientStorageSize;
	void *TransientStorage; 

	engine_log *Log;
};

#define GAME_UPDATE_AND_RENDER(name) void name(game_memory *GameMemory, game_input *GameInput, screen_buffer *OutScreenBuffer)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);


struct game_sound_output_buffer
{
	int SamplesPerSecond;
	int SampleCount;
	int16* Samples;
};

#define GAME_OUTPUT_SOUND(name) void name(game_memory *GameMemory, game_sound_output_buffer *SoundBuffer)
typedef GAME_OUTPUT_SOUND(game_output_sound);