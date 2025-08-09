/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   roster.h

   the Roster class

   GJW: 2-29-1992, original creation
   GJW: 10-1-1994, 63 (46 + 7 + 10) player rosters
   GJW: 8-26-1996, multi-player trades (rather than 1 player, an array
                     of player pointers is passed)
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _ROSTER_H
#define  _ROSTER_H

#include "lgefile.h"
#include "plyrlist.h"
#include "trade.h"

struct   FAR RosData;
struct   RosterDiskData;
class    PoolAlloc;
class    TeamNews;


#define  MANAGE_PRESEASON 0
#define  MANAGE_WEEKLY 1
#define  MANAGE_POSTGAME 2
#define  MANAGE_OWNERSHIP 3

class Roster
{
friend RosData;
friend RosterDiskData;

public:
   enum SlotCounts
   {
         NUM_QB_SLOTS =        2,
         NUM_RB_SLOTS =        3,
         NUM_REC_SLOTS =       5,
         NUM_OL_SLOTS =        6,
         NUM_DL_SLOTS =        4,
         NUM_LB_SLOTS =        5,
         NUM_DB_SLOTS =        7,
         NUM_K_SLOTS =         1,
         NUM_P_SLOTS =         1,
         NUM_OPEN_SLOTS =     12,
         NUM_INACTIVE_SLOTS =  7,
         NUM_INJ_SLOTS =      10,
         NUM_ROSTER_PLAYERS = 63,
         NUM_GAME_PLAYERS =   46,
         NUM_NONINJ_PLAYERS = NUM_ROSTER_PLAYERS - NUM_INJ_SLOTS,
   };

   enum SlotId
   {
         NO_SLOT = -1,
         QB_SLOT, RB_SLOT, REC_SLOT, OL_SLOT,
         DL_SLOT, LB_SLOT, DB_SLOT,
         K_SLOT,
         P_SLOT,
         OPEN_SLOT, INACTIVE_SLOT, INJ_SLOT,
         MAX_SLOT,
         OFF_SLOT,
         DEF_SLOT,
         MAX_DRAFT_LIST = OPEN_SLOT,
   };

   enum RosterSort
   {
         ROSTERSORT_ALLEN,
         ROSTERSORT_ALPHA,
         ROSTERSORT_NUMBER,
         ROSTERSORT_POSITION,
         ROSTERSORT_ABR,
         ROSTERSORT_INJURY,
         ROSTERSORT_IRSELECT,
         ROSTERSORT_SPECABR,
         ROSTERSORT_IRSPECABR,
         ROSTERSORT_RATINGS,
         ROSTERSORT_POSDEPTH,
         ROSTERSORT_POSRATINGS,
   };

//   enum ManageRoster
//   {
//         MANAGE_PRESEASON,
//         MANAGE_WEEKLY,
//         MANAGE_POSTGAME,
//         MANAGE_OWNERSHIP,
//   };

   enum DraftStatList
   {
         STATLIST_PASS,
         STATLIST_RUSH,
         STATLIST_ORUSH,
         STATLIST_OPASS,
         STATLIST_SACK,
         STATLIST_FG,
         STATLIST_PUNT,
         STATLIST_MAX,
   };

   enum DraftStrategy
   {
         DRAFTSTRAT_RUN_INSIDE, DRAFTSTRAT_RUN_OUTSIDE,
         DRAFTSTRAT_PASS_LONG, DRAFTSTRAT_PASS_SHORT,
         DRAFTSTRAT_DEFEND_RUN, DRAFTSTRAT_DEFEND_PASS,
         DRAFTSTRAT_RUSH_PASS,
         DRAFTSTRAT_KICKING, DRAFTSTRAT_PUNTING,
         DRAFTSTRAT_TRADE,
         DRAFTSTRAT_MAX,
   };

   enum DraftSelectType
   {
         DSTYPE_COLLEGE,
         DSTYPE_SUP,
         DSTYPE_WEEKLY,
         DSTYPE_GAMEREADY,
         DSTYPE_CEI,
   };

   struct FAR RosterPlayer
   {
      DYN_16      pId;
      RosPlayer   *pPtr;
      DYN_8       jersey;
   };

   struct FAR RosterSlots
   {
      RosterPlayer   qb [NUM_QB_SLOTS],
                     rb [NUM_RB_SLOTS],
                     rec [NUM_REC_SLOTS],
                     ol [NUM_OL_SLOTS],
                     dl [NUM_DL_SLOTS],
                     lb [NUM_LB_SLOTS],
                     db [NUM_DB_SLOTS],
                     k [NUM_K_SLOTS],
                     p [NUM_P_SLOTS],
                     open [NUM_OPEN_SLOTS],
                     inActive [NUM_INACTIVE_SLOTS],
                     inj [NUM_INJ_SLOTS];
   };

   struct FAR TrainAllocs
   {
      DYN_8          ac,      // acceleration
                     ag,      // agility
                     di,      // discipline
                     en,      // endurance
                     ha,      // hands
                     in,      // intelligence
                     sp,      // speed
                     st;      // strength
   };
   enum { TA_SIZE = sizeof (TrainAllocs) * P_SLOT };
   typedef void (*V_TAP_PLP) (TrainAllocs *taPtr,
                              const RosPlayer::Pos pos,
                              GenList<RosPlayer> *);

   enum { RW_SIZE = sizeof (RatingWeights) * RosPlayer::POS_MAX_SPEC };

private:
   // member functions

   // data members


protected:
   struct FAR DraftStrats
   {
      DYN_8    stratRanks [DRAFTSTRAT_MAX];
   };

   // member functions
   void           NEAR Load (ReadBFile &rlf, DYN_16 rId);
   void           NEAR LoadSlotIds (RosterDiskData *rddPtr);
   void           NEAR LoadSlotJerseys (RosterDiskData *rddPtr);
   void           NEAR LoadStrategies (RosterDiskData *rddPtr);
   void           NEAR LoadTraining (RosterDiskData *rddPtr);
   void           NEAR LoadWeights (RosterDiskData *rddPtr);
   void           NEAR LoadSlotPlayers (PlayerRdWr &prw);
   void           NEAR WriteSlotIds (RosterDiskData *rddPtr);
   void           NEAR WriteSlotJerseys (RosterDiskData *rddPtr);
   void           NEAR WriteStrategies (RosterDiskData *rddPtr);
   void           NEAR WriteTraining (RosterDiskData *rddPtr);
   void           NEAR WriteWeights (RosterDiskData *rddPtr);
   void           NEAR WriteSlotPlayers (PlayerRdWr &prw);
   void           CheckSlotPlayers ();
   DYN_16         NEAR AddPlayerList (GenList<RosPlayer> *pList,
                                       const RosPlayer::Pos pos,
                                       const SlotId sId,
                                       const DYN_16 manageFlag);
   DYN_32         NEAR AddPlayerList (GenList<RosPlayer> *pList,
                                       const RosPlayer::Pos pos,
                                       const SlotId sId,
                                       const RosPlayer *pPtr,
                                       const DYN_32 manageFlag);
   DYN_16         NEAR ManageSlots ();
   DYN_16         NEAR ManagePreSeason ();
   DYN_16         NEAR ManageWeekly ();
   DYN_16         NEAR ManagePostGame ();
   RosterPlayer   * NEAR FindFirstSlot (RosPlayer::Pos pos, DYN_16 injFlag);
   RosterPlayer   * NEAR FindOpenSlot (RosPlayer::Pos pos);
   RosterPlayer   * NEAR GetRPPtr (const SlotId sId);
   DYN_16         NEAR MoveToOpen (RosPlayer *pPtr);
   DYN_16         NEAR SlotPosValid (RosterPlayer *, RosterPlayer *, RosPlayer *);
   DYN_16         NEAR DraftCollege (GenList<RosPlayer> * [], DYN_U_16 *);
   DYN_16         NEAR DraftSup (GenList<RosPlayer> * []);
   DYN_16         NEAR DraftWeekly (GenList<RosPlayer> * [], DYN_U_16 *);
   DYN_16         NEAR DraftGameReady (GenList<RosPlayer> * [], DraftSelectType);
   DYN_16         NEAR DraftSelect (GenList<RosPlayer> * [], DYN_16, DYN_16, DYN_16);
   void           NEAR BubbleDepth (RosPlayer::Pos pos);
   void           NEAR TrainFunc (V_TAP_PLP, TrainAllocs *, GenList<RosPlayer> *);
   DYN_16         NEAR PlayerBetter (RosPlayer *pPtr);
   void           NEAR RetirePlayer (TeamNews &tn, RosterPlayer *rpPtr);
   // GJW: 9-27-1995
   void           NEAR GetInjCounts (DYN_U_16 *injCounts);

   // data members
   static PoolAlloc  *pa;        // where to get memory from
   static DYN_16     weeksToGo;  // how much of REGULAR season is left,
                                 // for use in deciding if players should
                                 // go on the DL
   RosData           *data;

public:
   // member functions
   static void    Init (PoolAlloc *paPtr);
   static void    DeInit ();
   static RosPlayer::Pos   SlotToPos (SlotId sId);
   static SlotId  PosToSlot (RosPlayer::Pos pos);
   static DYN_16  RemEmptyPlayerList (GenList<RosPlayer> *pList,
                                                SlotId sId);
   static DYN_16  RemIRPlayerList (GenList<RosPlayer> *pList);
   static DYN_16  NumPlayersTradeProof ();
   static void    GetExtRatWeights (RatingWeights *erwPtr);
   static void    SetExtRatWeights (RatingWeights *erwPtr);
   static void    SetWeeksToGo (const DYN_16 wTG);

   void           *operator new (size_t size);
   void           operator delete (void *t, size_t size);

                  Roster (ReadBFile &rlf, DYN_16 rId);
                  Roster ();
                  ~Roster ();

   void           LoadRoster (ReadBFile &rlf, DYN_16 rId);
   DYN_16         LoadPlayers (PlayerRdWr *prwPtr);
   DYN_16         SavePlayers (PlayerRdWr &prw);
   void           DeletePlayers ();
   void           LoadNewPlayers (DYN_8 *path, DYN_8 *fileName,
                                    DYN_16 srcId, DYN_16 thisId);

   void           Save (WriteBFile &wlf);
   DYN_16         SaveNewPlayers (PlayerRdWr &prw, DYN_16 newId);

   RosPlayer      *GetPlayer (SlotId sId, DYN_16 index);
   RosPlayer      *GetPlayer (DYN_16 id);

   DYN_16         GetJersey (DYN_16 pId);
   void           SetJersey (DYN_16 pId, DYN_16 number);

                  // the trailing DYN_16s control empty & injured selection
                  // all players on the team
   DYN_16         BuildPlayerList (GenList<RosPlayer> *pList,
                                    const DYN_16 emptyFlag,
                                    const DYN_16 injFlag);

                  // all players that play a certain position on the team
   DYN_16         BuildPlayerList (GenList<RosPlayer> *pList,
                                    const RosPlayer::Pos pos,
                                    const DYN_16 assEmptyFlag,
                                    const DYN_16 openEmptyFlag,
                                    const DYN_16 inActEmptyFlag,
                                    const DYN_16 injFlag,
                                    const DYN_16 injEmptyFlag);
   DYN_16         BuildPlayerList (GenList<RosPlayer> *pList,
                                    RosPlayer::Pos *positions);

                  // all players in a slot
   DYN_16         BuildPlayerList (GenList<RosPlayer> *pList,
                                    const SlotId sId,
                                    const DYN_16 emptyFlag);
   DYN_16         BuildPlayerList (GenList<RosPlayer> *pList,
                                    const RosPlayer::Pos pos,
                                    const SlotId sId,
                                    const DYN_16 emptyFlag);

   DYN_16         BuildPlayerList (GenList<RosPlayer> *pList []);

   void           SortPlayerList (GenList<RosPlayer> *pList, RosterSort sortType);
   void           CountProBowlPlayers (DYN_16 *pCounts);
   DYN_16         PickPlayers (GenList<RosPlayer> *pList [],
                                 DYN_16 newId,
                                 DYN_16 thisId);

   // new for All Pro Team JWM 12/26/96
   DYN_16         PickPlayers (GenList<RosPlayer> *pList,
                               DYN_16 newId,
                               DYN_16 thisId) ;

   DYN_16         RemoveNoTrades (GenList<RosPlayer> *pList,
                                    const RosPlayer *pPtr,
                                    const DYN_16 manageFlag) const;
   void           PoolPlayers (RosPlayerList &rplRef);

   DYN_16         ChangeSlots (GenList<RosPlayer> *pList, DYN_16 last, DYN_16 cur);
   DYN_16         SetSlots (DYN_16 *aIds, DYN_16 *oIds, DYN_16 *iIds);
   RosPlayer      *GetEmptyInj ();
   RosPlayer      *GetEmptyPlayer (RosPlayer *pPtr);
   DYN_16         GetNumIRPlayers ();

   DYN_8          NewJersey (RosPlayer *pPtr, DYN_16 pId);
   DYN_16         SetJerseyBounds (RosPlayer::Pos pos,
                                    DYN_8 &min1, DYN_8 &max1,
                                    DYN_8 &min2, DYN_8 &max2);
   DYN_16         VerifyNumber (RosPlayer::Pos pos, DYN_16 num, DYN_16 id);
   DYN_16         SwapPlayers (RosterPlayer *rp1, RosterPlayer *rp2);
   DYN_16         SwapPlayers (RosPlayer *p1, RosPlayer *p2);
   DYN_16         SlotPosValid (SlotId sId, RosPlayer::Pos pos);

   void           Age ();
   void           Retire (TeamNews &tn);
   DYN_16         Manage (DYN_16 mrData);
   RosPlayer      *AddPlayer (RosPlayer *newPPtr, RosPlayer *oldPPtr, DYN_16 manage);
   RosPlayer      *GetReleasePlayer (const DYN_16 forceFlag);
   void           ClearReleasePlayer ();
   DYN_16         DraftSelect (GenList<RosPlayer> *pList [], DraftSelectType, DYN_16);
   TradeStatus    TradeEval (RosPlayer **p1Ptr2, RosPlayer **p2Ptr2);
   DYN_16         WannaTrade (RosPlayer **dest,
                              SlotId *sIds, RosPlayer **ptPtr);
   DYN_32         UnBalancedTrade (DYN_16 *pIds, DYN_32 index, Roster *r);
   void           IRMove (RosPlayer *irPtr, RosPlayer *oPtr);
   DYN_16         GetDepth (RosPlayer *pPtr);
   void           SetProBowlPlayers (GenList<RosPlayer> *pList []);

   DYN_32         BuildTradePlayerList (TOManager &tom,
                                          Roster::SlotId *sIds,
                                          const RosPlayer **pPtrs,
                                          const DYN_32 manageFlag);

   DYN_16         AssignedFull ();
   void           GenerateStrategies (DYN_U_16 *ranks);
   void           HealPlayers ();
   void           SeasonReset (DYN_16 force);

   TrainAllocs    *GetTrainAllocs (SlotId sId);
   void           SetTrainAllocs (SlotId sId, TrainAllocs *taPtr);
   DYN_16         TrainAlloc ();
   void           AITrain ();
   void           Train ();
   RatingWeights  *GetRatWeights ();
   void           SetRatWeights (RatingWeights *rwPtr);

   void           Print (Printer *APrinter);

   // data members
};

#endif      // _ROSTER_H
