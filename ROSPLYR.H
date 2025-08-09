/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   player.h

   the RosPlayer class

   id             player's unique identification in this league
   pbr, abr       potential, actual basic ratings (attributes)
   injury         number of injury points and injury descriptions accumulated
                  in this season
   years          number of years played in league
   posGen         general position QB, RB, DB
   posSpecific    specific position (based on some pbr) QB, HB, S
   status
   firstName,     how humans remember people
   lastName

   GJW: 2-29-1992, original creation
   GJW: 6-12-1992, idxFile constructor
   GJW: 6-19-1992, DYN_U_32 constructor (so rosters load up idxFile data
                     into their own tables first, then create players)
   GJW: 9-9-1992, more injury info needed, EBRs are not
   GJW: 1-13-1994, added GetMaxPId () menthod that determines the maximum
                     valid player id based on the size of the file
   GJW: 9-16-1994, PoolAllocEmm implemented
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _ROSPLYR_H
#define  _ROSPLYR_H

#include    <stdio.h>
#include    <stdlib.h>

#include    "plyrrdwr.h"
//#include    "printer.h"
class Printer;

struct FAR RatingWeights
{
   DYN_U_16    ac : 4;
   DYN_U_16    ag : 4;
   DYN_U_16    di : 4;
   DYN_U_16    en : 4;
   DYN_U_16    ha : 4;
   DYN_U_16    in : 4;
   DYN_U_16    sp : 4;
   DYN_U_16    st : 4;
};

struct FAR  RosPlayerData;
class       PoolAlloc;

class FAR RosPlayer
{
public:
      enum Pos
      {
            POS_NONE = -1,
            POS_QB,     // specific positions
            POS_FB,     //  1
            POS_HB,     //  2
            POS_TE,     //  3
            POS_WR,     //  4
            POS_C,      //  5
            POS_G,      //  6
            POS_T,      //  7
            POS_DE,     //  8
            POS_DT,     //  9
            POS_LB,     // 10
            POS_CB,     // 11
            POS_S,      // 12
            POS_K,      // 13
            POS_P,      // 14

            POS_RB,     // 15
            POS_REC,    // 16
            POS_OL,     // 17
            POS_DL,     // 18
            POS_DB,     // 19
            POS_MAX,
            POS_MAX_SPEC = POS_RB,
            POS_MAX_OFF = POS_DE,
      };

      enum Rating
      {
            RATING_AC,     // acceleration
            RATING_AG,     // agility
            RATING_DI,     // discipline
            RATING_EN,     // endurance
            RATING_HA,     // hands
            RATING_IN,     // intelligence
            RATING_SP,     // speed
            RATING_ST,     // strength
            RATING_MAX,
            NO_RATING,
      };

      struct FAR Ratings
      {
         DYN_8    ratings [RATING_MAX];
      };

      enum InjuryStatus
      {
            INJURY_HEALTHY,
            INJURY_PROBABLE = 1,
            INJURY_QUESTION = 151,
            INJURY_DOUBT = 301,
            INJURY_OUT = 451,
            INJURY_OVER = 9999,
      };

      struct FAR InjuryData
      {
         DYN_16   points;
         DYN_8    type,
                  week;
      };

      enum
      {
         MIN_IR_WEEKS = 6
      };
      
private:
//    member functions

//    data members


protected:
   // member functions
   void                 NEAR Load (PlayerRdWr &, DYN_16);
   DYN_16               NEAR GetBR (const Rating prData, const Ratings *prPtr) const;
   void                 NEAR SetBR (const Rating prData, Ratings * const prPtr, DYN_16 newValue);
   DYN_16               NEAR SumBrs (DYN_8 FAR *prPtr, RatingWeights *rwPtr);
   void                 NEAR SuperStar ();

   // data members
   static PoolAlloc     *pa;        // where to memory from

   RosPlayerData        *dataPtr;   // pointer to actual data of what a player is

public:
//    member functions
      // to be called by League Init function only!
      static void       Init ();
      static void       DeInit ();

      void              FAR *operator new (size_t size);
      void              operator delete (void FAR *t, size_t size);

                        RosPlayer (PlayerRdWr &, DYN_16);
                        RosPlayer ();
                        RosPlayer (RosPlayer *pPtr);
                        ~RosPlayer ();

      void              Save (PlayerRdWr &);

      DYN_16            GetId () const;
      void              SetId (const DYN_16 newValue);
      DYN_16            GetInjPts () const;
      void              SetInjPts (const DYN_16 newValue);
      DYN_16            GetInjType () const;
      void              SetInjType (const DYN_16 newValue);
      DYN_16            GetInjWeek () const;
      void              SetInjWeek (const DYN_16 newValue);
      Pos               GetPosGen () const;
      void              SetPosGen (const Pos newValue);
      Pos               GetPosSpec () const;
      void              SetPosSpec (const Pos newValue);
      DYN_16            GetYears () const;
      void              SetYears (const DYN_16 newValue);
      DYN_16            GetStatus () const;
      void              SetStatus (const DYN_16 newValue);
      DYN_16            GetPBR (const Rating prData) const;
      void              SetPBR (const Rating prData, const DYN_16 value);
      DYN_16            GetABR (const Rating prData) const;
      void              SetABR (const Rating prData, const DYN_16 value);
      DYN_8             FAR *GetFirstName () const;
      void              SetFirstName (const DYN_8 FAR *name);
      DYN_8             FAR *GetLastName () const;
      void              SetLastName (const DYN_8 FAR *name);
      double            GetProBowlId () const;
      void              SetProBowlId (const double value);

      DYN_16            SumAbrs (RatingWeights *rwPtr);
      DYN_16            SumAbrs ( RosPlayer *p2, RosPlayer *p3 );
      DYN_16            SumPbrs (RatingWeights *rwPtr);
      DYN_16            SumInjAbrs (RatingWeights *rwPtr);

      void              Generate (const Pos pos, const DYN_16 newId,
                                    const DYN_8 FAR *fnPtr,
                                    const DYN_8 FAR *lnPtr,
                                    const Pos blessed);
      void              NewPlayer (const DYN_16 newId);
      void              SeasonReset (const DYN_16 force);
      void              Age ();

      DYN_16            Retire ();
      void              Train (Ratings *prPtr);
      void              ModRatings (const DYN_16 pbrPct, const DYN_16 abrPct);

      void              Copy (RosPlayer *);
      DYN_16            Compare (RosPlayer *);

      DYN_16            WeeksOut ();
      DYN_16            IREligible ();
      void              Heal (DYN_16);
      void              Injury (DYN_16, DYN_16);
      void              MakeNew ();

      void              Print (Printer *, DYN_16);

//    data members
};

#endif      // _ROSPLYR_H
