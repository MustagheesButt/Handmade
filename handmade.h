#if !defined(HANDMADE_H)

/*
 * HANDMADE_INTERNAL:
 *   0 - For public release
 *   1 - For developers only
 * 
 * HANDMADE_SLOW:
 *   0 - Slow code not allowed
 *   1 - Slow code welcome
 */

#if HANDMADE_SLOW
#define Assert(Expression) if (!(Expression)) {*(int*) 0 = 0;}
#else
#define Assert(Expression)
#endif

#define Kilobytes(Value) (Value * 1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)
#define Terabytes(Value) (Gigabytes(Value) * 1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

inline uint32 SafeTruncateUInt64(uint64 Value)
{
    Assert(Value <= 0xFFFFFFFF);
    return (uint32)Value;
}

/*
 * Services that the platform layer provides to the game
 */
#if HANDMADE_INTERNAL
struct debug_read_file_result
{
    uint32 ContentsSize;
    void *Contents;
};
internal debug_read_file_result DEBUGPlatformReadEntireFile(char *Filename);
void DEBUGPlatformFreeFileMemory(void *Memory);

bool32 DEBUGPlatformWriteEntireFile(char *Filename, uint32 MemorySize, void *Memory);
#endif

/*
 * Services that the game provides to platform layer
 */
struct game_offscreen_buffer
{
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

struct sound_output_buffer
{
    int SamplesPerSecond;
    int SampleCount;
    int16 *Samples;
};

struct game_button_state
{
    int HalfTransitionCount;
    bool32 EndedDown;
};

struct game_controller_input
{
    bool32 IsAnalog;

    real32 StartX;
    real32 StartY;

    real32 MinX;
    real32 MinY;

    real32 MaxX;
    real32 MaxY;

    real32 EndX;
    real32 EndY;
    union
    {
        game_button_state Buttons[6];
        struct
        {
            game_button_state Up;
            game_button_state Down;
            game_button_state Left;
            game_button_state Right;
            game_button_state LeftShoulder;
            game_button_state RightShoulder;
        };
    };
};

struct game_input
{
    game_controller_input Controllers[4];
};

struct game_state
{
    int ToneHz;
    int BlueOffset;
    int GreenOffset;
};

struct game_memory
{
    bool32 IsInitialized;
    uint64 PermanentStorageSize;
    void* PermanentStorage; // Required to be cleared to zero at startup
    uint64 TransientStorageSize;
    void* TransientStorage;
};

internal void GameUpdateAndRender(game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer, sound_output_buffer *SoundBuffer);

#define HANDMADE_H
#endif