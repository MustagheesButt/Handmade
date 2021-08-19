#if !defined(HANDMADE_H)

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

internal void GameUpdateAndRender(game_offscreen_buffer *Buffer, sound_output_buffer *SoundBuffer, int ToneHz);

#define HANDMADE_H
#endif