#include "handmade.h"

internal void RenderWeirdGradient(game_offscreen_buffer *Buffer, int XOffset, int YOffset) {
    uint8 *Row = (uint8 *)Buffer->Memory;
    
    for(int Y=0; Y < Buffer->Height; ++Y){
        uint32 *Pixel = (uint32 *)Row;
        
        for(int X = 0; X < Buffer->Width; ++X) {
            int Blue = (X + XOffset);
            int Green = (Y + YOffset);
            int Red = (X + YOffset);

            uint8 FinalBlue = WrapColor(Blue);
            uint8 FinalGreen = WrapColor(Green);
            uint8 FinalRed = WrapColor(Red);

            *Pixel++ = (FinalRed << 16) | (FinalGreen << 8) | FinalBlue;
        }

        Row += Buffer->Pitch;
    }
}

internal void GameUpdateAndRender(game_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset){
    RenderWeirdGradient(Buffer, BlueOffset, GreenOffset);
}