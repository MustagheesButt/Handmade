#include "handmade.h"

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

internal void GameUpdateAndRender(game_offscreen_buffer *Buffer)
{
    RenderWeirdGradiant(Buffer, 0, 0);
}