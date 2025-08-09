/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   schtemp.cpp

   GJW: 5-11-1992, original creation
   GJW: 6-11-1993, rotating conferences/divisions for 28 team leagues
   GJW: 2-1-1995, 30 team leagues
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   #includes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#include       <stdlib.h>
#include       <string.h>

#include       <ports.h>
#include       "dyndefs.h"


#include       "assertvm.h"
#include       "league.h"
#include       "fbmu.h"

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   consts
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   local utility prototypes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

static void    NEAR idFill (DYN_8 *ptr, const DYN_8 numTeams);
static DYN_8   * NEAR fillRandomTeams (DYN_8 *ptr, const DYN_8 numTeams);
static void    NEAR shuffleCD (DYN_8 *ptr, const DYN_16 numTeams);
static void    NEAR shuffleTIds (DYN_8 *div1Ptr, DYN_8 *div2Ptr, DYN_8 numTeams);
static void    NEAR openTemplateFile (ReadBFile &, DYN_16, DYN_8 *);
static DYN_8   NEAR fillPOTemplate (SchedPOGame *, SchedWeek *, ReadBFile &);

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   global data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   local data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */
INCLUDE_ASSERT;

static DYN_8   SCTText [] = "SCT:",
               SPTText [] = "SPT:",
               tplFileName [] = "stpl.dat";

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/*
   ---------------------------------------------------------------------
   NAME:          ScheduleTemplate

   FUNCTION:      ScheduleTemplate object CONSTRUCTOR

   INPUT:         none

   RETURNS:       -> created ScheduleTemplate object
   ---------------------------------------------------------------------
*/

ScheduleTemplate::ScheduleTemplate ()
{
   memset (this, 0, sizeof (ScheduleTemplate));
}

/*
   ---------------------------------------------------------------------
   NAME:          ~ScheduleTemplate

   FUNCTION:      ScheduleTemplate object DESTRUCTOR

   INPUT:         -> ScheduleTemplate object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

ScheduleTemplate::~ScheduleTemplate ()
{
}







/*
   =====================================================================
                           PUBLIC METHODS
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       Generate

   FUNCTION:   make a new schedule for the given configuration

               each game is encoded in 16 bits:
               [home team id] [visting team id]
               the actual team ids are to randomized from season to
               season (make an array of all the ids, and randomize based
               on DIVISIONS:
               e.g. 1234 5678 get mixed up as: 3142 8756, not 1563 2487)

   INPUT:      listPtr-> weekly list of game to fill in
               config = number of teams
               byes = should byes be included for 28 team league
               GJW: 4-30-1993, 1 or 2 per season
               GJW: 2-1-1995, 30 team leagues, with 1 or 2 byes as well
               GJW: 9-18-1995, only 1 30 team template, it has 1 bye in it

   RETURNS:    wList filled with games
   ---------------------------------------------------------------------
*/

void
ScheduleTemplate::Generate (SchedLeague *inslPtr,
                              DYN_16 config, DYN_16 byes)
{
   slPtr = inslPtr;

   // determine which template file to load
   DYN_8    numTeams;

   switch (config)
   {
      case League::CONFIG_8:
         numTeams = 8;
         break;

      case League::CONFIG_10:
         numTeams = 10;
         break;

      case League::CONFIG_12:
         numTeams = 12;
         break;

      case League::CONFIG_18:
         numTeams = 18;
         break;

      case League::CONFIG_28:
         numTeams = 28;
         break;

      case League::CONFIG_30:
         numTeams = 30;
         break;
   }

   if (config == League::CONFIG_28)
      config += byes;

   // GJW: 2-1-1995, 30 team scheds are at the end of ALL the 28 scheds
   // (there are 3, one with no byes, one with 1 and one with 2)
   // (I am trying to generate the absolute block number in the template
   // file, but use the value 2 rather than 3, beacuse CONFIG_28 is 4,
   // and CONFIG_30 is 5, which will account for an offset of 1 already)
   // GJW: 9-18-1995, there is only 1 template, byes will always be 0
   else if (config == League::CONFIG_30)
      config += (2 + byes);

   ReadBFile   rlf;
   openTemplateFile (rlf, config, SCTText);

   DYN_16      size = (DYN_16) rlf.Size ();
   DYN_8       *ptr = new DYN_8 [numTeams + size];

   // then fill in
   if (ptr)
   {
      DYN_8    *frtPtr = ptr;
      idFill (ptr, numTeams);

      switch (numTeams)
      {
         case 8:
            frtPtr = fillRandomTeams (frtPtr, 4);
            fillRandomTeams (frtPtr, 4);
            break;

         case 10:
            frtPtr = fillRandomTeams (frtPtr, 3);
            frtPtr = fillRandomTeams (frtPtr, 4);
            fillRandomTeams (frtPtr, 3);
            break;

         case 12:
            frtPtr = fillRandomTeams (frtPtr, 4);
            frtPtr = fillRandomTeams (frtPtr, 4);
            fillRandomTeams (frtPtr, 4);
            break;

         case 18:
            frtPtr = fillRandomTeams (frtPtr, 4);
            frtPtr = fillRandomTeams (frtPtr, 5);
            frtPtr = fillRandomTeams (frtPtr, 4);
            fillRandomTeams (frtPtr, 5);
            break;

         case 28:
            frtPtr = fillRandomTeams (frtPtr, 5);
            frtPtr = fillRandomTeams (frtPtr, 4);
            frtPtr = fillRandomTeams (frtPtr, 5);
            frtPtr = fillRandomTeams (frtPtr, 5);
            frtPtr = fillRandomTeams (frtPtr, 5);
            fillRandomTeams (frtPtr, 4);
            shuffleCD (ptr, numTeams);
            break;

         case 30:
            frtPtr = fillRandomTeams (frtPtr, 5);
            frtPtr = fillRandomTeams (frtPtr, 5);
            frtPtr = fillRandomTeams (frtPtr, 5);
            frtPtr = fillRandomTeams (frtPtr, 5);
            frtPtr = fillRandomTeams (frtPtr, 5);
            fillRandomTeams (frtPtr, 5);
            shuffleCD (ptr, numTeams);
            break;
      }

      rlf.Read (&ptr [numTeams], size);

      typedef union
      {
         DYN_8  *cPtr;
         DYN_16 *sPtr;
      } GENERIC_PTR;
      GENERIC_PTR    dataPtr;

      dataPtr.cPtr = &ptr [numTeams];
      DYN_8 regWeeks = *dataPtr.cPtr++;
      DYN_8 poWeeks = *dataPtr.cPtr++;
      DYN_8 gamesPerWeek = *dataPtr.cPtr++;

      slPtr->numReg = regWeeks;
      slPtr->numPO = poWeeks;

      // first the regular season
      swPtr = slPtr->sched;
      while (regWeeks--)
      {
         sgPtr = swPtr->games;

         DYN_8 i = gamesPerWeek,
               numGames = 0;

         while (i--)
         {
            DYN_16   gameData = *dataPtr.sPtr++;

            // don't process byes
            if (gameData)
            {
               numGames++;
               sgPtr->hTeam.id = ptr [(gameData >> 8) - 1];
               sgPtr->vTeam.id = ptr [(gameData & 0xFF) - 1];
               sgPtr->hTeam.pf = sgPtr->vTeam.pf = -1;

               sgPtr->ot = 0;
               sgPtr->status = SCHED_PLAY;
               sgPtr++;
            }
         }

         swPtr->numGames = numGames;
         swPtr++;
      }

      // then the playoffs (these games don't have team ids yet)
      DYN_8    poGames [4],
               *poPtr = poGames;

      // determine how many games each week
      switch (poWeeks)
      {
         case 1:
            poGames [0] = 1;
            break;

         case 2:
            poGames [0] = 2;
            poGames [1] = 1;
            break;

         case 3:
            poGames [0] = 4;
            poGames [1] = 2;
            poGames [2] = 1;
            break;

         case 4:
            poGames [0] = 4;
            poGames [1] = 4;
            poGames [2] = 2;
            poGames [3] = 1;
            break;
      }

      while (poWeeks--)
      {
         sgPtr = swPtr->games;

         DYN_8 i = *poPtr++;
         swPtr->numGames = i;
         while (i--)
         {
            sgPtr->hTeam.id = sgPtr->vTeam.id = -1;
            sgPtr->hTeam.pf = sgPtr->vTeam.pf = -1;
            sgPtr->ot = 0;
            sgPtr->status = SCHED_PLAY;
            sgPtr++;
         }

         swPtr++;
      }

      delete ptr;
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       SetPlayoffGames

   FUNCTION:   set the teams that participate in the playoffs

   INPUT:      listPtr-> list of regular & playoff games (used to
               determine who's still alive)
               curWeek = week number of season
               numReg = number of weeks in regular season (playoff week
                        number is curWeek - numReg)
               config = league configuration (for loading template)

   RETURNS:    poWeek's games set
   ---------------------------------------------------------------------
*/

void
ScheduleTemplate::SetPlayoffGames (void *lvPtr,
                              SchedLeague *inslPtr,
                              DYN_8 curWeek, DYN_16 config)
{
   // initialize private data
   lPtr = lvPtr;
   slPtr = inslPtr;

   // then open and read the template file
   ReadBFile   rlf;
   openTemplateFile (rlf, config, SPTText);

   DYN_8 temp;
   rlf.Read (&temp, sizeof (DYN_8));
   slPtr->numPO = temp;
   rlf.Read (&numDivWinners, sizeof (DYN_8));
   rlf.Read (&numWcs, sizeof (DYN_8));

   DYN_8          poWeek = 1 + curWeek - slPtr->numReg,
                  numGames;
   SchedPOGame poGames [MAX_PO_GAMES_PER_WEEK];
   swPtr = poPtr = &slPtr->sched [slPtr->numReg];

   while (poWeek)
   {
      // get list of games for this week
      numGames = fillPOTemplate (poGames, poPtr, rlf);

      if (--poWeek)
         poPtr++;
   }
   rlf.Close ();

   // poPtr-> THIS week's playoff game(s)
   // poGames = templates for the games
   // numGames = number of playoff games this week
   SchedPOGame *pogPtr = poGames;
   sgPtr = poPtr->games;

   while (numGames--)
   {
      // get each game & template
      // if teams have yet to be set in game, get them from template
      SchedTeam            *sthTeam = &sgPtr->hTeam,
                           *stvTeam = &sgPtr->vTeam;
      if (sthTeam->id == -1)
         sthTeam->id = FindTemplateTeam (pogPtr->hTeam);
      if (stvTeam->id == -1)
         stvTeam->id = FindTemplateTeam (pogPtr->vTeam);

      // make sure higher seeded team is at home
      if (FindPOSeed (sthTeam) > FindPOSeed (stvTeam))
      {
         DYN_16   tId = sthTeam->id;
         sthTeam->id = stvTeam->id;
         stvTeam->id = (DYN_8)tId;
      }

      sgPtr++;
      pogPtr++;
   }
}








/*
   =====================================================================
                           PRIVATE METHODS
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       FindTemplateTeam

   FUNCTION:   determine which team is to fill in the given position

   INPUT:      tmplt = team identifier
               divIds-> divisional winners
               wcIds-> wildcard winners
               if high bit(s) are set, it indicates a division or
               wild card winner
               if not, then the winner of a previous playoff game

   RETURNS:    id of team to fill this spot
   ---------------------------------------------------------------------
*/

DYN_8
ScheduleTemplate::FindTemplateTeam (DYN_8 tmplt)
{
   DYN_8    masked = (tmplt & ~TEMPLATE_MASK) - 1, // get just the game number
            tId;

   FillDivWcWinners ();

   // if a division winner or wildcard, get id from array
   if (tmplt & TEMPLATE_MASK)
   {

      DYN_8 *ptr;

      if (tmplt & WILDCARD)
      {
         ptr = wcIds;
         if (tmplt & CONF2)
            ptr += MAX_WC_WINNERS;
      }

      else
      {
         ptr = divIds;
         if (tmplt & CONF2)
            ptr += NUM_DIV;
      }

      ptr += masked;
      tId = *ptr;
   }

   // otherwise get the winner of the specified game
   else
   {
      SchedGame   *sgPtr = SetPOGamePtr (masked);
      SchedTeam   *sthTeam = &sgPtr->hTeam,
                  *stvTeam = &sgPtr->vTeam;

      if (sthTeam->pf > stvTeam->pf)
         tId = sthTeam->id;

      else
         tId = stvTeam->id;
   }

   return tId;
}

/*
   ---------------------------------------------------------------------
   NAME:       FillDivWcWinners

   FUNCTION:   for each conference:
               determine the winners of each division, and the winners of
               the wild card slots
               the arrays are filled with the best records first
               (i.e if the 3 divisional winners were:
               Conference 0               Conference 1
               Div 0    Div 1    Div 2    Div 0    Div 1    Div 2
               9-7      10-6     12-3     10-6     11-5     9-7
                        9-7      11-4
                                 10-6

               divPtr-> 12-3 10-6 9-7, 11-5 10-6 9-7

               SortTeamList will sort ALL the teams in the conference
               by record, so each team will have to be checked if it is a
               divisional or wildcard winner:
               list will be (for conference 0):
               12-3 11-4 10-6 10-6 9-7 9-7
               so the team that is 11-4 DOES NOT win a division, but rather
               the top wildcard spot)

   INPUT:      divIds, numDiv
               wcIds, numWc      array & number of team ids to fill
               if (divIds [0] != 0) then this has already been called,
               and there is no need to fill again

   RETURNS:    destination array filled with id of playoff qualifying teams
   ---------------------------------------------------------------------
*/

void
ScheduleTemplate::FillDivWcWinners ()
{
   if (divIds [0] == 0)
   {
      League   *leaguePtr = (League *) lPtr;

      // have to do this for every conference in the league
      DYN_16   numConfs = leaguePtr->GetNumSub ();
      for (DYN_16 i = 0; i < numConfs; i++)
      {
         DYN_8    *divPtr = &divIds [NUM_DIV * i],
                  *wcPtr = &wcIds [MAX_WC_WINNERS * i];
         GenList<Team>  tList;
         leaguePtr->BuildTeamList (&tList, (DYN_8)i);
         leaguePtr->SortTeamList (&tList, League::SORT_RECORD);
         tList.start ();

         DYN_8    nD = numDivWinners,
                  nWc = numWcs,
                  divFlag = 0;

         // go until all slots are filled
         while (nD + nWc)
         {
            Team  *tPtr = tList ();
            DYN_8 divBit = 1 << tPtr->GetDI ();

            // already found a division winner for this division?
            if (divFlag & divBit)
            {
               // try to make team a wildcard
               if (nWc)
               {
                  *wcPtr++ = (DYN_8)tPtr->GetId ();
                  nWc--;
               }
            }

            else
            {
               divFlag |= divBit;
               *divPtr++ = (DYN_8)tPtr->GetId ();
               nD--;
            }

            ++tList;
         }
      }
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       SetPOGamePtr

   FUNCTION:   given a playoff game number, go through the list of
               playoff games until the right one is found

   INPUT:      gameNum = game number to find

   RETURNS:    -> game wanted
   ---------------------------------------------------------------------
*/

SchedGame *
ScheduleTemplate::SetPOGamePtr (DYN_8 gameNum)
{
   swPtr = &slPtr->sched [slPtr->numReg];
   while (TRUE)
   {
      DYN_16   numGames = swPtr->numGames;

      if ((gameNum - numGames) >= 0)
      {
         gameNum -= numGames;
         swPtr++;
      }

      else
         break;
   }

   return &swPtr->games [gameNum];
}

/*
   ---------------------------------------------------------------------
   NAME:       FindPOSeed

   FUNCTION:   returns the playoff seeding of this team
               (1st divisional team = 0
               (Last wild card team = 12)

   INPUT:      stPtr-> team to find
               (if id == -1, return the last seed available)

   RETURNS:    ranking of team in playoffs
   ---------------------------------------------------------------------
*/

DYN_8
ScheduleTemplate::FindPOSeed (SchedTeam *stPtr)
{
   DYN_8    id = stPtr->id;

   DYN_8    seed = 0,
            *ptr = divIds;
   while (ptr < &divIds [sizeof (divIds)])
   {
      if (*ptr == id)
         return seed;
      seed++;
      ptr++;
   }

   ptr = wcIds;
   while (ptr < &wcIds [sizeof (wcIds)])
   {
      if (*ptr == id)
         return seed;
      seed++;
      ptr++;
   }

   return seed;
}






/*
   =====================================================================
                           LOCAL UTILITIES
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       idFill

   FUNCTION:   fill the array with indices

   INPUT:      ptr-> array to store random team ids
               numTeam = size of array

   RETURNS:    ptr-> team ids in sequential order
   ---------------------------------------------------------------------
*/

static void NEAR
idFill (DYN_8 *ptr, const DYN_8 numTeams)
{
   DYN_8    i = 1;
   for (i; i <= numTeams; i++, ptr++)
      *ptr = i;
}

/*
   ---------------------------------------------------------------------
   NAME:       fillRandomTeams

   FUNCTION:   the schedule ids are randomized every time a new league
               generates a schedule

               incomming array:
               1, 2, 3, 4, 5
               outgoing
               2, 4, 3, 5, 1

   INPUT:      ptr-> array of team ids in numerical order
               numTeam = size of array

   RETURNS:    &ptr [numTeams]
               ptr-> team ids in a random order
   ---------------------------------------------------------------------
*/

static DYN_8 * NEAR
fillRandomTeams (DYN_8 *ptr, const DYN_8 numTeams)
{
   // mix 'em all up, based on divisions
   DYN_8    *usePtr,
            numMixes = 10;
   while (numMixes--)
   {
      usePtr = ptr;
      for (DYN_8 i = 0; i < numTeams; i++, usePtr++)
      {
         // get a random index in the valid range,
         // and swap with the current position,
         // walking the current position through each of the valid indexes
         DYN_8 offset = getPosRand () % numTeams,
               temp = *usePtr;
         *usePtr = ptr [offset];
         ptr [offset] = temp;
      }
   }

   return ptr + numTeams;
}

/*
   ---------------------------------------------------------------------
   NAME:       shuffleCD

   FUNCTION:   to shut up the whiners, the conferences and divisions
               can get mixed around from season to season, so that
               a certain team doesn't 'always play the AFC East'

   INPUT:      ptr-> id's in conference/division, but mixed up team order
               (i.e.:
               2, 5, 3, 1, 4,
               8, 6, 7, 9,
               etc.

               swap around the 4 & 5 team divisions

               numTeams = 28 or 30, because of the new alignment

   RETURNS:    new array ordering
   ---------------------------------------------------------------------
*/

static void NEAR
shuffleCD (DYN_8 *ptr, const DYN_16 numTeams)
{
   DYN_8    swapType = getPosRand () % 4;

   // AFC East <-> AFC West, NFC East <-> NFC Central
   if (swapType == 1)
   {
      if (numTeams == 28)
      {
         shuffleTIds (&ptr [0], &ptr [5 + 4], 5);
         shuffleTIds (&ptr [5 + 4 + 5], &ptr [5 + 4 + 5 + 5], 5);
      }
      else
      {
         shuffleTIds (&ptr [0], &ptr [5 + 5], 5);
         shuffleTIds (&ptr [5 + 5 + 5], &ptr [5 + 5 + 5 + 5], 5);
      }
   }

   // AFC East <-> NFC East, AFC West <-> NFC Central, AFC Central <-> NFC West
   else if (swapType == 2)
   {
      if (numTeams == 28)
      {
         shuffleTIds (&ptr [0], &ptr [5 + 4 + 5], 5);
         shuffleTIds (&ptr [5 + 4], &ptr [5 + 4 + 5 + 5], 5);
         shuffleTIds (&ptr [5], &ptr [5 + 4 + 5 + 5 + 5], 4);
      }
      else
      {
         shuffleTIds (&ptr [0], &ptr [5 + 5 + 5], 5);
         shuffleTIds (&ptr [5 + 5], &ptr [5 + 5 + 5 + 5], 5);
         shuffleTIds (&ptr [5], &ptr [5 + 5 + 5 + 5 + 5], 5);
      }
   }

   // AFC East <-> NFC Central, AFC West <-> NFC East, AFC Central <-> NFC West
   else if (swapType == 3)
   {
      if (numTeams == 28)
      {
         shuffleTIds (&ptr [0], &ptr [5 + 4 + 5 + 5], 5);
         shuffleTIds (&ptr [5 + 4], &ptr [5 + 4 + 5], 5);
         shuffleTIds (&ptr [5], &ptr [5 + 4 + 5 + 5 + 5], 4);
      }
      else
      {
         shuffleTIds (&ptr [0], &ptr [5 + 5 + 5 + 5], 5);
         shuffleTIds (&ptr [5 + 5], &ptr [5 + 5 + 5], 5);
         shuffleTIds (&ptr [5], &ptr [5 + 5 + 5 + 5 + 5], 5);
      }
   }
}

static void NEAR
shuffleTIds (DYN_8 *div1Ptr, DYN_8 *div2Ptr, DYN_8 numTeams)
{
   while (numTeams--)
   {
      DYN_8    temp = *div1Ptr;
      *div1Ptr = *div2Ptr;
      *div2Ptr = temp;

      div1Ptr++;
      div2Ptr++;
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       openTemplateFile

   FUNCTION:   open the template file associated with the given
               league configuraton

   INPUT:      config = league configuration (number of teams, plus byes,
                        i.e. the index into the template file)
               string-> 1st block to vm_bfind

   RETURNS:    -> opened FILE
   ---------------------------------------------------------------------
*/

static void NEAR
openTemplateFile (ReadBFile &rlf, DYN_16 config, DYN_8 *string)
{
   sysAssert (rlf.Open (tplFileName) > 0, EXITMSG_SCHTEMP_LOAD);

   // search for ABSOLUTE block number
   sysAssert (rlf.Find (string, config + 1) != -1, EXITMSG_SCHTEMP_LOAD);
}

/*
   ---------------------------------------------------------------------
   NAME:       fillPOTemplate

   FUNCTION:   read in 1 week's worth of playoff templates

   INPUT:      pogPtr-> where to put info
               swPtr-> weekly schedule data
               fp-> FILE to read template from

   RETURNS:    pogPtr games for the week template set
   ---------------------------------------------------------------------
*/

static DYN_8 NEAR
fillPOTemplate (SchedPOGame *pogPtr, SchedWeek *swPtr, ReadBFile &rlf)
{
   DYN_8 nG = swPtr->numGames,
         numGames = nG;

   while (numGames--)
   {
      SchedPOGame poGame;
      rlf.Read (&poGame, sizeof (SchedPOGame));
      *pogPtr++ = poGame;
   }

   return nG;
}
