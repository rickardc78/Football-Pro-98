/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   gamesave.h

   the GameSave class

   GJW: 2-9-1993, original creation
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _GAMESAVE_H
#define  _GAMESAVE_H

#include    "strlist.h"

class GameSave : public StringList
{
private:
//    member functions

//    data members


protected:
//    member functions
      void           KillDir (const DYN_8 *dirName) const;

//    data members

public:
//    member functions
                     GameSave (DYN_8 *exbText, DYN_8 *lgeText);
                     ~GameSave ();

      DYN_8          *RestoreGame (DYN_16 index);

//    data members
};

#endif   // _GAMESAVE_H
