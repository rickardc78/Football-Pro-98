/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   percent.h

   the Percent class

   GJW: 8-18-1993, original creation
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _PERCENT_H
#define  _PERCENT_H

#include    "rosplyr.h"

class Percent {
private:
//    member functions

//    data members


protected:
//    member functions

//    data members
      static DYN_U_8    pbrPct [RosPlayer::POS_MAX_SPEC],
                        abrPct [RosPlayer::POS_MAX_SPEC];

public:
//    member functions
      DYN_U_8           GetPPct (RosPlayer::Pos pos);
      DYN_U_8           GetAPct (RosPlayer::Pos pos);

      void              SetPct (const DYN_8 *string);

//    data members
};

#endif   // _PERCENT_H

