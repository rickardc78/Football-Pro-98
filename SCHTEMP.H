/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   schtemp.h

   the ScheduleTemplate class
   SetPlayoffGames has a void *, in actuality it is a pointer to the League
   class, but I want to avoid circular header including (i.e. league includes
   schedule, which included this, which would include league...)
   it gets cast to League * in the method

   GJW: 5-11-1992, original creation
   GJW: 8-2-1992,  removed id from each game
   GJW: 2-1-1995, 30 teams per league means 15 games per week
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _SCDTEMP_H
#define  _SCDTEMP_H

const CONF1 = 0x80;
const CONF2 = 0x40;
const WILDCARD = 0x20;
const TEMPLATE_MASK = CONF1 | CONF2 | WILDCARD;

const MAX_CONF = 2;
const MAX_DIV = 3;
const MAX_TEAM = 5;
const MAX_LEAGUE_TEAMS = (MAX_CONF * MAX_DIV * MAX_TEAM);

const MAX_REG_WEEKS = 18;
const MAX_REG_GAMES_PER_WEEK = 15;
const MAX_PO_WEEKS = 4;
const MAX_PO_GAMES_PER_WEEK = 4;
const MAX_WC_WINNERS = 3;

struct FAR SchedTeam
{
   DYN_8       id,   // team identifier (-1 means team unknown)
               pf;   // how many points scored (-1 means game not yet played)
};

enum stData
{
      STDATA_ID,
      STDATA_PF,
};

struct FAR SchedPOGame
{
   DYN_8       hTeam,
               vTeam;
};

struct FAR SchedGame
{
   SchedTeam   hTeam,
               vTeam;      // teams & points
   DYN_8       ot,         // overtime indicator
               status;     // game's condition
};

enum sgData
{
      SGDATA_OT,
      SGDATA_STATUS,
};

enum sgTData
{
      SGTDATA_HTEAM,
      SGTDATA_VTEAM,
};

enum sgStatus
{
      SCHED_PLAY_PAUSE = -2,
      SCHED_SIM_PAUSE,
      SCHED_DONE,
      SCHED_SIM,
      SCHED_PLAY,
      SCHED_PAUSED,
};

struct FAR SchedWeek
{
   DYN_8       numGames;
   SchedGame   games [MAX_REG_GAMES_PER_WEEK];
};

struct FAR SchedLeague
{
   DYN_8       numReg,
               numPO;
   SchedWeek   sched [MAX_REG_WEEKS + MAX_PO_WEEKS];
};

class ScheduleTemplate
{
private:
//    member functions

//    data members


protected:
//    member functions
      DYN_8                   FindTemplateTeam (DYN_8);
      void                    FillDivWcWinners ();
      SchedGame               *SetPOGamePtr (DYN_8);
      DYN_8                   FindPOSeed (SchedTeam *);

//    data members
      DYN_8                   gamePerWeek,
                              config,
                              *games,
                              *curGame;
      SchedLeague             *slPtr;
      SchedWeek               *swPtr,
                              *poPtr;
      SchedGame               *sgPtr;
      void                    *lPtr;
      DYN_8                   divIds [MAX_CONF * MAX_DIV],
                              numDivWinners,
                              wcIds [MAX_CONF * MAX_WC_WINNERS],
                              numWcs;

public:
//    member functions
                              ScheduleTemplate ();
                              ~ScheduleTemplate ();

      void                    Generate (SchedLeague *, DYN_16, DYN_16);
      void                    SetPlayoffGames (void *, SchedLeague *, DYN_8, DYN_16);

//    data members
};

#endif      // _SCDTEMP_H
