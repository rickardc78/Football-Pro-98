/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   schedule.h

   the Schedule class

   GJW: 2-20-1992, original creation
   GJW: 4-22-1992, changed SchedTeam to be an id rather than conference,
                     division and team indices
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _SCHEDULE_H
#define  _SCHEDULE_H

#include    <stdio.h>

#include    "dlist.h"
#include    "schtemp.h"

const GAME_COMPLETED = 0x00;
const GAME_UNPLAYED = 0x80;
const GAME_NUM_MASK = 0x7f;

enum WeekData
{
      SCDWEEKDATA_NUM_WEEKS,
      SCDWEEKDATA_NUM_GAMES,
      SCDWEEKDATA_CUR_WEEK,
      SCDWEEKDATA_NUM_REGWEEKS,
      SCDWEEKDATA_NUM_POWEEKS,
};

enum SchedTData
{
      SCDTEAMDATA_WINS,
      SCDTEAMDATA_LOSSES,
      SCDTEAMDATA_TIES,
      SCDTEAMDATA_PF,
      SCDTEAMDATA_PA,
};

enum SchedWLTData
{
      SCDWLTDATA_HEAD2HEAD,
      SCDWLTDATA_DIV,
      SCDWLTDATA_CONF,
      SCDWLTDATA_COMMON,
      SCDWLTDATA_DIV_POINTS,
      SCDWLTDATA_CONF_POINTS,
      SCDWLTDATA_ALL_POINTS,
};

class League;
class Team;

class Schedule
{
public:
      enum PlayOffBerth
      {
            POB_NONE,
            POB_WC,
            POB_DIV,
      };

private:
//    member functions

//    data members


protected:
//    member functions
      void              Init (const League *leagPtr);
      DYN_16            Load (ReadBFile &rlf);
      DYN_16            GetTGData (SchedTData, DYN_8);
      DYN_16            SetWeekPtr (DYN_16);

      DYN_8             WLTHeadToHead (DYN_8, DYN_8);
      DYN_8             WLTDiv (DYN_8, DYN_8);
      DYN_8             WLTConf (DYN_8, DYN_8);
      DYN_8             WLTDivConf (DYN_8, DYN_8, DYN_8);
      DYN_8             WLTCommon (DYN_8, DYN_8);
      DYN_8             WLTDivPts (DYN_8, DYN_8);
      DYN_8             WLTConfPts (DYN_8, DYN_8);
      DYN_8             WLTDivConfPts (DYN_8, DYN_8, DYN_8);
      DYN_8             WLTAllPts (DYN_8, DYN_8);
      DYN_8             WLTWinId (DYN_8, DYN_8, DYN_8, DYN_8);

//    data members
      SchedLeague       *slPtr;
      SchedWeek         *swPtr;
      DYN_8             curWeek;
      League            *lPtr;
      SchedGame         *sgPtr;
      SchedTeam         *sthTeam,
                        *stvTeam;
      Team              *hTeam,
                        *vTeam;

public:
//    member functions
                        Schedule (ReadBFile &rlf, const League *leagPtr);
                        Schedule (const League *leagPtr);
                        ~Schedule ();

      void              GenerateNew (DYN_8, DYN_16);

      DYN_16            GetTData (SchedTData, DYN_8);
      DYN_16            GetTData (SchedTData, DYN_8, DYN_8);
      DYN_16            GetWData (WeekData, DYN_8);
      DYN_16            GetSGData (sgData);
      DYN_16            GetSTData (stData, DYN_16);
      Team              *GetTeam (sgTData);
      void              SetSGData (sgData, DYN_16);
      void              SetSTData (stData, DYN_16, DYN_16);

      void              GetStandings (DYN_8 *, zDList *);
      DYN_8             WinLossTie (DYN_8, DYN_8, SchedWLTData);

      DYN_16            SetSchedPtrs (DYN_8, DYN_8);
      DYN_16            FindTeamsGame (DYN_8, DYN_8);
      DYN_8             DetermineWeek ();
      void              GameComplete (DYN_8, DYN_8, DYN_8, DYN_8, DYN_8);
      DYN_8             SetPOMask ();
      void              SetPlayoffGames (DYN_16);

      DYN_16            GamesDoneThisWeek ();

      PlayOffBerth      CheckPlayOffBerth (Team *tPtr);

      void              Save (WriteBFile &wlf);
      void              ClearCurWeek ();

//    data members
};

#endif      // _SCHEDULE_H
