///////////////////////////////////////////////////////////////////////////////
//
//// Copyright (C) Mark Roddy
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
#pragma once

#include "htscpp.h"

/*
 * pointers to initialization sections
 */
extern "C" {
    PVFV __crtXia[];
    PVFV __crtXiz[];

    PVFV __crtXca[]; // c++
    PVFV __crtXcz[];

    PVFV __crtXpa[];
    PVFV __crtXpz[];

    PVFV __crtXta[];
    PVFV __crtXtz[];
}

void HtsBugCheckEx(
    PCHAR Message, 
    ULONG FileId, 
    BOOLEAN Internal);


#ifndef HTS_UNIQUE_FILE_ID
#error "source file must define HTS_UNIQUE_FILE_ID"
#endif

#define HtsFileId() HTS_UNIQUE_FILE_ID

#define stringer(x) #x

#define AssertAlways(x) if (!(x)) { \
    HtsBugCheckEx("Internal Assertion Failure (" #x ") in " __FILE__ " at " stringer(__LINE__),  HtsFileId(), TRUE); \
}

