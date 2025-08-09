/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   statdrvd.h

   the Derived Statistic Classes

   GJW: 7-14-1992, original creation
   GJW: 2-23-1995, a little modifying for 32-bit (user will no longer need
                     to know about the virtual function table stuff,
                     I'll make each class avoid copying that info into
                     the record buffer)
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _STATDRVD_H
#define  _STATDRVD_H

#include    "statbase.h"

struct STATGROUP_HEADER
{
         STATGROUP_HEADER     FAR *link;
         DYN_16               id,
                              type;
};

struct STATGROUP_TEAM
{
   struct STATGROUP_TYPE_DOWNS
   {
            DYN_16            rushing,
                              passing,
                              penalty;
   };

   struct STATGROUP_MADE_DOWNS
   {
            DYN_16            attempted,
                              made;
   };

   STATGROUP_TYPE_DOWNS first_downs;
   STATGROUP_MADE_DOWNS third_downs;
   STATGROUP_MADE_DOWNS fourth_downs;

   DYN_16               penalties,
                        yards_lost,          // on penalties
                        time_of_possession,

                        fumblesLost;

   STATGROUP_HEADER     FAR *list;
   DYN_16               count;
};

enum
{
   STATGROUP_RUSHING,               //  0
   STATGROUP_PASSING,               //  1
   STATGROUP_RECEIVING,             //  2
   STATGROUP_INTERCEPTIONS,         //  3
   STATGROUP_FUMBLES,               //  4
   STATGROUP_FUMBLE_RECOVERIES,     //  5
   STATGROUP_SACKS,                 //  6
   STATGROUP_SAFETIES,              //  7
   STATGROUP_TACKLES,               //  8
   STATGROUP_PUNTING,               //  9
   STATGROUP_PUNT_RETURNS,          // 10
   STATGROUP_KICKOFF_RETURNS,       // 11
   STATGROUP_PAT,                   // 12
   STATGROUP_FIELDGOAL,             // 13
   STATGROUP_PAT2,                  // 14
   STATGROUP_PENALTY,               // 15
};

enum StatTimeFrame
{
      STATTIMEFRAME_GAME,
      STATTIMEFRAME_SEASON,
      STATTIMEFRAME_CAREER,
      STATTIMEFRAME_MAX,
};

// PGSTAT = individual player GAME stat
// PSSTAT = individual player SEASON stat
// PCSTAT = individual player CAREER stat
// TGSTAT = team GAME stat
// TSSTAT = team SEASON stat
// TOGSTAT = team's Opponent GAME stat
// TOSSTAT = team SEASON stat

const PSTAT_SET_OFFSET = 32;
const TSTAT_SET_OFFSET = 34;

enum StatType
{
      STAT_ERR = -1,

      PGSTAT_PAT = (PSTAT_SET_OFFSET * 0),   // individual player Game stat
      PGSTAT_FG,
      PGSTAT_RUSH,
      PGSTAT_PASS,
      PGSTAT_PASSREC,
      PGSTAT_INT,
      PGSTAT_PUNT,
      PGSTAT_PUNTRET,
      PGSTAT_KICKRET,
      PGSTAT_FUMBLE,
      PGSTAT_FUMBLEREC,
      PGSTAT_SACK,
      PGSTAT_SAFETY,
      PGSTAT_TACKLE,
      PGSTAT_HOF,       // hall of fame award points // HOF JWM 9/11/96 // team's first downs
      PGSTAT_POG,       // player of game // HOF JWM 9/12/96  // team's downs attempted
      PGSTAT_PENALTY,  // team's misc
      PGSTAT_PAT2,      // 2 point conversions
      PGSTAT_MAX,

      PSSTAT_PAT = (PSTAT_SET_OFFSET * 1),   // individual player Season stat
      PSSTAT_FG,
      PSSTAT_RUSH,
      PSSTAT_PASS,
      PSSTAT_PASSREC,
      PSSTAT_INT,
      PSSTAT_PUNT,
      PSSTAT_PUNTRET,
      PSSTAT_KICKRET,
      PSSTAT_FUMBLE,
      PSSTAT_FUMBLEREC,
      PSSTAT_SACK,
      PSSTAT_SAFETY,
      PSSTAT_TACKLE,
      PSSTAT_HOF,       // hall of fame award points // HOF JWM 9/11/96 // team's first downs
      PSSTAT_APTEAM,    // this years all proteam
      PSSTAT_UNUSED_3,  // team's misc
      PSSTAT_PAT2,      // 2 point conversions
      PSSTAT_MAX,

      PCSTAT_PAT = (PSTAT_SET_OFFSET * 2),   // individual player Career stat
      PCSTAT_FG,
      PCSTAT_RUSH,
      PCSTAT_PASS,
      PCSTAT_PASSREC,
      PCSTAT_INT,
      PCSTAT_PUNT,
      PCSTAT_PUNTRET,
      PCSTAT_KICKRET,
      PCSTAT_FUMBLE,
      PCSTAT_FUMBLEREC,
      PCSTAT_SACK,
      PCSTAT_SAFETY,
      PCSTAT_TACKLE,
      PCSTAT_HOF,       // hall of fame award points // HOF JWM 9/11/96 // team's first downs
      PCSTAT_MVPROY,    // // HOF JWM 9/21/96 Most Valued players and Rookie o'year
      PCSTAT_HALLOFAME,  // team's misc
      PCSTAT_PAT2,      // 2 point conversions
      PCSTAT_MAX,

      TGSTAT_PAT = (PSTAT_SET_OFFSET * 3),   // team Game stat
      TGSTAT_FG,
      TGSTAT_RUSH,
      TGSTAT_PASS,
      TGSTAT_PASSREC,
      TGSTAT_INT,
      TGSTAT_PUNT,
      TGSTAT_PUNTRET,
      TGSTAT_KICKRET,
      TGSTAT_FUMBLE,
      TGSTAT_FUMBLEREC,
      TGSTAT_SACK,
      TGSTAT_SAFETY,
      TGSTAT_TACKLE,
      TGSTAT_DOWNSFIRST,
      TGSTAT_DOWNSATT,
      TGSTAT_MISC,
      TGSTAT_PAT2,      // 2 point conversions
      TGSTAT_MAX,

      TSSTAT_PAT = (TGSTAT_PAT + TSTAT_SET_OFFSET * 1),  // team Season stat
      TSSTAT_FG,
      TSSTAT_RUSH,
      TSSTAT_PASS,
      TSSTAT_PASSREC,
      TSSTAT_INT,
      TSSTAT_PUNT,
      TSSTAT_PUNTRET,
      TSSTAT_KICKRET,
      TSSTAT_FUMBLE,
      TSSTAT_FUMBLEREC,
      TSSTAT_SACK,
      TSSTAT_SAFETY,
      TSSTAT_TACKLE,
      TSSTAT_DOWNSFIRST,
      TSSTAT_DOWNSATT,
      TSSTAT_MISC,
      TSSTAT_PAT2,      // 2 point conversions
      TSSTAT_MAX,

      OTGSTAT_PAT = (TGSTAT_PAT + TSTAT_SET_OFFSET * 2), // team's Opponent Game stat
      OTGSTAT_FG,
      OTGSTAT_RUSH,
      OTGSTAT_PASS,
      OTGSTAT_PASSREC,
      OTGSTAT_INT,
      OTGSTAT_PUNT,
      OTGSTAT_PUNTRET,
      OTGSTAT_KICKRET,
      OTGSTAT_FUMBLE,
      OTGSTAT_FUMBLEREC,
      OTGSTAT_SACK,
      OTGSTAT_SAFETY,
      OTGSTAT_TACKLE,
      OTGSTAT_DOWNSFIRST,
      OTGSTAT_DOWNSATT,
      OTGSTAT_MISC,
      OTGSTAT_PAT2,      // 2 point conversions
      OTGSTAT_MAX,

      OTSSTAT_PAT = (TGSTAT_PAT + TSTAT_SET_OFFSET * 3), // team's Opponent Season stat
      OTSSTAT_FG,
      OTSSTAT_RUSH,
      OTSSTAT_PASS,
      OTSSTAT_PASSREC,
      OTSSTAT_INT,
      OTSSTAT_PUNT,
      OTSSTAT_PUNTRET,
      OTSSTAT_KICKRET,
      OTSSTAT_FUMBLE,
      OTSSTAT_FUMBLEREC,
      OTSSTAT_SACK,
      OTSSTAT_SAFETY,
      OTSSTAT_TACKLE,
      OTSSTAT_DOWNSFIRST,
      OTSSTAT_DOWNSATT,
      OTSSTAT_MISC,
      OTSSTAT_PAT2,      // 2 point conversions
      OTSSTAT_MAX,
};

enum StatSort
{
      // Base
      STATSORT_NUMBER,

      // Sum
      STATSORT_YARDS = STATSORT_NUMBER + 1,
      STATSORT_BEST,
      STATSORT_TDS,

      // PAT
      STATSORT_MADE = STATSORT_NUMBER + 1,

      // Pass
      STATSORT_COMPS = STATSORT_TDS + 1,
      STATSORT_INTS,
      STATSORT_SACKS,
      STATSORT_SACK_YARDS,

      // Punt
      STATSORT_BLOCKS = STATSORT_TDS + 1,
      STATSORT_RETS,
      STATSORT_RETYARDS,

      // PuntRet
      STATSORT_FAIRS = STATSORT_TDS + 1,

      // Rushing/Receiving
      STATSORT_RR = 32,

      // Passing
      STATSORT_PASS,
};

// these refer to the index of an array of DYN_16s
enum StatOffsets
{
      STATOFFSET_TYPE,
      STATOFFSET_ID,
      STATOFFSET_NUMBER,

      STATOFFSET_YARDS = STATOFFSET_NUMBER + 1,
      STATOFFSET_BEST,
      STATOFFSET_TDS,

      STATOFFSET_MADE = STATOFFSET_NUMBER + 1,

      STATOFFSET_LONGEST = STATOFFSET_NUMBER,
      STATOFFSET_RANGE10,
      STATOFFSET_RANGE20,
      STATOFFSET_RANGE30,
      STATOFFSET_RANGE40,
      STATOFFSET_RANGE50,

      STATOFFSET_PLAYERID   = STATOFFSET_NUMBER + 1,

      STATOFFSET_CONFINDEX  = STATOFFSET_NUMBER + 1,

      STATOFFSET_POINTCOUNT = STATOFFSET_NUMBER + 1,

      STATOFFSET_COMPS = STATOFFSET_TDS + 1,
      STATOFFSET_INTS,
      STATOFFSET_SACKS,
      STATOFFSET_SACK_YARDS,

      STATOFFSET_BLOCKS = STATOFFSET_TDS + 1,
      STATOFFSET_RETS,
      STATOFFSET_RETYARDS,
      STATOFFSET_TOUCHBACKS,

      STATOFFSET_FAIRS = STATOFFSET_TDS + 1,

      STATOFFSET_TRUSHING = STATOFFSET_NUMBER + 1,
      STATOFFSET_TPASSING,
      STATOFFSET_TPENALTY,

      STATOFFSET_TATT3 = STATOFFSET_NUMBER + 1,
      STATOFFSET_TMADE3,
      STATOFFSET_TATT4,
      STATOFFSET_TMADE4,

      STATOFFSET_TPENALTIES = STATOFFSET_NUMBER + 1,
      STATOFFSET_TYARDSLOST,
      STATOFFSET_TTIME,
      STATOFFSET_TFINAL,
      STATOFFSET_TFUMLOST,
};

// Summation (number, total, best, TDs)
class StatisticSum : public Statistic
{
private:
//    member functions

//    data members


public:
//    member functions

//    data members
      struct Data
      {
         DYN_16      total;
         DYN_U_16    best,
                     tds;
      };
      Data           data;

public:
//    member functions
                     StatisticSum ();
                     ~StatisticSum ();

      virtual void   Load (FILE *, DYN_16, DYN_16);
      virtual void   FillBuff (DYN_U_16 FAR *);
      virtual void   FillBuff (DYN_U_16 FAR *, DYN_16, DYN_16);
      virtual void   ReadBuff (DYN_U_16 FAR *);
      virtual void   UpdateBuff (DYN_U_16 FAR *);

//    data members
};

// Point After Touchdowns
class StatisticPAT : public Statistic
{
private:
//    member functions

//    data members


public:
//    member functions

//    data members
      struct Data
      {
         DYN_U_16    made;
      };
      Data           data;

public:
//    member functions
                     StatisticPAT ();
                     ~StatisticPAT ();

      virtual void   Load (FILE *, DYN_16, DYN_16);
      virtual void   FillBuff (DYN_U_16 FAR *);
      virtual void   FillBuff (DYN_U_16 FAR *, DYN_16, DYN_16);
      virtual void   ReadBuff (DYN_U_16 FAR *);
      virtual void   UpdateBuff (DYN_U_16 FAR *);

//    data members
};

// Field Goals
class StatisticFG : public Statistic
{
public:
//    data members
      struct Data
      {
         DYN_U_16    range10,    //  number attempted/made in each byte
                     range20,
                     range30,
                     range40,
                     range50;
      };
      Data           data;

//    member functions
                     StatisticFG ();
                     ~StatisticFG ();

      virtual void   Load (FILE *, DYN_16, DYN_16);
      virtual void   FillBuff (DYN_U_16 FAR *);
      virtual void   FillBuff (DYN_U_16 FAR *, DYN_16, DYN_16);
      virtual void   ReadBuff (DYN_U_16 FAR *);
      virtual void   UpdateBuff (DYN_U_16 FAR *);

//    data members
};

// Rushing
class StatisticRush : public StatisticSum
{
private:
//    member functions

//    data members


protected:
//    member functions

//    data members


public:
//    member functions
                     StatisticRush ();
                     ~StatisticRush ();

//    data members
};

// Passing
class StatisticPass : public StatisticSum
{
private:
//    member functions

//    data members


public:
//    member functions

//    data members
      struct Data
      {
         DYN_U_16    comps,
                     ints,
                     sacks,
                     sackYards;
      };
      Data           data;

public:
//    member functions
                     StatisticPass ();
                     ~StatisticPass ();

      virtual void   Load (FILE *, DYN_16, DYN_16);
      virtual void   FillBuff (DYN_U_16 FAR *);
      virtual void   FillBuff (DYN_U_16 FAR *, DYN_16, DYN_16);
      virtual void   ReadBuff (DYN_U_16 FAR *);
      virtual void   UpdateBuff (DYN_U_16 FAR *);

//    data members
};

// Pass Receiving
class StatisticPassRec : public StatisticSum
{
private:
//    member functions

//    data members


protected:
//    member functions

//    data members


public:
//    member functions
                     StatisticPassRec ();
                     ~StatisticPassRec ();

//    data members
};

// Interceptions
class StatisticInt : public StatisticSum
{
private:
//    member functions

//    data members


protected:
//    member functions

//    data members


public:
//    member functions
                     StatisticInt ();
                     ~StatisticInt ();

//    data members
};

// Punting
class StatisticPunt : public StatisticSum
{
private:
//    member functions

//    data members


public:
//    member functions

//    data members
      struct Data
      {
         DYN_U_16    blocks,
                     rets;
         DYN_16      retYards;
         DYN_U_16    touchBacks;
      };
      Data           data;

public:
//    member functions
                     StatisticPunt ();
                     ~StatisticPunt ();

      virtual void   Load (FILE *, DYN_16, DYN_16);
      virtual void   FillBuff (DYN_U_16 FAR *);
      virtual void   FillBuff (DYN_U_16 FAR *, DYN_16, DYN_16);
      virtual void   ReadBuff (DYN_U_16 FAR *);
      virtual void   UpdateBuff (DYN_U_16 FAR *);

//    data members
};

// Punt returns
class StatisticPuntRet : public StatisticSum
{
private:
//    member functions

//    data members


public:
//    member functions

//    data members
      struct Data
      {
         DYN_U_16    fairs;
      };
      Data           data;

public:
//    member functions
                     StatisticPuntRet ();
                     ~StatisticPuntRet ();

      virtual void   Load (FILE *, DYN_16, DYN_16);
      virtual void   FillBuff (DYN_U_16 FAR *);
      virtual void   FillBuff (DYN_U_16 FAR *, DYN_16, DYN_16);
      virtual void   ReadBuff (DYN_U_16 FAR *);
      virtual void   UpdateBuff (DYN_U_16 FAR *);

//    data members
};

// Kickoff returns
class StatisticKickRet : public StatisticSum
{
private:
//    member functions

//    data members


protected:
//    member functions

//    data members


public:
//    member functions
                     StatisticKickRet ();
                     ~StatisticKickRet ();

//    data members
};

// Fumbles
class StatisticFumble : public Statistic
{
private:
//    member functions

//    data members


protected:
//    member functions

//    data members


public:
//    member functions
                     StatisticFumble ();
                     ~StatisticFumble ();

//    data members
};

// Fumble recoveries
class StatisticFumbleRec : public StatisticSum
{
private:
//    member functions

//    data members


protected:
//    member functions

//    data members


public:
//    member functions
                     StatisticFumbleRec ();
                     ~StatisticFumbleRec ();

//    data members
};

// Sacks
class StatisticSack : public Statistic
{
private:
//    member functions

//    data members


protected:
//    member functions

//    data members


public:
//    member functions
                     StatisticSack ();
                     ~StatisticSack ();

//    data members
};

// Safeties
class StatisticSafety : public Statistic
{
private:
//    member functions

//    data members


protected:
//    member functions

//    data members


public:
//    member functions
                     StatisticSafety ();
                     ~StatisticSafety ();

//    data members
};

// Tackles
class StatisticTackle : public Statistic
{
private:
//    member functions

//    data members


protected:
//    member functions

//    data members


public:
//    member functions
                     StatisticTackle ();
                     ~StatisticTackle ();

//    data members
};


class StatisticPenalty : public Statistic
{
private:
//    member functions

//    data members


protected:
//    member functions

//    data members


public:
//    member functions
                     StatisticPenalty ();
                     ~StatisticPenalty ();

//    data members
};


// hall of fame // HOF JWM 9/11/96
class  StatisticHOF : public Statistic
{
private:
//    member functions

//    data members


protected:
//    member functions

//    data members


public:
//    member functions
                     StatisticHOF ();
                     ~StatisticHOF ();

      virtual void   Load (FILE *, DYN_16, DYN_16);
      virtual void   FillBuff (DYN_U_16 FAR *);
      virtual void   FillBuff (DYN_U_16 FAR *, DYN_16, DYN_16);
      virtual void   ReadBuff (DYN_U_16 FAR *);
      virtual void   UpdateBuff (DYN_U_16 FAR *);

      //    data members
      struct Data
      {
         DYN_U_16 confIndex ;
         DYN_16   pointCount ;
      };
      Data           data;

};


// hall of fame // HOF JWM 9/26/96
class StatisticHALLOFAME : public Statistic
{
private:
//    member functions

//    data members


protected:
//    member functions

//    data members


public:
//    member functions
                     StatisticHALLOFAME ();
                     ~StatisticHALLOFAME ();

      virtual void   Load (FILE *, DYN_16, DYN_16);
      virtual void   FillBuff (DYN_U_16 FAR *);
      virtual void   FillBuff (DYN_U_16 FAR *, DYN_16, DYN_16);
      virtual void   ReadBuff (DYN_U_16 FAR *);
      virtual void   UpdateBuff (DYN_U_16 FAR *);

      //    data members
      struct Data
      {
         DYN_16   pointCount ;
         DYN_U_16 YearRetired ;
         DYN_U_16 YearElected ;
      };
      Data           data;

};


// hall of fame // HOF JWM 9/12/96
class StatisticPOG : public Statistic
{
private:
//    member functions

//    data members


protected:
//    member functions

//    data members


public:
//    member functions
                     StatisticPOG ();
                     ~StatisticPOG ();

      virtual void   Load (FILE *, DYN_16, DYN_16);
      virtual void   FillBuff (DYN_U_16 FAR *);
      virtual void   FillBuff (DYN_U_16 FAR *, DYN_16, DYN_16);
      virtual void   ReadBuff (DYN_U_16 FAR *);
      virtual void   UpdateBuff (DYN_U_16 FAR *);

//    data members
      struct Data
      {
         DYN_U_16 playerId ;
         DYN_U_16 teamId ;
         DYN_U_16 opponentTeamId ;
      };
      Data           data;
};



// TEAM ONLY STATISTICS:
// first downs made
class StatisticTeamFirstDowns : public Statistic
{
private:
//    member functions

//    data members


public:
//    member functions

//    data members
      struct Data
      {
         DYN_U_16    rushing,
                     passing,
                     penalty;
      };
      Data           data;

public:
//    member functions
                     StatisticTeamFirstDowns ();
                     ~StatisticTeamFirstDowns ();

      virtual void   FillBuff (DYN_U_16 FAR *);
      virtual void   FillBuff (DYN_U_16 FAR *, DYN_16, DYN_16);
      virtual void   ReadBuff (DYN_U_16 FAR *);
      virtual void   UpdateBuff (DYN_U_16 FAR *);

//    data members
};

// significant downs attempted
class StatisticTeamAttDowns : public Statistic
{
private:
//    member functions

//    data members


public:
//    member functions

//    data members
                     // (base class Statistic's number holds OTHER team's id)
      struct Data
      {
         DYN_U_16    attempt3,
                     made3,
                     attempt4,
                     made4;
      };
      Data           data;

public:
//    member functions
                     StatisticTeamAttDowns ();
                     ~StatisticTeamAttDowns ();

      virtual void   FillBuff (DYN_U_16 FAR *);
      virtual void   FillBuff (DYN_U_16 FAR *, DYN_16, DYN_16);
      virtual void   ReadBuff (DYN_U_16 FAR *);
      virtual void   UpdateBuff (DYN_U_16 FAR *);

//    data members
};

// catch-all
class StatisticTeamMisc : public Statistic
{
private:
//    member functions

//    data members


public:
//    member functions

//    data members
                     // (base class Statistic's number holds OTHER team's id)
      struct Data
      {
         DYN_U_16    penalties,
                     yardsLost,
                     timePossession,
                     finalScore,
                     fumblesLost;
      };
      Data           data;

public:
//    member functions
                     StatisticTeamMisc ();
                     ~StatisticTeamMisc ();

      virtual void   FillBuff (DYN_U_16 FAR *);
      virtual void   FillBuff (DYN_U_16 FAR *, DYN_16, DYN_16);
      virtual void   ReadBuff (DYN_U_16 FAR *);
      virtual void   UpdateBuff (DYN_U_16 FAR *);

//    data members
};

#endif   // _STATDRVD_H
