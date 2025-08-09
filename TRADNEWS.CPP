/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   tradnews.cpp

   GJW: 5-14-1992, original creation
   GJW: 9-1-1992, all trades involving the same player are set to rejected
                  once he is accepted (i.e. offered to several teams)
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   #includes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#include <string.h>

#include <ports.h>
#include "dyndefs.h"

#include "FileUtil.h"
#include "TradNews.h"

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   consts
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   local utility prototypes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   global data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   local data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

static DYN_8   trnText [] = ".trn",
               TRNText [] = "TRN:";

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/*
   ---------------------------------------------------------------------
   NAME:          TradeNews

   FUNCTION:      TradeNews object CONSTRUCTOR

   INPUT:         none

   RETURNS:       -> created TradeNews object
   ---------------------------------------------------------------------
*/

TradeNews::TradeNews (DYN_8 *path, DYN_8 *fileName)
{
   Load (path, fileName);
}

/*
   ---------------------------------------------------------------------
   NAME:          ~TradeNews

   FUNCTION:      TradeNews object DESTRUCTOR

   INPUT:         -> TradeNews object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

TradeNews::~TradeNews ()
{
   Flush ();
}








/*
   =====================================================================
                           PUBLIC METHODS
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       Flush

   FUNCTION:   when starting a new week, kill all trade news

   INPUT:      pName, fName-> string for league filename path,

   RETURNS:    league.trn destroyed
   ---------------------------------------------------------------------
*/

void
TradeNews::Flush (const DYN_8 *path, const DYN_8 *fileName)
{
   unlink (makeFileName (0, (DYN_8 *) path, (DYN_8 *) fileName, trnText));
}

/*
   ---------------------------------------------------------------------
   NAME:       BuildTradeList

   FUNCTION:   fill the callers list with trades of the given status

   INPUT:      if none, make a list, fill will all trades, return list to
                  caller
               tList-> list to fill
               status = status of trades to fill list with
               id = only those trades affecting this team (0 = ALL)

   RETURNS:    tList filled with the given trades
   ---------------------------------------------------------------------
*/

GenList<Trade> *
TradeNews::BuildTradeList ()
{
   GenList<Trade> *tList = new GenList<Trade>;

   if (tList && !trades.start ())
   {
      while (TRUE)
      {
         Trade    *tPtr = trades ();
         tList->linkin (tPtr);

         if (++trades)
            break;
      }
   }

   return tList;
}

DYN_16
TradeNews::BuildTradeList (GenList<Trade> *tList, TradeStatus status,
                              DYN_16 id)
{
   DYN_16   numTrades = 0;

   if (!trades.start ())
   {
      while (TRUE)
      {
         Trade    *tPtr = trades ();

         if (tPtr->GetTSData (TRADE_FROM, TRADEINFODATA_STATUS) == status)
         {
            // filter trades based on id
            if (!id ||
               (id == tPtr->GetTSData (TRADE_FROM, TRADEINFODATA_TEAMID)) ||
               (id == tPtr->GetTSData (TRADE_TO, TRADEINFODATA_TEAMID)))
            {
               numTrades++;
               if (tList)
                  tList->linkin (tPtr);
            }
         }

         if (++trades)
            break;
      }
   }

   return numTrades;
}

/*
   ---------------------------------------------------------------------
   NAME:       SetTrade

   FUNCTION:   store info pertaining to a trade
               if status == TRADE_PROPOSED, this is a new trade and
               needs to be stored
               otherwise, this is the modification (ACCEPTED/REJECTED) of
               a previous trade

   INPUT:      tId1, pId1, tId2, pId2 = team & player identifiers
               status = trade proposed/accepted/rejected

   RETURNS:    trade added to list, or modified
   ---------------------------------------------------------------------
*/

void
TradeNews::SetTrade (DYN_16 t1Id, DYN_16 *p1Ids,
                     DYN_16 t2Id, DYN_16 *p2Ids,
                     TradeStatus status)
{
   // first search for this trade already (don't store multiple copies
   // of the same trade)
   DYN_16   linkinFlag = TRUE;

   if (!trades.start ())
   {
      while (TRUE)
      {
         Trade    *tPtr = (Trade *) trades ();
         DYN_16   t1 = tPtr->GetTSData (TRADE_FROM, TRADEINFODATA_TEAMID),
                  t2 = tPtr->GetTSData (TRADE_TO, TRADEINFODATA_TEAMID),
                  p1s [TradeInfo::MAX_PIDS],
                  p2s [TradeInfo::MAX_PIDS];
         tPtr->GetPIds (p1s, p2s);

         // find any trade involving ANY Player
         // (WHO HAS REMAINED ON THE SAME TEAM)
         #if   0
         if (((p1 == p1Id) && (t1 == t1Id)) ||
            ((p1 == p2Id) && (t1 == t2Id)) ||
            ((p2 == p1Id) && (t2 == t1Id)) ||
            ((p2 == p2Id) && (t2 == t2Id)))

         #else
         if (PlayerInvolved (t1Id, t1, p1Ids, p1s) ||
               PlayerInvolved (t2Id, t1, p2Ids, p2s) ||
               PlayerInvolved (t1Id, t2, p1Ids, p2s) ||
               PlayerInvolved (t2Id, t2, p2Ids, p2s))

         #endif
         {
            // if this is THE trade
            if (TheTrade (t1Id, t1, t2Id, t2, p1Ids, p1s, p2Ids, p2s))
            {

               // don't allow multiple copies of the same trade
               if (status == TRADE_PROPOSED)
               {
                  linkinFlag = FALSE;
                  break;
               }

               // proposer has changed mind
               else if (status == TRADE_DELETED)
                  trades.linkout ();

               // trade is either being accepted or rejected
               else
               {
                  linkinFlag = FALSE;
                  tPtr->SetTSData (status);
               }
            }

            // otherwise this is a proposal involving 1 of the same players
            else
            {
               // if ORIGINAL trade is being ACCEPTED, ANY OTHER trade
               // involving this player is set to REJECTED (he will no
               // longer be on the team)
               if (status == TRADE_ACCEPTED)
                  tPtr->SetTSData (TRADE_REJECTED);
            }
         }

         if (++trades)
            break;
      }
   }

   // if similar trade was not found, add it
   if (linkinFlag)
   {
      Trade    *tPtr = new Trade (t1Id, p1Ids, t2Id, p2Ids, status);
      if (tPtr)
      {
         trades.end ();
         trades.linkin (tPtr);
      }
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       Save

   FUNCTION:   open a file & write the "TRN:" block
               "TRN:"
               numTrades            sizeof (DYN_16)
               trades               numTrades * sizeof (Trade)

   INPUT:      pName-> path
               fName-> base filename

   RETURNS:    file updated
   ---------------------------------------------------------------------
*/

void
TradeNews::Save (DYN_8 *path, DYN_8 *fileName)
{
   WriteBFile  wlf;
   if (wlf.Open (makeFileName (0, path, fileName, trnText)))
   {
      wlf.OpenBlock (TRNText);

      DYN_16   numTrades = trades.size ();
      wlf.Write (&numTrades, sizeof (DYN_16));

      if (!trades.start ())
      {
         while (TRUE)
         {
            Trade    *tPtr = (Trade *) trades ();
            tPtr->Save (wlf);

            if (++trades)
               break;
         }
      }
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       Flush

   FUNCTION:   remove all the trades (this week has passed, so start
               this week anew)

   INPUT:      trades = linked list of all trades

   RETURNS:    trades = empty list
   ---------------------------------------------------------------------
*/

void
TradeNews::Flush ()
{
   if (!trades.start ())
   {
      while (TRUE)
      {
         Trade    *tPtr = (Trade *) trades.linkout ();

         if (tPtr)
            delete tPtr;

         else
            break;
      }
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       KillProposed

   FUNCTION:   change the status of all trades from PROPOSED to REJECTED

   INPUT:      trades = linked list of all trades

   RETURNS:    any pending trades are no longer pending
   ---------------------------------------------------------------------
*/

void
TradeNews::KillProposed ()
{
   if (!trades.start ())
   {
      while (TRUE)
      {
         Trade    *tPtr = trades ();

         if (tPtr->GetTSData (TRADE_FROM, TRADEINFODATA_STATUS) == TRADE_PROPOSED)
            tPtr->SetTSData (TRADE_REJECTED);

         if (++trades)
            break;
      }
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       KillPTrades

   FUNCTION:   if a player is somehow released to free agency,
               any pending trades of that player are changed to REJECTED

   INPUT:      pId = id of player going to free agent list

   RETURNS:    TRUE/FALSE if any trades had to change status
   ---------------------------------------------------------------------
*/

DYN_16
TradeNews::KillPTrades (DYN_16 pId)
{
   DYN_16   modified = FALSE;

   if (!trades.start ())
   {
      while (TRUE)
      {
         Trade    *tPtr = (Trade *) trades ();

         DYN_16   p1Ids [TradeInfo::MAX_PIDS],
                  p2Ids [TradeInfo::MAX_PIDS];

         tPtr->GetPIds (p1Ids, p2Ids);
         if ((pId == p1Ids [0]) || (pId == p2Ids [0]))
         {
            TradeStatus status = (TradeStatus) tPtr->GetTSData (TRADE_FROM, TRADEINFODATA_STATUS);

            // if trade is still pending:
            if (status != TRADE_ACCEPTED)
            {
               // trade is DELETED if FROM team trades the participant
               if (pId == p1Ids [0])
                  trades.linkout ();

               // trade is REJECTED if TO team trades the participant
               else
                  tPtr->SetTSData (TRADE_REJECTED);

               modified = TRUE;
            }
         }

         if (++trades)
            break;
      }
   }

   return modified;
}








/*
   =====================================================================
                           PRIVATE METHODS
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       Load

   FUNCTION:   if the file exists, read in the list of trades for the league

   INPUT:      pName-> string of path
               fName-> string of file to open & read

   RETURNS:    trades linked list created
   ---------------------------------------------------------------------
*/

void
TradeNews::Load (DYN_8 *pName, DYN_8 *fName)
{
   ReadBFile   rlf;
   if (rlf.Open (makeFileName (0, pName, fName, trnText)) > 0)
   {
      if (rlf.Find (TRNText, 1) != -1)
      {
         DYN_16   numTrades;
         rlf.Read (&numTrades, sizeof (DYN_16));

         while (numTrades--)
         {
            Trade    *tPtr = new Trade (rlf);
            if (tPtr)
               trades.linkin (tPtr);
         }
      }
   }
}


/*
   ---------------------------------------------------------------------
   NAME:       PlayerInvolved

   FUNCTION:   check if any player (who is still on the same team)
               is involved in the given trade)

   INPUT:      tId = id of team of interest
               thisId = id of team in THIS TRADE data
               pIds-> player ids of team of interest
               thisPIds-> player ids of THIS TRADE data

   RETURNS:    TRUE/FALSE if any players are involved
   ---------------------------------------------------------------------
*/

DYN_32
TradeNews::PlayerInvolved (DYN_16 tId, DYN_16 thisId,
                           DYN_16 *pIds, DYN_16 *thisPIds)
{
   DYN_32   involved = FALSE;

   if (tId == thisId)
   {
      DYN_32   i = TradeInfo::MAX_PIDS;
      while (!involved && i--)
      {
         // test each player id in the team of interest
         DYN_16   pId = *pIds++;

         if (pId)
         {
            // look for a match in the array of pIds in the current trade
            DYN_16   *tpPtr = thisPIds;

            DYN_32   i = TradeInfo::MAX_PIDS;
            while (i--)
            {
               DYN_16   thisId = *tpPtr++;
               if (thisId && (thisId == pId))
               {
                  involved = TRUE;
                  break;
               }
            }
         }
      }
   }

   return involved;
}

/*
   ---------------------------------------------------------------------
   NAME:       TheTrade

   FUNCTION:   determine if this is the trade that matches

   INPUT:      tId = id of team of interest
               thisId = id of team in THIS TRADE data
               pIds-> player ids of team of interest
               thisPIds-> player ids of THIS TRADE data

   RETURNS:    TRUE/FALSE if this matches exactly
   ---------------------------------------------------------------------
*/

DYN_32
TradeNews::TheTrade (DYN_16 t1Id, DYN_16 this1Id,
                     DYN_16 t2Id, DYN_16 this2Id,
                     DYN_16 *p1Ids, DYN_16 *this1PIds,
                     DYN_16 *p2Ids, DYN_16 *this2PIds)
{
   const DYN_16   CMPSIZE = TradeInfo::MAX_PIDS * sizeof (DYN_16);
   DYN_32         theTrade = FALSE;

   if ((t1Id == this1Id) && (t2Id == this2Id))
   {
      if (!memcmp (p1Ids, this1PIds, CMPSIZE) &&
            !memcmp (p2Ids, this2PIds, CMPSIZE))
         theTrade = TRUE;
   }

   else if ((t1Id == this2Id) && (t2Id == this1Id))
   {
      if (!memcmp (p1Ids, this2PIds, CMPSIZE) &&
            !memcmp (p2Ids, this1PIds, CMPSIZE))
         theTrade = TRUE;
   }

   return theTrade;
}







/*
   =====================================================================
                           LOCAL UTILITIES
   =====================================================================
*/
