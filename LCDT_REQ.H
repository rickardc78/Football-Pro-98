/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   lcdt_req.h

   GJW: 1-29-1991, original creation
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _LCDT_REQ_H
#define  _LCDT_REQ_H

#include    "genlist.h"
#include "request.h"
#include "gadgets.h"

//    external functions
#ifdef   _LEAGUE_H
extern Team *findTeamPtr (GenList<Team> *, DYN_16);
extern GenList<Team> *fillTeamNameReq (CGadget *, DYN_16, League *);
extern void fillTeamNameReq (CGadget *, DYN_16, GenList<Team> *);
extern void fillTeamRecordGad (DYN_16, Team *);
extern void saveTeamChanges (Team *tPtr, const DYN_16 nflChange);
extern void initEXBTeamNames (CGadget *, DYN_16, League *, DYN_16);
extern void leagueCopyPro (V_V_FUNC vvFunc);
#endif

#ifdef   _TEAM_H
#ifdef _PLYRLIST_H
extern void fillTradeRatings (Team *t1, RosPlayer **p1Ptrs,
                              Team *t2, RosPlayer **p2Ptrs);
extern void fillTradeRatings (GenList<RosPlayer> *pList, const Trade *tradePtr);
extern RosPlayer * FAR * listToPPtrs (GenList<RosPlayer> *pList);
#endif
#endif

extern DYN_U_16 swapBytes (DYN_U_16);
extern DYN_8 *getHealthText (DYN_16, DYN_16);
extern DYN_8 *getStatusText (DYN_16);
extern DYN_8 *getYearsText (DYN_16);
extern void fillTradeIds (DYN_16 *, DYN_16);
extern void makeLeagueGameReady ();
extern void loadFullLeague ();
extern void printAllRosters ();

extern void keyCheck (DYN_16 editActive);
extern void waitBox (ReqType typeId, DYN_16 reqId);


//    external variables
extern CRequester    *reqPtr;
extern CGadget       *gadPtr;
extern DYN_8         listBuffer [];

#ifdef _SEQUENCE_H
extern Sequence      sequence;
#endif

#ifdef _STRPOOL_H
extern StringPool    stringPool,
                     fStringPool;
#endif

#ifdef _BLOCALOC_H
extern BlockAlloc    *baePtr;
#endif

#endif   // _LCDT_REQ_H
