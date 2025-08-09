/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   nameplyr.h

   the NamePlayer class
   (allocates names from a list of first and last names)

   GJW: 3-26-1992, original creation
   GJW: 10-19-1994, store strings in EMM
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _NAMEPLYR_H
#define  _NAMEPLYR_H

#include "blocaloc.h"

struct NameBuffer
{
   DYN_8    FAR *start,
            FAR *end,
            FAR *cur;
};

class NamePlayer
{
private:
//    member functions

//    data members


protected:
//    member functions
      static void       NEAR MovePtr (NameBuffer &nb, DYN_16 seeks);

      void              NEAR ReadFile (DYN_8 *fName, NameBuffer &nb);

//    data members
      BlockAlloc        bae;
      NameBuffer        firstNames,
                        lastNames;

public:
//    member functions
                  NamePlayer ();
                  ~NamePlayer ();

   DYN_16         GetName (DYN_8 *fName, DYN_8 *lName);

//    data members
};

#endif      // _NAMEPLYR_H
