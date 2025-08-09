/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   strpool.h

   the StringPool & FStringPool class

   GJW: 3-18-1993, original creation
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _STRPOOL_H
#define  _STRPOOL_H

class StringPool
{
private:
//    member functions

//    data members
      DYN_16   poolSize;
      DYN_8    *pool,
               *cur;

protected:
//    member functions

//    data members


public:
//    member functions
               StringPool (DYN_16 size = 0);
               ~StringPool ();

      DYN_16   ReSize (DYN_16 size);
      DYN_8    *Alloc (DYN_16 size);

//    data members
};
#endif   // _STRPOOL_H
