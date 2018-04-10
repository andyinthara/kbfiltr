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
#define HTS_UNIQUE_FILE_ID 0x1204002

#include "htscpp_internal.h"

//
// we appropriate DriverEntry so the driver must use another name for this routine.
//


#ifdef __cplusplus
extern "C" {
#endif

    NTSTATUS
    Cp_DriverEntry(PDRIVER_OBJECT DriverObject,
                   PUNICODE_STRING RegistryPath); 
    NTSTATUS
    DriverEntry(PDRIVER_OBJECT DriverObject,
                   PUNICODE_STRING RegistryPath);

#ifdef __cplusplus
}
#endif  /* __cplusplus */


static
void __cdecl onexitinit (
        void
        );
static 
PVFV __cdecl onexit (
        PVFV func
        );
static
void __cdecl callPVFVArray (
        PVFV * pfbegin,
        PVFV * pfend
        );
static
void __cdecl doexit (
        int code,
        int quick,
        int retcaller
        );


static VOID
    _CppDriverUnload(PDRIVER_OBJECT DriverObject);

static PDRIVER_UNLOAD clientUnload = NULL;

NTSTATUS
DriverEntry(PDRIVER_OBJECT DriverObject,
               PUNICODE_STRING RegistryPath)

{

    onexitinit();
    

    callPVFVArray(__crtXca, __crtXcz);


    //
    // call the clients driver entry routine.
    //

    NTSTATUS Status = Cp_DriverEntry(DriverObject, RegistryPath);

    if (NT_SUCCESS(Status)) {

        //
        // we have to steal the unload vector if it exists
        //
        if (DriverObject->DriverUnload) {

            clientUnload = DriverObject->DriverUnload;
            DriverObject->DriverUnload = _CppDriverUnload;
        }

    } else {
        //
        // call the d'tors for global and static objects?
        //
        doexit (0, 0, 1);
    }
    return Status;
}

static
VOID
    _CppDriverUnload(PDRIVER_OBJECT DriverObject)
{
    
    //
    // call the clients unload routine
    //
    if (clientUnload) {

        clientUnload(DriverObject);

    }

    //
    // do what is appropriate for destorying global objects
    //
    doexit (0, 0, 1);

    return;

}

//
// this must be externally visible!
//
int __cdecl atexit (
        PVFV func
        )
{
        return (onexit(func) == NULL) ? -1 : 0;
}

//
// this was "initterm" but we ain't initting any 'terms' I can think
// of. What we are doing is calling an array of functions.
//
void __cdecl callPVFVArray (
        PVFV * pfbegin,
        PVFV * pfend
        )
{
    
    while ( pfbegin < pfend )
    {
        /*
         * if current table entry is non-NULL, call thru it.
         */
        if ( *pfbegin != NULL )
            (**pfbegin)();
        ++pfbegin;
    }
}

//
// the CRT code had some ugly stuff that created a variable length array of
// function pointers. This was scrapped in favor of a linked list.
//

typedef struct {
    LIST_ENTRY  link; // double linked list of exit functions
    PVFV         exitFunc;
} EXIT_FUNC_LIST, *PEXIT_FUNC_LIST;

LIST_ENTRY exitList;


void __cdecl onexitinit (
        void
        )
{
    //
    // this is a bit easier
    //
    InitializeListHead(&exitList);

}

PVFV __cdecl onexit (
        PVFV func
        )

{
    PEXIT_FUNC_LIST pFuncListEntry = 
        (PEXIT_FUNC_LIST)malloc(sizeof(EXIT_FUNC_LIST), 'EPcO');

    if (!pFuncListEntry) {

        return NULL;
    }
    pFuncListEntry->exitFunc = func;

    //
    // gee, creating a lifo list is rather trivial
    //
    InsertHeadList(&exitList, &pFuncListEntry->link);
        
    return func;
}

void drainExit()
{
    PEXIT_FUNC_LIST pFuncListEntry;

    while(!IsListEmpty(&exitList)) {

        //
        // this cast relies on link being the first field of
        // EXIT_FUNC_LIST.
        //        
        pFuncListEntry = (PEXIT_FUNC_LIST) RemoveHeadList(&exitList);
        AssertAlways((PVOID) pFuncListEntry == (PVOID)&pFuncListEntry->link);

        //
        // wrap this in a try/except handler?
        //
        if (pFuncListEntry->exitFunc) {

            pFuncListEntry->exitFunc();

        }

        free(pFuncListEntry);
    }

}

void __cdecl doexit (
        int,
        int quick,
        int
        )
{

    if (!quick) {
      
        drainExit();

        
    }

    
    return;

}

void HtsBugCheckEx(PCHAR Message, ULONG FileId, BOOLEAN Internal)
{
    DbgPrint("******************************************************\n");

    if (Internal) {
        DbgPrint("********** INTERNAL ASSERTION FAILURE ********\n");
    } else {
        DbgPrint("********** ASSERTION FAILURE  ********\n");
    }
    //
    // Print the message passed in to us.
    //
    DbgPrint("** %s **\n", Message);
    DbgPrint("********** Driver WILL NOW BUG CHECK ****************\n");
    KeBugCheckEx(FILE_SYSTEM, FileId, Internal,
        // if possible include the threadid
        KeGetCurrentIrql() <= DISPATCH_LEVEL ? (ULONG_PTR) PsGetCurrentThread(): 0, 
        // Might as well toss in the current irql
        KeGetCurrentIrql());
}








