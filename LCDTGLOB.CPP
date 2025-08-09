/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   lcdtglob.cpp

   GJW: 3-11-1993, original creation
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   #includes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#include       <ports.h>
#include       <dyndefs.h>

#include       "rosplyr.h"

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   consts
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   local utility prototypes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   global data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

// block identifiers
DYN_8 L02Text [] = "L02:",    // GJW: 7-18-1995, from L01 to L02
      C00Text [] = "C00:",
      D00Text [] = "D00:",
      T02Text [] = "T02:",    // GJW: 7-18-1995, from T01 to T02
      R01Text [] = "R01:",
      S00Text [] = "SCD:";

#if   (LCDT == 0)

DYN_8 PBText [] = "BOOKS",

      otText [] = ", (OT)",

      *tradeText [] =
      {
                  ", Proposed",
                  ", Accepted",
                  ", Rejected",
      };

#endif

RatingWeights ratWeights [RosPlayer::POS_MAX_SPEC] = {
// GJW: 8-27-1996, new valuing system
#if   1
//   ac   ag   di   en   ha   in   sp   st
   {  1,   1,   2,   2,   3,   5,   2,   4   },    // qb
   {  3,   2,   1,   2,   2,   1,   4,   5   },    // fb
   {  3,   2,   1,   2,   4,   1,   5,   2   },    // hb
   {  3,   2,   1,   2,   4,   1,   2,   5   },    // te
   {  3,   2,   1,   3,   5,   1,   4,   1   },    // wr
   {  3,   2,   2,   2,   3,   1,   2,   5   },    // c
   {  2,   3,   2,   2,   2,   1,   3,   5   },    // g
   {  5,   1,   2,   2,   2,   1,   2,   5   },    // t
   {  3,   3,   2,   2,   1,   1,   4,   4   },    // de
   {  5,   2,   2,   2,   1,   1,   2,   5   },    // dt
   {  2,   2,   1,   2,   3,   2,   4,   4   },    // lb
   {  3,   4,   1,   3,   2,   2,   4,   1   },    // cb
   {  2,   2,   1,   3,   4,   2,   4,   2   },    // s
   {  1,   1,   6,   1,   2,   1,   1,   7   },    // k
   {  1,   1,   6,   1,   2,   1,   1,   7   },    // p

#else
//   ac   ag   di   en   ha   in   sp   st
   {  0,   0,   0,   0,   0,   5,   0,   5   },    // qb
   {  0,   0,   0,   0,   0,   0,   5,   5   },    // fb
   {  0,   0,   0,   0,   5,   0,   5,   0   },    // hb
   {  0,   0,   0,   0,   5,   0,   0,   5   },    // te
   {  0,   0,   0,   0,   5,   0,   5,   0   },    // wr
   {  0,   0,   0,   0,   5,   0,   0,   5   },    // c
   {  0,   5,   0,   0,   0,   0,   0,   5   },    // g
   {  5,   0,   0,   0,   0,   0,   0,   5   },    // t
   {  5,   0,   0,   0,   0,   0,   0,   5   },    // de
   {  0,   5,   0,   0,   0,   0,   0,   5   },    // dt
   {  0,   0,   0,   0,   0,   0,   5,   5   },    // lb
   {  0,   5,   0,   0,   0,   0,   5,   0   },    // cb
   {  0,   0,   0,   0,   5,   0,   5,   0   },    // s
   {  0,   0,   0,   0,   0,   0,   0,  10   },    // k
   {  0,   0,   0,   0,   0,   0,   0,  10   },    // p
#endif
};

#if   (LCDT > 0)

DYN_8 msgFileText [] = "msg.dat";   // for assertvm.cpp

#endif

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   local data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */








/*
   =====================================================================
                           PUBLIC METHODS
   =====================================================================
*/







/*
   =====================================================================
                           PRIVATE METHODS
   =====================================================================
*/








/*
   =====================================================================
                           LOCAL UTILITIES
   =====================================================================
*/
