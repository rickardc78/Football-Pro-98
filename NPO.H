/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   npo.h

   the NewPlayerOrder class

   GJW: 8-22-1993, original creation
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _NPO_H
#define  _NPO_H

#include       <fstream.h>

class NewPlayerOrder {
private:
//    member functions

//    data members


protected:
//    member functions
      RosPlayer::Pos       GetPos (void);
      void                 ReadOrder (void);
      DYN_16               InsertPlayer (void);
      void                 BellCurve (void);
      void                 CheckHighLow (DYN_8 *high, DYN_8 *low,
                                          RosPlayer::Rating rating,
                                          RosPlayer *pPtr);
      void                 CopyList (void);

//    data members
      static DYN_16        bellFlag,
                           bellLow,
                           bellHigh;

      enum { MAX_LINE = 81 };

      ifstream             inf;
      RosPlayer::Pos       pos;
      GenList<RosPlayer>   *oldList,
                           *newList;
      DYN_8                inBuff [MAX_LINE];


public:
//    member functions
      static void          SetBellRange (const DYN_8 *string);
      void                 CheckNewOrder (void);

//    data members
};

#endif   // _NPO_H
