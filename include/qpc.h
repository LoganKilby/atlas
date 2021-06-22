/* date = May 22nd 2021 9:47 pm */

#ifndef QPC_H
#define QPC_H

#include "profileapi.h"

LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
LARGE_INTEGER Frequency;
int CounterStarted = 0;

void StartCounter(void)
{
    QueryPerformanceFrequency(&Frequency);
    QueryPerformanceCounter(&StartingTime);
    CounterStarted = 1;
}

long long EndCounter(long long *Dest)
{
    long long Result;
    
    if(CounterStarted)
    {
        QueryPerformanceCounter(&EndingTime);
        ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
        
        Result = (ElapsedMicroseconds.QuadPart * 1000000) / Frequency.QuadPart;
        
        if(Dest)
        {
            *Dest = Result;
        }
        
        CounterStarted = 0;
    }
    else
    {
        Result = 0;
    }
    
    return Result;
}

#endif //QPC_H
