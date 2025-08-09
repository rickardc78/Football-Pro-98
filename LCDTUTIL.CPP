/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   lcdtutil.c

   league/conference/division/team utilities

   some taken from other existing files:
   port.c

   GJW: 1-15-1992, original creation
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   #includes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

//#include       <alloc.h>
//#include       <dir.h>
#include       <stdio.h>
#include       <string.h>

#include       <ports.h>
#include       "dyndefs.h"

#include       "assertvm.h"
#include       "league.h"
#include       "lcdtutil.h"
#include       "fbmu.h"

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   #defines
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

const COPY_BLOCK_SIZE = 10;   // 2 << 10 = 1024

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   function prototypes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   global data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   local data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */
#if   (LCDT == 0)

INCLUDE_ASSERT;

#endif

#if   (LCDT == 0)

static DYN_8   *posText [] =
               {
                  "QB",
                  "FB", "HB",
                  "TE", "WR",
                  "C", "G", "T",
                  "DE", "DT",
                  "LB",
                  "CB", "S",
                  "K",
                  "P",

                  "RB",
                  "REC",
                  "OL",
                  "DL",
                  "DB",
               },

               anyText [] = "Any";

#endif

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#if   (LCDT == 0)
/*
   ---------------------------------------------------------------------
   NAME:       addPlayerList

   FUNCTION:   add the players in the given trade to playerList (unless
               the players are already in the list)

   INPUT:      tPtr-> Trade object (to get player ids from)

   RETURNS:    playerList a little bigger
   ---------------------------------------------------------------------
*/

static void NEAR
addPlayerList (GenList<RosPlayer> *pList, Trade *tPtr)
{
   // first, fill pIds with the ids of ALL players being traded
   DYN_16   pIds [TRADE_MAX * TradeInfo::MAX_PIDS],
            *pIdPtr = pIds,
         numP = 0;
   memset (pIds, 0, sizeof (pIds));

   for (TradeSideData tds = TRADE_FROM; tds < TRADE_MAX;
         tds = (TradeSideData) (tds + 1))
   {
      for (TradeInfoData tId = TRADEINFODATA_PLAYER0ID;
            tId < TRADEINFODATA_STATUS;
            tId = (TradeInfoData) (tId + 1))
      {
         DYN_16   pId = tPtr->GetTSData (tds, tId);
         if (pId)
         {
            *pIdPtr++ = pId;
            numP++;
         }
      }
   }

   // search the player list for ANY of these ids
   if (!pList->start ())
   {
      // keep looking 'til ALL players in list have been gone through,
      // or part of trade have been found
      DYN_16   numLeftToFind = numP;
      while (numLeftToFind)
      {
         // get next player in list (and the id)
         RosPlayer   *pPtr = (RosPlayer *) (*pList) ();
         DYN_16      pId = pPtr->GetId (),
                     nP = numP;

         // is this player part of the given trade?
         pIdPtr = pIds;
         while (nP--)
         {
            // if so, this player won't need to be added at the end
            if (*pIdPtr == pId)
            {
               // won't need to generate player of this id
               *pIdPtr = 0;

               numLeftToFind--;
               break;
            }

            pIdPtr++;
         }

         if (++(*pList))
            break;
      }
   }

   // if ANY player is new, save pointer to him
   pIdPtr = pIds;
   while (numP--)
   {
      // i.e. any player NOT found, will NOT have had id cleared from array
      if (*pIdPtr)
         pList->linkin (ActiveLeague->MakePlayer (*pIdPtr));

      pIdPtr++;
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       build/deletePlayerList

   FUNCTION:   create or destroy the list of ALL players involved in
               a trade

               GetPlayer () will create a player if not already created as
               part of a roster or free agent list. Therefore it is the
               responsibility of this module to delete any thusly created
               players (their status is _NONE)

   INPUT:      tradeList-> list of ALL trades if building
               playerList-> list of ALL players if deleting

   RETURNS:    playerList modified
   ---------------------------------------------------------------------
*/

GenList<RosPlayer> *
buildTradePlayerList (GenList<Trade> *tList)
{
   GenList<RosPlayer>   *pList = new GenList<RosPlayer>;

   vmAssert (pList);
   if (!tList->start ())
   {
      while (TRUE)
      {
         addPlayerList (pList, (*tList) ());
         if (++(*tList))
            break;
      }
   }

   return pList;
}

void
deleteTradePlayerList (GenList<RosPlayer> *pList)
{
   if (!pList->start ())
   {
      while (TRUE)
      {
         RosPlayer   *pPtr = (RosPlayer *) pList->linkout ();

         if (pPtr)
         {
            if (pPtr->GetStatus () == RosPlayer::POS_NONE)
               delete pPtr;
         }

         else
            break;
      }
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       fillTradeBuffer

   FUNCTION:   create a string describing a trade

   INPUT:      dest-> where to build string
               t1Ptr, p1Ptr, t2Ptr, p2Ptr -> teams & players involved

   RETURNS:    dest filled with a string
   ---------------------------------------------------------------------
*/

void
fillTradeBuffer (DYN_8 *dest,
                  Team *t1Ptr, RosPlayer **p1Ptrs,
                  Team *t2Ptr, RosPlayer **p2Ptrs)
{
   // team to team
   // if (t1Ptr && t2Ptr)
	{
      // build the string a piece at a time (sprintf returns the number
      // of bytes put into dest)
      DYN_16   numBytes = sprintf (dest, "%Fs ", t1Ptr->GetAbbr ());
      for (DYN_32 i = 0; i < TradeInfo::MAX_PIDS; i++)
      {
         RosPlayer   *pPtr = p1Ptrs [i];
         if (pPtr)
         {
            numBytes += sprintf (&dest [numBytes], "%s %Fs",
                        posText [pPtr->GetPosSpec ()],
                        pPtr->GetLastName ());

            if ((i < TradeInfo::MAX_PIDS - 1) && p1Ptrs [i + 1])
               numBytes += sprintf (&dest [numBytes], ",");
            numBytes += sprintf (&dest [numBytes], " ");
         }
      }

      numBytes += sprintf (&dest [numBytes], "for %Fs", t2Ptr->GetAbbr ());
      for (i = 0; i < TradeInfo::MAX_PIDS; i++)
      {
         RosPlayer   *pPtr = p2Ptrs [i];
         if (pPtr)
         {
            numBytes += sprintf (&dest [numBytes], " %s %Fs",
                        posText [pPtr->GetPosSpec ()],
                        pPtr->GetLastName ());

            if ((i < TradeInfo::MAX_PIDS - 1) && p2Ptrs [i + 1])
               numBytes += sprintf (&dest [numBytes], ", ");
         }
      }
	}

   // free agent signing
   // else if (!t1Ptr)
   //    sprintf (dest, "%Fs Signs Free Agent %s %Fs",
   //             t2Ptr->GetAbbr (),
   //             posText [p2Ptr->GetPosSpec ()],
   //             p2Ptr->GetLastName ());
}

/*
   ---------------------------------------------------------------------
   NAME:       fillTradeBuffer

   FUNCTION:   create a string describing a trade

   INPUT:      dest-> where to build string
               tradePtr-> trade object containing teams & players involved
               playerList-> list of ALL players involved in trades
               verbose = include the status words?

   RETURNS:    dest filled with a string
   ---------------------------------------------------------------------
*/

void
fillTradeBuffer (GenList<RosPlayer> *pList,
                  DYN_8 *dest,
                  Trade *tradePtr,
                  DYN_16 verbose)
{
   Team        *t1Ptr = ActiveLeague->GetTeam ((DYN_8) tradePtr->GetTSData (TRADE_FROM, TRADEINFODATA_TEAMID)),
               *t2Ptr = ActiveLeague->GetTeam ((DYN_8) tradePtr->GetTSData (TRADE_TO, TRADEINFODATA_TEAMID));
   DYN_16      pIds [TRADE_MAX][TradeInfo::MAX_PIDS],
               numP = 0;
   for (TradeSideData tsd = TRADE_FROM; tsd < TRADE_MAX;
         tsd = TradeSideData (tsd + 1))
   {
      DYN_16   *pIdPtr = (DYN_16 *) &pIds [tsd];
      for (TradeInfoData tid = TRADEINFODATA_PLAYER0ID; tid < TRADEINFODATA_STATUS;
            tid = TradeInfoData (tid + 1))
      {
         DYN_16   pId = tradePtr->GetTSData (tsd, tid);
         if (pId)
         {
            numP++;
            *pIdPtr++ = pId;
         }
      }
   }

   // find ALL the player pointers from the list of players
   DYN_16      p1Index = 0,
               p2Index = 0;
   RosPlayer   *p1Ptrs [TradeInfo::MAX_PIDS],
               *p2Ptrs [TradeInfo::MAX_PIDS];
   memset (p1Ptrs, 0, sizeof (p1Ptrs));
   memset (p2Ptrs, 0, sizeof (p2Ptrs));

   // keep looking 'til ALL are found
   pList->start ();
   while (numP)
   {
      RosPlayer   *pPtr = (RosPlayer *) (*pList) ();
      DYN_16      pId = pPtr->GetId ();

      for (DYN_16 tsd = TRADE_FROM; tsd < TRADE_MAX; tsd++)
      {
         DYN_16   *pIdPtr = (DYN_16 *) &pIds [tsd];
         for (DYN_16 tid = TRADEINFODATA_PLAYER0ID;
               tid < TRADEINFODATA_STATUS;
               tid++, pIdPtr++)
         {
            // player has been found!
            if (*pIdPtr == pId)
            {
               if (tsd == TRADE_FROM)
                  p1Ptrs [p1Index++] = pPtr;

               else
                  p2Ptrs [p2Index++] = pPtr;
            }
         }
      }

      if (++(*pList))
         break;
   }

   fillTradeBuffer (dest, t1Ptr, p1Ptrs, t2Ptr, p2Ptrs);

   // tack on status description (but not for free agent stuff)
   if (verbose)
      strcat (dest,
         tradeText [tradePtr->GetTSData (TRADE_FROM, TRADEINFODATA_STATUS)]);
}

/*
   ---------------------------------------------------------------------
   NAME:       fillTradeBuffer

   FUNCTION:   create a string describing a trade

   INPUT:      dest-> where to build string
               t1Ptr, p1Ptr, t2Ptr, p2Ptr -> teams & players involved

   RETURNS:    dest filled with a string
   ---------------------------------------------------------------------
*/

void
fillTradeBuffer (DYN_8 *dest, RosPlayer **pPtrs)
{
   DYN_16   numBytes = 0;
   for (DYN_32 i = 0; i < TradeInfo::MAX_PIDS; i++)
   {
      RosPlayer   *pPtr = pPtrs [i];
      if (pPtr)
      {
         numBytes += sprintf (&dest [numBytes], " %s %Fs",
                     posText [pPtr->GetPosSpec ()],
                     pPtr->GetLastName ());

         if ((i < TradeInfo::MAX_PIDS - 1) && pPtrs [i + 1])
            numBytes += sprintf (&dest [numBytes], ", ");
      }
   }
}
#endif

#if   (LCDT == 0)
/*
   ---------------------------------------------------------------------
   NAME:       getSpecPosText

   FUNCTION:   return string corresponding to specific position

   INPUT:      pos = player's specific position

   RETURNS:    -> string
   ---------------------------------------------------------------------
*/

DYN_8 *
getSpecPosText (DYN_16 pos)
{
   DYN_8 *ptr = 0;

   if (pos == RosPlayer::POS_NONE)
      ptr = anyText;

   else if (pos < RosPlayer::POS_MAX)
      ptr = posText [pos];

   return ptr;
}
#endif







/*
   =====================================================================
                           LOCAL UTILITIES
   =====================================================================
*/
