/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   trade.cpp

   GJW: 5-14-1992, original creation

  CAM	17 DEC 96	Added TOManager::Sort
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   #includes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#include <string.h>

#include <ports.h>
#include "dyndefs.h"
#include "vm_mylib.h"
#include "LcdtUtil.h"
#include "Trade.h"
#include "DList.h"
#include "Fbmufunc.h"
#include "RosPlyr.h"

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

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/*
   ---------------------------------------------------------------------
   NAME:          Trade

   FUNCTION:      Trade object CONSTRUCTOR

   INPUT:         t1Id, p1Id, t2Id, p2Id = team and player identifiers
                  trade gets created (team 1 proposed trade to team 2, team
                  2 id may be 0, in which case the destination is the
                  free agent roster)

   RETURNS:       -> created Trade object
   ---------------------------------------------------------------------
*/

Trade::Trade (DYN_16 t1Id, DYN_16 *p1Ids,
               DYN_16 t2Id, DYN_16 *p2Ids,
               TradeStatus stat)
{
   memset (&tradeData, 0, sizeof (tradeData));

   tradeData [TRADE_FROM].tId = (DYN_8) t1Id;
   tradeData [TRADE_TO].tId = (DYN_8) t2Id;

   DYN_16   *dPtr = tradeData [TRADE_FROM].pIds,
            *sPtr = p1Ids,
            numP = TradeInfo::MAX_PIDS;
   while (numP--)
   {
      if (*sPtr)
      {
         *dPtr++ = *sPtr++;
      }

      else
         break;
   }

   dPtr = tradeData [TRADE_TO].pIds;
   sPtr = p2Ids;
   numP = TradeInfo::MAX_PIDS;
   while (numP--)
   {
      if (*sPtr)
      {
         *dPtr++ = *sPtr++;
      }

      else
         break;
   }

   status = stat;
}

Trade::Trade (ReadBFile &rlf)
{
   rlf.Read (this, sizeof (Trade));
}

/*
   ---------------------------------------------------------------------
   NAME:          ~Trade

   FUNCTION:      Trade object DESTRUCTOR

   INPUT:         -> Trade object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

Trade::~Trade ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:       TOManager

   FUNCTION:   TradeOfferManager object CONSTRUCTOR

   INPUT:      none

   RETURNS:    data area allocated
   ---------------------------------------------------------------------
*/

TOManager::TOManager ()
   :  numTis (0),
      maxTis (MAX_OFFERS)
{
   tis = (TradeOffer *) new DYN_8 [maxTis * sizeof (TradeOffer)];

	 // -- Clear me up
	memset( tis, 0, maxTis * sizeof (TradeOffer) );
}

TOManager::~TOManager ()
{
   delete [] tis;
   tis = 0;
}







/*
   =====================================================================
                           PUBLIC METHODS
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       GetSetTSData

   FUNCTION:   return info to the caller
               (Set can only set new status for trade)

   INPUT:      tsData = which side (FROM/TO)
               tiData = what data wanted

   RETURNS:    data requested
   ---------------------------------------------------------------------
*/

DYN_16
Trade::GetTSData (const TradeSideData tsData, const TradeInfoData tiData) const
{
   const TradeInfo   *tiPtr = &tradeData [tsData];
   DYN_16            value;

   switch (tiData)
   {
      case TRADEINFODATA_TEAMID:
         value = tiPtr->tId;
         break;

      case TRADEINFODATA_PLAYER0ID:
         value = tiPtr->pIds [0];
         break;

      case TRADEINFODATA_PLAYER1ID:
         value = tiPtr->pIds [1];
         break;

      case TRADEINFODATA_PLAYER2ID:
         value = tiPtr->pIds [2];
         break;

      case TRADEINFODATA_STATUS:
         value = status;
         break;
   }

   return value;
}

void Trade::SetTSData (const TradeStatus newStatus)
{
   status = newStatus;
}

/*
   ---------------------------------------------------------------------
   NAME:       Save

   FUNCTION:   save this trade's info to disk
               tId      = sizeof (DYN_8)
               pId      = sizeof (DYN_16)
               tId      = sizeof (DYN_8)
               pId      = sizeof (DYN_16)
               status   = sizeof (DYN_8)

   INPUT:      fp -> file to write to
               -> trade object

   RETURNS:    file updated
   ---------------------------------------------------------------------
*/

void Trade::Save (WriteBFile &wlf)
{
   wlf.Write (this, sizeof (Trade));
}

/*
   ---------------------------------------------------------------------
   NAME:       GetPIds

   FUNCTION:   fill in caller's arrays with player ids in this trade

   INPUT:      p1Ids, p2Ids-> array of shorts to fill in

   RETURNS:    caller's arrays set
   ---------------------------------------------------------------------
*/

void Trade::GetPIds (DYN_16 *p1Ids, DYN_16 *p2Ids) const
{
   if (p1Ids)
      memcpy (p1Ids, tradeData [0].pIds, sizeof (tradeData [0].pIds));

   if (p2Ids)
      memcpy (p2Ids, tradeData [1].pIds, sizeof (tradeData [1].pIds));
}




/*
   ---------------------------------------------------------------------
   NAME:       AddTrade

   FUNCTION:   add trade offer to the array of trade offers
               don't add a trade that duplicates another

   INPUT:      to = reference to one to add

   RETURNS:    TRUE/FALSE if room in array to add to
   ---------------------------------------------------------------------
*/

DYN_32 TOManager::AddTrade (TradeOffer &to)
{
   DYN_32   ok = FALSE;

   if (numTis < maxTis)
   {
      ok = TRUE;

      DYN_32      tId = to.tId;

      TradeOffer  *toPtr = tis;
      DYN_32      nT = numTis;
      while (nT--)
      {
         if (toPtr->tId == tId)
         {
            // search for all player id's in the destination
            DYN_32   fCount = 0;
            for (DYN_32 i = 0; i < TradeInfo::MAX_PIDS; i++)
            {
               RosPlayer   *pPtr = to.pPtrs [i];

               if (pPtr)
               {
                  fCount++;   // another needs finding
                  for (DYN_32 i = 0; i < TradeInfo::MAX_PIDS; i++)
                  {
                     if (pPtr == toPtr->pPtrs [i])
                     {
                        fCount--;   // another has been found
                        break;
                     }
                  }
               }
            }

            if (!fCount)
            {
               ok = FALSE;
               break;
            }
         }

         toPtr++;
      }

      if (ok)
         tis [numTis++] = to;
   }

   return ok;
}

/*
   ---------------------------------------------------------------------
   NAME:       

   FUNCTION:   

   INPUT:      

   RETURNS:    
   ---------------------------------------------------------------------
*/

DYN_32 TOManager::GetTrade (TradeOffer &to, const DYN_32 index)
{
   DYN_32   ok = (index >= 0) && (index < numTis);

   if (ok)
      to = tis [index];

   return ok;
}

/*
   ---------------------------------------------------------------------
   NAME:       GetNumTrades

   FUNCTION:   

   INPUT:      

   RETURNS:    
   ---------------------------------------------------------------------
*/

DYN_32 TOManager::GetNumTrades ()
{
   return numTis;
}

/*
   ---------------------------------------------------------------------
   NAME:       InitOffer

   FUNCTION:   initialize caller's TradeOffer object

   INPUT:      to = reference to object to initialize
               t = team's id

   RETURNS:    to's id set, player pointers cleared
   ---------------------------------------------------------------------
*/

void TOManager::InitOffer (TradeOffer &to, const DYN_32 t)
{
   to.tId = t;
   to.pPtrs [0] = to.pPtrs [1] = to.pPtrs [2] = 0;
}




DYN_16 TOCompare1(void *i1, void *i2, DYN_16 sortType )
{
	TradeOffer *pr1 = (TradeOffer *) i1;
	TradeOffer *pr2 = (TradeOffer *) i2;
	DYN_16 iAns = 0;

	switch ( sortType )
	{
	case TOManager::ST_VALUE:
		if ( pr1->pPtrs[0]->SumAbrs( pr1->pPtrs[1], pr1->pPtrs[2] ) >=
					pr2->pPtrs[0]->SumAbrs( pr2->pPtrs[1], pr2->pPtrs[2] ) )
		{
			iAns = 0;
		}
		else
		{
			iAns = 1;
		}
		break;

	default:
		break;
	}

	return ( iAns );
} // TOCompare1



// -------------------------------------------------------------------------
//
//	TOManager::Sort
//		Sort the potential trades by the designated SortType
//
//	Input:
//		st		- SortType
//
//	Results:	none
//
// -------------------------------------------------------------------------
void TOManager::Sort( const SortType st )
{
	ArraySort( (void **) tis, numTis, st, TOCompare1, sizeof(TradeOffer) );
} // TOManager::Sort





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