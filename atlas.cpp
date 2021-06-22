#include "atlas.h"

#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include "include/stb_rect_pack.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "include/stb_image_write.h"

#define CUTE_FILES_IMPLEMENTATION
#include "include/cute_files.h"

#include "include/types.h"
#include "include/qpc.h"
#include "string.h"

#include "bounding_box.cpp"

// TODO: Implement more image formats

global_variable int GreedyWidth = 0;
global_variable int GreedyHeight = 0;

internal bool
ValidateExtension(format_enum Format, char FileExtension[32])
{
    for(int i = 0; i < ArrayCount(Extensions); ++i)
    {
        if(strcmp(FileExtension, Extensions[i]) == 0)
        {
            format_enum Result = (format_enum)i;
            
            return Format == Result;
        }
    }
    
    return false;
}

internal void
PrintHelpInfo(cmdline_arguments Arguments)
{
    if(Arguments.NoOp)
    {
        printf("atlas v%.2f\n", VERSION);
        printf("To use atlas:\n");
        printf("\t 1. Add the atlas executable to your PATH.\n");
        printf("\t 2. Navigate to the directory where your images are stored and call atlas.\n");
        printf("\t 3. Specify a format. Call atlas followed by 'help' to list supported formats along with other useful information.\n");
        printf("\t Example: \"atlas .png\"\n");
    }
    else if(Arguments.Help)
    {
        printf("Supported formats:\n");
        for(int i = 0; i < ArrayCount(Extensions); ++i)
        {
            printf("\t%s\n", Extensions[i]);
        }
        
        printf("\n");
        printf("Usage: atlas [file format]\n");
        printf("Example: \"atlas .png\"\n");
        printf("NOTE: atlas will only use images in current directory with the file extension you specified\n");
    }
}

internal void
CopyImageToBuffer(image Image, image_file_buffer *ImageBuffer, int XOffset, int YOffset, int ComponentsPerPixel)
{
    Assert((Image.Height <= ImageBuffer->Height) && (Image.Width <= ImageBuffer->Width));
    
    // NOTE: 32-bit Pixels: BB GG RR XX
    uint8 *Row = (uint8 *)ImageBuffer->Memory + (ImageBuffer->Pitch * YOffset);
    for(int Y = 0; Y < Image.Height; ++Y)
    {
        uint8 *PixelComponent = Row + (XOffset*ComponentsPerPixel);
        for(int X = 0; X < Image.Width; ++X)
        {
            for(int Comp = 0; Comp < ComponentsPerPixel; ++Comp)
            {
                *PixelComponent++ = *Image.Buffer++;
            }
        }
        Row += ImageBuffer->Pitch;
    }
    
    return;
}

internal int
WriteAtlasToFile(image_file_buffer *AtlasBuffer, format_enum Format)
{
    char Filename[10] = "atlas";
    strcat(Filename, Extensions[Format]);
    
    int WriteSuccess = 0;
    switch(Format)
    {
        case PNG:
        {
            // NOTE: Default png compression level is set to 8
            WriteSuccess = 
                stbi_write_png(Filename, AtlasBuffer->Width, AtlasBuffer->Height,
                               4, AtlasBuffer->Memory, AtlasBuffer->Width * AtlasBuffer->BytesPerPixel);
        } break;
#if 0
        case JPG:
        {
            // stb_image_write.h line 109: 
            // JPEG does ignore alpha channels in input data; quality is between 1 and 100.
            // Higher quality looks better but results in a bigger image.
            int Quality = 50; // TODO: I should let the user choose the quality maybe?
            
            WriteSuccess = stbi_write_jpg(Filename, AtlasBuffer->Width, AtlasBuffer->Height,
                                          4, AtlasBuffer->Memory);
        } break;
        case BMP:
        {
            WriteSuccess = stbi_write_bmp(char const *filename, int w, int h, int comp, const void *data);
        } break;
        case TGA:
        {
            WriteSuccess = stbi_write_tga(char const *filename, int w, int h, int comp, const void *data);
        } break;
        case HDR:
        {
            WriteSuccess = stbi_write_hdr(char const *filename, int w, int h, int comp, const float *data);
        } break;
#endif
    }
    
    return WriteSuccess;
}

internal format_enum
CheckFormatExtension(char *FileExtension)
{
    for(int i = 0; i < ArrayCount(Extensions); ++i)
    {
        if(strcmp(FileExtension, Extensions[i]) == 0)
        {
            return (format_enum)i;
        }
    }
    
    return INVALID_FORMAT;
}

internal void
ParseCommandLine(int argc, char *argv[], cmdline_arguments *Arguments)
{
    if(argc > 1)
    {
        for(int ArgIndex = 1; ArgIndex < argc; ++ArgIndex)
        {
            format_enum Extension = CheckFormatExtension(argv[ArgIndex]);
            if(Extension != INVALID_FORMAT)
            {
                Arguments->Format = Extension;
            }
            else if(strcmp(argv[ArgIndex], "help") == 0)
            {
                Arguments->Help = 1;
            }
            else
            {
                strcpy(Arguments->FilePath, argv[ArgIndex]);
                Arguments->FilePathSet = 1;
            }
        }
    }
    else
    {
        Arguments->NoOp = 1;
    }
}

int GetFileCountWorkingDir()
{
    cf_dir_t dir;
    cf_dir_open(&dir, ".");
    
    int Result = 0;
    while(dir.has_next)
    {
        cf_file_t file;
        cf_read_file(&dir, &file);
        
        if(!file.is_dir)
        {
            ++Result;
        }
        
        cf_dir_next(&dir);
    }
    
    cf_dir_close(&dir);
    return Result;
}

internal int
LoadImages(image **Images, cmdline_arguments Arguments)
{
    // TODO: Support more image formats
    
    int LowestComponent = 0;
    int ImageCount = 0;
    int ImageCapacity = 30;
    *Images = (image *)malloc(sizeof(image) * ImageCapacity);
    
    char *DirectoryPath = Arguments.FilePathSet ? Arguments.FilePath : ".";
    
    cf_dir_t Dir;
    cf_dir_open(&Dir, DirectoryPath);
    
    while(Dir.has_next)
    {
        cf_file_t File;
        cf_read_file(&Dir, &File);
        
        if(File.is_dir)
        {
            cf_dir_next(&Dir);
        }
        else
        {
            if(ValidateExtension(Arguments.Format, File.ext))
            {
                image Image = {};
                Image.Buffer = stbi_load(File.name, 
                                         &Image.Width, 
                                         &Image.Height, 
                                         &Image.ComponentsPerPixel,
                                         COMPONENTS_PER_PIXEL);
                Image.ComponentsPerPixel = COMPONENTS_PER_PIXEL;
                
                if(Image.Buffer)
                {
                    printf("%s loaded.\n", File.name, Image.ComponentsPerPixel);
                    
                    if(ImageCount == ImageCapacity)
                    {
                        ImageCapacity *= 2;
                        *Images = (image *)realloc(*Images, sizeof(image) * ImageCapacity);
                    }
                    
                    (*Images)[ImageCount].Buffer = Image.Buffer;
                    (*Images)[ImageCount].Width = Image.Width;
                    (*Images)[ImageCount].Height = Image.Height;
                    (*Images)[ImageCount].ComponentsPerPixel = Image.ComponentsPerPixel;
                    strcpy((*Images)[ImageCount].Filename, File.name);
                    ImageCount++;
                    
                    GreedyWidth += Image.Width;
                    GreedyHeight += Image.Height;
                }
            }
            
            cf_dir_next(&Dir);
        }
    }
    
    cf_dir_close(&Dir);
    return ImageCount;
}

internal void
InitializeBuffer(image_file_buffer *Buffer, int Width, int Height)
{
    if(Buffer->Memory)
    {
        free(Buffer->Memory);
    }
    
    Buffer->Width = Width;
    Buffer->Height = Height;
    
    int BytesPerPixel = 4;
    Buffer->BytesPerPixel = BytesPerPixel;
    
    int BitmapMemorySize = (Buffer->Width*Buffer->Height)*BytesPerPixel;
    Buffer->Memory = (unsigned char *)malloc(BitmapMemorySize);
    memset(Buffer->Memory, 0, BitmapMemorySize);
    Buffer->Pitch = Width*BytesPerPixel;
}

internal int
DebugLoadWritePng(const char *Filename)
{
    image Image;
    Image.Buffer = stbi_load(Filename, 
                             &Image.Width, 
                             &Image.Height, 
                             &Image.ComponentsPerPixel,
                             0);
    
    char Prefix[256] = "debug_";
    strcat(Prefix, Filename);
    
    int WriteSuccess = stbi_write_png(Prefix, Image.Width, Image.Height,
                                      Image.ComponentsPerPixel, Image.Buffer, Image.Width * Image.ComponentsPerPixel);
    return WriteSuccess;
}


int main(int argc, char *argv[])
{
    StartCounter();
    
    cmdline_arguments Arguments = {};
    ParseCommandLine(argc, argv, &Arguments);
    if(Arguments.NoOp || Arguments.Help)
    {
        PrintHelpInfo(Arguments);
        return 0;
    }
    
    image *Images = {};
    int ImageCount = 0;
    ImageCount = LoadImages(&Images, Arguments);
    
    if(ImageCount)
    {
        printf("%u images loaded.\n", ImageCount);
        int RectCount = ImageCount;
        stbrp_rect *Rects = (stbrp_rect *)malloc(sizeof(stbrp_rect) * RectCount);
        memset(Rects, 0, sizeof(stbrp_rect) * RectCount);
        
        for(int RectIndex = 0; RectIndex < ImageCount; ++RectIndex)
        {
            Rects[RectIndex].id = RectIndex;
            Rects[RectIndex].w = Images[RectIndex].Width;
            Rects[RectIndex].h = Images[RectIndex].Height;
        }
        
        StartCounter(); // Bounding Boxes
        int BoxCount;
        bounding_box *BoundingBoxes = CreateBoundingBoxes(&BoxCount, Rects, RectCount, GreedyWidth);
        printf("Bounding boxes generated. %.4lf seconds elapsed.\n", EndCounter(0) / 1000000.0l);
        
        if(BoundingBoxes)
        {
            int SolutionIndex = 0;
            int SolutionsFound = 0;
            int SolutionCount = 0;
            int NodeCount;
            stbrp_node* Nodes = {};
            stbrp_context Context;
            
            // TODO: This is embarrassing and needs to be re-written
            while(!SolutionsFound)
            {
                int StillSearching = 0;
                for(int BoxIndex = 0; BoxIndex < BoxCount; ++BoxIndex)
                {
                    if(BoundingBoxes[BoxIndex].IsValid && !BoundingBoxes[BoxIndex].IsSolved)
                    {
                        Context = {};
                        NodeCount = BoundingBoxes[BoxIndex].Width; // Line 136 stb_rect_pack.h
                        Nodes = (stbrp_node *)malloc(sizeof(stbrp_node) * NodeCount);
                        stbrp_init_target(&Context, BoundingBoxes[BoxIndex].Width,
                                          BoundingBoxes[BoxIndex].Height,  Nodes, NodeCount);
                        if(stbrp_pack_rects(&Context, Rects, RectCount))
                        {
                            BoundingBoxes[BoxIndex].IsSolved = 1;
                            SolutionCount++;
                        }
                        else
                        {
                            StillSearching = 1;
                            BoundingBoxes[BoxIndex].Height++;
                        }
                        free(Nodes);
                    }
                }
                
                if(!StillSearching)
                {
                    SolutionsFound = 1;
                }
            }
            
            // NOTE: THIS COULD BE AVOIDED IF I KEPT THE ARRAY OF BOXES SORTED
            int LeastArea = GreedyWidth * GreedyHeight;
            int ComputedArea = 0;
            int LeastAreaIndex = 0;
            
            int ComputedAreaAndDelta = LeastArea;
            int SmallestSquareIndex = -1;
            
            for(int BoxIndex = 0; BoxIndex < BoxCount; ++BoxIndex)
            {
                if(BoundingBoxes[BoxIndex].IsSolved)
                {
                    // NOTE: Trying out two different approaches right now.
                    // 1. The abosulte minimum area bounding box can be calculated easily, but the
                    // dimensions of this box are often very rectangular.
                    // 2. I'm preserving the information about the dimensions of the box by multiplying
                    // the area of the box by the difference between the width and height. As a result
                    // the most square-like box with the minimum area is found and located at
                    // the SmallestSquareIndex 
                    
                    ComputedArea = BoundingBoxes[BoxIndex].Width * BoundingBoxes[BoxIndex].Height;
                    if(ComputedArea < LeastArea)
                    {
                        LeastArea = ComputedArea;
                        LeastAreaIndex = BoxIndex;
                    }
                    
                    int MultipliedDelta = ComputedArea * abs(BoundingBoxes[BoxIndex].Width - BoundingBoxes[BoxIndex].Height);
                    if(SmallestSquareIndex >= 0)
                    {
                        if(MultipliedDelta < ComputedAreaAndDelta)
                        {
                            ComputedAreaAndDelta = MultipliedDelta;
                            SmallestSquareIndex= BoxIndex;
                        }
                    }
                    else
                    {
                        ComputedAreaAndDelta = MultipliedDelta;
                        SmallestSquareIndex= BoxIndex;
                    }
                }
            }
            
#if 1 // If minimum area rectangle, or minimum area rectangle of most similar w and h
            LeastAreaIndex = SmallestSquareIndex;
#endif
            
            // It would make more sense to add a stbrp_context to each bounding box
            // instead of packing the rects again!
            stbrp_context TestContext = {};
            int TestNodeCount = BoundingBoxes[LeastAreaIndex].Width; // Line 136 stb_rect_pack.h
            stbrp_node* TestNodes = (stbrp_node *)malloc(sizeof(stbrp_node) * TestNodeCount);
            stbrp_init_target(&TestContext, BoundingBoxes[LeastAreaIndex].Width,
                              BoundingBoxes[LeastAreaIndex].Height, TestNodes, TestNodeCount);
            Assert(stbrp_pack_rects(&TestContext, Rects, RectCount));
            
            image_file_buffer AtlasBuffer = {};
            InitializeBuffer(&AtlasBuffer, BoundingBoxes[LeastAreaIndex].Width, BoundingBoxes[LeastAreaIndex].Height);
            
            printf("Writing images...\n");
            for(int RectIndex = 0; RectIndex < ImageCount; ++RectIndex)
            {
                CopyImageToBuffer(Images[RectIndex], &AtlasBuffer, 
                                  (int)Rects[RectIndex].x, (int)Rects[RectIndex].y,
                                  Images[RectIndex].ComponentsPerPixel);
                
            }
            
            // NOTE: Could display buffer in a window here
            if(!WriteAtlasToFile(&AtlasBuffer, Arguments.Format))
            {
                printf("An error occured while writing image to file\n");
            }
        }
        else
        {
            // No possible bounding boxes (no images loaded successfully)
        }
    }
    else
    {
        printf("No %s images could be loaded from %s\n",
               Extensions[Arguments.Format],
               Arguments.FilePathSet ? Arguments.FilePath : argv[0]);
    }
    
    printf("Completed. %.4lf seconds elapsed.", (double)EndCounter(0) / 1000000.0l);
    return 0;
}