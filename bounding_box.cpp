// https://arxiv.org/ftp/arxiv/papers/1402/1402.0557.pdf

// 1. Generate a set of widths starting with the width of the widest rectangle up to the
// width of the greedy solution (Width of all the rectangles combined)
// 2. For each width, a height is initialized to the maximum of the following lower bounds:
// The height must be at least the height of the tallest rectangle in the instance. Third,
// for every pair of rectangles, if the sum of their widths exceed the width of the bounding box,
// then the bounding box height must be at least the sum of their heights, since they can't
// appear side-by-side, but one must be on top of the other. Fourth, the set of rectangles whose
// widths are greater than half the with of the bounding box must all be stacked vertically,
// including the rectangle of smallest height whose width is exactly half the width of the
// bounding box. Finally if certain properties exist for a given rectangle packing instance, we
// force the height to be greater than or equal to the width to break symmetry. For example,
// one sufficient property is having an instance consisting of just squares, since a solution in
// a W * H bounding box easily transforms into another one in a H * W bounding box. Another
// sufficient property is when every rectangle of dimensions w * h can correspond to another one
// of dimensions h * w.
// 3. These bounding boxes are used to initialize a min-heap sorted in non-decreasing
// order of area.
// 4. The containment solver is called (stb_rect_pack). If the box is infeasible then
// we increase the height of the box by one, and insert the new box back into the min-heap.

struct bounding_box
{
    int32 Width;
    int32 Height;
    bool32 IsValid;
    bool32 IsSolved;
};

internal bounding_box *
CreateBoundingBoxes(int *BoxCountResult, stbrp_rect *Rects, int RectCount, int GreedyWidth)
{
    if(RectCount == 1)
    {
        bounding_box *Result = (bounding_box *)malloc(sizeof(bounding_box));
        Result->Width = Rects[0].w;
        Result->Height = Rects[0].h;
        return Result;
    }
    else if(RectCount <= 0)
    {
        return 0;
    }
    
    int TempRectCount = RectCount - 1;
    int DimCount = 0;
    while(TempRectCount)
    {
        DimCount += TempRectCount--;
    }
    
    bounding_box *CombinedDimensions = (bounding_box *)malloc(sizeof(bounding_box) * DimCount);
    
    int MinimumWidth = 0; // Will be equal to the w of the widest rectangle
    int MinimumHeight = 0; // Will be equal to the h of the tallest rectangle
    int CombinedDimIndex = 0;
    bounding_box TempDimensions = {};
    for(int RectIndex = 0; RectIndex < RectCount; ++RectIndex)
    {
        // Finding the minimum width and height of the given rectangles
        if(Rects[RectIndex].w > MinimumWidth)
        {
            MinimumWidth = Rects[RectIndex].w;
        }
        
        if(Rects[RectIndex].h > MinimumHeight)
        {
            MinimumHeight = Rects[RectIndex].h;
        }
        
        // Calculating the combined area of every unique pair of rectangles
        for(int i = RectIndex + 1; i < RectCount; ++i)
        {
            TempDimensions.Width = Rects[RectIndex].w + Rects[i].w;
            TempDimensions.Height = Rects[RectIndex].h + Rects[i].h;
            CombinedDimensions[CombinedDimIndex++] = TempDimensions;
        }
    }
    
    int BoxCount = GreedyWidth - MinimumWidth;
    bounding_box *BoundingBoxes = (bounding_box *)malloc(sizeof(bounding_box) * BoxCount);
    
    for(int BoxIndex = 0; BoxIndex < BoxCount; ++BoxIndex)
    {
        BoundingBoxes[BoxIndex].Height = MinimumHeight;
        BoundingBoxes[BoxIndex].Width = MinimumWidth++;
        BoundingBoxes[BoxIndex].IsValid = 1;
        BoundingBoxes[BoxIndex].IsSolved = 0;
        
        for(int DimIndex = 0; DimIndex < DimCount; ++DimIndex)
        {
            if(BoundingBoxes[BoxIndex].Width < CombinedDimensions[DimIndex].Width &&
               BoundingBoxes[BoxIndex].Height < CombinedDimensions[DimIndex].Height)
            {
                BoundingBoxes[BoxIndex].IsValid = 0;
                break;
            }
        }
        
        // Check if the bounding box has enough height to store rectangles
        // that need to be stacked vertically
        int HeightRequired = 0;
        for(int RectIndex = 0; RectIndex < RectCount; RectIndex++)
        {
            if(Rects[RectIndex].w > (BoundingBoxes[BoxIndex].Width / 2))
            {
                HeightRequired += Rects[RectIndex].h;
            }
        }
        
        if(HeightRequired > BoundingBoxes[BoxIndex].Height)
        {
            BoundingBoxes[BoxIndex].IsValid = 0;
        }
    }
    
    free(CombinedDimensions);
    
    *BoxCountResult = BoxCount;
    return BoundingBoxes;
}
