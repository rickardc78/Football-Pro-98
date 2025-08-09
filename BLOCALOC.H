/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   blocaloc.h

   the BlockAlloc class

   allocate a big block of mem, then dish it out in pieces
   there is no Delete () since this isn't a memory allocation handler,
   it just grabs a big chunk at the start of an operation, and doles it
   out as needed. After the operation, Reset () is called to allow another
   operation to allocate what it needs from this same pool

   GJW: 11-11-1993, original creation
   GJW: 9-13-1994, ported over to football
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _BLOCALOC_H
#define  _BLOCALOC_H

class BlockAlloc
{
private:
//    member functions

//    data members

protected:
//    member functions
                     // the default (no argument) constructor is NOT
                     // accessible by the public
                     // a user is expected to provide a memory amount
                     // when instantiating this type of object
                     BlockAlloc ();

//    data members
      void           *block,     // -> original memory allocation
                     *curPtr;    // -> current empty location
      DYN_U_16       size,       // number of bytes in original allocation
                     numAlloc;   // number currently 'doled out'

public:
//    member functions
                     BlockAlloc (const DYN_U_16 memAmount);
      virtual        ~BlockAlloc ();

      virtual void   Reset ();
      virtual void   *New (const DYN_U_16 memSize);

//    data members (hopefully none!)
};
#endif   // _BLOCALOC_H
