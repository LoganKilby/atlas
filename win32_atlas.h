/* date = March 16th 2021 6:37 pm */

#ifndef WIN32_HANDMADE_H
#define WIN32_HANDMADE_H

struct game_memory
{
    bool32 IsInitialized;
    int64 PermanentStorageSize;
    void *PermanentStorage;
};

struct win32_offscreen_buffer
{
    // NOTE(casey): Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
    BITMAPINFO Info; 
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};

struct win32_window_dimension
{
    int Width;
    int Height;
};

struct game_input
{
    uint8 Up;
    uint8 Down;
    uint8 Left;
    uint8 Right;
    uint8 Shoot;
};

#endif //WIN32_HANDMADE_H
