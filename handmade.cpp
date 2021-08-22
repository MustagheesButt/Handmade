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

internal void GameUpdateAndRender(game_input *Input, game_offscreen_buffer *Buffer, sound_output_buffer *SoundBuffer)
{
    int ToneHz = 256;
    int BlueOffset = 0;
    int GreenOffset = 0;

    game_controller_input *Input0 = &Input->Controllers[0];
    if (Input0->IsAnalog)
    {
        // Use analog movement tuning
        ToneHz = 256 + (int)(128.0f * Input0->EndX);
        BlueOffset += (int)(4.0f * Input0->EndY);
    }
    else
    {
        // Use digital movement tuning
    }

    if (Input0->Down.EndedDown)
    {
        GreenOffset += 1;
    }

    GameSoundOutput(SoundBuffer, ToneHz);
    RenderWeirdGradiant(Buffer, BlueOffset, GreenOffset);
}