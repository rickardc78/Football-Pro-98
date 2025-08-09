/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   game.h

   everything associated with a game to be, currently, or was just played

   the Game class

   GJW: 1-30-1992, original creation
   GJW: 10-27-1992, added password valid flag for each side
   GJW: 4-21-1993, destructor will copy game.in to game.sav if appropriate
   GJW: 4-28-1993, coaching profiles added
   GJW: 7-20-1993, 1st and 2nd half plans & profiles
   GJW: 10-1-1994, remote cheating

  CAM	13 MAY 96	Added SetWindDir

컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _GAME_H
#define  _GAME_H

#include    "device.h"
#include    "modem.h"
#include    "league.h"
#include    "gameplan.h"
#include    "gamesave.h"
#include    "remote.h"

#define MAXEXT 5

class Game
{
public:
      enum GameTeamId
      {
            TEAM_HOME,
            TEAM_VISIT,
            TEAM_MAX,
      };

      enum Half
      {
            HALF_1ST,
            HALF_2ND,
            HALF_MAX,
      };

      enum
      {
            // off & def, profiles & plans, for both halves
            MAX_PLANS = HALF_MAX * (2 + 2)
      };

      enum WindDir
      {
            WD_N,
            WD_NE,
            WD_E,
            WD_SE,
            WD_S,
            WD_SW,
            WD_W,
            WD_NW,
      };

      enum Sim
      {
            GAME_SIM,
            GAME_PLAY,
            GAME_HIGHLIGHTS,
            GAME_DEMO,
      };

      enum Type
      {
            GAME_NONE = -1,
            GAME_EXB,
            GAME_LEAGUE,
            GAME_LEAGUE_PO,
            GAME_MAX
      };

      enum CopyType
      {
            GAME_PRESERVE,
            GAME_RESTORE,
      };

      struct GroundRules
      {
         DYN_8    fatigue,
                  fumbles,
                  injuries,
                  ints,
                  penalties,
                  clock45,
                  fastSim;
      };

      // coaching profile/game plan combinations
      // profile & plan for offense & defense, for both halves
      struct BallLogic
      {
         GamePlan          profile,
                           plan;
      };
      struct HalfBallLogic
      {
         struct BallLogic  off,
                           def;
      };

      // what does each 'side' have during the game
      struct GameSide
      {
         League         *lPtr;
         Team           *tPtr;
         HalfBallLogic  hgl [HALF_MAX];
         InputDevice    id;
         DYN_16         pm,
                        am,
                        jersey;
         DYN_8          pf,
                        passValid;
      };

private:
//    member functions

//    data members


protected:
//    member functions

      DYN_16         LoadIn (DYN_16 gameIndex, League::FullLoad lfl,
                              DYN_8 *hText, DYN_8 *vText);
      DYN_16         CountGDOBlocks () const;

//    data members
      FILE           *fpIn,
                     *fpOut;
      GameSide       *activeGsPtr,
                     gs [2];
      DYN_8          makeOrg,    // make a copy of game.in to game.org
                     peeking;    // indicates to LoadIn (), if it's ok
                                 // to delete stuff

      DYN_8          remoteCheat;   // prevents users from 'taking over'
                                    // during remote league games if
                                    // they start to lose
      ModemState     modem;
      DYN_16         qtrLength,
                     temp,
                     humid,
                     wind,
                     conditions;
      WindDir        windDir;
      Sim            gameSim;
      Type           exbLeague;
      DYN_8          weekNum,    // if a league game
                     gameNum;
      GroundRules    rules;
      DYN_16         baseTemp;
      GameSave       *gsPtr;
      DYN_8          pathBuff [MAXPATH],
                     nameBuff [MAXFILE + MAXEXT],
                     gpBuff [MAXPATH],
                     saved [MAX_REG_GAMES_PER_WEEK];

public:
//    member functions
                     Game ();
                     ~Game ();

      InputDevice    GetGSId (const GameTeamId gtId) const;
      DYN_16         GetGSPM (const GameTeamId gtId) const;
      DYN_16         GetGSAM (const GameTeamId gtId) const;
      DYN_16         GetGSJersey (const GameTeamId gtId) const;
      DYN_16         GetGSPF (const GameTeamId gtId) const;
      DYN_16         GetGSPassValid (const GameTeamId gtId) const;

      League         *GetGSLeague (const GameTeamId gtId) const;
      Team           *GetGSTeam (const GameTeamId gtId) const;
      GamePlan       *GetGSOffGamePlan (const GameTeamId gtId, const DYN_16 half) const;
      GamePlan       *GetGSDefGamePlan (const GameTeamId gtId, const DYN_16 half) const;
      GamePlan       *GetGSOffCoachPro (const GameTeamId gtId, const DYN_16 half) const;
      GamePlan       *GetGSDefCoachPro (const GameTeamId gtId, const DYN_16 half) const;
//      void           GSDefPlans (const GameTeamId gtId);
      void           GSDefPlans (const GameTeamId gtId, Team *tPtr);

      void           SetGSId (const GameTeamId gtId, const InputDevice value);
      void           SetGSPM (const GameTeamId gtId, const DYN_16 value);
      void           SetGSAM (const GameTeamId gtId, const DYN_16 value);
      void           SetGSJersey (const GameTeamId gtId, const DYN_16 value);
      void           SetGSPF (const GameTeamId gtId, const DYN_16 value);
      void           SetGSPassValid (const GameTeamId gtId, const DYN_16 value);

      void           SetGSLeague (const GameTeamId gtId, League *lPtr);
      void           SetGSTeam (const GameTeamId gtId, Team *tPtr);

      DYN_16         Get45Sec () const;
      DYN_16         GetBaseTemp () const;
      DYN_16         GetCond () const;
      Type           GetExbLeague () const;
      DYN_16         GetFastSim () const;
      DYN_16         GetFatigue () const;
      DYN_16         GetFumbles () const;
      DYN_16         GetGameNum () const;
      DYN_16         GetHumid () const;
      DYN_16         GetInjuries () const;
      DYN_16         GetInts () const;
      ModemState     GetModem () const;
      DYN_16         GetPenalties () const;
      Sim            GetPlaySim () const;
      DYN_16         GetQtr () const;
      DYN_8          GetRemoteCheat () const;
      DYN_16         GetTemp () const;
      DYN_16         GetWeekNum () const;
      DYN_16         GetWind () const;
      WindDir        GetWindDir () const;


      void           Set45Sec (const DYN_16 value);
      void           SetCond (const DYN_16 value);
      void           SetExbLeague (const Game::Type value);
      void           SetFastSim (const DYN_16 value);
      void           SetFatigue (const DYN_16 value);
      void           SetFumbles (const DYN_16 value);
      void           SetGameNum (const DYN_16 value);
      void           SetHumid (const DYN_16 value);
      void           SetInjuries (const DYN_16 value);
      void           SetInts (const DYN_16 value);
      void           SetModem (const ModemState value);
      void           SetPenalties (const DYN_16 value);
      void           SetPlaySim (const Game::Sim value);
      void           SetQtr (const DYN_16 value);
      void           SetRemoteCheat (const DYN_8 value);
      void           SetTemp (const DYN_16 value);
      void           SetWeekNum (const DYN_16 value);
      void           SetWind (const DYN_16 value);
      void           SetWindDir (const WindDir value);


      void           GameAutoSetup ();
      void           GenerateWeather (DYN_16 month);

      void           ComputerFinish (const Sim finish);

      DYN_16         Peek (DYN_16 gameIndex, DYN_8 *hTeam, DYN_8 *vTeam);
      DYN_16         LoadIn (DYN_16 gameIndex, Team **hTeam, Team **vTeam);
      DYN_16         LoadOut (DYN_16 gameIndex, StatFile *sfPtr, DYN_16 poFlag);
      DYN_16         LoadRemote ();
      DYN_16         SaveRemote ();
      void           Save ();
      void           Save (Remote::Type remoteFlag);
      void           CopyGameIn (CopyType preResFlag);

      Team           *Champion ();

      DYN_16         KillGameFiles (DYN_16);
      void           SaveGameFiles ();
      void           SetActiveLeague (const League::FullLoad lfl);

      GameSave       *GetGameSave (DYN_8 *exbText, DYN_8 *lgeText);
      void           DeleteGameSave ();
      void           InitGameSave (League *lPtr);
      DYN_16         CheckGameSave (DYN_16 weekNum, DYN_16 gameNum);
      DYN_16         RestoreGame (DYN_16 index);
      DYN_16         ConcatGameFiles ();
      void           KillRemoteFiles () const;

//    data members
      // a saved game has been found & restarted
      static DYN_8   sgFlag;
};

#endif      // _GAME_H
