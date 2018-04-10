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
//  Synopsis: 
// 
//
///////////////////////////////////////////////////////////////////////////////
#pragma once
#pragma warning(disable:4201)
//
// add externally visible runtime constructs here
//
#ifdef __cplusplus 
extern "C" {
#endif
    
#ifndef NT_INCLUDED
#include <ntddk.h>
#endif

#ifdef __cplusplus
}
#endif

#if WINVER == 0x0500
#define ExFreePoolWithTag( a, b ) ExFreePool( (a) )
#endif
//
// memory allocation support
//
//typedef unsigned int size_t;

#ifdef __cplusplus 

void * __cdecl operator new(size_t size);

void * __cdecl operator new(size_t size, void *location);

void *__cdecl operator new( size_t size, ULONG tag, POOL_TYPE pool= NonPagedPool);

void __cdecl operator delete(void * pVoid);

#endif

PVOID __cdecl malloc(size_t x,  ULONG id= 'ppc_', POOL_TYPE pool = NonPagedPool);

void __cdecl free(PVOID x, ULONG id= 'ppc_', POOL_TYPE pool = NonPagedPool);


__inline PVOID __cdecl malloc(size_t size,  ULONG id, POOL_TYPE pool)
{
    PVOID buffer = ExAllocatePoolWithTag(pool, size, id);

    if (buffer) {
        //
        // clear our buffer to zero
        //
        RtlZeroMemory(buffer, size);
    }

    return buffer;
}


__inline void __cdecl free(PVOID buffer, ULONG, POOL_TYPE)
{
    if (buffer != NULL) {
        //
        // placement delete still nfg, so just call ExFreePool
        //
        ExFreePool(buffer);
    }
}

#ifdef __cplusplus 

__inline void * __cdecl operator new(size_t size)
{
    return malloc(size, 'ppc_', NonPagedPool);
}


__inline void * __cdecl operator new(size_t,  void *location)
{
    return location;
}

__inline void *__cdecl operator new( size_t size, ULONG tag, POOL_TYPE pool)
{
    return malloc(size, tag, pool);
}

__inline void __cdecl operator delete(void * pVoid)
{
    free(pVoid);
}

inline void taggedDelete(PVOID buffer, ULONG tag)
{
    if (buffer != NULL) {
        ExFreePoolWithTag(buffer, tag);
    }
}
#endif
//
// light weight try finally
//
#ifndef TRY
#define TRY
#define LEAVE goto __tf_label__
#define FINALLY __tf_label__:
#endif

//
// runtime linkage support:
//

//
// All you have to do to get C++ runtime support is
// replace *your* driver entry routine with this one, as in:
// 
// CPP_DRIVER_ENTRY(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
//
// And link your driver with our C++ runtime library!
//

#define CPP_DRIVER_ENTRY(Do, Rp) extern "C" NTSTATUS Cp_DriverEntry(Do, Rp)

//
// and finally, as this might be generally useful:
//
typedef void (__cdecl *PVFV)(void);

//
// this is the standard ANSI atexit function. You can register
// any arbitrary void function with no parameters through this interface.
//
// Registered functions will be called in LIFO order on termination.
//
// Termination is defined here as DriverUnload.
//
int __cdecl atexit (
        PVFV func
        );


