/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   cdpool.h

   the CollegeDraftPool class
   derived from the RosPlayerPool class (which is just a linked list
   of RosPlayers)

   GJW: 4-15-1992, original creation
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _CDPOOL_H
#define  _CDPOOL_H

#include    "nameplyr.h"
#include    "plyrlist.h"

class CollegeDraftPool : public RosPlayerList
{
private:
   // member functions

   // data members


protected:
   // member functions
   void     Load (DYN_8 *, DYN_8 *, PlayerRdWr &);

   // data members

public:
   // member functions
            CollegeDraftPool ();
            CollegeDraftPool (DYN_8 *, DYN_8 *, PlayerRdWr &);
            ~CollegeDraftPool ();

   void     Save (DYN_8 *, DYN_8 *);

   DYN_16   MakePool (NamePlayer *, RosPlayer::Pos, DYN_16, DYN_16, RosPlayer::Pos);

   // data members
};

#endif      // _CDPOOL_H
