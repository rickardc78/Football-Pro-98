/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   tradnews.h

   the TradeNews class (a linked list of Trades)

   GJW: 5-14-1992, original creation
   GJW: 8-26-1996, multi-player
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _TradNews_H
#define  _TradNews_H

#include "Genlist.h"
#include "Trade.h"

class TradeNews
{
private:
   // member functions

   // data members


protected:
   // member functions
   void           Load (DYN_8 *, DYN_8 *);
   DYN_32         PlayerInvolved (DYN_16 tId, DYN_16 thisId,
                                    DYN_16 *pIds, DYN_16 *thisPIds);
   DYN_32         TheTrade (DYN_16 t1Id, DYN_16 this1Id,
                              DYN_16 t2Id, DYN_16 this2Id,
                              DYN_16 *p1Ids, DYN_16 *this1PIds,
                              DYN_16 *p2Ids, DYN_16 *this2PIds);

   // data members
   GenList<Trade> trades;


public:
   // member functions
                  TradeNews (DYN_8 *, DYN_8 *);
                  ~TradeNews ();

   static void    Flush (const DYN_8 *path, const DYN_8 *fileName);

   GenList<Trade> *BuildTradeList ();
   DYN_16         BuildTradeList (GenList<Trade> *, TradeStatus, DYN_16);

   void           SetTrade (DYN_16 t1Id, DYN_16 *p1Ids,
                              DYN_16 t2Id, DYN_16 *p2Ids,
                              TradeStatus status);
   void           Save (DYN_8 *path, DYN_8 *fileName);
   void           Flush ();
   void           KillProposed ();
   DYN_16         KillPTrades (DYN_16 pId);

   // data members
};

#endif      // _TradNews_H
