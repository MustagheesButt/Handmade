#if !defined(HANDMADE_H)

struct game_offscreen_buffer
{
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

internal void GameUpdateAndRender(game_offscreen_buffer *Buffer);

#define HANDMADE_H
#endif