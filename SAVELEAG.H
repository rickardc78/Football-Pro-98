/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   saveleag.h

   the SaveLeague class

   GJW: 5-26-1993, original creation
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _SAVELEAG_H
#define  _SAVELEAG_H

#include    "league.h"

class SaveLeague
{
private:
//    member functions

//    data members


protected:
//    member functions

//    data members
      static DYN_8   savePath [MAXPATH],
                     saveFName [MAXFILE];

public:
//    member functions
      static void    SavePath ();
      static League  *LoadLeague (League::FullLoad lfl);

//    data members
};

#endif   // _SAVELEAG_H
