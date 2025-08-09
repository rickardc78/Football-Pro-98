/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   magazine.h

   the Magazine class
   GJW: 12-9-1991, original creation
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _MAGAZINE_H
#define  _MAGAZINE_H

#include    <ports.h>
#include    <stdio.h>

#define NUM_MONTHS 12
#define MAX_NAME 24
#define NAME_DATE_MAX 64

class Magazine
{
protected:
//    member functions

//    data members


private:
//    member functions
      void     Load ();
      DYN_8    *ReadString (FILE *);
      void     ReadDate (DYN_8 *);
      void     FontSetup ();
      void     MakeND ();

//    data members
      DYN_8    *monthString,
               *monthNames [NUM_MONTHS],
               *pageText,
               *name;
      void     *font;
      DYN_16   fgcolor1, fgcolor2, fgcolor3, fgcolor4,
               xND, yND,
               xPG;
      DYN_8    mode,
               nameDate [NAME_DATE_MAX];


public:
//    member functions
               Magazine (void *fntPtr);
               ~Magazine ();

      void     PrintNDPAGE (DYN_16);

//    data members
};

#endif      // _MAGAZINE_H
