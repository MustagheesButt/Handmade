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

        tSine += (real32)(2.0f * PI32 * 1.0f/(real32)WavePeriod);
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
            uint8 Blue = (uint8)(X + XOffset);
            uint8 Green = (uint8)(Y + YOffset);

            *Pixel++ = (Green << 8 | Blue);
        }
        Row += Buffer->Pitch;
    }
}

internal void GameUpdateAndRender(game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer, sound_output_buffer *SoundBuffer)
{
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);

    game_state *State = (game_state *)Memory->PermanentStorage;

    if (!Memory->IsInitialized)
    {
        char *Filename = __FILE__;

        debug_read_file_result File = DEBUGPlatformReadEntireFile(Filename);
        if (File.Contents)
        {
            DEBUGPlatformWriteEntireFile("h:/out.txt", File.ContentsSize, File.Contents);
            DEBUGPlatformFreeFileMemory(File.Contents);
        }

        State->ToneHz = 256;
        Memory->IsInitialized = true;
    }

    for (int ControllerIndex = 0; ControllerIndex < ArrayCount(Input->Controllers); ++ControllerIndex)
    {
        game_controller_input *Input0 = GetController(Input, ControllerIndex);
        if (Input0->IsAnalog)
        {
            // Use analog movement tuning
            State->ToneHz = 256 + (int)(128.0f * Input0->StickAverageX);
            State->GreenOffset += (int)(4.0f * Input0->StickAverageY);
        }
        else
        {
            // Use digital movement tuning
        }

        if (Input0->ActionDown.EndedDown)
        {
            State->GreenOffset += 1;
        }
        if (Input0->ActionUp.EndedDown) State->GreenOffset -= 1;
    }

    GameSoundOutput(SoundBuffer, State->ToneHz);
    RenderWeirdGradiant(Buffer, State->BlueOffset, State->GreenOffset);
}