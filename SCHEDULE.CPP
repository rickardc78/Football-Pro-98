/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   schedule.cpp

   there are 2 sets (regular & playoffs) of 2 linked lists:
   first is a list of weeks
   seconds is a list of games for that week

   GJW: 2-20-1992, original creation
   GJW: 4-22-1992, changed references of teams from conference, division &
                     team indices to just the team's id
   GJW: 8-2-1992,  removed id from each game
   GJW: 8-27-1992, moved most data to far memory

  CAM		20 DEC 96	Moved GamesDoneThisWeek & DetermineWeek out of #ifdef section

컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   #includes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#include       <string.h>

#include       <ports.h>
#include       "dyndefs.h"

#include       "assertvm.h"
#include       "lcdtutil.h"
#include       "game.h"
#include       "fbmu.h"

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   consts
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

enum TmData
{
   TD_CI,
   TD_DI,
   TD_MAX
};

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   local utility prototypes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

static void       NEAR readWeeks (DYN_8 numWeeks, SchedLeague *slPtr,
                                    ReadBFile &rlf);
static void       NEAR writeWeeks (SchedLeague *slPtr, WriteBFile &wlf);
static SchedTeam  * NEAR findTeam (SchedGame *, DYN_8, DYN_8);
static void       NEAR moveToFront (zDList *, Team *);

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   global data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   local data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */
INCLUDE_ASSERT;

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/*
   ---------------------------------------------------------------------
   NAME:          Schedule

   FUNCTION:      Schedule object CONSTRUCTOR

   INPUT:         fp -> vm_bfile with data to extract
                  leaguePtr-> League that this schedule is connected to

                  if no fp, then there are no games to fill in

   RETURNS:       -> created Schedule object
   ---------------------------------------------------------------------
*/

Schedule::Schedule (ReadBFile &rlf, const League *leagPtr)
{
   Init (leagPtr);
   Load (rlf);
}

Schedule::Schedule (const League *leagPtr)
{
   memset (this, 0, sizeof (Schedule));
   Init (leagPtr);
}

/*
   ---------------------------------------------------------------------
   NAME:          ~Schedule

   FUNCTION:      Schedule object DESTRUCTOR

   INPUT:         -> Schedule object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

Schedule::~Schedule ()
{
   deleteFar (slPtr);
}







/*
   =====================================================================
                           PUBLIC METHODS
   =====================================================================
*/

#if   (LCDT == 0)
/*
   ---------------------------------------------------------------------
   NAME:       GenerateNew

   FUNCTION:   generate a new schedule for the league
               this year's schedule is dependent upon your finish last
               year

   INPUT:      config = league configuration (8, 10 etc)
               byes = should byes be generated for 28 team leagues

   RETURNS:    new schedule generated
   ---------------------------------------------------------------------
*/

void
Schedule::GenerateNew (DYN_8 byes, DYN_16 config)
{
   memset (slPtr, 0, sizeof (SchedLeague));
   ScheduleTemplate  ast;
   ast.Generate (slPtr, config, byes);
}
#endif   // LCDT

/*
   ---------------------------------------------------------------------
   NAME:       GetWData

   FUNCTION:   return specific data for a given week

   INPUT:      wData = type of weekly data wanted
               wIndex = number of week to get data for

   RETURNS:    data requested
   ---------------------------------------------------------------------
*/

DYN_16
Schedule::GetWData (WeekData wData, DYN_8 wIndex)
{
   DYN_16   value = 0;

   switch (wData)
   {
      case SCDWEEKDATA_NUM_WEEKS:
         value = slPtr->numReg + slPtr->numPO;
         break;

      case SCDWEEKDATA_NUM_GAMES:
         if (SetWeekPtr (wIndex) >= 0)
            value = swPtr->numGames;
         break;

      case SCDWEEKDATA_CUR_WEEK:
         value = curWeek;
         break;

      case SCDWEEKDATA_NUM_REGWEEKS:
         value = slPtr->numReg;
         break;

      case SCDWEEKDATA_NUM_POWEEKS:
         value = slPtr->numPO;
         break;
   }

   return value;
}

/*
   ---------------------------------------------------------------------
   NAME:       GetTData

   FUNCTION:   return team-specific data maintained by the schedule
               return 1 weeks worth, or sum of all games, depending on
               the number of parameters

               // GJW: 5-5-1992, don't include playoff games in sums

   INPUT:      stData = indicator of specific data wanted
               tId = which team is data wanted for
               wIndex (optional) = week number

   RETURNS:    data wanted
               -1 for some values (points for or against) indicates
               the game has yet to be played

               if entire season, -1 is not returned, but filtered out
   ---------------------------------------------------------------------
*/

DYN_16
Schedule::GetTData (SchedTData stData, DYN_8 tId)
{
   DYN_16   wIndex = 0,
            value = 0;

   while (wIndex < slPtr->numReg)
   {
      DYN_16   temp = GetTData (stData, tId, (DYN_8)wIndex++);
      if (temp != -1)
         value += temp;
   }

   return value;
}

DYN_16
Schedule::GetTData (SchedTData stData, DYN_8 tId, DYN_8 wIndex)
{
   if (SetWeekPtr (wIndex) >= 0)
      return GetTGData (stData, tId);

   return -1;  // if this week does not exist, return so
}

/*
   ---------------------------------------------------------------------
   NAME:       Get/SetSG/STData

   FUNCTION:   get or set some member of the currently active SchedGame
               or SchedTeam

   INPUT:      sg/stData = what type of data
               if stData, then hvIndex = home or visiting team
               if Set, value = what data should be set to

   RETURNS:    data set, or current state
   ---------------------------------------------------------------------
*/

DYN_16
Schedule::GetSGData (sgData data)
{
   DYN_16   value = 0;

   if (sgPtr)
   {
      switch (data)
      {
         case SGDATA_OT:
            value = sgPtr->ot;
            break;

         case SGDATA_STATUS:
            value = sgPtr->status;
            break;
      }
   }

   return value;
}

DYN_16
Schedule::GetSTData (stData data, DYN_16 hvIndex)
{
   SchedTeam   *stPtr = 0;

   switch (hvIndex)
   {
      case Game::TEAM_HOME:
         stPtr = sthTeam;
         break;

      case Game::TEAM_VISIT:
         stPtr = stvTeam;
         break;
   }

   DYN_16   value = 0;

   if (stPtr)
   {
      switch (data)
      {
         case STDATA_ID:
            value = stPtr->id;
            break;

         case STDATA_PF:
            value = stPtr->pf;
            break;
      }
   }

   return value;
}

/*
   ---------------------------------------------------------------------
   NAME:       GetTeam

   FUNCTION:   after SetSchedulePtrs () has been called, return
               hTeam or vTeam pointers

   INPUT:      sgtData = which team pointer to return

   RETURNS:    -> one of the 2 team involved
   ---------------------------------------------------------------------
*/

Team *
Schedule::GetTeam (sgTData sgtData)
{
   Team  *tPtr = 0;

   switch (sgtData)
   {
      case SGTDATA_HTEAM:
         tPtr = hTeam;
         break;

      case SGTDATA_VTEAM:
         tPtr = vTeam;
         break;
   }

   return tPtr;
}

void
Schedule::SetSGData (sgData data, DYN_16 value)
{
   if (sgPtr)
   {
      switch (data)
      {
         case SGDATA_OT:
            sgPtr->ot = (char)value;
            break;

         case SGDATA_STATUS:
            sgPtr->status = (char)value;
            break;
      }
   }
}

void
Schedule::SetSTData (stData data, DYN_16 hvIndex, DYN_16 value)
{
   SchedTeam   *stPtr = 0;

   switch (hvIndex)
   {
      case Game::TEAM_HOME:
         stPtr = sthTeam;
         break;

      case Game::TEAM_VISIT:
         stPtr = stvTeam;
         break;
   }

   if (stPtr)
   {
      switch (data)
      {
         case STDATA_PF:
            stPtr->pf = (char)value;
            break;
      }
   }
}

#if   (LCDT == 0)
/*
   ---------------------------------------------------------------------
   NAME:       GetStandings

   FUNCTION:   fill in an array of ids with the standings from best to
               worst
               (this is called by the draft to set the order, so
               just put everyone into order based on wins, except the
               2 teams in the championship game pick last, and next-to-last
               (which means they are the first 2 in the list))

   INPUT:      dest-> array to fill with ids
               tList

   RETURNS:    dest filled with team ids
   ---------------------------------------------------------------------
*/

void
Schedule::GetStandings (DYN_8 *dest, zDList *tList)
{
   Team  *champPtr,
         *runnerPtr;

   // get pointer to the championship game (the last game of the season)
   SetSchedPtrs (slPtr->numReg + slPtr->numPO - 1, 0);

   // and determine who won
   if (sthTeam->pf > stvTeam->pf)
   {
      champPtr = (Team *) hTeam;
      runnerPtr = (Team *) vTeam;
   }
   else
   {
      champPtr = (Team *) vTeam;
      runnerPtr = (Team *) hTeam;
   }

   // move these to players to the last 2 positions in the list
   // (with the winner being the very last)
   moveToFront (tList, runnerPtr);
   moveToFront (tList, champPtr);

   tList->start ();
   while (TRUE)
   {
      Team  *tPtr = (Team *) (tList->get ());
      *dest++ = (DYN_8)tPtr->GetId ();
      if (tList->fwd ())
         break;
   }
}
#endif

#if   (LCDT == 0) || (LCDT == 4)
/*
   ---------------------------------------------------------------------
   NAME:       WinLossTie

   FUNCTION:   determine which of 2 given teams has the better
               win-loss-tie record against
               each other
               against division
               against conference
               against common opponents

               (only regular season games are checked)
               winner starts at 0, and is incremented for each game won by
               team 1, decremented for each game won by team 2

   INPUT:      t1Id, t2Id = id's of the 2 teams
               wltData = type of comparison

   RETURNS:    0 if teams haven't played, or have split the number of
               games between them
               t1Id, t2Id if either team has won more games
   ---------------------------------------------------------------------
*/

DYN_8
Schedule::WinLossTie (DYN_8 t1Id, DYN_8 t2Id, SchedWLTData wltData)
{
   DYN_8    winner = 0;

   switch (wltData)
   {
      case SCDWLTDATA_HEAD2HEAD:
         winner = WLTHeadToHead (t1Id, t2Id);
         break;

      case SCDWLTDATA_DIV:
         winner = WLTDiv (t1Id, t2Id);
         break;

      case SCDWLTDATA_CONF:
         winner = WLTConf (t1Id, t2Id);
         break;

      case SCDWLTDATA_COMMON:
         winner = WLTCommon (t1Id, t2Id);
         break;

      case SCDWLTDATA_DIV_POINTS:
         winner = WLTDivPts (t1Id, t2Id);
         break;

      case SCDWLTDATA_CONF_POINTS:
         winner = WLTConfPts (t1Id, t2Id);
         break;

      case SCDWLTDATA_ALL_POINTS:
         winner = WLTAllPts (t1Id, t2Id);
         break;
   }

   // now return whether a winner was decided
   // (i.e. convert +/- to an id)
   if (winner > 0)
      winner = t1Id;

   else if (winner < 0)
      winner = t2Id;

   return winner;
}
#endif

#if   (LCDT == 0) || (LCDT == 4)
/*
   ---------------------------------------------------------------------
   NAME:       SetSchedPtrs

   FUNCTION:   sets internal pointers to the game specified

   INPUT:      wIndex = which week
               gIndex = which game

   RETURNS:    TRUE/FALSE if wanted game exists
               sgPtr, sthTeam, stvTeam, hTeam, vTeam
   ---------------------------------------------------------------------
*/

DYN_16
Schedule::SetSchedPtrs (DYN_8 wIndex, DYN_8 gIndex)
{
   // assume no week with this index
   DYN_16   ptrsSet = FALSE;

   sgPtr = 0;
   sthTeam = stvTeam = 0;
   hTeam = vTeam = 0;

   // don't bother if there aren't any games for the given week
   if (SetWeekPtr (wIndex) >= 0)
   {
      // if game table exists, and index is within range...
      if (swPtr->numGames && (gIndex < swPtr->numGames))
      {
         sgPtr = &swPtr->games [gIndex];
         sthTeam = &sgPtr->hTeam;
         stvTeam = &sgPtr->vTeam;

         // if teams haven't been set, don't search for them
         hTeam = vTeam = 0;
         if (sthTeam->id != -1)
            hTeam = lPtr->GetTeam (sthTeam->id);

         if (stvTeam->id != -1)
            vTeam = lPtr->GetTeam (stvTeam->id);

         ptrsSet = TRUE;
      }
   }

   return ptrsSet;
}
#endif

#if   (LCDT == 0) || (LCDT == 4)
/*
   ---------------------------------------------------------------------
   NAME:       FindTeamsGame

   FUNCTION:   determine if this team has a game in the given week

   INPUT:      tId = team identifier
               wIndex = week number

   RETURNS:    gameNumber of week if team plays
               -1 if not
   ---------------------------------------------------------------------
*/

DYN_16
Schedule::FindTeamsGame (DYN_8 tId, DYN_8 wIndex)
{
   DYN_16   numGames = GetWData (SCDWEEKDATA_NUM_GAMES, wIndex);

   for (DYN_16 gameNum = 0; gameNum < numGames; gameNum++)
   {
      if (SetSchedPtrs (wIndex, (DYN_8)gameNum))
      {
         if ((sthTeam->id == tId) || (stvTeam->id == tId))
            return gameNum;
      }
   }

   return -1;
}
#endif




/*
   ---------------------------------------------------------------------
   NAME:       DetermineWeek

   FUNCTION:   determine which week of the season is currently active
               by going through each week and finding which week has the
               first non-played game

               if curWeek is -1, it automatically goes to 0

   INPUT:      -> Schedule object

   RETURNS:    week number of 1st non-played game
               -1 = all games (including playoffs) done (i.e. end of season)
   ---------------------------------------------------------------------
*/
DYN_8	Schedule::DetermineWeek ()
{
   DYN_8 maxWeeks = slPtr->numReg + slPtr->numPO;

   curWeek = 0;
   if (maxWeeks)
   {
      swPtr = slPtr->sched;
      for (DYN_8 i = 0; i < maxWeeks; i++, swPtr++)
      {
         DYN_8 maxGames = swPtr->numGames;

         if (maxGames)
         {
            sgPtr = swPtr->games;
            for (DYN_8 i = 0; i < maxGames; i++, sgPtr++)
            {
               if (sgPtr->status != SCHED_DONE)
                  return curWeek;
            }
         }

         curWeek++;
      }
   }

   return curWeek = -1;
} // Schedule::DetermineWeek




#if   (LCDT == 0) || (LCDT == 4)

/*
   ---------------------------------------------------------------------
   NAME:       GameComplete

   FUNCTION:   update the schedule after completion of a league game
               if game really exits, and a score has not already been
               saved for it, then save the game's info

   INPUT:      wIndex = week of game
               gIndex = game number
               pfH, pfV = points for Home/Visitor
               otFlag = overTime indicator

   RETURNS:    schedule updated
   ---------------------------------------------------------------------
*/

void
Schedule::GameComplete (DYN_8 wIndex, DYN_8 gIndex,
                           DYN_8 pfH, DYN_8 pfV,
                           DYN_8 otFlag)
{
   if (SetSchedPtrs (wIndex, gIndex) && (sgPtr->status != SCHED_DONE))
   {
      sgPtr->status = SCHED_DONE;
      sgPtr->ot = otFlag;
      sthTeam->pf = pfH;
      stvTeam->pf = pfV;

      Team  *tPtr = (Team *) hTeam;
      tPtr->GenerateRecord (this);
      tPtr = (Team *) vTeam;
      tPtr->GenerateRecord (this);
   }
}
#endif   // LCDT

/*
   ---------------------------------------------------------------------
   NAME:       setPOMask

   FUNCTION:   set the mask that makes the
               week Number -> playoff description correspondance

               the playoffs behave a little nutty, in that there may be
               1, 2, 3 OR 4 weeks of them. The following table indicates
               which leagues play what games, and which playoff week number
               they are:

                     WC  DC  CC  LC
                8                1
               10    1           2
               12    1           2
               18    1       2   3
               28    1   2   3   4

               so I will make a bit-mask to rotate & deterimine if a playoff
               game corresponds to the given week

   INPUT:      numWeeks = number of playoff weeks

   RETURNS:    bits set indicating which week corresponds to which game
               bit #0 = week 1 of playoffs
               bit #3 = week 4 of playoffs
   ---------------------------------------------------------------------
*/

DYN_8
Schedule::SetPOMask ()
{
   DYN_8    poMask = 0;
   switch (slPtr->numPO)
   {
      case  1:
         poMask = 0x08;    // 1000
         break;

      case  2:
         poMask = 0x09;    // 1001
         break;

      case  3:
         poMask = 0x0d;    // 1101
         break;

      case  4:
         poMask = 0x0f;    // 1111
         break;
   }

   return poMask;
}




/*
   ---------------------------------------------------------------------
   NAME:       GamesDoneThisWeek

   FUNCTION:   return the number of games that have been completed for
               this week

   INPUT:      current week number

   RETURNS:    number of completed games
               0 through numGames
   ---------------------------------------------------------------------
*/

DYN_16
Schedule::GamesDoneThisWeek ()
{
   DYN_16   gamesDone = 0;

   DetermineWeek ();

   if (curWeek >= 0)
   {
      if (slPtr->numReg)
      {
         swPtr = &slPtr->sched [curWeek];
         DYN_8 maxGames = swPtr->numGames;

         if (maxGames)
         {
            sgPtr = swPtr->games;
            for (DYN_8 i = 0; i < maxGames; i++, sgPtr++)
            {
               if (sgPtr->status == SCHED_DONE)
                  gamesDone++;
            }
         }
      }
   }

   // if all games (including league championship) are done, return so
   else
      gamesDone = 1;

   return gamesDone;
} // Schedule::GamesDoneThisWeek




#if   (LCDT == 0) || (LCDT == 4)
/*
   ---------------------------------------------------------------------
   NAME:       SetPlayoffGames

   FUNCTION:   the playoffs are underway,
               determine what teams move on to the next week where applicable
               only need to call template when the next week's games need
               to be set (but this is called by the league object after
               every playoff game)

   INPUT:      config = league configuration (8, 10, 12, 18 or 28 team)
               current week number determines which round of playoffs is
               active, and which teams have to be checked for advancement

   RETURNS:    playoff games set
   ---------------------------------------------------------------------
*/

void
Schedule::SetPlayoffGames (DYN_16 config)
{
   // check if any of this week's game don't have teams set
   DetermineWeek ();

   ScheduleTemplate  *ast = new ScheduleTemplate;
   vmAssert (ast);
   ast->SetPlayoffGames (lPtr, slPtr, curWeek, config);
   delete ast;
}

/*
   ---------------------------------------------------------------------
   NAME:       CheckPlayOffBerth

   FUNCTION:   determine if given team has qualified for the playoffs

   INPUT:      tId = team id to check

   RETURNS:    0 = NOPE
               1 = at least wild card
               2 = divisional winner
   ---------------------------------------------------------------------
*/

Schedule::PlayOffBerth
Schedule::CheckPlayOffBerth (Team *tPtr)
{
   PlayOffBerth   pob = POB_NONE;

   // only bother determining after midway point of season
   if (curWeek > (slPtr->numReg / 2))
   {
   }

   return pob;
}

/*
   ---------------------------------------------------------------------
   NAME:       Save

   FUNCTION:   write a "SCD:" block to the file
               "SCD:"
               numReg               sizeof (DYN_8)
               numPO                sizeof (DYN_8)
               curWeek              sizeof (DYN_8)

               for number of weeks (regular & playoff):
                  numGames          sizeof (DYN_8)
                  visiting/home     (2 * sizeof (SchedGame))
                                                   (id & points)
                  otFlag            sizeof (DYN_8)
                  status            sizeof (DYN_8)

   INPUT:      fp-> FILE object already opened for writing to
               Schedule object

   RETURNS:    file updated
   ---------------------------------------------------------------------
*/

void
Schedule::Save (WriteBFile &wlf)
{
   wlf.OpenBlock (S00Text);

   DYN_8    temp;
   temp = slPtr->numReg;
   wlf.Write (&temp, sizeof (DYN_8));

   temp = slPtr->numPO;
   wlf.Write (&temp, sizeof (DYN_8));

   wlf.Write (&curWeek, sizeof (DYN_8));

   writeWeeks (slPtr, wlf);

   wlf.CloseBlock ();
}

/*
   ---------------------------------------------------------------------
   NAME:       ClearCurWeek

   FUNCTION:   when creating a new season, and dumping all players
               into a free agent pool, the schedule has to be set
               so that the first game is NOT ready to be played

   INPUT:      none

   RETURNS:    curWeek set to -1
   ---------------------------------------------------------------------
*/

void
Schedule::ClearCurWeek ()
{
   curWeek = -1;
}
#endif   // LCDT









/*
   =====================================================================
                           PRIVATE METHODS
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       Init

   FUNCTION:   do some initialization common to all constructors

   INPUT:      leaguePtr-> League that this schedule is connected to

   RETURNS:    set lPtr
               allocate memory for schedule storage
   ---------------------------------------------------------------------
*/

void
Schedule::Init (const League *leagPtr)
{
   lPtr = (League *) leagPtr;
   slPtr = (SchedLeague *) newFar (sizeof (SchedLeague), 0);
   sysAssert (slPtr, EXITMSG_SCHED_MEM);
}


/*
   ---------------------------------------------------------------------
   NAME:       Load

   FUNCTION:   read in the schedule contained in the file

   INPUT:      fp-> FILE

   RETURNS:    schedule loaded if found
               0 if block not found
   ---------------------------------------------------------------------
*/

DYN_16
Schedule::Load (ReadBFile &rlf)
{
   DYN_16   value = FALSE;

   if (rlf.Find (S00Text, 1) != -1L)
   {
      DYN_8 temp;
      rlf.Read (&temp, sizeof (DYN_8));
      slPtr->numReg = temp;
      rlf.Read (&temp, sizeof (DYN_8));
      slPtr->numPO = temp;
      rlf.Read (&curWeek, sizeof (DYN_8));

      readWeeks (slPtr->numReg + slPtr->numPO, slPtr, rlf);
      value = TRUE;
   }

   return value;
}

/*
   ---------------------------------------------------------------------
   NAME:       GetTGData

   FUNCTION:   try & find the team in the given week's list of games

   INPUT:      stData = indicator of specific data wanted
               swPtr-> list of this week's games
               tId = which team is data wanted for

   RETURNS:    -1 if game not yet played
               0 if team not found
               otherwise, data requested
   ---------------------------------------------------------------------
*/

DYN_16
Schedule::GetTGData (SchedTData stData, DYN_8 tId)
{
   DYN_16   value = 0;
   DYN_8    maxGame = swPtr->numGames;

   if (maxGame)
   {
      sgPtr = swPtr->games;
      for (DYN_8 i = 0; i < maxGame; i++, sgPtr++)
      {
         SchedTeam   *ttPtr = findTeam (sgPtr, tId, TRUE);

         if (ttPtr)
         {
            if (sgPtr->status == SCHED_DONE)
            {
               SchedTeam   *otPtr = findTeam (sgPtr, tId, FALSE);

               switch (stData)
               {
                  case SCDTEAMDATA_WINS:
                     if (ttPtr->pf > otPtr->pf)
                        value = 1;
                     break;

                  case SCDTEAMDATA_LOSSES:
                     if (ttPtr->pf < otPtr->pf)
                        value = 1;
                     break;

                  case SCDTEAMDATA_TIES:
                     if (ttPtr->pf == otPtr->pf)
                        value = 1;
                     break;

                  case SCDTEAMDATA_PF:
                     value = ttPtr->pf;
                     break;

                  case SCDTEAMDATA_PA:
                     value = otPtr->pf;
                     break;
               }

               break;
            }

            else
               value = -1;

            // team only has 1 game per week, so stop looking
            break;
         }
      }
   }

   return value;
}

#if   (LCDT == 0) || (LCDT == 4)
/*
   ---------------------------------------------------------------------
   NAME:       WLT???

   FUNCTION:   determine if one team has beaten the other based on
               some season performance

   INPUT:      t1Id, t2Id = id of the 2 teams

   RETURNS:    + value = t1 is winning team
               - value = t2 is winning team
               (0 indicated this tie-breaker had no effect)
   ---------------------------------------------------------------------
*/

DYN_8
Schedule::WLTHeadToHead (DYN_8 t1Id, DYN_8 t2Id)
{
   // go through the entire season,
   // week-by-week and check if the 2 teams have played
   DYN_8    wIndex,
            winner = 0,
            maxWeek = slPtr->numReg;

   for (wIndex = 0; wIndex < maxWeek; wIndex++)
   {
      // does team 1 have a game this week?
      if (FindTeamsGame (t1Id, wIndex) != -1)
      {
         // make sure game is finished
         if (sgPtr->status == SCHED_DONE)
         {
            DYN_8    hId = sthTeam->id,
                     vId = stvTeam->id;

            // and the opponent is team 2
            if ((hId == t2Id) || (vId == t2Id))
               winner += WLTWinId (t1Id, t2Id, hId, vId);
         }
      }
   }

   return winner;
}

DYN_8
Schedule::WLTDiv (DYN_8 t1Id, DYN_8 t2Id)
{
   // go through the entire season,
   // week-by-week and check both teams for divisional games
   DYN_8    wIndex,
            winner1 = 0,
            winner2 = 0,
            maxWeek = slPtr->numReg;

   for (wIndex = 0; wIndex < maxWeek; wIndex++)
   {
      winner1 += WLTDivConf (t1Id, wIndex, TD_DI);
      winner2 += WLTDivConf (t2Id, wIndex, TD_DI);
   }

   // return which team has more divisional wins
   return winner1 - winner2;
}

DYN_8
Schedule::WLTConf (DYN_8 t1Id, DYN_8 t2Id)
{
   // go through the entire season,
   // week-by-week and check both teams for conference games
   DYN_8    wIndex,
            winner1 = 0,
            winner2 = 0,
            maxWeek = slPtr->numReg;

   for (wIndex = 0; wIndex < maxWeek; wIndex++)
   {
      winner1 += WLTDivConf (t1Id, wIndex, TD_CI);
      winner2 += WLTDivConf (t2Id, wIndex, TD_CI);
   }

   // return which team has more conference wins
   return winner1 - winner2;
}

DYN_8
Schedule::WLTDivConf (DYN_8 tId, DYN_8 wIndex, DYN_8 data)
{
   DYN_8    winner = 0;

   // does team have a game this week?
   if (FindTeamsGame (tId, wIndex) != -1)
   {
      // make sure game is finished
      if (sgPtr->status == SCHED_DONE)
      {
         DYN_8    hId = sthTeam->id,
                  vId = stvTeam->id,
                  oId;

         // get the id of the OTHER team
         if (hId == tId)
            oId = vId;

         else
            oId = hId;

         // make sure opponent is from the same division/conference
         Team     *tThisPtr = lPtr->GetTeam (tId),
                  *tOtherPtr = lPtr->GetTeam (oId);

         // GJW: 1-4-1995, when testing DIVISIONAL games,
         // make sure the teams are also in the same CONFERENCE
         DYN_16   testWinner = FALSE,
                  sameConf = tThisPtr->GetCI () == tOtherPtr->GetCI ();
         if (data == TD_DI)
            testWinner = sameConf &&
                           (tThisPtr->GetDI () == tOtherPtr->GetDI ());
         else if (data == TD_CI)
            testWinner = sameConf;

         if (testWinner)
            winner = WLTWinId (tId, oId, hId, vId);
      }
   }

   return winner;
}

//#pragma  argsused

DYN_8
Schedule::WLTCommon (DYN_8 t1Id, DYN_8 t2Id)
{
   // go through the entire season,
   // week-by-week and check if the 2 teams have played a common opponent
   DYN_8    winner = 0;
   return winner;
}

DYN_8
Schedule::WLTDivPts (DYN_8 t1Id, DYN_8 t2Id)
{
   // go through the entire season,
   // week-by-week and determine points scored and given up against
   // divisional opponents
   DYN_8    wIndex;
   DYN_16   winner1 = 0,
            winner2 = 0,
            maxWeek = slPtr->numReg;

   for (wIndex = 0; wIndex < maxWeek; wIndex++)
   {
      winner1 += WLTDivConfPts (t1Id, wIndex, TD_DI);
      winner2 += WLTDivConfPts (t2Id, wIndex, TD_DI);
   }

   // return which team has more greater divisional point spread
   return winner1 - winner2;
}

DYN_8
Schedule::WLTConfPts (DYN_8 t1Id, DYN_8 t2Id)
{
   // go through the entire season,
   // week-by-week and determine points scored and given up against
   // divisional opponents
   DYN_8    wIndex;
   DYN_16   winner1 = 0,
            winner2 = 0,
            maxWeek = slPtr->numReg;

   for (wIndex = 0; wIndex < maxWeek; wIndex++)
   {
      winner1 += WLTDivConfPts (t1Id, wIndex, TD_CI);
      winner2 += WLTDivConfPts (t2Id, wIndex, TD_CI);
   }

   // return which team has more greater divisional point spread
   return winner1 - winner2;
}

DYN_8
Schedule::WLTDivConfPts (DYN_8 tId, DYN_8 wIndex, DYN_8 data)
{
   DYN_8    pts = 0;

   // does team have a game this week?
   if (FindTeamsGame (tId, wIndex) != -1)
   {
      // make sure game is finished
      if (sgPtr->status == SCHED_DONE)
      {
         DYN_8    hId = sthTeam->id,
                  vId = stvTeam->id,
                  oId;

         // get the id of the OTHER team
         if (hId == tId)
            oId = vId;

         else
            oId = hId;

         // make sure opponent is from the same division/conference
         Team     *tThisPtr = lPtr->GetTeam (tId);
         Team     *tOtherPtr = lPtr->GetTeam (oId);
         DYN_16   tIndex = data == TD_CI ? tThisPtr->GetCI () : tThisPtr->GetDI (),
                  oIndex = data == TD_CI ? tOtherPtr->GetCI () : tOtherPtr->GetDI ();
         if (tIndex == oIndex)
         {
            // get points differential (and make positive)
            pts = abs (sthTeam->pf - stvTeam->pf);

            // if this team didn't win, pts are negative
            if (WLTWinId (tId, oId, hId, vId) < 0)
               pts = -pts;
         }
      }
   }

   return pts;
}

DYN_8
Schedule::WLTAllPts (DYN_8 t1Id, DYN_8 t2Id)
{
   // just return pf/pa margins
   Team     *t1Ptr = lPtr->GetTeam (t1Id),
            *t2Ptr = lPtr->GetTeam (t2Id);

   return   (t1Ptr->GetPF () - t1Ptr->GetPA ()) -
            (t2Ptr->GetPF () - t2Ptr->GetPA ());
}

/*
   ---------------------------------------------------------------------
   NAME:       WLTWinId

   FUNCTION:   determine which team has won the game

   INPUT:      t1Id, t2Id = team identifiers
               hId, vId = home/visitor team identifiers
               sth, stvTeam-> SchedTeam pointer to home/visiting info
               (must convert home/visit to t1/t2Id)

   RETURNS:    -1 = t2 won
                0 = tie game
               +1 = t1 won
   ---------------------------------------------------------------------
*/

DYN_8
Schedule::WLTWinId (DYN_8 t1Id, DYN_8 t2Id, DYN_8 hId, DYN_8 vId)
{
   DYN_8    winId = 0,
            hPf = sthTeam->pf,
            vPf = stvTeam->pf;

   // determine who won (home or visiting)
   if (hPf > vPf)
      winId = hId;

   else if (vPf > hPf)
      winId = vId;

   // convert home/visiting id to +/- winner
   // ties (winId == 0) are ignored
   DYN_8    winner = 0;
   if (winId == t1Id)
      winner = 1;

   else if (winId == t2Id)
      winner = -1;

   return winner;
}
#endif   // LCDT









/*
   =====================================================================
                           LOCAL UTILITIES
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       readWeeks

   FUNCTION:   read all of the season's information

   INPUT:      numWeeks = number of weeks worth of games to read
               listPtr-> list to store game info for (regular or playoff)
               fp-> FILE where data comes from

   RETURNS:    list's games set
   ---------------------------------------------------------------------
*/

static void NEAR
readWeeks (DYN_8 numWeeks, SchedLeague *slPtr, ReadBFile &rlf)
{
   SchedWeek   *swPtr = slPtr->sched;
   for (DYN_8 weekNum = 0; weekNum < numWeeks; weekNum++, swPtr++)
   {
      DYN_8       numGames;
      rlf.Read (&numGames, sizeof (DYN_8));

      DYN_16      size = numGames * sizeof (SchedGame);
      SchedWeek   swBuff;

      rlf.Read (swBuff.games, size);
      swPtr->numGames = numGames;
      memcpy (swPtr->games, swBuff.games, size);
   }
}

#if   (LCDT == 0) || (LCDT == 4)
/*
   ---------------------------------------------------------------------
   NAME:       writeWeeks

   FUNCTION:   write all of the season's information

   INPUT:      listPtr-> list of game info to write
               fp-> FILE where data goes to

   RETURNS:    schedule's games written to disk
   ---------------------------------------------------------------------
*/

static void NEAR
writeWeeks (SchedLeague *slPtr, WriteBFile &wlf)
{
   DYN_8       numWeeks = slPtr->numReg + slPtr->numPO;
   SchedWeek   *swPtr = slPtr->sched;

   for (DYN_8 weekNum = 0; weekNum < numWeeks; weekNum++, swPtr++)
   {
      DYN_8       numGames = swPtr->numGames;
      DYN_16      size = numGames * sizeof (SchedGame);
      SchedWeek   swBuff;
      memcpy (swBuff.games, swPtr->games, size);

      wlf.Write (&numGames, sizeof (DYN_8));
      wlf.Write (swBuff.games, size);
   }
}
#endif   // LCDT

/*
   ---------------------------------------------------------------------
   NAME:       findTeam

   FUNCTION:   determine if the team of the given indices is involved in
               the passed in game

   INPUT:      sgPtr-> a couple of teams
               tId = team to check for
               flag = TRUE find THIS team
                      FALSE find OPPONENT's team

   RETURNS:    0 if team not in this game
               -> this or opponent's team
   ---------------------------------------------------------------------
*/

static SchedTeam * NEAR
findTeam (SchedGame *sgPtr, DYN_8 tId, DYN_8 flag)
{
   SchedTeam   *stPtr = &sgPtr->hTeam;

   if (stPtr->id == tId)
   {
      if (flag)
         return stPtr;

      else
         return &sgPtr->vTeam;
   }

   stPtr = &sgPtr->vTeam;

   if (stPtr->id == tId)
   {
      if (flag)
         return stPtr;

      else
         return &sgPtr->hTeam;
   }

   // not in this game
   return 0;
}

/*
   ---------------------------------------------------------------------
   NAME:       SetWeekPtr

   FUNCTION:   set schedule pointer to the given week

   INPUT:      wIndex = week in question

   RETURNS:    -1 if week does not exist
               0 & swPtr set to week of games
   ---------------------------------------------------------------------
*/

DYN_16
Schedule::SetWeekPtr (DYN_16 wIndex)
{
   // assume no week with this index
   DYN_16   ptrSet = -1,

   // if game table exists, and index is within range, return so
            maxWeek = slPtr->numReg + slPtr->numPO;

   if ((wIndex >= 0) && maxWeek && (wIndex < maxWeek))
   {
      swPtr = &slPtr->sched [wIndex];
      ptrSet = 0;
   }

   return ptrSet;
}

/*
   ---------------------------------------------------------------------
   NAME:       moveToFront

   FUNCTION:   move the given team's pointer to the first position in
               the list

   INPUT:      tList-> list of team pointers
               firstPtr-> team to move to the front

   RETURNS:    tList altered a bit
   ---------------------------------------------------------------------
*/

static void NEAR
moveToFront (zDList *tList, Team *firstPtr)
{
   if (!tList->start ())
   {
      Team  *newPtr = firstPtr;

      while (TRUE)
      {
         // get the team at the current position
         Team  *oldPtr = (Team *) (tList->get ());

         // put in a new one
         tList->update (newPtr);

         // move this player down the chain
         newPtr = oldPtr;

         // stop when old position of firstPtr has someone replace it
         if (newPtr == firstPtr)
            return;

         tList->fwd ();
      }
   }
}
