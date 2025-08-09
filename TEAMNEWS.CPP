/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   teamnews.cpp

   this file contains ENGLISH text
   string for announcing career ending injuries

   GJW: 12-08-1992, original creation
   GJW: 6-17-1993, Retired player information is very much like CEI
                     (-1, -1 is stored in .tmn file to indicate start of
                     retired data)

  CAM		10 NOV 96	Put trades first (again) due to selection problems

컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   #includes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#include       <io.h>
#include       <string.h>

#include       <ports.h>
#include       "dyndefs.h"

#include    "vm_mylib.h"

#include       "assertvm.h"
#include       "fileutil.h"
#include       "game.h"
#include       "league.h"
#include       "teamnews.h"
#include       "lcdtutil.h"
#include       "fbmu.h"

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
INCLUDE_ASSERT;

static DYN_8   tmnText [] = ".tmn",
               ceiText [] = "Career Ending Injury for",
               retiredText [] = "has retired after",
               seasonText [] = "season",
               seasonsText [] = "seasons";

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/*
   ---------------------------------------------------------------------
   NAME:          TeamNews

   FUNCTION:      TeamNews object CONSTRUCTOR

   INPUT:         pName, fName-> string for league filename path
                  tId = team id (if 0, called by league for data file
                     writing)

   RETURNS:       -> created Team object
   ---------------------------------------------------------------------
*/

TeamNews::TeamNews (DYN_8 *pName, DYN_8 *fName, DYN_16 tId)
{
   #if   (LCDT == 0)

   if (tId)
   {
      // first in list are trades
      TNBase   *tPtr = new TNTrade (tId);
      vmAssert (tPtr);
      news.linkin (tPtr);

      // then how the team did last week
      tPtr = new TNGame (tId);
      vmAssert (tPtr);
      news.linkin (tPtr);

      // then any career ending injury players
      FILE  *fp = my_fopen (makeFileName (0, pName, fName, tmnText), "rb");
      tPtr = new TNCEI (tId, fp);
      vmAssert (tPtr);
      news.linkin (tPtr);
      tPtr = new TNRetired (tId, fp);
      vmAssert (tPtr);
      news.linkin (tPtr);
      if (fp)
         my_fclose (fp);
   }
   #endif
}

/*
   ---------------------------------------------------------------------
   NAME:          ~TeamNews

   FUNCTION:      TeamNews object DESTRUCTOR

   INPUT:         -> TeamNews object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

TeamNews::~TeamNews ()
{
   #if   (LCDT == 0)

   if (!news.start ())
   {
      while (TRUE)
      {
         TNBase   *tntPtr = (TNBase *) news.linkout ();

         if (tntPtr)
            delete tntPtr;

         else
            break;
      }
   }
   #endif
}

#if   (LCDT == 0) || (LCDT == 4)
/*
   ---------------------------------------------------------------------
   NAME:       TNBase/Trade/Game/CEI

   FUNCTION:   team news constructors

   INPUT:      tId = team id

   RETURNS:    team news item constructed
   ---------------------------------------------------------------------
*/

TNBase::TNBase ()
{
}

//#pragma  argsused
TNBase::TNBase (DYN_16 tId)
{
}

TNBase::~TNBase ()
{
}

TNTrade::TNTrade (DYN_16 tId)
{
   #if   (LCDT == 0)

   tn = ActiveLeague->LoadTrades ();

   tn->BuildTradeList (&tradeList, TRADE_PROPOSED, tId);
   tn->BuildTradeList (&tradeList, TRADE_ACCEPTED, tId);
   tn->BuildTradeList (&tradeList, TRADE_REJECTED, tId);

   pList = buildTradePlayerList (&tradeList);
   #endif
}

TNTrade::~TNTrade ()
{
   #if   (LCDT == 0)

   delete tn;

   deleteTradePlayerList (pList);
   delete pList;
   #endif
}

TNGame::TNGame (DYN_16 tId)
{
   #if   (LCDT == 0)

   buffer [0] = 0;

   Schedule *sched = ActiveLeague->GetSchedule ();
   DYN_16   weekNum = sched->GetWData (SCDWEEKDATA_CUR_WEEK, 0);
   if (weekNum <= 0)
   {
      if (ActiveLeague->GetStage () == STAGE_PLAYOFFS_END)
         weekNum = sched->GetWData (SCDWEEKDATA_NUM_WEEKS, 0) - 1;
   }

   // if schedule started, and all the games have been done,
   // need to get LAST week's games (since none are ready to show for
   // this one)
   else if (!sched->GamesDoneThisWeek ())
      weekNum--;

   DYN_16   gameNum = sched->FindTeamsGame ((DYN_8)tId, (DYN_8)weekNum);
   if ((gameNum != -1) &&
      (sched->GetSGData (SGDATA_STATUS) == SCHED_DONE))
   {
      DYN_8    pfH = (DYN_8)sched->GetSTData (STDATA_PF, Game::TEAM_HOME),
               pfV = (DYN_8)sched->GetSTData (STDATA_PF, Game::TEAM_VISIT);
      DYN_16   wIndex,
               lIndex;
      Team     *hTeam = ActiveLeague->GetTeam ((DYN_8)sched->GetSTData (STDATA_ID, Game::TEAM_HOME)),
               *vTeam = ActiveLeague->GetTeam ((DYN_8)sched->GetSTData (STDATA_ID, Game::TEAM_VISIT)),
               *wTeam,
               *lTeam;

      // print home team first, ONLY if they win
      if (pfH > pfV)
      {
         wIndex = Game::TEAM_HOME;
         lIndex = Game::TEAM_VISIT;
         wTeam = hTeam;
         lTeam = vTeam;
      }

      else
      {
         wIndex = Game::TEAM_VISIT;
         lIndex = Game::TEAM_HOME;
         wTeam = vTeam;
         lTeam = hTeam;
      }

      sprintf (buffer, "%s %d, %s %d%s",
                  wTeam->GetName (),
                  sched->GetSTData (STDATA_PF, wIndex),
                  lTeam->GetName (),
                  sched->GetSTData (STDATA_PF, lIndex),
                  sched->GetSGData (SGDATA_OT) ? otText : "");
   }
   #endif
}

TNGame::~TNGame ()
{
}

TNCEI::TNCEI (const DYN_16 tId, FILE *fp)
{
   if (fp)
   {
      while (TRUE)
      {
         CEIDiskData    cdd;
         if (my_fread (&cdd, sizeof (CEIDiskData), 1, fp) != 1)
            break;

         // Retired data follows
         if (cdd.tId == -1)
            break;

         // if THIS team lost a CEI player,
         // store a pointer to the player in the list
         if (cdd.tId == tId)
            pList.linkin (ActiveLeague->MakePlayer (cdd.pId));
      }
   }
}

TNCEI::~TNCEI ()
{
   if (!pList.start ())
   {
      while (TRUE)
      {
         RosPlayer   *pPtr = (RosPlayer *) pList.linkout ();
         if (pPtr)
            delete pPtr;
         else
            break;
      }
   }
}

TNRetired::TNRetired (const DYN_16 tId, FILE *fp)
{
   if (fp)
   {
      while (TRUE)
      {
         CEIDiskData    cdd;
         if (my_fread (&cdd, sizeof (CEIDiskData), 1, fp) != 1)
            break;

         // if THIS team lost a CEI player,
         // store a pointer to the player in the list
         if (cdd.tId == tId)
            pList.linkin (ActiveLeague->MakePlayer (cdd.pId));
      }
   }
}

TNRetired::~TNRetired ()
{
   if (!pList.start ())
   {
      while (TRUE)
      {
         RosPlayer   *pPtr = (RosPlayer *) pList.linkout ();
         if (pPtr)
            delete pPtr;
         else
            break;
      }
   }
}

TNPSaver::TNPSaver ()
{
}

TNPSaver::~TNPSaver ()
{
   if (!idList.start ())
   {
      while (TRUE)
      {
         CEIDiskData *cddPtr = idList.linkout ();
         if (cddPtr)
            delete cddPtr;

         else
            break;
      }
   }
}

void TNPSaver::Add (const DYN_16 tId, const DYN_16 pId)
{
   CEIDiskData *cddPtr = new CEIDiskData ();
   cddPtr->tId = (char)tId;
   cddPtr->pId = pId;
   idList.linkin (cddPtr);
}

void
TNPSaver::Save (FILE *fp)
{
   if (!idList.start ())
   {
      while (TRUE)
      {
         CEIDiskData *cddPtr = idList ();
         my_fwrite (cddPtr, sizeof (CEIDiskData), 1, fp);

         if (++idList)
            break;
      }
   }
}
#endif









/*
   =====================================================================
                           PUBLIC METHODS
   =====================================================================
*/

#if   (LCDT == 0)
/*
   ---------------------------------------------------------------------
   NAME:       Flush

   FUNCTION:   when starting a new week, kill all team news

   INPUT:      pName, fName-> string for league filename path,

   RETURNS:    league.tmn destroyed
   ---------------------------------------------------------------------
*/

void TeamNews::Flush (const DYN_8 *pName, const DYN_8 *fName)
{
   unlink (makeFileName (0, (DYN_8 *) pName, (DYN_8 *) fName, tmnText));
}

/*
   ---------------------------------------------------------------------
   NAME:       GetPList

   FUNCTION:   return pointer to the player list
               (those involved in trades, or CEIs)

   INPUT:      none

   RETURNS:    -> list of players involved in team news
   ---------------------------------------------------------------------
*/

GenList<RosPlayer> *TeamNews::GetPList ()
{
   TNTrade  *tntPtr = (TNTrade *) GetTNItem (TNI_TRADES);
   return tntPtr->pList;
}

/*
   ---------------------------------------------------------------------
   NAME:       GetTradeList

   FUNCTION:   return a pointer to the trade list of the team news

   INPUT:      none

   RETURNS:    -> trade list
   ---------------------------------------------------------------------
*/

GenList<Trade> *TeamNews::GetTradeList ()
{
   // get TNTrade object from news list
   TNTrade  *tntPtr = (TNTrade *) GetTNItem (TNI_TRADES);
   return &tntPtr->tradeList;
}

/*
   ---------------------------------------------------------------------
   NAME:       GetTrade

   FUNCTION:   return pointer to a particular trade

   INPUT:      index = which trade number wanted

   RETURNS:    -> to the given trade (0 means no trade at that spot)
   ---------------------------------------------------------------------
*/

Trade *TeamNews::GetTrade (const DYN_16 index)
{
   return GetTrade ((TNTrade *) GetTNItem (TNI_TRADES), index);
}

/*
   ---------------------------------------------------------------------
   NAME:       FillString

   FUNCTION:   copy the team news info into the given buffer

   INPUT:      fillPtr-> where to put string
               index = which news to create

   RETURNS:    0 if no news for this line
   ---------------------------------------------------------------------
*/

DYN_8 *TeamNews::FillString (DYN_8 *fillPtr, DYN_16 index)
{
   DYN_8    *ptr = 0;

   // give trades the first shot at filling
   TNTrade  *tntPtr = (TNTrade *) GetTNItem (TNI_TRADES);
   Trade    *tPtr = GetTrade (tntPtr, index);
   if (tPtr)
   {
      ptr = fillPtr;
      fillTradeBuffer (GetPList (), ptr, tPtr, TRUE);
   }

   else
   {
      // if no trade for that index, try the score for the last game
      index -= tntPtr->tradeList.size ();

      TNGame   *tngPtr = (TNGame *) GetTNItem (TNI_GAME);
      if (GetGame (tngPtr, fillPtr, index))
         ptr = fillPtr;

      else
      {
	      // try for a Career Ending Injury (adjusting index to 0 based again)
         index -= tngPtr->Size ();

         RosPlayer   *pPtr = GetCEI (index);
         if (pPtr)
         {
            ptr = fillPtr;
            sprintf (ptr, "%s %s %s %s",
                              ceiText,
                              getSpecPosText (pPtr->GetPosSpec ()),
                              pPtr->GetFirstName (),
                              pPtr->GetLastName ());
         }

         else
         {
            TNCEI *tncPtr = (TNCEI *) GetTNItem (TNI_CEI);
            index -= tncPtr->pList.size ();

            RosPlayer   *pPtr = GetRetired (index);
            if (pPtr)
            {
               ptr = fillPtr;
               DYN_16   yrs = pPtr->GetYears ();
               sprintf (ptr, "%s %s %s %s %d %s",
                                 getSpecPosText (pPtr->GetPosSpec ()),
                                 pPtr->GetFirstName (),
                                 pPtr->GetLastName (),
                                 retiredText,
                                 yrs,
                                 yrs == 1 ? seasonText : seasonsText);
            }
         }
      }
   }

   return ptr;
}

/*
   ---------------------------------------------------------------------
   NAME:       Size

   FUNCTION:   how many team news items are there?

   INPUT:      tradeList-> list of trades for this team

   RETURNS:    sum of all team news objects
   ---------------------------------------------------------------------
*/

DYN_16 TeamNews::Size ()
{
   TNTrade     *tntPtr = (TNTrade *) GetTNItem (TNI_TRADES);
   TNGame      *tngPtr = (TNGame *) GetTNItem (TNI_GAME);
   TNCEI       *tncPtr = (TNCEI *) GetTNItem (TNI_CEI);
   TNRetired   *tnrPtr = (TNRetired *) GetTNItem (TNI_RETIRED);
   return tntPtr->tradeList.size () +
            tngPtr->Size () +
            tncPtr->pList.size () +
            tnrPtr->pList.size () +
            0;
}
#endif

#if   (LCDT == 4)
/*
   ---------------------------------------------------------------------
   NAME:       AddNews

   FUNCTION:   store another player who has a career-ending injury

   INPUT:      pName, fName-> string for league filename path,
               tId = team id
               pId = player's id

   RETURNS:    league.tmn a little bigger
   ---------------------------------------------------------------------
*/

void TeamNews::AddNews (const DYN_16 tId, const DYN_16 pId)
{
   TNPSaver *tnpPtr;

   // if no Retire object yet, create it (making it THE ONLY TNBase object
   // in the list!)
   if (!news.size ())
   {
      tnpPtr = new TNPSaver ();
      vmAssert (tnpPtr);
      news.linkin (tnpPtr);
   }

   // since it IS THE ONLY TNBase object in the list (news),
   // get pointer to it via the 'wrong' enum
   else
      tnpPtr = (TNPSaver *) GetTNItem (TNI_PSAVER);

   tnpPtr->Add (tId, pId);
}
#endif

#if   (LCDT == 0)
/*
   ---------------------------------------------------------------------
   NAME:       AddRetire

   FUNCTION:   store another player who has hung up the spikes

   INPUT:      tId = team id
               pId = player's id

   RETURNS:    TNPSaver list a little bigger
   ---------------------------------------------------------------------
*/

void TeamNews::AddRetire (const DYN_16 tId, const DYN_16 pId)
{
   TNPSaver *tnpPtr;

   // if no Retire object yet, create it (making it THE ONLY TNBase object
   // in the list!)
   if (!news.size ())
   {
      tnpPtr = new TNPSaver ();
      vmAssert (tnpPtr);
      news.linkin (tnpPtr);
      tnpPtr->Add (-1, -1);
   }

   // since it IS THE ONLY TNBase object in the list (news),
   // get pointer to it via the 'wrong' enum
   else
      tnpPtr = (TNPSaver *) GetTNItem (TNI_PSAVER);

   tnpPtr->Add (tId, pId);

   StatFile	*sfPtr = ActiveLeague->GetStats (TRUE);
   sfPtr->AddToHALLOFAME((DYN_U_16)pId);
StatisticHALLOFAME *debug = (StatisticHALLOFAME *) sfPtr->GetStat(PCSTAT_HALLOFAME, pId) ;

}
#endif

#if   (LCDT == 0) || (LCDT == 4)
/*
   ---------------------------------------------------------------------
   NAME:       Save

   FUNCTION:   after 'start new season' (retirements)
               or after a game (CEIs)
               news will contain a list of players that may no longer
               be part of the league
               write out all the team and player ids to the file

   INPUT:      pName, fName-> string for league filename path,

   RETURNS:    league.tmn updated
   ---------------------------------------------------------------------
*/

void TeamNews::Save (const DYN_8 *pName, const DYN_8 *fName)
{
   if (!news.start ())
   {
      TNPSaver *tnpPtr = (TNPSaver *) news ();
      FILE     *fp = my_fopen (makeFileName (0, (DYN_8 *) pName, (DYN_8 *) fName, tmnText), "ab");
      if (fp)
      {
         tnpPtr->Save (fp);
         my_fclose (fp);
      }
   }
}
#endif





/*
   =====================================================================
                           PRIVATE METHODS
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       GetTNItem

   FUNCTION:   return a pointer to the news item wanted

   INPUT:      ni = which set of news

   RETURNS:    -> news item (trade, game or cei)
   ---------------------------------------------------------------------
*/

TNBase *TeamNews::GetTNItem (const NewsItem ni)
{
   TNBase   *tnbPtr = 0;

   news.start ();
   if (ni)
      if (news += ni)
         return tnbPtr;
   tnbPtr = news ();
   return tnbPtr;
}

/*
   ---------------------------------------------------------------------
   NAME:       GetTrade

   FUNCTION:   return pointer to a particular trade

   INPUT:      index = which trade number wanted

   RETURNS:    -> to the given trade (0 means no trade at that spot)
   ---------------------------------------------------------------------
*/

Trade *TeamNews::GetTrade (TNTrade *tntPtr, const DYN_16 index)
{
   Trade *tPtr = 0;
   if (!tntPtr->tradeList.start ())
   {
      if (index)
         if (tntPtr->tradeList += index)
            return tPtr;

      tPtr = tntPtr->tradeList ();
   }

   return tPtr;
}

/*
   ---------------------------------------------------------------------
   NAME:       GetGame

   FUNCTION:   determine if this team played a game last week,
               if so, fill in the string

   INPUT:      tngPtr-> Team News Game object
               ptr-> where to fill string
               index = which game to print for
                        (only fill when this is 0)

   RETURNS:    TRUE/FALSE if game string was filled in
   ---------------------------------------------------------------------
*/

DYN_16 TeamNews::GetGame (TNGame *tngPtr, DYN_8 *ptr, const DYN_16 index)
{
   DYN_16   fillFlag = FALSE;

   if (!index && tngPtr->Size ())
   {
      strcpy (ptr, tngPtr->buffer);
      fillFlag = TRUE;
   }

   return fillFlag;
}

/*
   ---------------------------------------------------------------------
   NAME:       GetCEI

   FUNCTION:   return player id to a particular CEI player

   INPUT:      index = which CEI number wanted

   RETURNS:    id of CEI player (0 means none at that spot)
   ---------------------------------------------------------------------
*/

RosPlayer *TeamNews::GetCEI (const DYN_16 index)
{
   RosPlayer   *pPtr = 0;
   TNCEI       *tncPtr = (TNCEI *) GetTNItem (TNI_CEI);
   if (!tncPtr->pList.start ())
   {
      if (index)
         if (tncPtr->pList += index)
            return pPtr;

      pPtr = tncPtr->pList ();
   }

   return pPtr;
}

/*
   ---------------------------------------------------------------------
   NAME:       GetRetired

   FUNCTION:   return pointer to a particular retired player

   INPUT:      index = which retired player wanted

   RETURNS:    -> retired player (0 means none at that spot)
   ---------------------------------------------------------------------
*/

RosPlayer *TeamNews::GetRetired (const DYN_16 index)
{
   RosPlayer   *pPtr = 0;
   TNRetired   *tnrPtr = (TNRetired *) GetTNItem (TNI_RETIRED);
   if (!tnrPtr->pList.start ())
   {
      if (index)
         if (tnrPtr->pList += index)
            return pPtr;

      pPtr = tnrPtr->pList ();
   }

   return pPtr;
}









/*
   =====================================================================
                           LOCAL UTILITIES
   =====================================================================
*/


// TeamNews.cpp