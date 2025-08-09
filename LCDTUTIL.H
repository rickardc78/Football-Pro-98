/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   lcdtutil.h

   GJW: 1-25-1991, original creation
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _LCDTUTIL_H
#define  _LCDTUTIL_H

#include       <stdio.h>
#include       <stddef.h>

#include       "genlist.h"

//    external functions
extern DYN_8 *getSpecPosText (DYN_16);

#if   (LCDT == 0)
#ifdef   _STATFILE_H
#ifdef   _PLYRLIST_H
extern void pbQBFunc (DYN_16 weekNum, GenList<RosPlayer> *pList,
                        StatFile *sfPtr);
extern void pbFBFunc (DYN_16 weekNum, GenList<RosPlayer> *pList,
                        StatFile *sfPtr);
extern void pbHBFunc (DYN_16 weekNum, GenList<RosPlayer> *pList,
                        StatFile *sfPtr);
extern void pbTEFunc (DYN_16 weekNum, GenList<RosPlayer> *pList,
                        StatFile *sfPtr);
extern void pbWRFunc (DYN_16 weekNum, GenList<RosPlayer> *pList,
                        StatFile *sfPtr);
extern void pbCFunc (DYN_16 weekNum, GenList<RosPlayer> *pList,
                        StatFile *sfPtr);
extern void pbGFunc (DYN_16 weekNum, GenList<RosPlayer> *pList,
                        StatFile *sfPtr);
extern void pbTFunc (DYN_16 weekNum, GenList<RosPlayer> *pList,
                        StatFile *sfPtr);
extern void pbDEFunc (DYN_16 weekNum, GenList<RosPlayer> *pList,
                        StatFile *sfPtr);
extern void pbDTFunc (DYN_16 weekNum, GenList<RosPlayer> *pList,
                        StatFile *sfPtr);
extern void pbLBFunc (DYN_16 weekNum, GenList<RosPlayer> *pList,
                        StatFile *sfPtr);
extern void pbCBFunc (DYN_16 weekNum, GenList<RosPlayer> *pList,
                        StatFile *sfPtr);
extern void pbSFunc (DYN_16 weekNum, GenList<RosPlayer> *pList,
                        StatFile *sfPtr);
extern void pbKFunc (DYN_16 weekNum, GenList<RosPlayer> *pList,
                        StatFile *sfPtr);
extern void pbPFunc (DYN_16 weekNum, GenList<RosPlayer> *pList,
                        StatFile *sfPtr);
#endif
#endif
#endif

#ifdef   _TEAM_H
#ifdef   _PLYRLIST_H
extern void fillTradeBuffer (DYN_8 *dest,
                              Team *t1Ptr, RosPlayer **p1Ptr,
                              Team *t2Ptr, RosPlayer **p2Ptr);
extern void fillTradeBuffer (GenList<RosPlayer> *pList,
                              DYN_8 *dest, Trade *tradePtr, DYN_16 verbose);
extern void fillTradeBuffer (DYN_8 *dest, RosPlayer **pPtrs);
extern void deleteTradePlayerList (GenList<RosPlayer> *);
#endif
#endif

#ifdef   _TradNews_H
extern GenList<RosPlayer> *buildTradePlayerList (GenList<Trade> *);
#endif

//    external variables
extern DYN_8   L02Text [],    // GJW: 7-18-1995, changed from L01 to L02
               C00Text [],
               D00Text [],
               T02Text [],    // GJW: 7-18-1995, changed from T01 to T02
               R01Text [],
               S00Text [];

#if   (LCDT == 0)
extern DYN_8 PBText [];
extern DYN_8 otText [];
extern DYN_8 *tradeText [];
#endif

#ifdef   _ROSPLYR_H
extern RatingWeights ratWeights [];
#endif

#endif   // _LCDTUTIL_H
