#if !defined(GENERAL_H)
#include "general.h"
#define GENERAL_H
#endif
#if !defined(HANDMADE_H)

//timing, Controller/Keyboard, Bitmap buffer to use, sound buffer to use

struct game_offscreen_buffer{
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};

internal void GameUpdateAndRender(game_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset);

#define HANDMADE_H
#endif