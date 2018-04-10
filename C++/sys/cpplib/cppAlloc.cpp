///////////////////////////////////////////////////////////////////////////////
// Copyright (C) Mark Roddy
//
//  Hollis Technology Solutions
//  94 Dow Road
//  Hollis, NH 03049
//  info@hollistech.com
//  www.hollistech.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
//
//
//  Synopsis: 
// 
///////////////////////////////////////////////////////////////////////////////
#define HTS_UNIQUE_FILE_ID 0x1204000

#include "hstcpp_internal.h"


PVOID __cdecl malloc(ULONG x,  ULONG id, POOL_TYPE pool)
{
    PVOID buffer = ExAllocatePoolWithTag(pool, x, id);

    if (buffer) {
        //
        // clear our buffer to zero
        //
        RtlZeroMemory(buffer, x);
    }

    return buffer;
}


void __cdecl free(PVOID x)
{
    if (x != NULL) {

        ExFreePool(x);
    }
}


void * __cdecl operator new(size_t size)
{
    return malloc(size, 'ppcO', NonPagedPool);
}


void * __cdecl operator new(size_t size,  void *location)
{
    return location;
}

void *__cdecl operator new( size_t size, ULONG tag, POOL_TYPE pool)
{
    return malloc(size, tag, pool);
}

void __cdecl operator delete(void * pVoid)
{
    free(pVoid);
}

