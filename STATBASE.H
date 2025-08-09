/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   statbase.h

   the Statistic base class

   GJW: 7-13-1992, original creation
   GJW: 2-23-1995, updated for 32-bit
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _STATBASE_H
#define  _STATBASE_H

#include    <stdio.h>

class Statistic
{
private:
//    member functions

//    data members


public:
//    member functions

//    data members
      struct Data
      {
         DYN_U_16    type,    // kind of statistic
                     id,      // team or player
                     number;  // how many somethings
      };
      Data           data;

public:
//    member functions
                     Statistic ();
                     ~Statistic ();

      virtual void   Load (FILE *, DYN_16, DYN_16);
      virtual void   FillBuff (DYN_U_16 FAR *);
      virtual void   FillBuff (DYN_U_16 FAR *, DYN_16, DYN_16);
      virtual void   ReadBuff (DYN_U_16 FAR *);
      virtual void   UpdateBuff (DYN_U_16 FAR *);

      DYN_U_16       GetData (DYN_16);

//    data members
};

#endif   // _STATBASE_H
