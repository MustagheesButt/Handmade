#include "handmade.h"

internal void GameSoundOutput(sound_output_buffer *SoundBuffer, int ToneHz)
{
    local_persist real32 tSine;
    int16 ToneVolume = 3000;
    int WavePeriod = SoundBuffer->SamplesPerSecond/ToneHz;

    int16 *SampleOut = SoundBuffer->Samples;
    for (int SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; ++SampleIndex)
    {
        real32 SineValue = sinf(tSine);
        int16 SampleValue = (int16)(SineValue * ToneVolume);
        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;

        tSine += 2.0f * PI32 * 1.0f/(real32)WavePeriod;
    }
}

internal void RenderWeirdGradiant(game_offscreen_buffer *Buffer, int XOffset, int YOffset)
{
    // TODO: What optimizer does with Buffer by pointer vs by value
    uint8 *Row = (uint8 *)Buffer->Memory;
    for (int Y = 0; Y < Buffer->Height; Y++)
    {
        uint32 *Pixel = (uint32 *)Row;
        for (int X = 0; X < Buffer->Width; X++)
        {
            uint8 Blue = (X + XOffset);
            uint8 Green = (Y + YOffset);

            *Pixel++ = (Green << 8 | Blue);
        }
        Row += Buffer->Pitch;
    }
}

internal void GameUpdateAndRender(game_offscreen_buffer *Buffer, sound_output_buffer *SoundBuffer, int ToneHz)
{
    GameSoundOutput(SoundBuffer, ToneHz);
    RenderWeirdGradiant(Buffer, 0, 0);
}