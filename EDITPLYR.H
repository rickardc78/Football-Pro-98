/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   editplyr.h

   the EditPlayer class,
   dervied from RosPlayer, only allows modification of data

   GJW: 7-30-1993, original creation
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _EDITPLYR_H
#define  _EDITPLYR_H

#include    "rosplyr.h"

class FAR EditPlayer : public RosPlayer {
private:
//    member functions

//    data members


protected:
//    member functions
      void              GeneratePBRs (void);
      void              GenerateABRs (void);
      void              Train (void);

//    data members
      DYN_8             lowPPct,
                        highPPct,
                        lowAPct,
                        highAPct;

public:
//    member functions
                        EditPlayer ();
                        ~EditPlayer ();

      void              ReRoll (DYN_U_8 &lowPValue, DYN_U_8 &highPValue,
                                 DYN_U_8 &lowAValue, DYN_U_8 &highAValue);

//    data members

};

#endif   // _EDITPLYR_H
