/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   lchamps.h

   the LeagueChamps class

   GJW: 2-5-1993, original creation
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _LCHAMPS_H
#define  _LCHAMPS_H

#include    "strlist.h"

class LeagueChamps : public StringList
{
private:
//    member functions

//    data members


protected:
//    member functions

//    data members

public:
//    member functions
                     LeagueChamps (DYN_8 *path, DYN_8 *fName,
                                    DYN_16 inception);
                     ~LeagueChamps ();

      static void    WriteChamp (DYN_8 *path, DYN_8 *fName,
                                 DYN_8 FAR *wTeam, DYN_8 wScore,
                                 DYN_8 FAR *lTeam, DYN_8 lScore);
      static DYN_16  Copy (DYN_8 *buffer,
                           DYN_8 *dPath, DYN_8 *dfName,
                           DYN_8 *sPath, DYN_8 *sfName);

//    data members
};

#endif   // _LCHAMPS_H
