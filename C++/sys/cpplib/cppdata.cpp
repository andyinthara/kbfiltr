///////////////////////////////////////////////////////////////////////////////
//
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
#define HTS_UNIQUE_FILE_ID 0x1204001

#include "htscpp_internal.h"
//
// this stuff must be just like the VisualC++ compiler wants it
// to be. If you have a licensed copy of VisualC++ then refer to the
// CRT library source code for details.
//
#define _CRTALLOC(x) __declspec(allocate(x))

#pragma section(".CRT$XCA",long,read)
#pragma section(".CRT$XCZ",long,read)

_CRTALLOC(".CRT$XCA") PVFV __crtXca[] = { NULL };
_CRTALLOC(".CRT$XCZ") PVFV __crtXcz[] = { NULL };

//
// this may or may not be required
//
#if defined (_M_IA64) || defined (_M_AMD64)

#pragma comment(linker, "/merge:.CRT=.rdata")

#else

#pragma comment(linker, "/merge:.CRT=.rdata")

#endif

#if 0
#if defined (_M_IA64) || defined (_M_AMD64)

#pragma section(".CRT$XCA",long,read)
#pragma section(".CRT$XCZ",long,read)
#pragma data_seg(".CRT$XCA")
__declspec(allocate(".CRT$XCA")) PVFV __crtXca[] = { NULL }; // c++
#pragma data_seg(".CRT$XCZ")
__declspec(allocate(".CRT$XCZ")) PVFV __crtXcz[] = { NULL }; 

#else

#pragma data_seg(".CRT$XCA")
PVFV __crtXca[] = { NULL }; // c++
#pragma data_seg(".CRT$XCZ")
PVFV __crtXcz[] = { NULL };

#endif

#pragma data_seg()  // return control to the normal ddk process
//
// this may or may not be required
//
#if defined (_M_IA64) || defined (_M_AMD64)

#pragma comment(linker, "/merge:.CRT=.rdata")

#else

#pragma comment(linker, "/merge:.CRT=.data")

#endif
#endif

