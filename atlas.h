/* date = May 18th 2021 5:31 pm */

#ifndef ATLAS_H
#define ATLAS_H

#define STBI_NO_JPEG
//#define STBI_NO_PNG
#define STBI_NO_BMP
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM 

#define COMPONENTS_PER_PIXEL 4
#define VERSION 0.1

#include "include/types.h"

struct image_file_buffer
{
    // NOTE: Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
    unsigned char *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

enum format_enum 
{
    PNG,
    BMP,
    TGA,
    //GIF,
    HDR,
    //PIC,
    //PNM,
    INVALID_FORMAT
};

char *Extensions[]
{
    ".png",
    ".bmp",
    ".tga",
    ".hdr",
};

struct image
{
    unsigned char *Buffer;
    int Width;
    int Height;
    int ComponentsPerPixel;
    char Filename[256];
};

struct cmdline_arguments
{
    bool32 NoOp;
    bool32 Help;
    bool32 FilePathSet;
    int UserWidth;
    int UserHeight;
    char FilePath[256];
    char DesireFileName[256];
    enum format_enum Format = INVALID_FORMAT;
};


#endif //ATLAS_H
