/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   trade.h

   the Trade class

   GJW: 5-14-1992, original creation
   GJW: 8-26-1996, multi-player
	CAM	17 DEC 96	Added TOManager Sort routine

컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _Trade_H
#define  _Trade_H

#include "lgefile.h"

struct TradeInfo
{
   enum
   {
      MAX_PIDS = 3
   };

   DYN_8    tId;              // which team
   DYN_16   pIds [MAX_PIDS];  // which player(s)
};

enum TradeInfoData
{
   TRADEINFODATA_TEAMID,
   TRADEINFODATA_PLAYER0ID,
   TRADEINFODATA_PLAYER1ID,
   TRADEINFODATA_PLAYER2ID,
   TRADEINFODATA_STATUS,
};

enum TradeStatus
{
   TRADE_PROPOSED,
   TRADE_ACCEPTED,
   TRADE_REJECTED,
   TRADE_DELETED,
};

enum TradeSideData
{
   TRADE_FROM,       // who proposed
   TRADE_TO,         // who accepted
   TRADE_MAX,
};

class Trade
{
private:
   // member functions

   // data members


protected:
   // member functions

   // data members
   TradeInfo      tradeData [TRADE_MAX];
   TradeStatus    status;

public:
   // member functions
                  // team 1, player 1, team 2, player 2
                  Trade (DYN_16 t1Id, DYN_16 *p1Ids,
                           DYN_16 t2Id, DYN_16 *p2Ids,
                           TradeStatus stat);
                  Trade (ReadBFile &rlt);
                  ~Trade ();

   DYN_16         GetTSData (const TradeSideData tsData,
                              const TradeInfoData tiData) const;
   void           SetTSData (const TradeStatus newStatus);
   void           Save (WriteBFile &wlt);

   void           GetPIds (DYN_16 *p1Ids, DYN_16 *p2Ids) const;

   // data members
};

// GJW: 9-9-1996, for multiple-player trades
class Team;
class RosPlayer;

struct TradeOffer
{
   RosPlayer   *pPtrs [TradeInfo::MAX_PIDS];
   DYN_32      tId;
};

class TOManager
{
private:
   // member functions

   // data members


protected:
   // member functions

   // data members
   enum
   {
      MAX_OFFERS = 1024 * 16,
   };

   TradeOffer  *tis;
   short       numTis,
               maxTis;


public:

   // data members

	enum SortType
	{
		ST_VALUE,		// added 17 DEC 96 CAM
	};
   // member functions
                  TOManager ();
                  ~TOManager ();

   DYN_32         AddTrade		( TradeOffer &to );
   DYN_32         GetTrade		( TradeOffer &to, const DYN_32 index );
   DYN_32         GetNumTrades( void );
   void           InitOffer	( TradeOffer &to, const DYN_32 t );
	void				Sort			( const SortType st );
};

#endif      // _Trade_H
