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
///////////////////////////////////////////////////////////////////////////////

#include "htscpp.h"

CPP_DRIVER_ENTRY(PDRIVER_OBJECT DriverObject,
                 PUNICODE_STRING RegistryPath);

enum baseType { BIRD, PLANE, SUPERMAN };

class base {

public:
    base();
    virtual ~base();
    virtual baseType isa()=0;
};

class global : public base {

public:
    global(int x);
    global();
    virtual ~global();

    int getX();
    virtual baseType isa();

private:
    int m_x;
};


base::base() 
{
#if DBG
    DbgPrint("base: this %x\n", this);
#endif
}

base::~base()
{
#if DBG
    DbgPrint("~base: this %x\n", this);
#endif
}

global::global(int x)
{
#if DBG
    DbgPrint("global( %x): this %x\n", x, this);
#endif    
    m_x = x;
}

global::global()
{
#if DBG
    DbgPrint("global: this %x\n", this);
#endif    
    m_x = 0;
}

global::~global()
{
#if DBG
    DbgPrint("~global: this %x\n", this);
#endif    
    m_x = 0;
}

int global::getX()
{
#if DBG
    DbgPrint("global::getX m_x %x this %x\n", m_x, this);
#endif    
    return m_x;
}

baseType global::isa()
{
#if DBG
    DbgPrint("global::isa this %x\n", this);
#endif    
    return BIRD;
}

global one(2);

global two;

global other(3);

extern "C" void
testUnload(PDRIVER_OBJECT DriverObject);

CPP_DRIVER_ENTRY(PDRIVER_OBJECT DriverObject,
                 PUNICODE_STRING )
{
#if DBG
    DbgPrint("CPP_DRIVER_ENTRY\n");
#endif

    DriverObject->DriverUnload = testUnload;

    if (one.getX() != 2) {

        return STATUS_UNSUCCESSFUL;

    }

     if (two.getX() != 0) {

        return STATUS_UNSUCCESSFUL;

    }

    global * three = new('vrdT') global(4); // this is not global!

    if (!three) {
        //
        // allocation failure!
        //

        return STATUS_UNSUCCESSFUL;
    }

    if (other.getX() != 3) {

        return STATUS_UNSUCCESSFUL;

    }

    global four(4444);

    if (four.getX() != 4444) {

        delete three;

        return STATUS_UNSUCCESSFUL;

    }

    delete three;

    return STATUS_SUCCESS;

}

void
testUnload(PDRIVER_OBJECT )
{
#if DBG
    DbgPrint("testUnload\n");
#endif

    return;
}



