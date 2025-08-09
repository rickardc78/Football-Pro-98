#include "FBWIn.h"
/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   roster.cpp

   GJW: 2-29-1992, original creation
   GJW: 4-8-1992, there are no more NULL players, instead an 'empty player
                  is created (i.e. one who's general and specific positions
                  are NONE)
   GJW: 4-12-1992, changed LoadSlotPlayers () so that it doesn't read
                  in a specific player (i.e. by searching for ID), since
                  the players were written out in the order they are
                  to be read in, reading them in the same sequence is
                  faster
                  order they are kept on disk
   GJW: 6-10-1992, added LCDT define for controlling what get compiled
                   for the various libraries:
                     shell (LCDT = 0)
                     arcade (LCDT = A_LCDT)
                     editor (LCDT = ED_LCDT)
   GJW: 8-31-1992, during player selection, maximum number of certain
                     types needed to be implemented to stop a roster
                     from having 2 kickers, and no dls, lbs and dbs in
                     any open slots
   GJW: 3-5-1993, don't have to do all roster moves via OPEN slots
   GJW: 3-15-1993, modified AI for drafting/trading so that individual
                     players are examined more closely, not the positions
                     they play
   GJW: 3-24-1993, new format on disk (R00:)
   GJW: 4-7-1993, RosterMem functionality
   GJW: 4-14-1993, use GenListMems since the list size is limited to
                     NUM_GAME_PLAYERS,
                     NUM_ROSTER_PLAYERS,
                     or (NUM_DB_SLOTS + NUM_OPEN_SLOTS) is all circumstances
   GJW: 5-10-1993, ProBowl player list building & roster making
                     Note that BuildPlayerList fills an array based on
                     SPECIFIC position, not SLOTID!
   GJW: 5-24-1993, if BuildPlayerList if OFF_SLOT or DEF_SLOT, fill by
                     SPECIFIC position, so order looks better
   GJW: 7-26-1993, setTrainAllocs () concentrates on KEY RATINGS rather
                     than on the worst abr %age of each position
   GJW: 7-29-1993, teams will only search for trades 20% of the time
                     (outgoing, i.e. in ManageWeekly ())
   GJW: 8-2-1993, retirement based on depth chart so key starters don't
                  quit early
   GJW: 8-16-1993, teams DON'T HAVE TO DRAFT during college phase
   GJW: 9-22-1993, retirement based on years in league, position and depth
                     chart
   GJW: 9-23-1993, don't pick kickers/punters during college draft, or when
                     choosing 'best player overall'
   GJW: 10-11-1993, LoadPlayers () and LoadSlotPlayers () are passed the
                     largest valid player id, so that if a number greater
                     is passed, the id will be set to 0
   GJW: 1-13-1994, LoadSlotPlayers (), asks the RosPlayer object to return
                     the greatest valid id based on the file size
                     (the league's idea of the most valid player id doesn't
                     always jive with the number of players written to disk,
                     because the player can reboot before the player file
                     is flushed)
   GJW: 9-28-1994, new format on disk (R01:)
                     63 player roster featuring INACTIVE slots,
                     and expanded injured reserve
   GJW: 10-8-1994, new IR rules, once a player is on, can't come off until
                     next season
   GJW: 10-16-1994, upped the openCounts, since there are now more slots
                     to fill
   GJW: 11-15-1994, there is no screen for the user to modify the RatWeights
                     data, so at load time, just have each team read
                     from the default data, rather than the file
   GJW: 8-31-1995, reduced values in openCounts
	CAM	8 OCT 96		Removed memcpy's from ManageSlots per GJW
   JWM   26 DEC 96   Added All pro team to pro bowl PickPlayers().

컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   #includes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

// GJW: 9-20-1994, optimitzation do all kinds of awful things to this
// code (it subtracts values willy-nilly from the stack pointer, rather
// than eax, and other weirdness)
//#pragma option -Od
 
//#include       <dir.h>
#include       <dos.h>
#include       <string.h>

#include       <ports.h>
#include       "dyndefs.h"

#include       "assertvm.h"
#include       "fileutil.h"
#include       "league.h"
#include       "lcdtutil.h"
#include       "poolaloc.h"
#include			"Printer.h"
#include       "fbmu.h"
#include       "statfile.h"


/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   consts
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#define  PRINT_PLAYERS_ARR FALSE // TRUE

const NUM_ALLOC = (30 * 3) + 1;

const NUM_TRADE_SLOTS = 3; // how many positions are examined for each trade
const BIG_VALUE = 32000;   // an arbitrary high number

const MIN_RETIRE = 4;
const MAX_RETIRE = 11;
const EARLY_RETIRE_PCT = 40;
const MID_RETIRE_PCT = 40;
const MRA_MIN = 5;
const TRADE_WITHIN_PCT = 90;
const TRADE_HIT_PCT = 33;

struct FAR RosData
{
   void                 FAR * NEAR operator new (size_t size);
   void                 NEAR operator delete (void FAR *t, size_t size);

   DYN_16               NEAR AssignedFull () const;

   static PoolAlloc     *pa;  // where to get memory from

   DYN_16               id;
   Roster::RosterSlots  rs;
   Roster::DraftStrats  ds;
   Roster::TrainAllocs  trainAllocs [Roster::P_SLOT];
   RatingWeights        rw [RosPlayer::POS_MAX_SPEC];
   RosPlayer            *releasePPtr;     // who team dumps for draft choice
};

struct RosterDiskData
{
   DYN_16                  id,
                           pIds [Roster::NUM_ROSTER_PLAYERS];
   DYN_8                   jerseys [Roster::NUM_ROSTER_PLAYERS];
   Roster::DraftStrats     ds;
   Roster::TrainAllocs     trainAllocs [Roster::P_SLOT];
   RatingWeights           rw [RosPlayer::POS_MAX_SPEC];
};

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   utility prototypes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

static DYN_16                 NEAR getNumSlots (const Roster::SlotId sId);
static Roster::SlotId         NEAR posToSlot (const RosPlayer::Pos pos);
static DYN_16                 NEAR verifyNumber (const Roster::RosterPlayer *rpPtr,
                                                   const DYN_16 num,
                                                   const DYN_16 id);
static Roster::RosterPlayer   * NEAR findRosterPlayer (
                                                   const Roster::RosterSlots *rsPtr,
                                                   const RosPlayer *pPtr);
static Roster::RosterPlayer   * NEAR findRosterPlayer (
                                                   const Roster::RosterSlots *rsPtr,
                                                   const DYN_16 pId);
static Roster::RosterPlayer   * NEAR findRosterPlayer (
                                                   Roster::RosterPlayer *rpPtr,
                                                   DYN_16 sId,
                                                   const DYN_16 maxSlot,
                                                   const DYN_16 id);

#if   (LCDT == 4)
static Roster::RosterPlayer   * NEAR findCEIPlayer (const Roster::RosterSlots *rsPtr);
#endif

static void    NEAR emptyRosterPlayer (Roster::RosterPlayer *rpPtr);
static DYN_16  compareFunc (void FAR *iPtr, void FAR *plusgPtr, DYN_16 sortType);
static DYN_16  NEAR allenCompare (RosPlayer *p1, RosPlayer *p2);
static DYN_16  NEAR alphaCompare (RosPlayer *p1, RosPlayer *p2);
static DYN_16  NEAR injyCompare (RosPlayer *p1, RosPlayer *p2);
static DYN_16  NEAR irSelectCompare (RosPlayer *p1, RosPlayer *p2);
static DYN_16  NEAR numberCompare (RosPlayer *p1, RosPlayer *p2);
static DYN_16  NEAR positionCompare (RosPlayer *p1, RosPlayer *p2);
static DYN_16  NEAR abrCompare (RosPlayer *p1, RosPlayer *p2);
static DYN_16  NEAR abrSpecCompare (RosPlayer *p1, RosPlayer *p2);
static DYN_16  NEAR irAbrSpecCompare (RosPlayer *p1, RosPlayer *p2);
static void    NEAR swapDSL (DYN_8 FAR *, DYN_16, DYN_16);
static DYN_16  NEAR posToDraft (DYN_U_16 *, DYN_U_16 *);
static DYN_16  NEAR posToRelease (DYN_U_16 *, DYN_U_16 *);
static void    NEAR fillPosDrafts (DYN_U_16 *, DYN_8 FAR *);
static DYN_16  NEAR tradeProof (Roster::RosterSlots *rsPtr, RosPlayer *pPtr);

#if   (LCDT == 0)
static void    setTrainAllocs (Roster::TrainAllocs *taPtr,
                                 const RosPlayer::Pos pos,
                                 GenList<RosPlayer> *pList);
static void    executeTrainAllocs (Roster::TrainAllocs *taPtr,
                                    const RosPlayer::Pos pos,
                                    GenList<RosPlayer> *pList);
static DYN_16  NEAR ratingsCompare (RosPlayer *p1, RosPlayer *p2);
static DYN_16  NEAR posDepthCompare (RosPlayer *p1, RosPlayer *p2);
static DYN_16  NEAR posRatingsCompare (RosPlayer *p1, RosPlayer *p2);
static Roster::SlotId   NEAR getSlot (Roster::RosterSlots *rsPtr, const DYN_16 pId);
#endif

static void    NEAR encrypt (DYN_16 *ptr16);

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   global data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   local data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

INCLUDE_ASSERT;

// where to get memory from
PoolAlloc      *Roster::pa,
               *RosData::pa;
DYN_16         Roster::weeksToGo;

static DYN_8   lgeText [] = ".lge",
               pyrText [] = ".pyr";

// each draft strategy has a specific statistic it examines
static Roster::DraftStatList  FAR dslStratStat [] =
                              {
                                 Roster::STATLIST_RUSH,    // for DRAFTSTRAT_RUN_INSIDE
                                 Roster::STATLIST_RUSH,    // for DRAFTSTRAT_RUN_OUTSIDE
                                 Roster::STATLIST_PASS,    // for DRAFTSTRAT_PASS_LONG
                                 Roster::STATLIST_PASS,    // for DRAFTSTRAT_PASS_SHORT
                                 Roster::STATLIST_ORUSH,   // for DRAFTSTRAT_DEFEND_RUN
                                 Roster::STATLIST_OPASS,   // for DRAFTSTRAT_DEFEND_PASS
                                 Roster::STATLIST_SACK,    // for DRAFTSTRAT_RUSH_PASS
                                 Roster::STATLIST_FG,      // for DRAFTSTRAT_KICKING
                                 Roster::STATLIST_PUNT,    // for DRAFTSTRAT_PUNTING
                                 Roster::STATLIST_MAX,     // for DRAFTSTRAT_TRADE (i.e. none)
                              };

// each position has a special set of strategy modifiers

// when the generic shellsort algorithym calls this module's sort routine,
// the specific roster it is referring to is not passed back, only the
// 2 players who are being compared. Therefore, a pointer to the array
// of players has to be kept hanging around
static Roster::RosterSlots *rsSortPtr;
static RatingWeights       *rwSortPtr;
static Roster              *rSortPtr;
static RatingWeights       extRatWeights [RosPlayer::POS_MAX_SPEC];

static RosPlayer::Pos      ceiPos;

static RosPlayer::Pos      FAR slotToPos [Roster::MAX_DRAFT_LIST + 3] =
                           {
                              RosPlayer::POS_QB,
                              RosPlayer::POS_RB,
                              RosPlayer::POS_REC,
                              RosPlayer::POS_OL,
                              RosPlayer::POS_DL,
                              RosPlayer::POS_LB,
                              RosPlayer::POS_DB,
                              RosPlayer::POS_K,
                              RosPlayer::POS_P,
                              RosPlayer::POS_NONE,    // OPEN
                              RosPlayer::POS_NONE,    // INACTIVE
                              RosPlayer::POS_NONE,    // INJ
                           };

                           // GJW: 8-31-1995
static DYN_8               FAR openCounts [Roster::MAX_DRAFT_LIST] =
                           {
                              2,    // number of QBs     2, 1
                              5,    // number of RBs     7, 3
                              5,    // number of RECs    6, 3
                              5,    // number of OLs     6, 3
                              5,    // number of DLs     6, 3
                              4,    // number of LBs     7, 3
                              3,    // number of DBs     5, 2
                              1,    // number of Ks      1, 1
                              1,    // number of Ps      1, 1
                           };

static DYN_8               FAR tradeProofCounts [Roster::MAX_DRAFT_LIST] =
                           {
                              1,    // number of QBs
                              2,    // number of RBs
                              4,    // number of RECs
                              5,    // number of OLs
                              3,    // number of DLs
                              4,    // number of LBs
                              6,    // number of DBs
                              1,    // number of Ks
                              1,    // number of Ps
                           };

static DYN_8               FAR mandatoryYears [Roster::MAX_DRAFT_LIST] =
                           {
                              16,   // QBs retire at
                              14,   // RBs
                              13,   // RECs
                              15,   // OLs
                              14,   // DLs
                              15,   // LBs
                              13,   // DBs
                              16,   // Ks
                              12,   // Ps
                           };

//
// number of players at each SPECIFIC position, for both open & assigned
// slots, that are to be put on the all star team
//
struct ProBowlCounts
{
   DYN_8    assigned,
            open;
};

static struct ProBowlCounts   FAR pbCounts [RosPlayer::POS_MAX_SPEC] =
                              {
                                 {  2, 1 },     // qb

                                 {  2, 0 },     // fb
                                 {  1, 2 },     // hb

                                 {  2, 0 },     // te
                                 {  3, 2 },     // wr

                                 {  2, 0 },     // c
                                 {  3, 0 },     // g
                                 {  1, 2 },     // t

                                 {  4, 0 },     // de
                                 {  0, 2 },     // dt

                                 {  5, 3 },     // lb

                                 {  4, 0 },     // cb
                                 {  3, 0 },     // s

                                 {  1, 0 },     // k

                                 {  1, 0 },     // p
                              };

static DYN_8   FAR posPriority [RosPlayer::POS_MAX_SPEC] =
               {
                  14,   // RosPlayer::POS_QB
                  12,   // RosPlayer::POS_FB
                  13,   // RosPlayer::POS_HB
                  10,   // RosPlayer::POS_TE
                  11,   // RosPlayer::POS_WR
                   9,   // RosPlayer::POS_C
                   8,   // RosPlayer::POS_G
                   7,   // RosPlayer::POS_T
                   4,   // RosPlayer::POS_DE
                   3,   // RosPlayer::POS_DT
                   2,   // RosPlayer::POS_LB
                   0,   // RosPlayer::POS_CB
                   1,   // RosPlayer::POS_S
                   6,   // RosPlayer::POS_K
                   5,   // RosPlayer::POS_P
               };

static   RatingWeights  manageWeights [RosPlayer::POS_MAX_SPEC] =
                        {
                        //   ac   ag   di   en   ha   in   sp   st
                           {  0,   0,   0,   0,   0,   7,   0,   7   }, // qb
                           {  0,   0,   0,   0,   0,   0,   6,   7   }, // fb
                           {  0,   0,   0,   0,   6,   0,   7,   0   }, // hb
                           {  0,   0,   0,   0,   6,   0,   0,   7   }, // te
                           {  0,   0,   0,   0,   6,   0,   7,   0   }, // wr
                           {  0,   0,   0,   0,   7,   0,   0,   7   }, // c
                           {  0,   7,   0,   0,   0,   0,   0,   7   }, // g
                           {  7,   0,   0,   0,   0,   0,   0,   7   }, // t
                           {  8,   0,   0,   0,   0,   0,   0,   7   }, // de
                           {  0,   8,   0,   0,   0,   0,   0,   7   }, // dt
                           {  0,   0,   0,   0,   0,   0,   8,   7   }, // lb
                           {  0,   6,   0,   0,   0,   0,   7,   0   }, // cb
                           {  0,   0,   0,   0,   7,   0,   7,   0   }, // s
                           {  0,   0,   0,   0,   0,   0,   0,  14   }, // k
                           {  0,   0,   0,   0,   0,   0,   0,  14   }, // p
                        };

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/*
   ---------------------------------------------------------------------
   NAME:          Roster

   FUNCTION:      Roster object CONSTRUCTOR

   INPUT:         fp-> file with rosters in it
                  id = which one to find in the file

   RETURNS:       -> created Roster object
                  with player id's loaded
   ---------------------------------------------------------------------
*/

Roster::Roster (ReadBFile &rlf, DYN_16 rId)
{
   memset (extRatWeights, 0, RW_SIZE);
   data = new RosData;
   Load (rlf, rId);
}

#if   (LCDT == 0)
Roster::Roster ()
{
   memset (extRatWeights, 0, RW_SIZE);
   data = new RosData;
}
#endif

/*
   ---------------------------------------------------------------------
   NAME:          ~Roster

   FUNCTION:      Roster object DESTRUCTOR

   INPUT:         -> Roster object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

Roster::~Roster ()
{
   DeletePlayers ();
   delete data;
}





/*
   ---------------------------------------------------------------------
   NAME:       new/delete

   FUNCTION:   the memory allocation/deallocation overloading functions
               (I have overloaded so that memory can come from a block rather
               than piece by piece)

   INPUT:      size & ptr = mandatory parameters for overloading the
               new/delete operators

   RETURNS:    -> memory allocations
   ---------------------------------------------------------------------
*/

void *
Roster::operator new (size_t size)
{
   // if the size if wrong (indicating a derived class),
   // allocate out of conventional memory
   vmAssert (pa && (size == sizeof (Team)));
   void  *t = pa->New ();
   vmAssert (t);

   memset (t, 0, sizeof (Team));
   return t;
}

void
Roster::operator delete (void *t, size_t size)
{
   // only process items that are of the expected size
   vmAssert (pa && (size == sizeof (Team)));
   pa->Delete (t);
}



void FAR *
RosData::operator new (size_t size)
{
   // if there is no allocation object, or the size if wrong (indicating
   // a derived class), allocate out of conventional memory
   // only process items that are of the expected size
   vmAssert (pa && (size == sizeof (RosData)));

   void  FAR *t = pa->New ();
   vmAssert (t);

   // return a 'clean slate' to the constructor
   memset (t, 0, sizeof (RosData));
   return t;
}

void
RosData::operator delete (void FAR *t, size_t size)
{
   // only process items that are of the expected size
   vmAssert (pa && (size == sizeof (RosData)));
   pa->Delete (t);
}








/*
   =====================================================================
                           THE METHODS
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       Init

   FUNCTION:   initialize the pointer that operators new & delete
               use for allocating Roster object memory
               initialize pointer for RosData memory management

   INPUT:      paPtr-> object handling memory management

   RETURNS:    PoolAlloc object for Roster created
               PoolAlloc object for RosData created
   ---------------------------------------------------------------------
*/

void
Roster::Init (PoolAlloc *paPtr)
{
   pa = paPtr;
   RosData::pa = new PoolAlloc (sizeof (RosData), NUM_ALLOC);
   vmAssert (RosData::pa);
}
void
Roster::DeInit (void)
{
   delete RosData::pa;
}

#if   (LCDT == 0) || (LCDT == 4)
/*
   ---------------------------------------------------------------------
   NAME:       SlotToPos

   FUNCTION:   convert a slotId to a general player position

   INPUT:      slotId

   RETURNS:    player position for slot
               if slot is OPEN, INACTIVE or INJ, pos == NONE
   ---------------------------------------------------------------------
*/

RosPlayer::Pos
Roster::SlotToPos (SlotId sId)
{
   RosPlayer::Pos pos = RosPlayer::POS_NONE;

   switch (sId)
   {
      case QB_SLOT:
         pos = RosPlayer::POS_QB;
         break;

      case RB_SLOT:
         pos = RosPlayer::POS_RB;
         break;

      case REC_SLOT:
         pos = RosPlayer::POS_REC;
         break;

      case OL_SLOT:
         pos = RosPlayer::POS_OL;
         break;

      case DL_SLOT:
         pos = RosPlayer::POS_DL;
         break;

      case LB_SLOT:
         pos = RosPlayer::POS_LB;
         break;

      case DB_SLOT:
         pos = RosPlayer::POS_DB;
         break;

      case K_SLOT:
         pos = RosPlayer::POS_K;
         break;

      case P_SLOT:
         pos = RosPlayer::POS_P;
         break;
   }

   return pos;
}

/*
   ---------------------------------------------------------------------
   NAME:       PosToSlot

   FUNCTION:   given a player's position (general or specific),
               return the slot id that matches

   INPUT:      pos = position

   RETURNS:    corresponding slot
   ---------------------------------------------------------------------
*/

Roster::SlotId
Roster::PosToSlot (RosPlayer::Pos pos)
{
   return posToSlot (pos);
}

/*
   ---------------------------------------------------------------------
   NAME:       RemEmptyPlayerList

   FUNCTION:   given a list created by one of the above functions,
               remove any empty players (i.e. pos = NONE)

   INPUT:      pList -> list of players
               sId = removal control
                     (NO_SLOT = ANY empty
                     otherwise, only those that match)

   RETURNS:    sizeof list after deletions
   ---------------------------------------------------------------------
*/

DYN_16
Roster::RemEmptyPlayerList (GenList<RosPlayer> *pList, SlotId sId)
{
   DYN_16   nP = pList->size ();

   if (nP)
   {
      pList->start ();

      while (nP--)
      {
         RosPlayer   *pPtr = (*pList) ();

         // take out all empty players,
         // who match the given slot type (usually OPEN)
         // if sId == NO_SLOT, remove ALL emptys
         if ((pPtr->GetPosGen () == RosPlayer::POS_NONE) &&
               ((sId == NO_SLOT) || (pPtr->GetStatus () == sId)))
            pList->linkout ();

         else
            ++(*pList);
      }
   }

   return pList->size ();
}

/*
   ---------------------------------------------------------------------
   NAME:       RemIRPlayerList

   FUNCTION:   given a list created by one of the above functions,
               remove any players who are on IR

   INPUT:      pList -> list of players

   RETURNS:    sizeof list after deletions
   ---------------------------------------------------------------------
*/

DYN_16
Roster::RemIRPlayerList (GenList<RosPlayer> *pList)
{
   DYN_16   nP = pList->size ();

   if (nP)
   {
      pList->start ();

      while (nP--)
      {
         RosPlayer   *pPtr = (*pList) ();

         // take out any player who's from the injured reserve
         if (pPtr->GetStatus () == INJ_SLOT)
            pList->linkout ();

         else
            ++(*pList);
      }
   }

   return pList->size ();
}

/*
   ---------------------------------------------------------------------
   NAME:       NumPlayersTradeProof

   FUNCTION:   Determines how many players on ANY team will not be
               traded.

   INPUT:      void

   RETURNS:    DYN_16
                  Number of players that will not available for trading.
   ---------------------------------------------------------------------
*/

DYN_16
Roster::NumPlayersTradeProof ()
{
   DYN_8    index       = MAX_DRAFT_LIST,
            FAR *countPtr   = tradeProofCounts;
   DYN_16   numPlayers  = 0;

   while (index--)
      numPlayers += *countPtr++;

   return numPlayers;
}
#endif

#if   (LCDT == 0)
/*
   ---------------------------------------------------------------------
   NAME:       Get/SetExtRatWeights

   FUNCTION:   get/set externalRatingWeights block

   INPUT:      erwPtr-> weights to get/set

   RETURNS:    void
   ---------------------------------------------------------------------
*/

void
Roster::GetExtRatWeights (RatingWeights *erwPtr)
{
   *erwPtr = extRatWeights [0];
}

void
Roster::SetExtRatWeights (RatingWeights *erwPtr)
{
   DYN_16         pos;
   RatingWeights  *aRWPtr = extRatWeights;

   // The external rating weights for each position are identical
   for (pos = 0; pos < RosPlayer::POS_MAX_SPEC; pos++, aRWPtr++)
      *aRWPtr = *erwPtr;
}

/*
   ---------------------------------------------------------------------
   NAME:       SetWeeksToGo

   FUNCTION:   when making DL intelligence decisions, need to know
               the number of weeks remaining in the regular season
               (if player is hurt more than there are weeks,
               put the player on, otherwise, let the player heal,
               and get back into the lineup a little on down the road)

   INPUT:      wTG = weeks to go

   RETURNS:    
   ---------------------------------------------------------------------
*/

void
Roster::SetWeeksToGo (const DYN_16 wTG)
{
   weeksToGo = wTG;

   // if end of season, or start of playoffs,
   // roster still needs to manage as if a there's still a game to play
   if (!weeksToGo)
      weeksToGo = 1;
}
#endif

/*
   ---------------------------------------------------------------------
   NAME:       LoadRoster

   FUNCTION:   re-read the team's roster info

   INPUT:      fp-> FILE to get data from
               id = id of team to reload for

   RETURNS:    team's roster initialized again
   ---------------------------------------------------------------------
*/

void
Roster::LoadRoster (ReadBFile &rlf, DYN_16 rId)
{
   Load (rlf, rId);

   // in case any player pointers have been shifted, re-align them
   RosterPlayer   *rpPtr = data->rs.qb;
   while (rpPtr < &data->rs.inj [NUM_INJ_SLOTS])
   {
      // if player doesn't match, find where he went & swap
      RosPlayer   *pPtr = rpPtr->pPtr;
      if (pPtr)
      {
         DYN_16      pId = pPtr->GetId ();

         if (rpPtr->pId != pId)
         {
            // find where the player is now
            RosterPlayer   *swapPtr = findRosterPlayer (&data->rs, pId);

            // and exchange
            if (swapPtr)
            {
               rpPtr->pPtr = swapPtr->pPtr;
               swapPtr->pPtr = pPtr;
            }
         }
      }

      rpPtr++;
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       LoadPlayers

   FUNCTION:   read in a Roster's players from the file
               (if players have already been loaded, don't bother re-reading)

   INPUT:      fp -> file to get player info from (if 0, then just
               inquiring if players have actually been loaded)

   RETURNS:    TRUE/FALSE if players have been loaded
               roster's player's info loaded
   ---------------------------------------------------------------------
*/

DYN_16
Roster::LoadPlayers (PlayerRdWr *prwPtr)
{
   // check for a valid pointer in the first slot, indicating players
   // have already been loaded
   // don't really load if fp == 0 (an inquiry only is being made)
   if ((data->rs.qb [0].pPtr == 0) && prwPtr)
   {
      LoadSlotPlayers (*prwPtr);

      #if   (LCDT == 0)
      CheckSlotPlayers ();
      #endif
   }

   // return whether players have actually been loaded
   return data->rs.qb [0].pPtr != 0;
}

#if (LCDT == 0) || (LCDT == 4)
/*
   ---------------------------------------------------------------------
   NAME:       SavePlayers

   FUNCTION:   write out the actual player data for each player on the team

   INPUT:      fp -> file to write player info to

   RETURNS:    player data written for each non-empty slot
   ---------------------------------------------------------------------
*/

DYN_16
Roster::SavePlayers (PlayerRdWr &prw)
{
   // check for a valid pointer in the first slot, indicating players
   // have already been loaded
   if ((data->rs.qb [0].pPtr != 0))
   {
      CheckSlotPlayers ();
      WriteSlotPlayers (prw);
   }

   return NUM_ROSTER_PLAYERS;
}
#endif

/*
   ---------------------------------------------------------------------
   NAME:       DeletePlayers

   FUNCTION:   delete all the allocated players on this roster

   INPUT:      player pointers

   RETURNS:    players mem given back to system
   ---------------------------------------------------------------------
*/

void
Roster::DeletePlayers ()
{
   // don't bother if the players haven't been loaded
   if (data->rs.qb [0].pPtr != 0)
   {
      RosterPlayer   *rpPtr = data->rs.qb;
      while (rpPtr < &data->rs.inj [NUM_INJ_SLOTS])
      {
         delete rpPtr->pPtr;

         // no player there any more
         rpPtr->pPtr = 0;
         rpPtr++;
      }
   }
}

#if (LCDT == 0)
/*
   ---------------------------------------------------------------------
   NAME:       LoadNewPlayers

   FUNCTION:   Load the players that this roster USED to have prior to
               it being saved as a new roster

   INPUT:      path, filename, source roster id, this roster id

   RETURNS:    old players loaded
   ---------------------------------------------------------------------
*/

void
Roster::LoadNewPlayers (DYN_8 *path, DYN_8 *fileName, DYN_16 srcId, DYN_16 thisId)
{
   DYN_8    name [MAXPATH];
   makeFileName (name, path, fileName, lgeText);

   ReadBFile   rlf;
   sysAssert (rlf.Open (name) > 0, EXITMSG_ROSTER_LOAD);
   Load (rlf, srcId);
   rlf.Close ();

   PlayerRdWr  prw;
   makeFileName (name, path, fileName, pyrText);
   sysAssert (prw.Open (name), EXITMSG_ROSTER_LOAD);

   LoadPlayers (&prw);
   prw.Close ();

   data->id = thisId;
}
#endif

/*
   ---------------------------------------------------------------------
   NAME:       Save

   FUNCTION:   write the "R01:" block
               "R01:"
               id                   sizeof (DYN_16)
               63 player id's       63 * sizeof (DYN_16)
               63 uniform numbers
               draft strategies
               training camp allocations

   INPUT:      fp-> file
               Roster object

   RETURNS:    file updated
   ---------------------------------------------------------------------
*/

#if   (LCDT == 0) || (LCDT == 4)
void
Roster::Save (WriteBFile &wlf)
{
   wlf.OpenBlock (R01Text);

   RosterDiskData rdd;

   rdd.id = data->id;
   WriteSlotIds (&rdd);
   WriteSlotJerseys (&rdd);
   WriteStrategies (&rdd);
   WriteTraining (&rdd);
   WriteWeights (&rdd);

   encrypt (&rdd.id);
   wlf.Write (&rdd, sizeof (RosterDiskData));
   wlf.CloseBlock ();
}
#endif

/*
   ---------------------------------------------------------------------
   NAME:       SaveNewPlayers

   FUNCTION:   this roster is being copied to a new league, all
               the players get new ids & reset to start of season ratings

   INPUT:      pId = which id to start with

   RETURNS:    pId for next team to start with (i.e. old pId += number of
               players on team)
   ---------------------------------------------------------------------
*/

#if   (LCDT == 0)
DYN_16
Roster::SaveNewPlayers (PlayerRdWr &prw, DYN_16 newId)
{
   // each player is given a new id (their old one probably conflicts with
   // some player from another league)

   RosterPlayer   *rpPtr = data->rs.qb;
   while (rpPtr < data->rs.inj)
   {
      // don't save empty players as if they were real
      if (rpPtr->pId)
      {
         rpPtr->pId = newId;
         rpPtr->pPtr->NewPlayer (newId);
         rpPtr->pPtr->Save (prw);

         newId++;
      }

      rpPtr++;
   }

   // IR players get tossed
   while (rpPtr < &data->rs.inj [NUM_INJ_SLOTS])
   {
      delete rpPtr->pPtr;
      emptyRosterPlayer (rpPtr);
      rpPtr++;
   }

   return newId;
}
#endif

/*
   ---------------------------------------------------------------------
   NAME:       GetPlayer

   FUNCTION:   get a pointer to the player at the given SLOT & POSITION

   INPUT:      sId = which slot
               index = which one

               or id
               (if id = 0, check releasePPtr to see if this is a draft
               release)

   RETURNS:    -> player
               players status is what slot player comes from
               NULL = invalid sId/index
   ---------------------------------------------------------------------
*/

RosPlayer *
Roster::GetPlayer (SlotId sId, DYN_16 index)
{
   RosPlayer      *pPtr = 0;

   switch (sId)
   {
      case QB_SLOT:
         if (index < NUM_QB_SLOTS)
            pPtr = data->rs.qb [index].pPtr;
         break;

      case RB_SLOT:
         if (index < NUM_RB_SLOTS)
            pPtr = data->rs.rb [index].pPtr;
         break;

      case REC_SLOT:
         if (index < NUM_REC_SLOTS)
            pPtr = data->rs.rec [index].pPtr;
         break;

      case OL_SLOT:
         if (index < NUM_OL_SLOTS)
            pPtr = data->rs.ol [index].pPtr;
         break;

      case DL_SLOT:
         if (index < NUM_DL_SLOTS)
            pPtr = data->rs.dl [index].pPtr;
         break;

      case LB_SLOT:
         if (index < NUM_LB_SLOTS)
            pPtr = data->rs.lb [index].pPtr;
         break;

      case DB_SLOT:
         if (index < NUM_DB_SLOTS)
            pPtr = data->rs.db [index].pPtr;
         break;

      case K_SLOT:
         if (index < NUM_K_SLOTS)
            pPtr = data->rs.k [index].pPtr;
         break;

      case P_SLOT:
         if (index < NUM_P_SLOTS)
            pPtr = data->rs.p [index].pPtr;
         break;

      case OPEN_SLOT:
         if (index < NUM_OPEN_SLOTS)
            pPtr = data->rs.open [index].pPtr;
         break;

      case INACTIVE_SLOT:
         if (index < NUM_INACTIVE_SLOTS)
            pPtr = data->rs.inActive [index].pPtr;
         break;

      case INJ_SLOT:
         if (index < NUM_INJ_SLOTS)
            pPtr = data->rs.inj [index].pPtr;
         break;
   }

   if (pPtr)
      pPtr->SetStatus (sId);

   return pPtr;
}

RosPlayer *
Roster::GetPlayer (DYN_16 id)
{
   RosterPlayer   *rpPtr;
   SlotId         slotId, maxSlot;

   if (id)
   {
      rpPtr = data->rs.qb;
      slotId = QB_SLOT;
      maxSlot = MAX_SLOT;
   }

   // if looking for a player with no id,
   // check if releasing a player for making draft choice room
   // if not, search only the open slots
   else
   {
      if (data->releasePPtr && (data->releasePPtr->GetId () == 0))
         return data->releasePPtr;

      rpPtr = data->rs.open;
      slotId = OPEN_SLOT;
      maxSlot = INJ_SLOT;
   }

   rpPtr = findRosterPlayer (rpPtr, slotId, maxSlot, id);
   if (rpPtr)
      return rpPtr->pPtr;

   else
      return 0;
}

/*
   ---------------------------------------------------------------------
   NAME:       Get/SetJersey

   FUNCTION:   return or set jersey of given player

   INPUT:      rpData = what data to modify/return
               id = which player

   RETURNS:    data requested or updated
   ---------------------------------------------------------------------
*/

DYN_16
Roster::GetJersey (DYN_16 id)
{
   RosterPlayer   *rpPtr = findRosterPlayer (data->rs.qb, QB_SLOT, MAX_SLOT, id);
   DYN_16         value = 0;

   if (rpPtr)
      value = rpPtr->jersey;

   return value;
}

void
Roster::SetJersey (DYN_16 id, DYN_16 value)
{
   RosterPlayer   *rpPtr = findRosterPlayer (data->rs.qb, QB_SLOT, MAX_SLOT, id);

   if (rpPtr)
      rpPtr->jersey = (char)value;
}

/*
   ---------------------------------------------------------------------
   NAME:       BuildPlayerList

   FUNCTION:   return list of player pointers

   INPUT:      pList -> where list goes (may be 0, user just wants a count)
               emptyFlag, injFlag control whether empty or injured players
                  are to be included in list

               parameters:
                  none     all players on team
                  pos      by GENERAL position
                  sId      by slot

   RETURNS:    sizeof list
               list set with pointers
   ---------------------------------------------------------------------
*/

DYN_16
Roster::BuildPlayerList (GenList<RosPlayer> *pList,
                           const DYN_16 emptyFlag,
                           const DYN_16 injFlag)
{
   // build entire team
   // which slot to stop search
   SlotId         maxSlot = injFlag ? INJ_SLOT : INACTIVE_SLOT;

   DYN_16         numPlayers = 0;
   RosterPlayer   *rpPtr = data->rs.qb;
   for (SlotId slotId = QB_SLOT; slotId <= maxSlot; slotId = SlotId(slotId + 1))
   {
      DYN_16   numSlots = getNumSlots (slotId);

      while (numSlots--)
      {
         DYN_16      addFlag = TRUE;

         // if the slot is empty,
         // and user DOESN'T want empty, don't add
         if ((rpPtr->pId == 0) && !emptyFlag)
            addFlag = FALSE;

         if (addFlag)
         {
            numPlayers++;

            if (pList)
            {
               rpPtr->pPtr->SetStatus (slotId);
               pList->linkin (rpPtr->pPtr);
            }
         }

         rpPtr++;
      }
   }

   return numPlayers;
}

DYN_16
Roster::BuildPlayerList (GenList<RosPlayer> *pList,
                           const RosPlayer::Pos pos,
                           const DYN_16 assEmptyFlag,
                           const DYN_16 openEmptyFlag,
                           const DYN_16 inActEmptyFlag,
                           const DYN_16 injFlag,
                           const DYN_16 injEmptyFlag)
{
   // first get all the players that are in the assigned slot
   DYN_16   numPlayers;
   if (pos == RosPlayer::POS_NONE)
   {
      numPlayers = AddPlayerList (pList, RosPlayer::POS_QB, QB_SLOT, assEmptyFlag);
      numPlayers += AddPlayerList (pList, RosPlayer::POS_RB, RB_SLOT, assEmptyFlag);
      numPlayers += AddPlayerList (pList, RosPlayer::POS_REC, REC_SLOT, assEmptyFlag);
      numPlayers += AddPlayerList (pList, RosPlayer::POS_OL, OL_SLOT, assEmptyFlag);
      numPlayers += AddPlayerList (pList, RosPlayer::POS_DL, DL_SLOT, assEmptyFlag);
      numPlayers += AddPlayerList (pList, RosPlayer::POS_LB, LB_SLOT, assEmptyFlag);
      numPlayers += AddPlayerList (pList, RosPlayer::POS_DB, DB_SLOT, assEmptyFlag);
      numPlayers += AddPlayerList (pList, RosPlayer::POS_K, K_SLOT, assEmptyFlag);
      numPlayers += AddPlayerList (pList, RosPlayer::POS_P, P_SLOT, assEmptyFlag);
   }

   else
   {
      SlotId   slotId;
      switch (pos)
      {
         case RosPlayer::POS_QB:
            slotId = QB_SLOT;
            break;

         case RosPlayer::POS_FB:
         case RosPlayer::POS_HB:
         case RosPlayer::POS_RB:
            slotId = RB_SLOT;
            break;

         case RosPlayer::POS_TE:
         case RosPlayer::POS_WR:
         case RosPlayer::POS_REC:
            slotId = REC_SLOT;
            break;

         case RosPlayer::POS_C:
         case RosPlayer::POS_G:
         case RosPlayer::POS_T:
         case RosPlayer::POS_OL:
            slotId = OL_SLOT;
            break;

         case RosPlayer::POS_DE:
         case RosPlayer::POS_DT:
         case RosPlayer::POS_DL:
            slotId = DL_SLOT;
            break;

         case RosPlayer::POS_LB:
            slotId = LB_SLOT;
            break;

         case RosPlayer::POS_CB:
         case RosPlayer::POS_S:
         case RosPlayer::POS_DB:
            slotId = DB_SLOT;
            break;

         case RosPlayer::POS_K:
            slotId = K_SLOT;
            break;

         case RosPlayer::POS_P:
            slotId = P_SLOT;
            break;
      }

      numPlayers = AddPlayerList (pList, pos, slotId, assEmptyFlag);
   }

   // then find everybody that matches in the open & inactive slots
   numPlayers += AddPlayerList (pList, pos, OPEN_SLOT, openEmptyFlag);
   numPlayers += AddPlayerList (pList, pos, INACTIVE_SLOT, inActEmptyFlag);

   // then find any injured reserves
   if (injFlag)
      numPlayers += AddPlayerList (pList, pos, INJ_SLOT, injEmptyFlag);

   return numPlayers;
}

DYN_16
Roster::BuildPlayerList (GenList<RosPlayer> *pList, RosPlayer::Pos *positions)
{
   DYN_16         numPlayers = 0;
   RosPlayer::Pos pos;

   while ((pos = *positions++) != RosPlayer::POS_NONE)
      numPlayers += BuildPlayerList (pList, pos, FALSE, FALSE, FALSE, TRUE, FALSE);

   return numPlayers;
}

DYN_16
Roster::BuildPlayerList (GenList<RosPlayer> *pList,
                                 const SlotId sId,
                                 const DYN_16 emptyFlag)
{
   DYN_16   numPlayers;

   if (sId >= OFF_SLOT)
   {
      if (sId == OFF_SLOT)
      {
         numPlayers  = BuildPlayerList (pList, RosPlayer::POS_QB,  TRUE, FALSE, FALSE, TRUE, FALSE);
         numPlayers += BuildPlayerList (pList, RosPlayer::POS_HB, FALSE, FALSE, FALSE, TRUE, FALSE);
         numPlayers += BuildPlayerList (pList, RosPlayer::POS_FB,  TRUE, FALSE, FALSE, TRUE, FALSE);
         numPlayers += BuildPlayerList (pList, RosPlayer::POS_WR, FALSE, FALSE, FALSE, TRUE, FALSE);
         numPlayers += BuildPlayerList (pList, RosPlayer::POS_TE,  TRUE, FALSE, FALSE, TRUE, FALSE);
         numPlayers += BuildPlayerList (pList, RosPlayer::POS_C,  FALSE, FALSE, FALSE, TRUE, FALSE);
         numPlayers += BuildPlayerList (pList, RosPlayer::POS_G,  FALSE, FALSE, FALSE, TRUE, FALSE);
         numPlayers += BuildPlayerList (pList, RosPlayer::POS_T,   TRUE, FALSE, FALSE, TRUE, FALSE);
         numPlayers += BuildPlayerList (pList, RosPlayer::POS_K,   TRUE, FALSE, FALSE, TRUE, FALSE);
         numPlayers += BuildPlayerList (pList, RosPlayer::POS_P,   TRUE, FALSE, FALSE, TRUE, FALSE);
      }

      else
      {
         numPlayers  = BuildPlayerList (pList, RosPlayer::POS_DE, FALSE, FALSE, FALSE, TRUE, FALSE);
         numPlayers += BuildPlayerList (pList, RosPlayer::POS_DT,  TRUE, FALSE, FALSE, TRUE, FALSE);
         numPlayers += BuildPlayerList (pList, RosPlayer::POS_LB,  TRUE, FALSE, FALSE, TRUE, FALSE);
         numPlayers += BuildPlayerList (pList, RosPlayer::POS_S,  FALSE, FALSE, FALSE, TRUE, FALSE);
         numPlayers += BuildPlayerList (pList, RosPlayer::POS_CB,  TRUE, FALSE, FALSE, TRUE, FALSE);
      }

      // POS_MAX will match NO ONE, so only empty opens will be added
      numPlayers += AddPlayerList (pList, RosPlayer::POS_MAX, OPEN_SLOT, TRUE);
      numPlayers += AddPlayerList (pList, RosPlayer::POS_MAX, INACTIVE_SLOT, TRUE);
   }

   else
      numPlayers = AddPlayerList (pList, RosPlayer::POS_NONE, sId, emptyFlag);

   return numPlayers;
}

DYN_16
Roster::BuildPlayerList (GenList<RosPlayer> *pList,
                                 const RosPlayer::Pos pos,
                                 const SlotId sId,
                                 const DYN_16 emptyFlag)
{
   return AddPlayerList (pList, pos, sId, emptyFlag);
}

/*
   ---------------------------------------------------------------------
   NAME:       BuildPlayerList

   FUNCTION:   return list of player pointers

   INPUT:      for ProBowl selecting, pList [] is an array of Lists,
               so put players into a list indexed by SPECIFIC position
               also, each player's SOURCE TEAM number is put into
               the player's status area

   RETURNS:    number of players added to all lists
   ---------------------------------------------------------------------
*/

DYN_16
Roster::BuildPlayerList (GenList<RosPlayer> *pList [])
{
   DYN_16         numPlayers = 0;
   RosterPlayer   *rpPtr = data->rs.qb;
   for (SlotId sId = QB_SLOT; sId < INJ_SLOT; sId = SlotId(sId + 1))
   {
      DYN_16   numSlots = getNumSlots (sId);
      while (numSlots--)
      {
         if (rpPtr->pId)
         {
            RosPlayer   *pPtr = rpPtr->pPtr;
            if (pPtr->GetInjPts () < RosPlayer::INJURY_QUESTION)
            {
               pPtr->SetStatus (data->id);
               DYN_16   index = pPtr->GetPosSpec ();
               if (pList [index])
               {
                  pList [index]->linkin (pPtr);
                  numPlayers++;
               }
            }
         }

         rpPtr++;
      }
   }

   return numPlayers;
}

/*
   ---------------------------------------------------------------------
   NAME:       SortPlayerList

   FUNCTION:   given a list created by one of the above functions, sort
               the list via a shell-sort based on some RosPlayer's info

               rsSortPtr must be set so that when numberCompare gets called
               it can find the RosterPlayerOSTER_PLAYER structure that corresponds to
               a given player. The jersey is not contained within the player,
               but the roster, so there has to be a tie back to the roster
               that the player came from.

   INPUT:      pList -> where list goes
               RosterSort = type of sort desired

   RETURNS:    list sorted based on type of sort wanted
   ---------------------------------------------------------------------
*/

void
Roster::SortPlayerList (GenList<RosPlayer> *pList, RosterSort sortType)
{
   rsSortPtr = &data->rs;
   rwSortPtr = data->rw;
   rSortPtr  = this;
   ListShellSort ((zDList *) pList, sortType, compareFunc);
}

#if   (LCDT == 0)
/*
   ---------------------------------------------------------------------
   NAME:       CountProBowlPlayers

   FUNCTION:   run through the list of assigned and open players,
               counting each SPECIFIC position, who isn't injured

   INPUT:      pCounts = array of player counts to add to

   RETURNS:    pCounts updated
   ---------------------------------------------------------------------
*/

void
Roster::CountProBowlPlayers (DYN_16 *pCounts)
{
   RosterPlayer   *rpPtr = data->rs.qb;
   for (SlotId sId = QB_SLOT; sId < INJ_SLOT; sId = SlotId(sId + 1))
   {
      DYN_16   numSlots = getNumSlots (sId);
      while (numSlots--)
      {
         if (rpPtr->pId)
         {
            RosPlayer   *pPtr = rpPtr->pPtr;
            if (pPtr->GetInjPts () < RosPlayer::INJURY_QUESTION)
               pCounts [pPtr->GetPosSpec ()]++;
         }

         rpPtr++;
      }
   }
}

/* new for All Pro Team JWM 12/26/96
   ---------------------------------------------------------------------
   NAME:       PickPlayers

   FUNCTION:   Fill the roster with the all pro team

   INPUT:      pStatList = list of All Pro Team statistics

   RETURNS:    pId for next team to start with
               (i.e. old pId += number of players on team)
               0 means not enough players for some position
   ---------------------------------------------------------------------
*/

DYN_16
Roster::PickPlayers (GenList<RosPlayer> *pList, DYN_16 newId, DYN_16 thisId)
{
   DYN_16               ohOh = FALSE;
   data->id = thisId;
   RosterPlayer         *arpPtr = data->rs.qb;
      RosPlayer   *srcPPtr, *destPPtr ;

   pList->start ();

   // first fill in the assigned with the number
   DYN_16    numPlayers = pList->size();
   while (numPlayers-- && !ohOh)
   {
      srcPPtr = pList->get (),
      destPPtr = new RosPlayer (srcPPtr);
      destPPtr->NewPlayer (newId);
      arpPtr->pId = newId;
      arpPtr->pPtr = destPPtr;
      arpPtr->jersey = NewJersey (destPPtr, newId);

      newId++;
      arpPtr++;
      pList->fwd() ;
   }

   return ohOh ? 0 : newId;
}

/*
   ---------------------------------------------------------------------
   NAME:       PickPlayers

   FUNCTION:   pick the top players at each position,
               filling in both assigned and open slots

   INPUT:      pList = array of player lists, so roster can grab the best
                        arranged by SPECIFIC positions
               newId = id of first player on team
               rId = roster's new id

   RETURNS:    pId for next team to start with
               (i.e. old pId += number of players on team)
               0 means not enough players for some position
   ---------------------------------------------------------------------
*/

DYN_16
Roster::PickPlayers (GenList<RosPlayer> *pList [], DYN_16 newId, DYN_16 thisId)
{
   data->id = thisId;

   DYN_16               ohOh = FALSE;
   struct ProBowlCounts FAR *pbCnt = pbCounts;
   RosterPlayer         *arpPtr = data->rs.qb,
                        *orpPtr = data->rs.open;
   RosPlayer::Pos       pos = RosPlayer::POS_QB;
   for (; (pos < RosPlayer::POS_MAX_SPEC) && !ohOh; pos = RosPlayer::Pos(pos + 1), pbCnt++)
   {
      if (pList [pos])
      {
         pList [pos]->start ();

         // first fill in the assigned with the number
         DYN_8    numPlayers = pbCnt->assigned;
         while (numPlayers-- && !ohOh)
         {
            RosPlayer   *srcPPtr = (*pList [pos]) (),
                        *destPPtr = new RosPlayer (srcPPtr);
            destPPtr->NewPlayer (newId);
            arpPtr->pId = newId;
            arpPtr->pPtr = destPPtr;
            arpPtr->jersey = NewJersey (destPPtr, newId);

            ohOh |= ++(*pList [pos]);
            newId++;
            arpPtr++;
         }

         // then fill in the open with the number
         numPlayers = pbCnt->open;
         while (numPlayers-- && !ohOh)
         {
            RosPlayer   *srcPPtr = (*pList [pos]) (),
                        *destPPtr = new RosPlayer (srcPPtr);
            destPPtr->NewPlayer (newId);
            orpPtr->pId = newId;
            orpPtr->pPtr = destPPtr;
            orpPtr->jersey = NewJersey (destPPtr, newId);

            ohOh |= ++(*pList [pos]);
            newId++;
            orpPtr++;
         }
      }
   }

   // open must pick the best of the rest
   return ohOh ? 0 : newId;
}

/*
   ---------------------------------------------------------------------
   NAME:       RemoveNoTrades

   FUNCTION:   remove all players the team isn't interested in parting
               with
               those that are tradeproof
               those that are better than the incoming player

   INPUT:      pList-> list of players at a certain general position
               manageFlag = TRUE/FALSE to proceed

   RETURNS:    number of players still in list
   ---------------------------------------------------------------------
*/

DYN_16
Roster::RemoveNoTrades (GenList<RosPlayer> *pList,
                           const RosPlayer *pPtr,
                           const DYN_16 manageFlag) const
{
   DYN_16   np = pList->size (),
            numPlayers = np;

   if (manageFlag && !pList->start ())
   {
      while (numPlayers--)
      {
         RosPlayer   *thisPPtr = (*pList) ();

         DYN_16   remove = tradeProof (&data->rs, thisPPtr);
         if (!remove)
         {
            rwSortPtr = (RatingWeights FAR *) &data->rw;
            remove = abrSpecCompare ((RosPlayer *) pPtr, thisPPtr);
         }

         if (remove)
         {
            pList->linkout ();
            np--;
         }

         else
            (++(*pList));
      }
   }

   return np;
}
/*
   ---------------------------------------------------------------------
   NAME:       PoolPlayers

   FUNCTION:   put every player on the team into a free agent pool

   INPUT:      rplRef = pool to put players into

   RETURNS:    every player removed from this team
   ---------------------------------------------------------------------
*/

void
Roster::PoolPlayers (RosPlayerList &rplRef)
{
   // go through everyone on the team
   RosPlayer            *pPtrs [NUM_ROSTER_PLAYERS];
   Roster::RosterPlayer *rpPtr = data->rs.qb;
   DYN_16               i = 0,
                        numP = NUM_ROSTER_PLAYERS;
   while (numP--)
   {
      if (rpPtr->pId)
      {
         // putting the player into the list
         pPtrs [i++] = rpPtr->pPtr;
         emptyRosterPlayer (rpPtr);
      }

      rpPtr++;
   }

   rplRef.AddPlayers (pPtrs, i);
}
#endif

#if   (LCDT == 0) || (LCDT == 4)
/*
   ---------------------------------------------------------------------
   NAME:       ChangeSlots

   FUNCTION:   1 player is moving slots, process

   INPUT:      pList-> players in the list
               lastPos = index of where player was
               curPos = index of where player is going to

   RETURNS:    TRUE/FALSE if move is allowed
               can't move:
               a qb into a rec slot,
               a healthy player into an ir slot
   ---------------------------------------------------------------------
*/

DYN_16
Roster::ChangeSlots (GenList<RosPlayer> *pList, DYN_16 lastPos, DYN_16 curPos)
{
   DYN_16   changeOk = TRUE;

   // check which direction to shuffle list
   if (lastPos > curPos)
   {
      if (!pList->start ())
      {
         if (lastPos)
            (*pList) += lastPos;

         // get -> to player moving
         RosPlayer   *movpPtr = (*pList) ();

         // move all in-betweeners up 1 spot
         DYN_16   count = lastPos - curPos;
         while (count-- && changeOk)
         {
            // get previous player in list
            --(*pList);
            RosPlayer   *pPtr = (*pList) ();

            // store pointer in position 1 forward
            ++(*pList);
            pList->update (pPtr);
            changeOk &= SwapPlayers (pPtr, movpPtr);

            // oink-and-shuffle up the list
            --(*pList);
            pList->update (movpPtr);
         }
      }
   }

   else
   {
      if (!pList->start ())
      {
         if (lastPos)
            (*pList) += lastPos;

         // get -> to player moving
         RosPlayer   *movpPtr = (*pList) ();

         // move all in-betweeners up 1 spot
         DYN_16   count = curPos - lastPos;
         while (count-- && changeOk)
         {
            // get next player in list
            ++(*pList);
            RosPlayer   *pPtr = (*pList) ();

            // and store pointer in position 1 back
            --(*pList);
            pList->update (pPtr);
            changeOk = SwapPlayers (pPtr, movpPtr);

            // oink-and-shuffle down the list
            ++(*pList);
            pList->update (movpPtr);
         }
      }
   }

   return changeOk;
}
#endif

#if   (LCDT == 0)
/*
   ---------------------------------------------------------------------
   NAME:       SetSlots

   FUNCTION:   user may have modified what players should be where,
               and passes in the ids of which slots contain which players
               ASSUMES user hasn't violated any of the NUM_??_SLOT
               values. That is, there aren't 3 QBs, or 8 DBs in the
               aIds set of ids

   INPUT:      aIds-> assigned player ids (in no particular order)
               oIds-> open player ids
               iIds-> inactive player ids

   RETURNS:    TRUE/FALSE if players have changed positions
   ---------------------------------------------------------------------
*/

DYN_16
Roster::SetSlots (DYN_16 *aIds, DYN_16 *oIds, DYN_16 *iIds)
{
   RosterSlots    newRS;
   memset (&newRS, 0, sizeof (RosterSlots));

   // 1st slot of each assigned position
   RosterPlayer   *assPtrs [MAX_DRAFT_LIST];
   assPtrs [QB_SLOT] = newRS.qb;
   assPtrs [RB_SLOT] = newRS.rb;
   assPtrs [REC_SLOT] = newRS.rec;
   assPtrs [OL_SLOT] = newRS.ol;
   assPtrs [DL_SLOT] = newRS.dl;
   assPtrs [LB_SLOT] = newRS.lb;
   assPtrs [DB_SLOT] = newRS.db;
   assPtrs [K_SLOT] = newRS.k;
   assPtrs [P_SLOT] = newRS.p;

   RosPlayer      *epPtr = GetEmptyPlayer (0);

   // get each id, and put the player into the next available assigned slot
   DYN_16         numP = NUM_GAME_PLAYERS - NUM_OPEN_SLOTS;
   while (numP--)
   {
      RosterPlayer   *rpPtr;
      DYN_16         pId = *aIds++;
      if (pId)
      {
         // get original data
         rpPtr = findRosterPlayer (&data->rs, pId);

         // and store in the next available, corresponding slot
         DYN_16         index = posToSlot (rpPtr->pPtr->GetPosGen ());
         RosterPlayer   *aPtr = assPtrs [index];
         *aPtr = *rpPtr;
         assPtrs [index] = ++aPtr;
      }
   }

   // copy in all open players
   RosterPlayer   *oPtr = newRS.open;
   numP = NUM_OPEN_SLOTS;
   while (numP--)
   {
      DYN_16   pId = *oIds++;
      if (pId)
         // store player in (new?) position
         *oPtr++ = *findRosterPlayer (&data->rs, pId);
   }

   oPtr = newRS.inActive;
   numP = NUM_INACTIVE_SLOTS;
   while (numP--)
   {
      DYN_16   pId = *iIds++;
      if (pId)
         // store player in (new?) position
         *oPtr++ = *findRosterPlayer (&data->rs, pId);
   }

   // go back in and fill in any empty slots
   oPtr = newRS.qb;
   numP = NUM_GAME_PLAYERS + NUM_INACTIVE_SLOTS;
   while (numP--)
   {
      // the slot needs an empyt if there is no id
      if (!oPtr->pId)
      {
         // if an empty is going in, there must have been one to start with
         vmAssert (epPtr);

         RosPlayer   *pPtr = epPtr;
         epPtr = GetEmptyPlayer (epPtr);
         *oPtr = *findRosterPlayer (&data->rs, pPtr);
      }

      oPtr++;
   }

   // now determine if anyone changed slots
   DYN_16   changes = FALSE;
   numP = NUM_GAME_PLAYERS + NUM_INACTIVE_SLOTS;
   oPtr = newRS.qb;
   while (numP-- && !changes)
   {
      DYN_16   pId = oPtr->pId;
      if (pId &&
            (getSlot (&newRS, pId) != getSlot (&data->rs, pId)))
         changes = TRUE;

      oPtr++;
   }

   if (changes)
      memcpy (&data->rs, &newRS,
               (NUM_GAME_PLAYERS + NUM_INACTIVE_SLOTS) * sizeof (RosterPlayer));

   return changes;
}
#endif

#if   (LCDT == 0) || (LCDT == 4)
/*
   ---------------------------------------------------------------------
   NAME:       GetEmptyInj

   FUNCTION:   return -> first empty injured player

   INPUT:      injured slots

   RETURNS:    -> first empty player
               0 if none
   ---------------------------------------------------------------------
*/

RosPlayer *
Roster::GetEmptyInj ()
{
   RosPlayer   *pPtr = 0;

   RosterPlayer   *rpPtr = GetRPPtr (INJ_SLOT);
   DYN_16         numSlots = getNumSlots (INJ_SLOT);

   while (numSlots--)
   {
      if (!rpPtr->pId)
      {
         pPtr = rpPtr->pPtr;
         break;
      }

      rpPtr++;
   }

   return pPtr;
}

/*
   ---------------------------------------------------------------------
   NAME:       GetEmptyPlayer

   FUNCTION:   find the first empty slot (assigned or open) that this
               player can move into

   INPUT:      pPtr-> player coming off IR, or draft choice,
               that needs a regular spot on the roster
               if pPtr == 0, or a null player, find first empty slot,
               regardless of position

   RETURNS:    -> first empty player
               0 if none
   ---------------------------------------------------------------------
*/

RosPlayer *
Roster::GetEmptyPlayer (RosPlayer *pPtr)
{
   SlotId   sId = MAX_SLOT;

   if (pPtr)
      sId = posToSlot (pPtr->GetPosGen ());

   data->releasePPtr = 0;

   // find first WHEREVER (excluding IR)
   if (sId == MAX_SLOT)
   {
      // where to start looking
      // if an EMPTY player passed in, start looking AFTER that player,
      // otherwise, start at the beginning)
      RosterPlayer   *rpPtr;
      if (pPtr)
         rpPtr = findRosterPlayer (&data->rs, pPtr) + 1;
      else
         rpPtr = data->rs.qb;

      while (rpPtr < data->rs.inj)
      {
         if (!rpPtr->pId)
         {
            data->releasePPtr = rpPtr->pPtr;
            break;
         }

         rpPtr++;
      }
   }

   // find first of specific position
   else
   {
      // first, look for any open ASSIGNED slots
      RosterPlayer   *rpPtr = GetRPPtr (sId);
      DYN_16         numSlots = getNumSlots (sId);

      while (numSlots--)
      {
         if (!rpPtr->pId)
         {
            data->releasePPtr = rpPtr->pPtr;
            break;
         }

         rpPtr++;
      }

      // in no player found, try the open & inactive slots
      if (!data->releasePPtr)
      {
         rpPtr = GetRPPtr (OPEN_SLOT);
         numSlots = getNumSlots (OPEN_SLOT) + getNumSlots (INACTIVE_SLOT);

         while (numSlots--)
         {
            if (!rpPtr->pId)
            {
               data->releasePPtr = rpPtr->pPtr;
               break;
            }

            rpPtr++;
         }
      }
   }

   return data->releasePPtr;
}

/*
   ---------------------------------------------------------------------
   NAME:       GetNumIRPlayers

   FUNCTION:   return count of the number of non-empty players are on
               injured reserve

   INPUT:      this-> set of players

   RETURNS:    count of those on IR
   ---------------------------------------------------------------------
*/

DYN_16
Roster::GetNumIRPlayers ()
{
   RosterPlayer   *rpPtr = GetRPPtr (INJ_SLOT);
   DYN_16         numInj = 0,
                  numSlots = getNumSlots (INJ_SLOT);

   while (numSlots--)
   {
      if (rpPtr->pId)
         numInj++;

      rpPtr++;
   }

   return numInj;
}

/*
   ---------------------------------------------------------------------
   NAME:       NewJersey

   FUNCTION:   Generate a unique jersey number for the new guy on the team

   INPUT:      pPtr-> this player
               pId = this guy's id
               (when searching for other players with a given number,
               need to know who this player is)

   RETURNS:    next available number
   ---------------------------------------------------------------------
*/

DYN_8
Roster::NewJersey (RosPlayer *pPtr, DYN_16 pId)
{
   RosPlayer::Pos pos = pPtr->GetPosGen ();
   DYN_8          minNum1,
                  maxNum1,
                  minNum2,
                  maxNum2;
   SetJerseyBounds (pos, minNum1, maxNum1, minNum2, maxNum2);

   while (minNum1 < maxNum2)
   {
      if (VerifyNumber (pos, minNum1, pId) > 0)
         break;

      else
         minNum1++;
   }

   return minNum1;
}

/*
   ---------------------------------------------------------------------
   NAME:       SetJerseyBounds

   FUNCTION:   return to the caller the minimum and maximum jersey
               values this position can have

   INPUT:      pos = position in question (if _NONE, return 0 through 99)
               min1, max1, min2, max2 -> where to put bounds
               (a defensive lineman can be in 2 groups, hence the
               double set)

   RETURNS:    number of set (1 or 2)
   ---------------------------------------------------------------------
*/

DYN_16
Roster::SetJerseyBounds (RosPlayer::Pos pos,
                                 DYN_8 &min1, DYN_8 &max1,
                                 DYN_8 &min2, DYN_8 &max2)
{
   DYN_16   numBounds = 1;
   DYN_8    minNum1 = 0,
            maxNum1 = 99,
            minNum2 = 0,
            maxNum2 = 99;

   switch (pos)
   {
      case RosPlayer::POS_QB:
      case RosPlayer::POS_K:
      case RosPlayer::POS_P:
         minNum1 = minNum2 = 1;
         maxNum1 = maxNum2 = 19;
         break;

      case RosPlayer::POS_RB:
         minNum1 = minNum2 = 20;
         maxNum1 = maxNum2 = 49;
         break;

      case RosPlayer::POS_REC:
         minNum1 = minNum2 = 80;
         maxNum1 = maxNum2 = 89;
         break;

      case RosPlayer::POS_OL:
         minNum1 = minNum2 = 50;
         maxNum1 = maxNum2 = 69;
         break;

      case RosPlayer::POS_DL:
         minNum1 = 60;
         maxNum1 = 79;
         minNum2 = 90;
         maxNum2 = 99;
         numBounds = 2;
         break;

      case RosPlayer::POS_LB:
         minNum1 = 50;
         maxNum1 = 59;
         minNum2 = 70;
         maxNum2 = 79;
         numBounds = 2;
         break;

      case RosPlayer::POS_DB:
         minNum1 = minNum2 = 20;
         maxNum1 = maxNum2 = 49;
         break;
   }

   min1 = minNum1;
   max1 = maxNum1;
   min2 = minNum2;
   max2 = maxNum2;

   return numBounds;
}

/*
   ---------------------------------------------------------------------
   NAME:       VerifyNumber

   FUNCTION:   determine if the user has entered a valid number:
               must be within the correct range,
               must not be used by another player

   INPUT:      pos = general position of player
               num = prospective number
               id = the player's unique id (a search will yield a player
                  who has the same jersey number, i.e. this player, so ignore
                  this player when searching)

   RETURNS:    < 0 means number taken by another
               == 0 means number out of range
               > 0 means number ok to use
   ---------------------------------------------------------------------
*/

DYN_16
Roster::VerifyNumber (RosPlayer::Pos pos, DYN_16 num, DYN_16 id)
{
   DYN_8    minNum1,
            maxNum1,
            minNum2,
            maxNum2;

   // first do bounds checking
   SetJerseyBounds (pos, minNum1, maxNum1, minNum2, maxNum2);
   if (((num < minNum1) || (num > maxNum1)) &&
      ((num < minNum2) || (num > maxNum2)))
      return 0;

   // then check all other players (skipping THIS player)
   RosterPlayer   *rpPtr = data->rs.qb;
   while (rpPtr < &data->rs.inj [NUM_INJ_SLOTS])
   {
      if (verifyNumber (rpPtr, num, id))
         return -1;
      rpPtr++;
   }

   // new number is ok
   return num;
}

/*
   ---------------------------------------------------------------------
   NAME:       SwapPlayers

   FUNCTION:   2 players are changing their slot positions
               (make sure the switch is legal)

   INPUT:      rp1, rp2-> 2 exchanging slots
               or
               p1, p2-> 2 exchanging players

   RETURNS:    players swapped flag
   ---------------------------------------------------------------------
*/

DYN_16
Roster::SwapPlayers (RosterPlayer *rp1, RosterPlayer *rp2)
{
   DYN_16   validSwap = FALSE;

   // make sure swap is valid (i.e. can't move a kicker from an
   // open slot to the punter slot
   if (SlotPosValid (data->rs.qb, rp1, rp2->pPtr) &&
      SlotPosValid (data->rs.qb, rp2, rp1->pPtr))
   {
      RosterPlayer   temp = *rp1;
      *rp1 = *rp2;
      *rp2 = temp;

      DYN_16   status = rp1->pPtr->GetStatus ();
      rp1->pPtr->SetStatus (rp2->pPtr->GetStatus ());
      rp2->pPtr->SetStatus (status);

      validSwap = TRUE;
   }

   return validSwap;
}

DYN_16
Roster::SwapPlayers (RosPlayer *p1, RosPlayer *p2)
{
   DYN_16   validSwap = FALSE;

   if (p1 != p2)
      validSwap = SwapPlayers (findRosterPlayer (&data->rs, p1),
                                 findRosterPlayer (&data->rs, p2));

   return validSwap;
}
#endif

#if   (LCDT == 0) || (LCDT == 4)
/*
   ---------------------------------------------------------------------
   NAME:       SlotPosValid

   FUNCTION:   can the given player position occupy the given slot
               (i.e. a Punter can be in the punter, open or injured slots,
               but nowhere else)

               ANYONE can go TO an open or injured slot

   INPUT:      sId = slot for player
               pos = player's general position

   RETURNS:    player and slot are valid for each other
   ---------------------------------------------------------------------
*/

DYN_16
Roster::SlotPosValid (SlotId sId, RosPlayer::Pos pos)
{
   DYN_16      validPos = FALSE;

   // empty players can go anywhere
   if (pos == RosPlayer::POS_NONE)
      validPos = TRUE;

   else
   {
      // ok if player is GOING TO an open slot
      // the player matches the slot
      if (
            (sId == OPEN_SLOT) ||
            (sId == INACTIVE_SLOT) ||
            (sId == INJ_SLOT) ||
            (sId == posToSlot (pos))
         )
         validPos = TRUE;
   }

   return validPos;
}
#endif

#if   (LCDT == 0)
/*
   ---------------------------------------------------------------------
   NAME:       Age

   FUNCTION:   at the start of each season, everybody gets a little older

   INPUT:      roster object

   RETURNS:    all the players have 1 more year of experience, with
               possible degradation of skills
   ---------------------------------------------------------------------
*/

void
Roster::Age ()
{
   RosterPlayer   *rpPtr = data->rs.qb;
   while (rpPtr < &data->rs.inj [NUM_INJ_SLOTS])
   {
      if (rpPtr->pId)
      {
         RosPlayer   *pPtr = rpPtr->pPtr;
         pPtr->Age ();
      }

      rpPtr++;
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       Retire

   FUNCTION:   at the start of each season, every player that has enough
               'points', quits the game

               GJW: 8-2-1993, path 'depth', (i.e. 0 or 1) to retirement
               so that 'starters' don't retire as quickly

               GJW: 9-23-1993, new algorithym:
               retire between 4 & 11 players (= numToRetire):
                  ALL retire at the mandatory age
                     QB    16 years
                     RB    14
                     REC   13
                     OL    15
                     DL    15
                     LB    15
                     DB    13
                     P     12
                     K     16
                  if numToRetire not yet reached:
                     25% chanch of a 'Timm Rosenbach' happening,
                        i.e. player w/1 - 5 years, but lowest on the depth
                        chart
                  if numToRetire not yet reached:
                     25% chanch that 2 players with 6+ years retire,
                        but NOT top of depth chart
                  if numToRetire not yet reached:
                     pick who goes based on who's closest to retirement age
                     (but within 5 years of it)

   INPUT:      roster object

   RETURNS:    all the players are evaluated, and removed if they qualify
   ---------------------------------------------------------------------
*/

void
Roster::Retire (TeamNews &tn)
{
   DYN_16   numToRetire = getPosRange (MIN_RETIRE, MAX_RETIRE),
            numRetired = 0;

   // first, any CEI players, or those at the mandatory age, call it quits
   RosterPlayer   *rpPtr = data->rs.qb;
   while (rpPtr < &data->rs.inj [NUM_INJ_SLOTS])
   {
      if (rpPtr->pId)
      {
         DYN_16      retire = FALSE;
         RosPlayer   *pPtr = rpPtr->pPtr;
         if (pPtr->GetInjPts () == RosPlayer::INJURY_OVER)
            retire = TRUE;
         else if (pPtr->GetYears () >= mandatoryYears [posToSlot (pPtr->GetPosGen ())])
            retire = TRUE;

         if (retire)
         {
            RetirePlayer (tn, rpPtr);
            numRetired++;
         }
      }

      rpPtr++;
   }

   // have to boot some more
   if (numRetired < numToRetire)
   {
      // 'Timm Rosenbach' test (i.e. 'surprise retirement'):
      if (getRandPercent () < EARLY_RETIRE_PCT)
      {
         // to be the lowest on the depth chart,
         // you must be in an open slot
         DYN_16         pCount = 0;
         RosterPlayer   *rpPtr = data->rs.open,
                        *trPtrs [RosPlayer::POS_MAX_SPEC];

         memset (trPtrs, 0, sizeof (trPtrs));
         while (rpPtr < &data->rs.inj [NUM_INJ_SLOTS])
         {
            if (rpPtr->pId)
            {
               RosPlayer   *pPtr = rpPtr->pPtr;
               DYN_16      years = pPtr->GetYears ();

               // further down the depth chart replaces any previous player
               if (years <= 5)
               {
                  trPtrs [pPtr->GetPosSpec ()] = rpPtr;
                  pCount++;
               }
            }

            rpPtr++;
         }

         // if any player(s) made it, pick 1 at random
         if (pCount)
         {
            DYN_16   pOffset = getPosRange (RosPlayer::POS_QB, RosPlayer::POS_MAX_SPEC - 1);

            // keep looking for a pointer that's been set
            while (TRUE)
            {
               if (trPtrs [pOffset])
               {
                  rpPtr = trPtrs [pOffset];
                  break;
               }

               else
               {
                  if (++pOffset == RosPlayer::POS_MAX_SPEC)
                     pOffset = RosPlayer::POS_QB;
               }
            }

            RetirePlayer (tn, rpPtr);
            numRetired++;
         }
      }

      // 2 players with 6+ years:
      if (numRetired < numToRetire)
      {
         if (getRandPercent () < MID_RETIRE_PCT)
         {
            // to be the lowest on the depth chart,
            // you must be in an open slot,
            // so start with the last injured guy and work 'up'
            RosterPlayer   *rpPtr = &data->rs.inj [NUM_INJ_SLOTS - 1];
            DYN_16         pCount = 2;

            // keep going until:
            //    enough have retired
            //    2 players have retired via this mechanism
            //    looked at all the qualifying slots
            while ((numRetired < numToRetire) &&
                     pCount &&
                     (rpPtr > data->rs.open))
            {
               if (rpPtr->pId)
               {
                  RosPlayer   *pPtr = rpPtr->pPtr;
                  DYN_16      years = pPtr->GetYears ();

                  if (years >= 6)
                  {
                     RetirePlayer (tn, rpPtr);
                     numRetired++;
                     pCount--;
                  }
               }

               rpPtr--;
            }
         }
      }

      // Closest to mandatory age:
      while (numRetired < numToRetire)
      {
         // to be the lowest on the depth chart,
         // you must be in an open slot,
         // so start with the last injured guy and work 'up'
         RosterPlayer   *rpPtr = data->rs.qb,
                        *retPtr = 0;
         DYN_16         mraValue = MRA_MIN;
         while (rpPtr < &data->rs.inj [NUM_INJ_SLOTS])
         {
            if (rpPtr->pId)
            {
               RosPlayer   *pPtr = rpPtr->pPtr;
               DYN_16      yearsAway = mandatoryYears [posToSlot (pPtr->GetPosGen ())] -
                                          pPtr->GetYears ();
               // don't bother with 1st stringers
               if ((yearsAway < mraValue) && (GetDepth (pPtr) > 1))
               {
                  mraValue = yearsAway;
                  retPtr = rpPtr;
               }
            }

            rpPtr++;
         }

         // if a player was found (someone should be),
         // calls it quits
         if (retPtr)
         {
            RetirePlayer (tn, retPtr);
            numRetired++;
         }

         // otherwise noone qualifies,
         // so don't bother with anyone else
         else
            numRetired = numToRetire;
      }
   }

   // everyone moves up a notch or two to fill retiree's holes
   ManageSlots ();
}

/*
   ---------------------------------------------------------------------
   NAME:       Manage

   FUNCTION:   call the appropriate roster manipulation AI

   INPUT:      mrData = event to oraganize roster for
               pList = array of free agents
               losses = how bad is this team?

   RETURNS:    TRUE/FALSE if roster was modified
               roster organized
   ---------------------------------------------------------------------
*/

DYN_16
Roster::Manage (DYN_16 mrData)
{
   DYN_16   update = FALSE;

   switch (mrData)
   {
      case MANAGE_PRESEASON:
         update = ManagePreSeason ();
         break;

      case MANAGE_WEEKLY:
         update = ManageWeekly ();
         break;

      case MANAGE_POSTGAME:
         update = ManagePostGame ();
         break;

      // human sold team back to computer, adjust slots
      case MANAGE_OWNERSHIP:
         break;
   }

   // move all the best players to assigned slots (from open)
   return ManageSlots () || update;
}
#endif

#if   (LCDT == 0) || (LCDT == 4)
/*
   ---------------------------------------------------------------------
   NAME:       AddPlayer

   FUNCTION:   a player is being added to the team via:
               a college draftee
               a trade
               a free agent signing

               the 2 players are swapped (i.e. one is removed from the
               team, the other added)
               (the destination slot is always OPEN, or INACTIVE)

   INPUT:      newPPtr-> player being added to the roster
               (if 0, then this just released someone to the free agent pool
               without signing another, 'bubble' up the depth chart so
               that this empyt player occupies an open slot)

               oldPPtr-> player leaving
               manage = computer managed team indicator

   RETURNS:    pointer to the player leaving
               newPPtr-> joins the team
               oldPPTr-> looking for work
   ---------------------------------------------------------------------
*/

RosPlayer *
Roster::AddPlayer (RosPlayer *newPPtr, RosPlayer *oldPPtr, DYN_16 manage)
{
   // assume player will come from anywhere on the roster
   RosterPlayer   *rpPtr = data->rs.qb;
   SlotId         sId = QB_SLOT;
   RosPlayer::Pos oldPos = RosPlayer::POS_NONE;

   // if a new player is joining the team,
   // determine where to look for player that is leaving
   if (newPPtr)
   {
      // (if playing the same position, don't have to re-assign leaver to
      // an open slot, new player will just slide right in)
      if (!oldPPtr)
         oldPPtr = GetEmptyPlayer (newPPtr);
      if (!oldPPtr)
      {
         // remove the worst player on the team
         RosterPlayer   *rpPtr = FindOpenSlot (RosPlayer::POS_NONE);
         DYN_16   p1Id = 0,
                  p2Id = rpPtr->pId;
         ActiveLeague->TradePlayers (0, &p1Id, data->id, &p2Id, TRADE_PROPOSED);
         oldPPtr = GetEmptyPlayer (newPPtr);
         vmAssert (oldPPtr);
      }
      oldPos = oldPPtr->GetPosGen ();

      if (newPPtr->GetPosGen () != oldPos)
      {
         // make sure SetStatus () is called for player to be moved
         DYN_16   oldId = 0;
         if (oldPPtr)
            oldId = oldPPtr->GetId ();
         GetPlayer (oldId);

         if (MoveToOpen (oldPPtr))
         {
            sId = OPEN_SLOT;
            rpPtr = data->rs.open;
         }
      }
   }

   // stick new player on team at slot occupied by leaver
   DYN_16         pId = -1;
   for (; (sId <= INJ_SLOT) && (pId == -1); sId = SlotId(sId + 1))
   {
      DYN_16   numSlots = getNumSlots (sId);
      while (numSlots-- && (pId == -1))
      {
         // look for the player who's leaving
         if (rpPtr->pPtr == oldPPtr)
         {
            // if getting a new player...
            if (newPPtr)
            {
               rpPtr->pPtr = newPPtr;
               pId = rpPtr->pId = newPPtr->GetId ();
            }

            // otherwise just released someone
            else
            {
               emptyRosterPlayer (rpPtr);
               pId = 0;
            }
            break;
         }

         rpPtr++;
      }
   }

   // then give the new player a jersey (searching for the first valid number)
   if (newPPtr)
      rpPtr->jersey = NewJersey (newPPtr, pId);

   else
      BubbleDepth (oldPos);

   // now put best players into assigned slots
   // (if computer managing)
   if (manage)
      ManageSlots ();

   return oldPPtr;
}

/*
   ---------------------------------------------------------------------
   NAME:       GetReleasePlayer

   FUNCTION:   return to caller the player selected as expendable during
               the choice evaluation (DraftSelect)
               if none, and a player is REQUIRED,
               return the player in the last open slot
               clear the pointer once determined (for next time)

               // GJW: 9-27-1995
               make sure the last player (the worst inActive) isn't injured,
               which is what is making the player be the worst on the
               team
               i.e., look for the poorest, healthy inActive Player

   INPUT:      forceFlag = TRUE if roster MUST release someone
               releasePPtr-> player chosen by DraftSelect ()

   RETURNS:    ->player no longer needed
               releasePPtr = 0
   ---------------------------------------------------------------------
*/

RosPlayer *
Roster::GetReleasePlayer (const DYN_16 forceFlag)
{
   if (!data->releasePPtr && forceFlag)
   {
      RosterPlayer   *rpPtr = &data->rs.inActive [NUM_INACTIVE_SLOTS - 1];
      DYN_16         numP = NUM_INACTIVE_SLOTS;
      while (numP--)
      {
         if (!rpPtr->pPtr->GetInjPts ())
         {
            data->releasePPtr = rpPtr->pPtr;
            break;
         }
         rpPtr--;
      }

      // if EVERYONE is injured, then pick the worst
      if (!data->releasePPtr)
         data->releasePPtr = data->rs.inActive [NUM_INACTIVE_SLOTS - 1].pPtr;
   }

   return data->releasePPtr;
}

void
Roster::ClearReleasePlayer ()
{
   data->releasePPtr = 0;
}

/*
   ---------------------------------------------------------------------
   NAME:       DraftSelect

   FUNCTION:   pick the best player based on the draft strategies
               check each list in order, to determine what if the best
               choice is better than the worst player on the team

   INPUT:      pList -> array of draft choice lists (i.e. a bunch of
                  lists based on general position)
               dsFlag = draft type:
                           college
                           supplemental
                           weekly
                           gameReady
                           cei
               if college, draft based solely on strategies
               if supplemental, look to fill any assigned holes first
               if cei, find the player who won't play anymore, and make a new
                  one

               manage = computer manager TRUE/FALSE (if handling CEI, only
                           release player, don't sign a new one)

   RETURNS:    index of pList to select top player from
               (computer teams always choose the player with best ABRs)
               -1 means drop out of draft
   ---------------------------------------------------------------------
*/

DYN_16
Roster::DraftSelect (GenList<RosPlayer> *pList [], DraftSelectType dsFlag,
                        DYN_16 manager)
{
   // apply strategies multipliers to each position
   // posDrafts contains a point value for each of the slots (plus 1 for
   // trade, which means take the best available without regards to
   // position)
   DYN_16      listIndex = -1;
   DYN_U_16    posDrafts [MAX_DRAFT_LIST + 1];
   fillPosDrafts (posDrafts, data->ds.stratRanks);

   #if   (LCDT == 0)
   if (dsFlag == DSTYPE_COLLEGE)
      // draft by need during college phase
      listIndex = DraftCollege (pList, posDrafts);

   // fill any holes during the other roster-fill times
   else if (dsFlag == DSTYPE_SUP)
      listIndex = DraftSup (pList);

   else if (dsFlag == DSTYPE_WEEKLY)
      listIndex = DraftWeekly (pList, posDrafts);

   else if (dsFlag == DSTYPE_GAMEREADY)
      listIndex = DraftGameReady (pList, DSTYPE_GAMEREADY);
   #endif

   #if   (LCDT == 4)
   if (dsFlag == DSTYPE_CEI)
   {
      // find the player out for life, and 'Retire' the bum
      RosterPlayer   *rpPtr = findCEIPlayer (&data->rs);
      if (rpPtr)
      {
         ceiPos = rpPtr->pPtr->GetPosGen ();
         rpPtr->pId = 0;
         rpPtr->jersey = 0;
         rpPtr->pPtr->MakeNew ();
      }

      // make sure replacement player is generated
      listIndex = DraftGameReady (pList, DSTYPE_CEI);

      // but don't take if human owned
      if (!manager)
         listIndex = -1;
   }
   #endif

   return listIndex;
}
#endif

/*
   ---------------------------------------------------------------------
   NAME:       UnBalancedTrade

   FUNCTION:   determine if this human controlled team can make the
               trade (enough assigned and open holes to complete the
               deal (team is getting more players than it is giving up))

   INPUT:      pIds-> ALL ids of the other team
               i = index of first unbalanced player
               r-> OTHER roster (to get player pointers from)

   RETURNS:    TRUE/FALSE if there are enough 'holes' to make this trade
   ---------------------------------------------------------------------
*/

DYN_32
Roster::UnBalancedTrade (DYN_16 *pIds, DYN_32 index, Roster *r)
{
   DYN_32   ok = TRUE,
            pCounts [INJ_SLOT],
            *pcPtr = pCounts;

   // fill in pCounts with number SUPPOSED to have
   for (SlotId sId = QB_SLOT; sId < INJ_SLOT; sId = SlotId (sId + 1), pcPtr++)
      *pcPtr = getNumSlots (sId);

   // then go through every slot on the team, and subtract if there's
   // a player there
   RosterPlayer   *rpPtr = data->rs.qb;
   pcPtr = pCounts;
   for (sId = QB_SLOT; sId < INJ_SLOT; sId = SlotId (sId + 1), pcPtr++)
   {
      DYN_32   numSlots = getNumSlots (sId);
      while (numSlots--)
      {
         if (rpPtr->pId)
            (*pcPtr)--;

         rpPtr++;
      }
   }

   // pCounts is now an array of empty player counts (i.e. number of
   // holes remaining)
   // go through the 'extra' players in the trade, to see if there
   // are any holes
   for (; index < TradeInfo::MAX_PIDS; index++)
   {
      DYN_16   id = pIds [index];
      if (id)
      {
         RosPlayer   *pPtr = r->GetPlayer (id);
         vmAssert (pPtr);
         DYN_32      posIndex = posToSlot (pPtr->GetPosGen ());

         // player will take up an assigned slot
         if (pCounts [posIndex])
            pCounts [posIndex]--;

         // if those are full, player will take an open slot
         else if (pCounts [OPEN_SLOT])
            pCounts [OPEN_SLOT]--;

         // if those are full, player will take an inactive slot
         else if (pCounts [INACTIVE_SLOT])
            pCounts [INACTIVE_SLOT]--;

         // there is no room at the inn, trade can't be completed yet
         else
         {
            ok = FALSE;
            break;
         }
      }
   }

   return ok;
}

#if   (LCDT == 0)
/*
   ---------------------------------------------------------------------
   NAME:       TradeEval

   FUNCTION:   should the trade take place (is team getting a better
               player at a spot it needs, than it is giving up)

               GJW: 3-15-1993, don't analyze the position played by
               the players involved
               GJW: 4-26-1993, take pointers rather than ids

               GJW: 9-16-1996, multi-player trade evaluations added

   INPUT:      p1Ptrs = player(s) I'm getting
               p2Ptrs = player(s) I'm giving up

   RETURNS:    ACCEPTED/REJECTED
   ---------------------------------------------------------------------
*/

TradeStatus
Roster::TradeEval (RosPlayer **p1Ptrs, RosPlayer **p2Ptrs)
{
   // assume the worst
   TradeStatus    ts = TRADE_REJECTED;

   DYN_32   numIn = 0,
            numOut = 0,
            injFlag = FALSE;
   for (DYN_32 i = 0; i < TradeInfo::MAX_PIDS; i++)
   {
      // first, computer teams don't accept players with injuries
      RosPlayer   *pPtr = p1Ptrs [i];
      if (pPtr)
      {
         if (pPtr->GetInjPts () >= RosPlayer::INJURY_QUESTION)
         {
            injFlag = TRUE;
            break;
         }
         numIn++;
      }

      if (p2Ptrs [i])
         numOut++;
   }

   if (numIn == numOut)
   {
      DYN_32   ok = TRUE;
      for (i = 0; i < numIn; i++)
      {
         RosPlayer   *p1Ptr = p1Ptrs [i],
                     *p2Ptr = p2Ptrs [i];

         // is the incoming player better than the worst guy at this position?
         // is the outgoing player not a 'frachise' player
         if (tradeProof (&data->rs, p2Ptr) || !PlayerBetter (p1Ptr))
         {
            ok = FALSE;
            break;
         }
      }

      if (ok)
         ts = TRADE_ACCEPTED;
   }

   else
   {
      DYN_32   pctHit = TRADE_HIT_PCT,
               ptsComing = 0,
               ptsGoing = 0;

      for (i = 0; i < TradeInfo::MAX_PIDS; i++)
      {
         RosPlayer   *p1Ptr = p1Ptrs [0],
                     *p2Ptr = p2Ptrs [0];

         // first, the 'balanced' trade portion of players, must be
         // within 10% of each other
         if (p1Ptr && p2Ptr)
         {
            DYN_32   p1Pts = p1Ptr->SumAbrs ((RatingWeights *) data->rw),
                     p2Pts = p2Ptr->SumAbrs ((RatingWeights *) data->rw);

            if (p2Pts >= ((p1Pts * TRADE_WITHIN_PCT) / 100))
            {
               ptsComing += p1Pts;
               ptsGoing += p2Pts;
            }

            else
            {
               ptsComing = 0;
               break;
            }
         }

         // otherwise add player's into TOTAL count
         // incoming take a hit to avoid the 3 schmoes for 1 quality
         // player fiasco
         else if (p1Ptr)
            ptsComing += (p1Ptr->SumAbrs ((RatingWeights *) data->rw) *
                           pctHit) / 100;

         else if (p2Ptr)
            ptsGoing += p2Ptr->SumAbrs ((RatingWeights *) data->rw);
      }

      if (ptsComing > ptsGoing)
         ts = TRADE_ACCEPTED;
   }

   return ts;
}

/*
   ---------------------------------------------------------------------
   NAME:       WannaTrade

   FUNCTION:   if a trade for the given player would be a good thing

               GJW: 8-28-1996, multi-player trading

   INPUT:      dest-> where to put players given up by this team (if 0,
                        caller just wants to know if we're interested,
                        not specifically the trade)
               pos = position another team wants
               ptPtr-> ARRAY of players other team is willing to give up

   RETURNS:    ->Player THIS team would be willing to give up
               0 means GO TO HELL!
   ---------------------------------------------------------------------
*/

DYN_16
Roster::WannaTrade (RosPlayer **dest, SlotId *sIds, RosPlayer **ptPtr)
{
   // is this player better than the worst on the team?
   RosPlayer   *pPtr = 0;
   DYN_32      done = FALSE;

   for (DYN_32 i = 0; !done && (i < TradeInfo::MAX_PIDS); i++)
   {
      // get other team's offered player
      RosPlayer   *thisP = ptPtr [i];

      if (thisP)
      {
         if ((thisP->GetInjPts () < RosPlayer::INJURY_QUESTION) &&
               PlayerBetter (thisP))
         {
            // make a list of players that match position other team wants
            // (maximum number is known, so let list allocate node memory
            // in a big chunk)
            GenList<RosPlayer>	pList (NUM_GAME_PLAYERS, (DYN_16) 0);
            if (BuildPlayerList (&pList, SlotToPos (sIds [i]),
                                 FALSE, FALSE, FALSE, FALSE, FALSE))
            {
               pList.start ();

               // give away the best non-trade proof player
               // GJW: 8-25-1993, the worst player
               DYN_16   abrValue = BIG_VALUE;   // 0;
               rwSortPtr = (RatingWeights *) data->rw;

               while (TRUE)
               {
                  RosPlayer   *bpPtr = pList ();

                  // player going out must:
                  // not be hurt
                  // not be tradeproof
                  // not be better than player coming in
                  if (!bpPtr->IREligible () &&
                        !tradeProof (&data->rs, bpPtr) &&
                        !abrSpecCompare (thisP, bpPtr))
                  {
                     DYN_16   ratings = bpPtr->SumAbrs ((RatingWeights *) data->rw);
                     if (ratings < abrValue)
                     {
                        pPtr = bpPtr;
                        abrValue = ratings;

                        // if 'just wondering', then stop searching
                        if (!dest)
                        {
                           done = TRUE;
                           break;
                        }
                     }
                  }

                  if (++pList)
                     break;
               }
            }
         }
      }
   }

   if (dest)
      *dest = pPtr;

   return pPtr != 0;
}

/*
   ---------------------------------------------------------------------
   NAME:       IRMove

   FUNCTION:   transfer a player from IR to open
               if the open player is injured enough, or empty, he goes to IR,
               otherwise he is released to free agency

   INPUT:      irPtr-> player coming off IR
               oPtr-> open player

   RETURNS:    roster slots manipulated
   ---------------------------------------------------------------------
*/

void
Roster::IRMove (RosPlayer *irPtr, RosPlayer *oPtr)
{
   RosterPlayer   *irrpPtr = findRosterPlayer (&data->rs, irPtr),
                  *orpPtr = findRosterPlayer (&data->rs, oPtr);

   // if open player is empty,
   // or injured enough
   // just swap 'em
   if ((orpPtr->pId == 0) || (oPtr->WeeksOut () >= RosPlayer::MIN_IR_WEEKS))
      SwapPlayers (irrpPtr, orpPtr);

   // otherwise the open player has to be released first (i.e. trade to
   // free agent list)
   else
   {
      RosPlayer::Pos pos = oPtr->GetPosGen ();

      DYN_16   p1Id = 0,
               p2Id = orpPtr->pId;
      ActiveLeague->TradePlayers (0, &p1Id, data->id, &p2Id, TRADE_PROPOSED);
      orpPtr = FindFirstSlot (pos, FALSE);
      vmAssert (!orpPtr->pId);
      IRMove (irPtr, orpPtr->pPtr);
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       GetDepth

   FUNCTION:   return what number on the depth chart this player is
               count number of players of same SPECIFIC position 'ahead'
               of this player
               assigned come first

   INPUT:      pPtr-> player to find

   RETURNS:    index (1's based) of player
               0 indicates error (player not on roster, or is an EMPTY slot)
   ---------------------------------------------------------------------
*/

DYN_16
Roster::GetDepth (RosPlayer *pPtr)
{
   DYN_16   depth = 0;

   if (pPtr)
   {
      RosPlayer::Pos pos = pPtr->GetPosSpec ();

      if (pos != RosPlayer::POS_NONE)
      {
         RosPlayer      *psPtr;

         // just start at the top, and go down
         RosterPlayer   *rpPtr = data->rs.qb;
         while (rpPtr < data->rs.inj)
         {
            psPtr = rpPtr->pPtr;

            // found?
            if (psPtr == pPtr)
               break;

            // count all SIMILAR players
            else
            {
               if (pos == psPtr->GetPosSpec ())
                  depth++;
            }

            rpPtr++;
         }

         // make 1's based (if player found)
         if (psPtr == pPtr)
            depth++;
      }
   }

   return depth;
}

/*
   ---------------------------------------------------------------------
   NAME:       BuildTradePlayerList

   FUNCTION:   the AI for offering a trade response to a human
               if THIS is a human team, then the offer will depend
               on how many players are to be received:
               if 1, then work like the old way, offer everyone
               that matches the position
               if 2 or 3, offer every possible combination that uses
               players in opened slots

   INPUT:      tom = reference to TradeOffer manager (data handler)
               sIds-> position wanted per trade slot
               pPtrs-> players per trade slot

   RETURNS:    number of trades this team added to tom
   ---------------------------------------------------------------------
*/

DYN_32
Roster::BuildTradePlayerList (TOManager &tom,
                              Roster::SlotId *sIds,
                              const RosPlayer **pPtrs,
                              const DYN_32 manageFlag)
{
   DYN_32      numT = tom.GetNumTrades (),   // number of trades stored thusfar
               numIn= 0;                     // number of players incoming
   for (DYN_32 i = 0; i < TradeInfo::MAX_PIDS; i++)
   {
      if (pPtrs [i])
         numIn++;
   }

   // multi-player trade
   if (numIn > 1)
   {
      // make a copy of everyone on this team
      RosterSlots  rs;
      memcpy (&rs, &data->rs, sizeof (rs));

      // then get players to go, for each player coming
      DYN_32               numPs [TradeInfo::MAX_PIDS];
      GenList<RosPlayer>   *pLists [TradeInfo::MAX_PIDS];
      for (i = 0; i < TradeInfo::MAX_PIDS; i++)
      {
         pLists [i] = new GenList<RosPlayer> (NUM_NONINJ_PLAYERS, (DYN_16) 0);
         numPs [i] = 0;
      }
      
      for (DYN_32 i = 0; i < numIn; i++)
      {
         const RosPlayer      *pPtr = pPtrs [i];
         SlotId               slotId = sIds [i];
         RosPlayer::Pos       pos = SlotToPos (slotId);
         GenList<RosPlayer>   *pList = pLists [i];
         DYN_32               numP = 0;

         if (manageFlag)
         {
            // computer AI
            if (pos != RosPlayer::POS_NONE)
               numP = AddPlayerList (pList, pos, slotId, pPtr, manageFlag);
         }

         numP += AddPlayerList (pList, pos, OPEN_SLOT, pPtr, manageFlag);
         numP += AddPlayerList (pList, pos, INACTIVE_SLOT, pPtr, manageFlag);

         numPs [i] = numP;
      }

      // now generate all possible combination of trades
      TradeOffer  to;
      tom.InitOffer (to, data->id);

      pLists [0]->start ();
      for (i = 0; i < numPs [0]; i++)
      {
         to.pPtrs [0] = (*pLists [0]) ();

         pLists [1]->start ();
         for (DYN_32 j = 0; j < numPs [1]; j++)
         {
            RosPlayer   *pPtr = (*pLists [1]) ();
            if (pPtr != to.pPtrs [0])
            {
               to.pPtrs [1] = pPtr;

               pLists [2]->start ();
               for (DYN_32 k = 0; k < TradeInfo::MAX_PIDS; k++)
               {
                  pPtr = (*pLists [2]) ();
                  if (pPtr || !k)
                  {
                     if ((pPtr != to.pPtrs [0]) && (pPtr != to.pPtrs [1]))
                     {
                        to.pPtrs [2] = pPtr;

                        // then see if the team will accept this trade
                        // don't show user trades that won't happen
                        if (TradeEval ((RosPlayer **) pPtrs, to.pPtrs) == TRADE_ACCEPTED)
                           tom.AddTrade (to);
                     }
                  }

                  ++(*pLists [2]);
               }
            }

            ++(*pLists [1]);
         }

         ++(*pLists [0]);
      }

      for (i = 0; i < TradeInfo::MAX_PIDS; i++)
         delete pLists [i];

      // then restore the info
      memcpy (&data->rs, &rs, sizeof (rs));
   }


   // single-player trade
   else
   {
      // single-player trade, offer all players that match sIds [0]
      // (just like last version) (if computer managed, those that
      // aren't trade proof)
      const RosPlayer      *pPtr = *pPtrs;
      SlotId               slotId = *sIds;
      RosPlayer::Pos       pos = SlotToPos (slotId);
      GenList<RosPlayer>   pList (NUM_NONINJ_PLAYERS, (DYN_16) 0);
      DYN_32               numP;

      if (pos == RosPlayer::POS_NONE)
      {
         numP  = AddPlayerList (&pList, RosPlayer::POS_QB,   QB_SLOT, pPtr, manageFlag);
         numP += AddPlayerList (&pList, RosPlayer::POS_RB,   RB_SLOT, pPtr, manageFlag);
         numP += AddPlayerList (&pList, RosPlayer::POS_REC, REC_SLOT, pPtr, manageFlag);
         numP += AddPlayerList (&pList, RosPlayer::POS_OL,   OL_SLOT, pPtr, manageFlag);
         numP += AddPlayerList (&pList, RosPlayer::POS_DL,   DL_SLOT, pPtr, manageFlag);
         numP += AddPlayerList (&pList, RosPlayer::POS_LB,   LB_SLOT, pPtr, manageFlag);
         numP += AddPlayerList (&pList, RosPlayer::POS_DB,   DB_SLOT, pPtr, manageFlag);
         numP += AddPlayerList (&pList, RosPlayer::POS_K,     K_SLOT, pPtr, manageFlag);
         numP += AddPlayerList (&pList, RosPlayer::POS_P,     P_SLOT, pPtr, manageFlag);
      }
      else
         numP = AddPlayerList (&pList, pos, slotId, pPtr, manageFlag);

      // then find everybody that matches in the open & inActive slots
      numP += AddPlayerList (&pList, pos, OPEN_SLOT, pPtr, manageFlag);
      numP += AddPlayerList (&pList, pos, INACTIVE_SLOT, pPtr, manageFlag);

      TradeOffer  to;
      tom.InitOffer (to, data->id);

      pList.start ();
      while (numP--)
      {
         RosPlayer   *pPtr = pList ();
         to.pPtrs [0] = pPtr;
         tom.AddTrade (to);
         ++pList;
      }
   }

   return tom.GetNumTrades () - numT;
}

/*
   ---------------------------------------------------------------------
   NAME:       AssignedFull

   FUNCTION:   Determine if all assigned spots contain players
               (necessary step for calling arcade game or play editor)

   INPUT:      none

   RETURNS:    TRUE/FALSE if all assigned slots are filled
   ---------------------------------------------------------------------
*/

DYN_16
Roster::AssignedFull ()
{
   return data->AssignedFull ();
}

DYN_16 NEAR
RosData::AssignedFull () const
{
   // 10-1-1994, new starting rule, must have 46 players for a game
   const Roster::RosterPlayer *rpPtr = rs.qb,
                              *endPtr = rpPtr + Roster::NUM_GAME_PLAYERS; // rs.open;
   while (rpPtr < endPtr)
   {
      if (!rpPtr->pId)
         return FALSE;

      rpPtr++;
   }

   return TRUE;
}

/*
   ---------------------------------------------------------------------
   NAME:       GenerateStrategies

   FUNCTION:   figure out what drafting strategies should be implemented
               this upcomming season

               The is a list of strategies kept, sorted by what the team
               needs to do most
               Determine what rank the team does best and worst
               move the strategy that corresponds to the rank down or up 1

   INPUT:      ranks-> array of rankings for given set of stats
               relation to all teams in the league

               for example (from team management document):
               the current strategies are:
               DRAFTSTRAT_DEFEND_RUN
               DRAFTSTRAT_KICKING
               DRAFTSTRAT_TRADE
               DRAFTSTRAT_PASS_LONG
               DRAFTSTRAT_RUN_INSIDE
               DRAFTSTRAT_PUNTING
               DRAFTSTRAT_RUSH_PASS
               DRAFTSTRAT_PASS_SHORT
               DRAFTSTRAT_RUN_OUTSIDE
               DRAFTSTRAT_DEFEND_PASS

               and the team has the following ranks []:
               STATLIST_PASS =  7
               STATLIST_RUSH =  3
               STATLIST_ORUSH = 2
               STATLIST_OPASS = 6
               STATLIST_SACK =  1
               STATLIST_FG =    6
               STATLIST_PUNT =  10

               sacking is best (drop rush pass 1)
               punting is worst (up punting 1)
               DRAFTSTRAT_DEFEND_RUN
               DRAFTSTRAT_KICKING
               DRAFTSTRAT_TRADE
               DRAFTSTRAT_PASS_LONG
               DRAFTSTRAT_PUNTING
               DRAFTSTRAT_RUN_INSIDE
               DRAFTSTRAT_PASS_SHORT
               DRAFTSTRAT_RUSH_PASS
               DRAFTSTRAT_RUN_OUTSIDE
               DRAFTSTRAT_DEFEND_PASS

               (when it's time for this team to draft/trade it will determine
               who should be taken based on order of need)

   RETURNS:    drafting strategies set, based on last year's
               statistical placement within the league
   ---------------------------------------------------------------------
*/

void
Roster::GenerateStrategies (DYN_U_16 *ranks)
{
   DYN_U_16 *bestPtr = ranks,
            *worstPtr = ranks,
            *ptr = ranks;

   // set pointers to worst and best rankings
   // best is closest to 0, or the lower number
   while (ptr < (ranks + STATLIST_MAX))
   {
      if (*ptr < *bestPtr)
         bestPtr = ptr;

      if (*ptr > *worstPtr)
         worstPtr = ptr;

      ptr++;
   }

   // set DraftStatList indices (i.e. which list is best worst as an
   // integer, rather than a pointer)
   DraftStatList  bestDSL = (DraftStatList ) (bestPtr - ranks),
                  worstDSL = (DraftStatList ) (worstPtr - ranks);

   // now alter the corresponding DraftStrategy slots
   // any that use the best statistic are lowered
   // any that use the worst statistic are risen
   for (DraftStrategy ds = DRAFTSTRAT_RUN_INSIDE; ds < DRAFTSTRAT_MAX; ds = DraftStrategy(ds + 1))
   {
      // get what statistic is used to evaluate this strategy
      DraftStatList  dsl = dslStratStat [ds];

      // if not trading, check if strategy should move up 1
      if (dsl != STATLIST_MAX)
      {
         if (dsl == worstDSL)
            swapDSL (data->ds.stratRanks, ds, -1);

         else if (dsl == bestDSL)
            swapDSL (data->ds.stratRanks, ds, 1);
      }
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       HealPlayers

   FUNCTION:   have each player on the team heal thyself

   INPUT:      roster of players

   RETURNS:    all players healed
   ---------------------------------------------------------------------
*/

void
Roster::HealPlayers ()
{
   RosterPlayer   *rpPtr = data->rs.qb;
   while (rpPtr < data->rs.inj)
   {
      // don't bother healing empty players
      if (rpPtr->pId)
         rpPtr->pPtr->Heal (FALSE);

      rpPtr++;
   }

   // injured reserve heal at a better rate
   while (rpPtr < &data->rs.inj [NUM_INJ_SLOTS])
   {
      // don't bother healing empty players
      if (rpPtr->pId)
         rpPtr->pPtr->Heal (TRUE);

      rpPtr++;
   }
}
#endif

#if   (LCDT == 0) || (LCDT == 4)
/*
   ---------------------------------------------------------------------
   NAME:       SeasonReset

   FUNCTION:   at the end of the last season, everybody gets fully healed

   INPUT:      roster of players

   RETURNS:    all players healed
   ---------------------------------------------------------------------
*/

void
Roster::SeasonReset (DYN_16 force)
{
   RosterPlayer   *rpPtr = data->rs.qb;
   while (rpPtr < &data->rs.inj [NUM_INJ_SLOTS])
   {
      // don't bother healing empty players
      if (rpPtr->pId)
         rpPtr->pPtr->SeasonReset (force);

      rpPtr++;
   }
}
#endif

#if   (LCDT == 0)
/*
   ---------------------------------------------------------------------
   NAME:       Get/SetTrainAllocs

   FUNCTION:   return or modify a block of training allocation percentages
               (data is handled as a group per position slot (i.e. a
               pointer to the training allocations for QB is returned, or
               modified, not just one of the ratings))

   INPUT:      sId = which slot

   RETURNS:    block of percentages returned or updated
   ---------------------------------------------------------------------
*/

Roster::TrainAllocs *
Roster::GetTrainAllocs (SlotId sId)
{
   TrainAllocs   *taPtr = 0;

   if ((sId < P_SLOT) && data)
      taPtr = &data->trainAllocs [sId];

   return taPtr;
}

void
Roster::SetTrainAllocs (SlotId sId, TrainAllocs *taPtr)
{
   if (sId < P_SLOT)
      memcpy (&data->trainAllocs [sId], taPtr, sizeof (TrainAllocs));
}

/*
   ---------------------------------------------------------------------
   NAME:       trainAlloc

   FUNCTION:   determine if the given team has allocated 100% of time for
               each slot

   INPUT:      this-> roster allocations to check

   RETURNS:    TRUE/FALSE if training camp is set for this team
   ---------------------------------------------------------------------
*/

DYN_16
Roster::TrainAlloc ()
{
   DYN_16   allocSet = TRUE;

   // check for 100% allotment for each position on the roster
   TrainAllocs   *taPtr = (TrainAllocs *) data->trainAllocs;
   for (SlotId sId = QB_SLOT; sId < P_SLOT; sId = SlotId(sId + 1), taPtr++)
   {
      if ((taPtr->ac +
            taPtr->ag +
            taPtr->di +
            taPtr->en +
            taPtr->ha +
            taPtr->in +
            taPtr->sp +
            taPtr->st) < 100)
      {
         allocSet = FALSE;
         break;
      }
   }

   return allocSet;
}

/*
   ---------------------------------------------------------------------
   NAME:       AITrain

   FUNCTION:   perform computer owned AI for training camp attribute
               allotments
               allocations are done thusly:
               10% for each
               sum player PBR percentages
               the 3 lowest percentages receive addition 10, 5 and 5 %

   INPUT:      team's roster

   RETURNS:    TrainAllocs modified
   ---------------------------------------------------------------------
*/

void
Roster::AITrain ()
{
   GenList<RosPlayer>   pList (NUM_DB_SLOTS +
                                 NUM_OPEN_SLOTS +
                                 NUM_INACTIVE_SLOTS,
                                 (DYN_16) 0);
   TrainFunc (setTrainAllocs, data->trainAllocs, &pList);
}

/*
   ---------------------------------------------------------------------
   NAME:       Train

   FUNCTION:   modify all players abrs based on percentage of time
               devoted to each attribute

   INPUT:      TrainAllocs, players

   RETURNS:    all players' attributes bumped upwards
   ---------------------------------------------------------------------
*/

void
Roster::Train ()
{
   GenList<RosPlayer>   pList (NUM_DB_SLOTS +
                                 NUM_OPEN_SLOTS +
                                 NUM_INACTIVE_SLOTS,
                                 (DYN_16) 0);
   TrainFunc (executeTrainAllocs, data->trainAllocs, &pList);
   memset (data->trainAllocs, 0, TA_SIZE);
}
#endif

#if   (LCDT == 0) || (LCDT == 3)
/*
   ---------------------------------------------------------------------
   NAME:       Get/SetRatWeights

   FUNCTION:   return/set ratingWeights block

   INPUT:      rwPtr-> weights to set

   RETURNS:    ->current settings
   ---------------------------------------------------------------------
*/

RatingWeights *
Roster::GetRatWeights ()
{
   return data->rw;
}
#endif

#if   (LCDT == 0)
void
Roster::SetRatWeights (RatingWeights *rwPtr)
{
}
#endif

/*
   ---------------------------------------------------------------------
   NAME:       Print

   FUNCTION:   print player's data to screen

   INPUT:      -> Roster object

   RETURNS:    string printed to screen
   ---------------------------------------------------------------------
*/

void
Roster::Print (Printer *APrinter)
{
   #if   (LCDT == 0)
   #if   PRINT_PLAYERS_ARR
   APrinter->SetLeftMargin (0);

   // first ids
   APrinter->Print ("open_block \"R01:\"\nword\n%2d\n", data->id);
   RosterPlayer   *rpPtr = data->rs.qb;
   for (SlotId sId = QB_SLOT; sId <= INJ_SLOT; sId = SlotId(sId + 1))
   {
      DYN_16   numSlots = getNumSlots (sId);
      while (numSlots--)
      {
         APrinter->Print ("% 4d  ", rpPtr->pId);
         rpPtr++;
      }
      APrinter->Print ("\n");
   }
   APrinter->Print ("\n");

   // then jersey numbers
   APrinter->Print ("// jersey numbers\nbyte\n");
   rpPtr = data->rs.qb;
   for (sId = QB_SLOT; sId <= INJ_SLOT; sId = SlotId(sId + 1))
   {
      DYN_16   numSlots = getNumSlots (sId);
      while (numSlots--)
      {
         APrinter->Print ("% 2d", rpPtr->jersey);
         rpPtr++;
      }
      APrinter->Print ("\n");
   }
   APrinter->Print ("// strategies\n");
   APrinter->Print ("0 0 0 0 0 0 0 0                 // QB training camp allocations\n");
   APrinter->Print ("0 0 0 0 0 0 0 0                 // RB\n");
   APrinter->Print ("0 0 0 0 0 0 0 0                 //  R\n");
   APrinter->Print ("0 0 0 0 0 0 0 0                 // OL\n");
   APrinter->Print ("0 0 0 0 0 0 0 0                 // DL\n");
   APrinter->Print ("0 0 0 0 0 0 0 0                 // LB\n");
   APrinter->Print ("0 0 0 0 0 0 0 0                 // DB\n");
   APrinter->Print ("0 0 0 0 0 0 0 0                 // PK\n");
   APrinter->Print ("#include    \"ratwait.h\"\n");
   APrinter->Print ("close_block\n");
   APrinter->Print ("\n");


   // then players themselves
   rpPtr = data->rs.qb;
   for (sId = QB_SLOT; sId <= INJ_SLOT; sId = SlotId(sId + 1))
   {
      DYN_16   numSlots = getNumSlots (sId);
      while (numSlots--)
      {
         if (rpPtr->pId)
         {
            rpPtr->pPtr->Print (APrinter, TRUE);
            APrinter->Print ("\n");
         }
         else
            APrinter->Print ("id:    0\n<Empty>\n\n");
         rpPtr++;
      }
   }

   #else
   APrinter->Print ("Roster:\n");
   RosterPlayer   *rpPtr = data->rs.qb;

   for (SlotId sId = QB_SLOT; sId <= INJ_SLOT; sId = SlotId(sId + 1))
   {
      APrinter->Print ("---------------------------------\n");
      APrinter->Print ("slot: %d\n\n", sId);
      DYN_16   numSlots = getNumSlots (sId);

      while (numSlots--)
      {
         if (rpPtr->pId)
         {
            APrinter->Print ("jersey: %2d\n", rpPtr->jersey);
            rpPtr->pPtr->Print (APrinter, FALSE);
         }
         else
            APrinter->Print ("id:    0\n<Empty>\n\n");
         rpPtr++;
      }

      APrinter->Print ("\n");
   }

   APrinter->Print ("---------------------------------\n");
   APrinter->Print ("---------------------------------\n\n");
   #endif
   #endif
}






/*
   =====================================================================
                           PRIVATE METHODS
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       Load

   FUNCTION:   read in a Roster's players ids from file

   INPUT:      fp-> file with id's in it
               rId = id of this roster

   RETURNS:    roster's player ids loaded
   ---------------------------------------------------------------------
*/

//extern void int3(void);
void NEAR
Roster::Load (ReadBFile &rlf, DYN_16 rId)
{
   // look for the roster block with this id
   if (rlf.Find (R01Text, rId) != -1)
   {
      RosterDiskData rdd;
      rlf.Read (&rdd, sizeof (RosterDiskData));
      encrypt (&rdd.id);

      data->id = rdd.id;
      LoadSlotIds (&rdd);
      LoadSlotJerseys (&rdd);
      LoadStrategies (&rdd);
      LoadTraining (&rdd);
      LoadWeights (&rdd);
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       LoadSlotIds

   FUNCTION:   read in the ids of the player for each slot on the roster

   INPUT:      -> FILE to get ids from

   RETURNS:    all roster slot ids read (RosterPlayer.pId)
   ---------------------------------------------------------------------
*/

void NEAR
Roster::LoadSlotIds (RosterDiskData *rddPtr)
{
   DYN_16         *sIdPtr = rddPtr->pIds;
   RosterPlayer   *rpPtr = data->rs.qb;
   while (rpPtr < &data->rs.inj [NUM_INJ_SLOTS])
   {
      rpPtr->pId = *sIdPtr++;
      rpPtr++;
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       LoadSlotJerseys

   FUNCTION:   read in the jerseys for each slot on the roster

   INPUT:      -> FILE to get ids from

   RETURNS:    all roster slot jerseys read (RosterPlayer.jersey)
   ---------------------------------------------------------------------
*/

void NEAR
Roster::LoadSlotJerseys (RosterDiskData *rddPtr)
{
   DYN_8          *jPtr = rddPtr->jerseys;
   RosterPlayer   *rpPtr = data->rs.qb;
   while (rpPtr < &data->rs.inj [NUM_INJ_SLOTS])
   {
      rpPtr->jersey = *jPtr++;
      rpPtr++;
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       LoadStrategies

   FUNCTION:   read in the drafting strategies
               (the order is how the team should aquire players)

   INPUT:      -> FILE to get strategies from

   RETURNS:    DraftStrats area filled
   ---------------------------------------------------------------------
*/

void NEAR
Roster::LoadStrategies (RosterDiskData *rddPtr)
{
   memcpy (&data->ds, &rddPtr->ds, sizeof (DraftStrats));
}

/*
   ---------------------------------------------------------------------
   NAME:       LoadTraining

   FUNCTION:   read in the training camp allocations

   INPUT:      -> FILE to get allocations from

   RETURNS:    trainAllocs area filled
   ---------------------------------------------------------------------
*/

void NEAR
Roster::LoadTraining (RosterDiskData *rddPtr)
{
   memcpy (data->trainAllocs, rddPtr->trainAllocs, TA_SIZE);
}

/*
   ---------------------------------------------------------------------
   NAME:       LoadWeights

   FUNCTION:   read in the player rating weights

   INPUT:      -> FILE to get weights from

   RETURNS:    rwPtr area filled
   ---------------------------------------------------------------------
*/

void NEAR
Roster::LoadWeights (RosterDiskData *rddPtr)
{
   // GJW: 11-15-1994, just read the default table (i.e. using the
   // specific ratings)
   // memcpy (&data->rw, &rddPtr->rw, RW_SIZE);
   memcpy (&data->rw, &ratWeights, RW_SIZE);
}

/*
   ---------------------------------------------------------------------
   NAME:       LoadSlotPlayers

   FUNCTION:   read in each player associated with each slot

   INPUT:      -> FILE to get players from
               -> Roster object (ids already loaded)

   RETURNS:    all roster slot player pointers set (RosterPlayer.pPtr)
   ---------------------------------------------------------------------
*/

void NEAR
Roster::LoadSlotPlayers (PlayerRdWr &prw)
{
   DYN_U_16       maxPId = prw.GetMaxId ();
   RosterPlayer   *rpPtr = data->rs.qb;
   while (rpPtr < &data->rs.inj [NUM_INJ_SLOTS])
   {
      DYN_16   id = rpPtr->pId;

      // GJW: 10-11-1993, check upper bound before loading
      // vmAssert ((id < PlayerRdWr::BASE_PLAYER_ID) || (id >= maxPId));
      if ((id < PlayerRdWr::BASE_PLAYER_ID) || (id >= maxPId))
         rpPtr->pId = id = 0;

      // if a player in this slot, get data from disk
      if (id)
         rpPtr->pPtr = new RosPlayer (prw, id);

      // otherwise create an 'empty' player
      else
         rpPtr->pPtr = new RosPlayer ();

      rpPtr++;
   }
}


#if   (LCDT == 0) || (LCDT == 4)
/*
   ---------------------------------------------------------------------
   NAME:       WriteSlotIds

   FUNCTION:   write out the ids of the player for each slot on the roster

   INPUT:      -> FILE to put ids to

   RETURNS:    all roster slot ids written
   ---------------------------------------------------------------------
*/

void NEAR
Roster::WriteSlotIds (RosterDiskData *rddPtr)
{
   DYN_16   *sIdPtr = rddPtr->pIds;

   RosterPlayer   *rpPtr = data->rs.qb;
   while (rpPtr < &data->rs.inj [NUM_INJ_SLOTS])
   {
      *sIdPtr++ = rpPtr->pId;
      rpPtr++;
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       WriteSlotJerseys

   FUNCTION:   write out the jerseys for each slot on the roster

   INPUT:      -> FILE to put ids to

   RETURNS:    all roster slot jerseys written
   ---------------------------------------------------------------------
*/

void NEAR
Roster::WriteSlotJerseys (RosterDiskData *rddPtr)
{
   DYN_8    *jPtr = rddPtr->jerseys;

   RosterPlayer   *rpPtr = data->rs.qb;
   while (rpPtr < &data->rs.inj [NUM_INJ_SLOTS])
   {
      *jPtr++ = rpPtr->jersey;
      rpPtr++;
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       WriteStrategies

   FUNCTION:   write out the drafting strategies
               (the order is how the team should aquire players)

   INPUT:      -> FILE to put strategies to

   RETURNS:    DraftStrats area written
   ---------------------------------------------------------------------
*/

void NEAR
Roster::WriteStrategies (RosterDiskData *rddPtr)
{
   memcpy (&rddPtr->ds, &data->ds, sizeof (DraftStrats));
}

/*
   ---------------------------------------------------------------------
   NAME:       WriteTraining

   FUNCTION:   write out the training camp allocations

   INPUT:      -> FILE to put allocations to

   RETURNS:    trainAllocs area written
   ---------------------------------------------------------------------
*/

void NEAR
Roster::WriteTraining (RosterDiskData *rddPtr)
{
   memcpy (rddPtr->trainAllocs, data->trainAllocs, TA_SIZE);
}

/*
   ---------------------------------------------------------------------
   NAME:       WriteWeights

   FUNCTION:   write out the player rating weights

   INPUT:      -> FILE to put weights to

   RETURNS:    rwPtr area written
   ---------------------------------------------------------------------
*/

void NEAR
Roster::WriteWeights (RosterDiskData *rddPtr)
{
   memcpy (&rddPtr->rw, &data->rw, RW_SIZE);
}

/*
   ---------------------------------------------------------------------
   NAME:       WriteSlotPlayers

   FUNCTION:   write out each player associated with each slot

   INPUT:      -> FILE to put players to

   RETURNS:    each player object is determined, and called to write
               itself
   ---------------------------------------------------------------------
*/

void NEAR
Roster::WriteSlotPlayers (PlayerRdWr &prw)
{
   RosterPlayer   *rpPtr = data->rs.qb;
   while (rpPtr < &data->rs.inj [NUM_INJ_SLOTS])
   {
      DYN_16   id = rpPtr->pId;

      // if a player in this slot, write data to disk
      if (id)
         rpPtr->pPtr->Save (prw);

      rpPtr++;
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       CheckSlotPlayers

   FUNCTION:   make sure all ASSIGNED players are in fact, that type
               of player (e.g. no LBs placed in a REC slot)

   INPUT:      roster players

   RETURNS:    if a 'bad' player is found:
                  if an open player (empty, or of the CORRECT assigned)
                  is found, the player is swapped
                  if not,
                  the player is removed from the roster
   ---------------------------------------------------------------------
*/

void
Roster::CheckSlotPlayers ()
{
   RosterPlayer   *rpPtr = data->rs.qb;

   for (SlotId sId = QB_SLOT; sId < OPEN_SLOT; sId = SlotId(sId + 1))
   {
      DYN_16         numSlots = getNumSlots (sId);
      RosPlayer::Pos pos = SlotToPos (sId);

      while (numSlots--)
      {
         RosPlayer      *pPtr = rpPtr->pPtr;
         RosPlayer::Pos thisPos = pPtr->GetPosGen ();

         // oh-oh!
         if ((thisPos != RosPlayer::POS_NONE) && (thisPos != pos))
         {
            // find open player at the ASSIGNED position
            RosterPlayer   *swapPtr = FindOpenSlot (pos);

            // if one, then move 'bad' player to open
            if (swapPtr)
               SwapPlayers (swapPtr, rpPtr);

            // if not, adios! (becomes an empty player),
            // and the guy 'retires' (isn't sent to free agency)
            else
            {
               rpPtr->pId = 0;
               rpPtr->jersey = 0;
               pPtr->MakeNew ();
            }
         }

         rpPtr++;
      }
   }
}
#endif

/*
   ---------------------------------------------------------------------
   NAME:       AddPlayerList

   FUNCTION:   search through the given slots and add any players
               to the list that match

   INPUT:      pList-> list to put players into (if 0, just return player count)
               pos = what players to find (_NONE means everyone)
               sId = which slot to search
               emptyFlag = include empty slots flag

   RETURNS:    pList a little longer if players were found
               (each player's status is filled with what list he
               came from (i.e. assigned, open, injured))
   ---------------------------------------------------------------------
*/

DYN_16 NEAR
Roster::AddPlayerList (GenList<RosPlayer> *pList,
                        const RosPlayer::Pos pos,
                        const SlotId sId,
                        const DYN_16 emptyFlag)
{
   RosterPlayer   *rpPtr = GetRPPtr (sId);
   DYN_16         maxIndex = getNumSlots (sId),
                  numPlayers = 0;

   for (DYN_16 i = 0; i < maxIndex; i++, rpPtr++)
   {
      RosPlayer      *pPtr = rpPtr->pPtr;
      if (pPtr)
      {
         RosPlayer::Pos genPos = pPtr->GetPosGen (),
                        specPos = pPtr->GetPosSpec ();

         // put this player into the list if:
         // the player matches the general position asked for
         // there is no player at this position (an empty slot!) & they are wanted
         // the caller wants all players, and this is a non-empty player
         if ((pos == RosPlayer::POS_NONE) ||
               ((genPos == RosPlayer::POS_NONE) && emptyFlag) ||
               (genPos == pos) || (specPos == pos))
         {
            if (pList)
               pList->linkin (pPtr);

            pPtr->SetStatus (sId);
            numPlayers++;
         }
      }
   }

   return numPlayers;
}

DYN_32 NEAR
Roster::AddPlayerList (GenList<RosPlayer> *pList,
                        const RosPlayer::Pos pos,
                        const SlotId sId,
                        const RosPlayer *pPtr,
                        const DYN_32 manageFlag)
{
   RosterPlayer   *rpPtr = GetRPPtr (sId);
   DYN_32         nS = getNumSlots (sId),
                  numPlayers = 0;

   while (nS--)
   {
      RosPlayer      *thisPPtr = rpPtr->pPtr;
      if (thisPPtr)
      {
         RosPlayer::Pos genPos = thisPPtr->GetPosGen (),
                        specPos = thisPPtr->GetPosSpec ();

         // put this player into the list if:
         // the player matches the general position asked for
         // the caller wants all players, and this is a non-empty player
         if ((genPos != RosPlayer::POS_NONE) &&
               ((pos == RosPlayer::POS_NONE) ||
               (genPos == pos) || (specPos == pos)))
         {
            DYN_32   addPlayer;

            if (pPtr && manageFlag)
            {
               DYN_32   remove = tradeProof (&data->rs, thisPPtr);
               if (!remove)
               {
                  rwSortPtr = (RatingWeights FAR *) &data->rw;
                  remove = abrSpecCompare ((RosPlayer *) pPtr, thisPPtr);
               }

               addPlayer = !remove;
            }
            else
               addPlayer = TRUE;

            if (addPlayer)
            {
               if (pList)
                  pList->linkin (thisPPtr);

               thisPPtr->SetStatus (sId);
               numPlayers++;
            }
         }
      }

      rpPtr++;
   }

   return numPlayers;
}

#if   (LCDT == 0) || (LCDT == 4)
/*
   ---------------------------------------------------------------------
   NAME:       ManageSlots

   FUNCTION:   every time a roster move takes place, (or team ownership
               is given back to the computer) all the players'
               slot assignments are re-evaluated
               the best players are kept assigned, the 'extras' go to
               the open slots
               then the open slots are shuffled into abr order

               GJW: 3-12-1993, this manages ALL slots, moving players
               based on injuries, and their key ratings
               (kind of like bubbling, but for ALL slots)

               make a destination RosterSlots structure, sort the players,
               and write them into the destination, then copy the destination
               back into the original (not bothering with injured)

               GJW: 10-4-1994, since there are now inactive slots,
               all players that aren't assigned, are dumped into a pool
               of opens, then sorted, then pulled out based on who's best
               rather than by position

   INPUT:      players in their slots

   RETURNS:    TRUE/FALSE if roster was modified
               players shuffled
   ---------------------------------------------------------------------
*/

DYN_16 NEAR
Roster::ManageSlots ()
{
   // get a pointer to first EMPTY player on team
   RosPlayer      *epPtr = GetEmptyPlayer (0);

   RosterSlots    newRS;
   RosterPlayer   *arpPtr = newRS.qb,     // 1st assigned slot (QB)
                  *orpPtr = newRS.open;   // 1st open area
   memset (&newRS, 0, sizeof (RosterSlots));

   // then build lists, sort and fill for each slot type
   GenList<RosPlayer>  pList (NUM_ROSTER_PLAYERS, (DYN_16) 0),
                        extraList (NUM_OPEN_SLOTS + NUM_INACTIVE_SLOTS, (DYN_16) 0);
   for (SlotId sId = QB_SLOT; sId < OPEN_SLOT; sId = SlotId(sId + 1))
   {
      // only take non-injured players
      RosPlayer::Pos pos = SlotToPos (sId);
      AddPlayerList (&pList, pos, sId, FALSE);
      AddPlayerList (&pList, pos, OPEN_SLOT, FALSE);
      AddPlayerList (&pList, pos, INACTIVE_SLOT, FALSE);
      SortPlayerList (&pList, ROSTERSORT_IRSPECABR);

      // now fill in the destination set of players
      // first those that MUST be filled
      pList.start ();
      RosterPlayer   *rpPtr;
      DYN_16         numSlots = getNumSlots (sId);
      while (numSlots--)
      {
         RosPlayer      *pPtr = pList.linkout ();

         // if no player, have to fill assigned with empty
         if (!pPtr)
         {
            #if   FALSE    // GJW: 8-30-1993, debugging dump
            // to figure out why it would assert
            // if empty isn't found for some reason,
            // just bag this sort, and hope to catch it later
            if (!epPtr)
            {
               FILE  *fpOut = fopen ("blah.out", "wt");
               fprintf (fpOut, "rId: %d, slot: %d, nP: %d\n", id, sId, numP);

               DYN_16   np1 = AddPlayerList (0, pos, sId, FALSE),
                        np2 = AddPlayerList (0, pos, OPEN_SLOT, FALSE);
               fprintf (fpOut, "  np1: %d, np2: %d\n", np1, np2);

               arpPtr = data->rs.qb;
               while (arpPtr < &data->rs.inj [NUM_INJ_SLOTS])
               {
                  fprintf (fpOut, "  id: %d\n", arpPtr->pId);
                  fprintf (fpOut, "  ptr: %p\n", arpPtr->pPtr);
                  fprintf (fpOut, "  num: %d\n", arpPtr->jersey);
                  fprintf (fpOut, "  pos Gen: %d, Spec:%d\n",
                              arpPtr->pPtr->GetPosGen (),
                              arpPtr->pPtr->GetPosSpec ());
                  arpPtr++;
               }

               fprintf (fpOut, "\n");
               arpPtr = newRS.qb;
               while (arpPtr < &newRS.inj [NUM_INJ_SLOTS])
               {
                  fprintf (fpOut, "  id: %d\n", arpPtr->pId);
                  fprintf (fpOut, "  ptr: %p\n", arpPtr->pPtr);
                  fprintf (fpOut, "  num: %d\n", arpPtr->jersey);
                  fprintf (fpOut, "  pos Gen: %d, Spec:%d\n",
                              arpPtr->pPtr->GetPosGen (),
                              arpPtr->pPtr->GetPosSpec ());
                  arpPtr++;
               }

               fclose (fpOut);
            }

            #else
            vmAssert (epPtr);

            #endif
            pPtr = epPtr;
            epPtr = GetEmptyPlayer (epPtr);
         }

         rpPtr = findRosterPlayer (&data->rs, pPtr);
         *arpPtr++ = *rpPtr;
      }

      // any 'extra' players go into the pool of open players
      numSlots = pList.size ();
      while (numSlots--)
         extraList.linkin (pList.linkout ());

      pList.ReSize (NUM_ROSTER_PLAYERS);
   }

   // extraList contains all non-assigned players
   // sort based on injury-modified ratings, then store the best in
   // the open area, the rest are inactive
   // GJW: 10-10-1995, use a slightly different table of ratings multipliers
   // because certain positions (QBs, HBs), have a little higher 'normal'
   // value, so they bubble to the top of the list
   // use the manageWeights table, which attempts to correct for this
//   RatingWeights  rwHold [RosPlayer::POS_MAX_SPEC];
//   memcpy (rwHold, data->rw, sizeof (rwHold));
//   memcpy (data->rw, manageWeights, sizeof (data->rw));
   SortPlayerList (&extraList, ROSTERSORT_IRSPECABR);
//   memcpy (data->rw, rwHold, sizeof (rwHold));

   extraList.start ();
   DYN_16   numP = extraList.size ();

   #if   0
   FILE  *fp = fopen ("manage.out", "at");

   fprintf (fp, "for team: %d\n", data->id);
   while (numP--)
   {
      
      RosPlayer   *pPtr = extraList ();

      fprintf (fp, "%s (%s, %d): %d\n",
               pPtr->GetLastName (),
               getSpecPosText (pPtr->GetPosSpec ()), pPtr->GetPosSpec (),
               pPtr->SumInjAbrs (manageWeights));
               
      ++extraList;
   }
   fprintf (fp, "\n");
   fclose (fp);

   extraList.start ();
   numP = extraList.size ();
   #endif

   while (numP--)
      *orpPtr++ = *findRosterPlayer (&data->rs, extraList.linkout ());

   // then any empty players left are put into the remaining open/inactive slots
   while (orpPtr < newRS.inj)
   {
      RosterPlayer   *rpPtr = findRosterPlayer (&data->rs, epPtr);
      *orpPtr++ = *rpPtr;
      epPtr = GetEmptyPlayer (epPtr);
   }

   // copy over the new order (don't bother with IR)
   memcpy (&data->rs, &newRS,
               (NUM_GAME_PLAYERS + NUM_INACTIVE_SLOTS) * sizeof (RosterPlayer));

   return FALSE;
}
#endif

#if   (LCDT == 0)
/*
   ---------------------------------------------------------------------
   NAME:       ManagePreSeason

   FUNCTION:   all players have been aged and retired, now:
               move IR players to open slots or assigned slots
               move as many players out of open slots into assigned as
               will fit (so that there is plenty of room to draft players)
               reshuffle roster
               anyone in IR slot is put into the free agent list

   INPUT:      players in their slots

   RETURNS:    TRUE/FALSE if roster was modified
               many more open slots
   ---------------------------------------------------------------------
*/

DYN_16 NEAR
Roster::ManagePreSeason ()
{
   DYN_16   update = FALSE;

   // move injured players to open or assigned slots
   RosterPlayer   *rpPtr = data->rs.inj;
   for (DYN_16 i = 0; i < NUM_INJ_SLOTS; i++, rpPtr++)
   {
      // get injured player
      RosPlayer      *ipPtr = rpPtr->pPtr;
      RosPlayer::Pos thisPos = ipPtr->GetPosGen ();

      // if not an empty slot, then try to make player assigned or open
      if (thisPos != RosPlayer::POS_NONE)
      {
         RosterPlayer   *swapRPPtr = FindFirstSlot (thisPos, TRUE);
         RosPlayer      *swapPPtr = swapRPPtr->pPtr;

         // if the injured player is better, he gets a spot
         // (he'll always be better than an empty, who's abrs sum to 0)
         if (abrCompare (swapPPtr, ipPtr))
         {
            SwapPlayers (swapRPPtr, rpPtr);
            update = TRUE;
         }
      }
   }

   // move as many open to assigned as possible
   rpPtr = data->rs.open;
   for (i = 0; i < NUM_OPEN_SLOTS + NUM_INACTIVE_SLOTS; i++, rpPtr++)
   {
      // get another open player
      // if not an empty slot, then try to make player assigned
      RosPlayer      *opPtr = rpPtr->pPtr;
      RosPlayer::Pos thisPos = opPtr->GetPosGen ();

      // don't move empty players from open slots
      if (thisPos != RosPlayer::POS_NONE)
      {
         SlotId         sId = posToSlot (thisPos);
         DYN_16         numSlots = getNumSlots (sId);
         RosterPlayer   *aspPtr = GetRPPtr (sId);

         while (numSlots--)
         {
            // if player in an assigned spot is empty, swap 'em
            RosPlayer   *apPtr = aspPtr->pPtr;
            if (apPtr->GetPosGen () == RosPlayer::POS_NONE)
            {
               SwapPlayers (aspPtr, rpPtr);
               update = TRUE;
               break;
            }

            aspPtr++;
         }
      }
   }

   // release any players in an IR slot (they are the lowest ABR players for
   // that position)
   rpPtr = data->rs.inj;
   for (i = 0; i < NUM_INJ_SLOTS; i++, rpPtr++)
   {
      RosPlayer   *ipPtr = rpPtr->pPtr;
      if (ipPtr->GetPosGen () != RosPlayer::POS_NONE)
      {
         // release by trading to free agent list, with no one wanted in return
         DYN_16   p1Id = 0,
                  p2Id = rpPtr->pId;
         ActiveLeague->TradePlayers (0, &p1Id, data->id, &p2Id, TRADE_PROPOSED);
         update = TRUE;
      }
   }

   return update;
}

/*
   ---------------------------------------------------------------------
   NAME:       ManagePostGame

   FUNCTION:   determine Injured Reserve moves for this week

   INPUT:      players in their slots

   RETURNS:    TRUE/FALSE if roster was modified
   ---------------------------------------------------------------------
*/

DYN_16 NEAR
Roster::ManagePostGame ()
{
   // first, check if there are any holes this week
   // (empty, or players with 0 injury points)
   RosterPlayer   *rpPtr = data->rs.inj;
   DYN_16         update = FALSE,
                  irHoles = 0,   // empty AND totally healed players
                  numSlots = getNumSlots (INJ_SLOT);

   while (numSlots--)
   {
      // GJW: 10-8-1994, no one ever comes off IR mid season
      // if (!rpPtr->pId || !rpPtr->pPtr->GetInjPts ())
      if (!rpPtr->pId)
         irHoles++;

      rpPtr++;
   }

   // don't bother if the IR players there can't come off
   if (irHoles)
   {
      DYN_16               numIREligible = 0;
      GenList<RosPlayer>  irList (NUM_GAME_PLAYERS, (DYN_16) 0);
      RosPlayer            *pPtr;

      // look at everybody, and check for IR Eligibles
      // count number of players who can get on
      rpPtr = data->rs.qb;
      while (rpPtr < data->rs.inj)
      {
         // can this player go on IR?
         // all empty slots are 'eligible' so don't test them
         if (rpPtr->pId)
         {
            pPtr = rpPtr->pPtr;

            // GJW: 10-8-1994, be more conservative with placing players
            // on IR, the player doesn't go on, if injury will be healed
            // PRIOR to the end of the season
            // if (pPtr->IREligible ())
            if (pPtr->IREligible () && (pPtr->WeeksOut () >= weeksToGo))
            {
               // GJW: 3-16-1993, got rid of all the 'ai', (80 % and
               // tradeproof). If a player is hurt, try to get them on.
               irList.linkin (pPtr);
               numIREligible++;
            }
         }

         rpPtr++;
      }

      // if players are trying to get on,
      // do slot management
      if (numIREligible)
      {
         // put players into sickest order
         SortPlayerList (&irList, ROSTERSORT_IRSELECT);

         // any injured players will first have to be moved to an open slot
         // before transfer to injured slot
         // irHoles is sum of empty and healed players (each time a new player
         // goes into an IR slot, irHoles is decremented)
         irList.start ();
         while (irHoles)
         {
            // get pointer to player who wants to go on
            // make sure this player wasn't an open player let go via a
            // prior call to MoveToOpen () that moved an assigned player into
            // his slot
            pPtr = irList ();
            if (GetPlayer (pPtr->GetId ()))
            {
               // get an IR slot for this player
               rpPtr = data->rs.inj;
               numSlots = getNumSlots (INJ_SLOT);
               RosPlayer   *ipPtr;
               DYN_16      ok = FALSE;
               while (numSlots--)
               {
                  ipPtr = rpPtr->pPtr;

                  // GJW: 10-8-1994, again, no one ever comes off, mid season
                  // if (!rpPtr->pId || !ipPtr->GetInjPts ())
                  if (!rpPtr->pId)
                  {
                     ok = TRUE;
                     break;
                  }

                  rpPtr++;
               }

               // ipPtr-> IR slot for player to go to
               // pPtr-> player going there
               if (ok)
               {
                  // GJW: 10-8-1994, there is no player to move out
                  IRMove (ipPtr, pPtr);
                  irHoles--;
               }
            }

            if (++irList)
               break;
         }
      }

      // GJW: 10-8-1994, no one comes back mid season
      #if   0
      // move any healed players FROM IR to OPEN or ASSIGNED, if they
      // are better than any player currently there
      rpPtr = data->rs.inj;
      numSlots = getNumSlots (INJ_SLOT);
      while (numSlots--)
      {
         // can this IR player come off?
         RosPlayer   *ipPtr = rpPtr->pPtr;
         if (rpPtr->pId && !ipPtr->GetInjPts ())
         {
            // find any open slot, or crappy player
            RosterPlayer   *worstRPPtr = FindFirstSlot (ipPtr->GetPosGen (), TRUE);

            // found someone to replace
            if (worstRPPtr)
               IRMove (ipPtr, worstRPPtr->pPtr);
         }
      }
      #endif
   }

   return update;
}

/*
   ---------------------------------------------------------------------
   NAME:       ManageWeekly

   FUNCTION:   determine trades for this week
               (set my priorities, then call the league with what
               needs to be done, and it will call the trade proposer
               appropriately)

   INPUT:      players in their slots

   RETURNS:    TRUE/FALSE if roster was modified
   ---------------------------------------------------------------------
*/

DYN_16 NEAR
Roster::ManageWeekly ()
{
   DYN_16   modified = FALSE;

   if (getRandPercent () < 20)
   {
      SlotId         slotToTradeFor [NUM_TRADE_POS];
      DYN_U_16       posDrafts [MAX_DRAFT_LIST + 1];
      fillPosDrafts (posDrafts, data->ds.stratRanks);

      // posDrafts contains a set of values for each position
      // scan for the worst values, and pass them as the slots wanted to
      // the league's trade finder
      for (DYN_8 i = 0; i < NUM_TRADE_POS;)
      {
         DYN_16         ptd = posToDraft (posDrafts, &posDrafts [MAX_DRAFT_LIST + 1]);
         RosPlayer::Pos pos = slotToPos [ptd];
         posDrafts [ptd] = 0;

         // always trade for a specific position (never 'best overall')
         if (pos != RosPlayer::POS_NONE)
            slotToTradeFor [i++] = (SlotId) ptd;
      }


      // find the players on the team that are tradeable (lower players on
      // the thing this team does best)
      fillPosDrafts (posDrafts, data->ds.stratRanks);
      RosPlayer   *pArray [NUM_TRADE_PLAYERS];
      memset (pArray, 0, sizeof (pArray));

      DYN_8 paCount = 0;
      for (i = 0; (i < NUM_TRADE_POS) && (paCount < NUM_TRADE_PLAYERS);)
      {
         DYN_16         ptd = posToRelease (posDrafts, &posDrafts [MAX_DRAFT_LIST + 1]);
         RosPlayer::Pos pos = slotToPos [ptd];
         posDrafts [ptd] = BIG_VALUE;

         // always trade away a specific position (never 'best overall')
         if (pos != RosPlayer::POS_NONE)
         {
            // found the next set of players
            i++;
            GenList<RosPlayer>   pList (NUM_ROSTER_PLAYERS, (DYN_16) 0);
            if (BuildPlayerList (&pList, pos, FALSE, FALSE, FALSE, FALSE, FALSE))
            {
               // start with open and work back towards assigned
               pList.end ();
               while (paCount < NUM_TRADE_PLAYERS)
               {
                  RosPlayer   *pPtr = pList ();

                  if (!tradeProof (&data->rs, pPtr))
                     pArray [paCount++] = pPtr;

                  if (--pList)
                     break;
               }
            }
         }
      }

      modified = ActiveLeague->ManageWeekly (data->id, pArray, slotToTradeFor);
   }

   return modified;
}
#endif

#if   (LCDT == 0) || (LCDT == 4)
/*
   ---------------------------------------------------------------------
   NAME:       FindFirstSlot

   FUNCTION:   search for the first available place on the roster for
               a given position

               check for an empty assigned slot
               if none, check for an empty open slot
               if none, find the weakest player at this position

   INPUT:      pos = position of slot to find
               injFlag = player looking for a spot is coming off injured reserve,
               so if he's better than ANYONE in an OPEN slot, take that player
               (if FALSE, then this is a draft choice, and must be better
               than any player at the same position)

   RETURNS:    ->RosterPlayer slot to put player in
   ---------------------------------------------------------------------
*/

Roster::RosterPlayer * NEAR
Roster::FindFirstSlot (RosPlayer::Pos pos, DYN_16 injFlag)
{
   SlotId         sId = posToSlot (pos);
   DYN_16         numSlots = getNumSlots (sId),
                  lowestABRs = BIG_VALUE;
   RosterPlayer   *retPtr = GetRPPtr (sId),
                  *findPtr = retPtr;
//   RatingWeights  *rwPtr = injFlag ? 0 : data->rw;

   // look for an empty assigned slot
   while (numSlots--)
   {
      if (!findPtr->pId)
         return findPtr;

      else
      {
         RosPlayer   *apPtr = findPtr->pPtr;
         DYN_16      abrSum = apPtr->SumAbrs (data->rw);

         if (abrSum < lowestABRs)
         {
            lowestABRs = abrSum;
            retPtr = findPtr;
         }
      }

      findPtr++;
   }

   // then an empty open slot
   numSlots = NUM_OPEN_SLOTS + NUM_INACTIVE_SLOTS;
   findPtr = GetRPPtr (OPEN_SLOT);
   while (numSlots--)
   {
      // found an empty player
      if (!findPtr->pId)
         return findPtr;

      // on the open list,
      // if player is coming from injured, he'll take anyone's spot who is
      // worse than he is
      // otherwise, only compare players that can play this position
      RosPlayer      *apPtr = findPtr->pPtr;
      RosPlayer::Pos thisPos = apPtr->GetPosGen ();
      if (injFlag || (thisPos == pos))
      {
         DYN_16   abrSum = apPtr->SumAbrs (data->rw);

         if (abrSum < lowestABRs)
         {
            lowestABRs = abrSum;
            retPtr = findPtr;
         }
      }

      findPtr++;
   }

   return retPtr;
}

/*
   ---------------------------------------------------------------------
   NAME:       FindOpenSlot

   FUNCTION:   search for the first OPEN SLOT for the given position

               check for an EMPTY open slot
               if none, find the WEAKEST GUY at THIS POSITION

   INPUT:      pos = position of slot to find
               (if NONE, then find the worst player on the team)

   RETURNS:    ->RosterPlayer slot to put player in
               0 means nowhere
   ---------------------------------------------------------------------
*/

Roster::RosterPlayer * NEAR
Roster::FindOpenSlot (RosPlayer::Pos pos)
{
   DYN_16         numSlots = NUM_OPEN_SLOTS + NUM_INACTIVE_SLOTS,
                  lowestABRs = BIG_VALUE;
   RosterPlayer   *retPtr = 0,
                  *findPtr = GetRPPtr (OPEN_SLOT);

   while (numSlots--)
   {
      // found an empty player
      if (!findPtr->pId)
         return findPtr;

      // is this open player the worst at this position?
      RosPlayer      *apPtr = findPtr->pPtr;
      RosPlayer::Pos thisPos = apPtr->GetPosGen ();
      if ((pos == RosPlayer::POS_NONE) || (thisPos == pos))
      {
         DYN_16   abrSum = apPtr->SumAbrs (data->rw);

         if (abrSum < lowestABRs)
         {
            lowestABRs = abrSum;
            retPtr = findPtr;
         }
      }

      findPtr++;
   }

   return retPtr;
}
#endif

/*
   ---------------------------------------------------------------------
   NAME:       GetRPPtr

   FUNCTION:   return a RosterPlayer pointer to the first slot of the
               given id

   INPUT:      sId = which slots

   RETURNS:    -> RosterPlayer [0] for that slot
   ---------------------------------------------------------------------
*/

Roster::RosterPlayer * NEAR
Roster::GetRPPtr (const SlotId sId)
{
   RosterPlayer  *rpPtr = 0;

   switch (sId)
   {
      case QB_SLOT:
         rpPtr = data->rs.qb;
         break;

      case RB_SLOT:
         rpPtr = data->rs.rb;
         break;

      case REC_SLOT:
         rpPtr = data->rs.rec;
         break;

      case OL_SLOT:
         rpPtr = data->rs.ol;
         break;

      case DL_SLOT:
         rpPtr = data->rs.dl;
         break;

      case LB_SLOT:
         rpPtr = data->rs.lb;
         break;

      case DB_SLOT:
         rpPtr = data->rs.db;
         break;

      case K_SLOT:
         rpPtr = data->rs.k;
         break;

      case P_SLOT:
         rpPtr = data->rs.p;
         break;

      case OPEN_SLOT:
         rpPtr = data->rs.open;
         break;

      case INACTIVE_SLOT:
         rpPtr = data->rs.inActive;
         break;

      case INJ_SLOT:
         rpPtr = data->rs.inj;
         break;
   }

   return rpPtr;
}

#if   (LCDT == 0) || (LCDT == 4)
/*
   ---------------------------------------------------------------------
   NAME:       MoveToOpen

   FUNCTION:   the given player is being released,
               or traded away,
               or put on IR
               put in an open slot

   INPUT:      pPtr-> player to move to open

   RETURNS:    TRUE/FALSE if player was an empty player (they aren't moved)
               player put into an open slot
               another player may be moved into his assigned slot,
               or released to free agency to make room
   ---------------------------------------------------------------------
*/

DYN_16 NEAR
Roster::MoveToOpen (RosPlayer *pPtr)
{
   DYN_16         movedFlag = FALSE;
   RosPlayer::Pos pos = pPtr ? pPtr->GetPosGen () : RosPlayer::POS_NONE;

   // don't bother moving empty players
   if (pos != RosPlayer::POS_NONE)
   {
      // don't move players that are already there
      if (pPtr->GetStatus () < OPEN_SLOT)
      {
         DYN_16         releaseFlag = TRUE;
         RosterPlayer   *rpPtr = FindOpenSlot (pos);

         // keep track if a player must go to free agency to make room
         if (rpPtr)
         {
            SwapPlayers (pPtr, rpPtr->pPtr);
            releaseFlag = FALSE;
         }

         // If I can't (there is no open player of the same position) then
         // the worst open player is released to make room
         // (injured players ARE NOT allowed to go to free-agent list,
         // however, it's possible that all the OPEN players are hurt a
         // little bit, and couldn't be moved, thereby causing all kinds
         // of problems. Therefore, I will 'heal' the player going to
         // free agent list, so next team that gets the player will get a healthy
         // player)
         if (releaseFlag)
         {
            rpPtr = FindOpenSlot (RosPlayer::POS_NONE);
            DYN_16   p1Id = 0,
                     p2Id = rpPtr->pId;
            ActiveLeague->TradePlayers (0, &p1Id, data->id, &p2Id, TRADE_PROPOSED);

            // use pointer to newly created empty player
            rpPtr = FindOpenSlot (RosPlayer::POS_NONE);
            SwapPlayers (pPtr, rpPtr->pPtr);
         }
      }

      movedFlag = TRUE;
   }

   return movedFlag;
}

/*
   ---------------------------------------------------------------------
   NAME:       SlotPosValid

   FUNCTION:   determine if the given player can go into the given slot

   INPUT:      data->rs. array of RosterPlayer slots
               rpPtr-> slot (determine specific slot id by offset)
               pPtr-> player (get general position)

   RETURNS:    whether player can go into the given slot
   ---------------------------------------------------------------------
*/

DYN_16 NEAR
Roster::SlotPosValid (RosterPlayer *rsPtr, RosterPlayer *rpPtr, RosPlayer *pPtr)
{
   // which slot is player GOING TO
   SlotId      sId = QB_SLOT;
   while (TRUE)
   {
      rsPtr += getNumSlots (sId);

      if (rsPtr > rpPtr)
         break;

      // GJW 8-24-1995
      sId = SlotId (sId + 1);
   }

   return SlotPosValid (sId, pPtr->GetPosGen ());
}
#endif

#if   (LCDT == 0)
/*
   ---------------------------------------------------------------------
   NAME:       DraftCollege/Sup/Weekly/GameReady

   FUNCTION:   pick a player from the given lists of players for
               some roster fill-in function:

               College: picks based on wants

               Supplemental: fill holes, then anybody worth taking
               Weekly: fill holes, then anybody worth taking
               GameReady: fill holes

               Weekly
               GJW: 8-31-1992, also gets rid of 'extra' players in
               open slots (i.e. don't carry 4 qbs)
               GJW: 9-23-1993, don't draft punters & kickers

   INPUT:      pList-> array of players for each slot
               posDrafts-> rankings of all the positions to take

   RETURNS:    index of list to take best player from
   ---------------------------------------------------------------------
*/

DYN_16 NEAR
Roster::DraftCollege (GenList<RosPlayer> *pList [], DYN_U_16 *posDrafts)
{
   // for college draft, roster MUST take SOMEBODY, so go through the
   // prioritized list looking for best player at the given positions
   DYN_16   listIndex = -1;
   while (TRUE)
   {
      // get next position to try
      DYN_16   ptd = posToDraft (posDrafts, &posDrafts [MAX_DRAFT_LIST + 1]),
               mustFlag;

      // ptd = slot to fill
      // (if -1, then all the choices have been exhausted, and there are
      // no draft choices good enough to bump someone off the team, so take
      // the best overall)
      if (ptd == -1)
         mustFlag = TRUE;

      // if != -1, then see if best player at a specific slot is better than
      // this team's worst player
      // if not, go on to the next slot category
      else
      {
         posDrafts [ptd] = 0;
         mustFlag = FALSE;
      }

      // don't burn a pick on punter or kicker
      if ((ptd != K_SLOT) && (ptd != P_SLOT))
         listIndex = DraftSelect (pList, ptd, mustFlag, FALSE);

      // if no one found, and this team was down to it's last search type,
      // bail out
      if (listIndex == -1)
      {
         if (mustFlag == TRUE)
            break;
      }

      // otherwise, stop searching if someone found
      else
         break;
   }

   return listIndex;
}

DYN_16 NEAR
Roster::DraftSup (GenList<RosPlayer> *pList [])
{
   // for supplemental draft, fill any ASSIGNED holes that exist
   // check for any Assigned or Open HOLES that need filling
   DYN_16   listIndex = DraftGameReady (pList, DSTYPE_SUP);

   // if none, then see if there is ANYONE better than what we got on
   // the team
   if (listIndex == -1)
      listIndex = DraftSelect (pList, OPEN_SLOT, FALSE, FALSE);

   return listIndex;
}

DYN_16 NEAR
Roster::DraftWeekly (GenList<RosPlayer> *pList [], DYN_U_16 *posDrafts)
{
   // for weekly draft, roster tries to take somebody based on
   // prioritized list looking for best player at the given positions
   DYN_16   listIndex = -1;

   // GJW: 9-27-1995, a team may NEED a player at a certain position,
   // because they only have the minimum needed (say 3 RBs), but one is
   // injured (but not enough to go on IR, say Doubtful-4). The team may
   // not draft an RB because they can't release someone to make room,
   // so they keep a weakened (perhaps several) players in assigned slots.
   // If this is the case, set the mustPick flag so DraftSelect () WILL
   // take a player
   // first, get a count of any significantly injured players, occupying
   // assigned slots
   DYN_U_16    injCounts [MAX_DRAFT_LIST + 1];
   GetInjCounts (injCounts);
   // GetInjCounts () will fill all values for ASSIGNED slots,
   // the value for Trade, needs to be initialized as well
   injCounts [MAX_DRAFT_LIST] = 0;

   while (TRUE)
   {
      // get next position to try
      DYN_16   ptd = posToDraft (posDrafts, &posDrafts [MAX_DRAFT_LIST + 1]);

      // ptd = slot to fill
      // if -1, then all the choices have been exhausted, and there are no
      // draft choices good enough to bump someone off the team, so bag
      if (ptd == -1)
         break;

      // if != -1, then see if best player at a specific slot is better than
      // this team's worst player
      // if not, go on to the next slot category
      else
      {
         posDrafts [ptd] = 0;
         listIndex = DraftSelect (pList, ptd, injCounts [ptd] != 0, FALSE);
         if (listIndex != -1)
            break;
      }
   }

   return listIndex;
}
#endif

#if   (LCDT == 0) || (LCDT == 4)
DYN_16 NEAR
Roster::DraftGameReady (GenList<RosPlayer> *pList [], DraftSelectType dsType)
{
   DYN_16   listIndex = -1;

   RosterPlayer   *rpPtr = data->rs.qb;
   for (SlotId sId = QB_SLOT; (sId < INACTIVE_SLOT) && (listIndex == -1); sId = SlotId(sId + 1))
   {
      DYN_16   numSlots = getNumSlots (sId);
      while (numSlots-- && (listIndex == -1))
      {
         // if there is an open slot, team MUST take anyone available
         if (!rpPtr->pId)
         {
            listIndex = DraftSelect (pList, sId, TRUE, TRUE);

            // if there is no one at this position to draft (i.e. list
            // is empty), then make a new player if this is REQUIRED to fill
            // the hole (assigned slots only)
            // OR, if this is CEI, then at the very least, make a new player
            // to fill the vacancy
            if (dsType == DSTYPE_CEI)
            {
               sId = posToSlot (ceiPos);
               ActiveLeague->GenerateFAPoolPlayer (ceiPos, sId);
               listIndex = DraftSelect (pList, sId, TRUE, TRUE);
            }

            else if ((listIndex == -1) &&
                     (dsType == DSTYPE_GAMEREADY))
            {
               // if team doesn't have enough open players,
               // and there aren't any free agents left to pick,
               // just generate a random position to take
               // (if there's only 1 free agent left, that player will
               // have been found, and chosen)
               if (sId == OPEN_SLOT)
                  // GJW 8-24-1995
                  sId = SlotId (getPosRand () % K_SLOT);

               ActiveLeague->GenerateFAPoolPlayer (slotToPos [sId], sId);
               listIndex = DraftSelect (pList, sId, TRUE, TRUE);
            }
         }
         rpPtr++;
      }
   }

   return listIndex;
}

/*
   ---------------------------------------------------------------------
   NAME:       DraftSelect

   FUNCTION:   pick the player that the above routines have decided
               is important

   INPUT:      pList-> array of players for each slot
               ptd = which list to use (if == -1, pick any)
               mustPick = must take the best player available at any slot
               mustMatch = player being let go must match the position of
                           player selected (FALSE during suplemental draft,
                           so that 'extra' players (i.e. the 4th or 5th QB)
                           can be let go)

   RETURNS:    index of list to choose top player from
               -1 means don't pick anyone
               releasePPtr->player to dump when selection is made
   ---------------------------------------------------------------------
*/

DYN_16 NEAR
Roster::DraftSelect (GenList<RosPlayer> *pList [],
                        DYN_16 ptd, DYN_16 mustPick, DYN_16 mustMatch)
{
   // if ANY one will do, set correct index for table conversion
   if (ptd == -1)
      ptd = OPEN_SLOT;

   RosPlayer::Pos pos = slotToPos [ptd];
   RosPlayer      *draftee = 0;

   // get the best player in the draft
   if (pos == RosPlayer::POS_NONE)
   {
      // go through ALL the lists, getting the best player
      DYN_16         bestABRs = 0;
      DYN_8          FAR *countPtr = openCounts;
      RosPlayer::Pos FAR *posPtr = slotToPos;

      // GJW: 9-23-1993, when taking the best overall player,
      // don't consider punters/kickers (they're a dime a dozen)
      // for (DYN_16 i = 0; i < MAX_DRAFT_LIST; i++, countPtr++, posPtr++)
      // {
      for (DYN_16 i = 0; i < K_SLOT; i++, countPtr++, posPtr++)
      {
         if (!pList [i]->start ())
         {
            // if list is not empty, check the player's credentials
            RosPlayer   *pPtr = (*pList [i]) ();

            // if, during supplemental phase,
            // don't bother checking this player if we have too many already
            if (mustMatch ||
               (BuildPlayerList (0, *posPtr, FALSE, FALSE, FALSE, FALSE, FALSE) <
                  (*countPtr + getNumSlots (posToSlot (*posPtr)))))
            {
               DYN_16   thisABRs = pPtr->SumAbrs (data->rw);
               if (thisABRs > bestABRs)
               {
                  bestABRs = thisABRs;
                  draftee = pPtr;
                  ptd = i;
               }
            }
         }
      }
   }

   // get the best player at THIS position
   else
   {
      // if, during supplemental phase,
      // don't bother checking this player if we have too many already
      SlotId   sId = posToSlot (pos);
      if (mustMatch ||
         (BuildPlayerList (0, pos, FALSE, FALSE, FALSE, FALSE, FALSE) <
         (openCounts [sId] + getNumSlots (sId))))
      {
         if (!pList [ptd]->start ())
            draftee = (*pList [ptd]) ();
      }
   }

   // a potential candidate
   if (draftee)
   {
      // compare this potential player with future teammates
      RosterPlayer   *worstRPPtr = 0;
      RosPlayer::Pos pos = draftee->GetPosGen ();

      if (mustMatch)
         // find the worst player on the team that PLAYS THIS POSITION
         // (may be an empty player!)
         worstRPPtr = FindFirstSlot (pos, FALSE);

      else
      {
         // determine if ANY PLAYER in an open slot makes the team
         // have 'too many'. (i.e. a team does not need 3 punters)
         DYN_8          FAR *countPtr = openCounts;
         RosPlayer::Pos FAR *posPtr = slotToPos,
                        pos;
         while ((pos = *posPtr) != RosPlayer::POS_NONE)
         {
            // get a count of the number of players at each position,
            // and compare to the maximum allowed
            if (BuildPlayerList (0, pos, FALSE, FALSE, FALSE, FALSE, FALSE) >
               (*countPtr + getNumSlots (posToSlot (pos))))
            {

               // since there is an expendable,
               // make sure this selection takes place
               if (!mustPick)
                  mustPick = 2;
               worstRPPtr = FindFirstSlot (pos, FALSE);
               break;
            }

            countPtr++;
            posPtr++;
         }
      }

      if (worstRPPtr)
         data->releasePPtr = worstRPPtr->pPtr;

      // if mustPick == TRUE, he's automatically taken,
      // otherwise we can be choosy (weren't ordered to by caller,
      // or decided to because of 'excess' players), but don't get rid
      // of someone just because
      if (mustPick != TRUE)
      {
         // if no expendables, just do a positional comparision
         if (!worstRPPtr)
         {
            worstRPPtr = FindFirstSlot (pos, FALSE);
            data->releasePPtr = worstRPPtr->pPtr;
         }

         // take only if he's better than the worst player
         // must pass draftee as 2nd player, so that if the two players tie,
         // I'll keep the player already on team, otherwise I get stuck in
         // an infinite loop always trading these 2 players for one another
         // GJW: 8-25-1993, only compare players of SAME SPECIFIC position
         RosPlayer::Pos pos = data->releasePPtr->GetPosSpec ();
         if ((pos != RosPlayer::POS_NONE) &&
            ((pos != draftee->GetPosSpec ()) ||
            !abrSpecCompare (data->releasePPtr, draftee)))
            draftee = 0;
      }
   }

   // if player didn't survive comparision, return no body found
   if (!draftee)
   {
      data->releasePPtr = 0;
      ptd = -1;
   }

   return ptd;
}

/*
   ---------------------------------------------------------------------
   NAME:       BubbleDepth

   FUNCTION:   if roster has released someone, everyone else gets to
               move up the ladder one slot

   INPUT:      pos = position of player leaving
               sId = slot type of player leaving

   RETURNS:    all players at this position are moved up 1 if a spot
               exits
   ---------------------------------------------------------------------
*/

void NEAR
Roster::BubbleDepth (RosPlayer::Pos pos)
{
   if (pos != RosPlayer::POS_NONE)
   {
      SlotId         sId = posToSlot (pos);
      RosterPlayer   *rpdPtr = GetRPPtr (sId),
                     *rpsPtr = rpdPtr + 1,
                     *rpePtr = rpdPtr + getNumSlots (sId);

      // first bubble assigned
      while (rpsPtr < rpePtr)
      {
         if (rpdPtr->pId)
            rpdPtr = rpsPtr;

         // only bother if this player is EMPTY
         else
         {
            // and player bubbling up is not empty
            if (rpsPtr->pId)
            {
               SwapPlayers (rpdPtr, rpsPtr);
               rpdPtr = rpsPtr;
            }
         }

         rpsPtr++;
      }

      // then open
      rpsPtr = GetRPPtr (OPEN_SLOT);
      rpePtr = GetRPPtr (INJ_SLOT);
      while (rpsPtr < rpePtr)
      {
         if (rpdPtr->pId)
            rpdPtr = rpsPtr;

         // only bother if this player is EMPTY
         else
         {
            // and player bubbling up is not empty
            // and of the right position
            if (rpsPtr->pId &&
               (rpsPtr->pPtr->GetPosGen () == pos))
            {
               SwapPlayers (rpdPtr, rpsPtr);
               rpdPtr = rpsPtr;
            }
         }

         rpsPtr++;
      }
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       TrainFunc

   FUNCTION:   execute a training camp-oriented function on all the players
               on this team

   INPUT:      func-> function to call for each position
               taPtr-> TrainAllocs array
               pList-> List of players on this team (enters/leaves empty)
                  don't get any empty or ir (there are no injured yet) players

   RETURNS:    function called for each group of players
   ---------------------------------------------------------------------
*/

void NEAR
Roster::TrainFunc (V_TAP_PLP func,
                  TrainAllocs *taPtr, GenList<RosPlayer> *pList)
{
   // QB
   BuildPlayerList (pList, RosPlayer::POS_QB, FALSE, FALSE, FALSE, FALSE, FALSE);
   func (taPtr++, RosPlayer::POS_QB, pList);
   pList->ReSize (NUM_DB_SLOTS + NUM_OPEN_SLOTS + NUM_INACTIVE_SLOTS);

   // RB
   BuildPlayerList (pList, RosPlayer::POS_RB, FALSE, FALSE, FALSE, FALSE, FALSE);
   func (taPtr++, RosPlayer::POS_RB, pList);
   pList->ReSize (NUM_DB_SLOTS + NUM_OPEN_SLOTS + NUM_INACTIVE_SLOTS);

   // REC
   BuildPlayerList (pList, RosPlayer::POS_REC, FALSE, FALSE, FALSE, FALSE, FALSE);
   func (taPtr++, RosPlayer::POS_REC, pList);
   pList->ReSize (NUM_DB_SLOTS + NUM_OPEN_SLOTS + NUM_INACTIVE_SLOTS);

   // OL
   BuildPlayerList (pList, RosPlayer::POS_OL, FALSE, FALSE, FALSE, FALSE, FALSE);
   func (taPtr++, RosPlayer::POS_OL, pList);
   pList->ReSize (NUM_DB_SLOTS + NUM_OPEN_SLOTS + NUM_INACTIVE_SLOTS);

   // DL
   BuildPlayerList (pList, RosPlayer::POS_DL, FALSE, FALSE, FALSE, FALSE, FALSE);
   func (taPtr++, RosPlayer::POS_DL, pList);
   pList->ReSize (NUM_DB_SLOTS + NUM_OPEN_SLOTS + NUM_INACTIVE_SLOTS);

   // LB
   BuildPlayerList (pList, RosPlayer::POS_LB, FALSE, FALSE, FALSE, FALSE, FALSE);
   func (taPtr++, RosPlayer::POS_LB, pList);
   pList->ReSize (NUM_DB_SLOTS + NUM_OPEN_SLOTS + NUM_INACTIVE_SLOTS);

   // DB
   BuildPlayerList (pList, RosPlayer::POS_DB, FALSE, FALSE, FALSE, FALSE, FALSE);
   func (taPtr++, RosPlayer::POS_DB, pList);
   pList->ReSize (NUM_DB_SLOTS + NUM_OPEN_SLOTS + NUM_INACTIVE_SLOTS);

   // P/K
   BuildPlayerList (pList, RosPlayer::POS_K, FALSE, FALSE, FALSE, FALSE, FALSE);
   BuildPlayerList (pList, RosPlayer::POS_P, FALSE, FALSE, FALSE, FALSE, FALSE);
   func (taPtr, RosPlayer::POS_K, pList);
   pList->ReSize (NUM_DB_SLOTS + NUM_OPEN_SLOTS + NUM_INACTIVE_SLOTS);
}

/*
   ---------------------------------------------------------------------
   NAME:       PlayerBetter

   FUNCTION:   determine if the incoming player is better than the worst player
               on the team (AT THAT POSITION)

   INPUT:      rpPtr-> entire set of players on this team
               pPtr-> player to evaluate

   RETURNS:    TRUE/FALSE if player is better than the worst
   ---------------------------------------------------------------------
*/

DYN_16 NEAR
Roster::PlayerBetter (RosPlayer *pPtr)
{
   DYN_16                  better = FALSE,
                           pFound = FALSE,
                           pAbrs = pPtr->SumAbrs (data->rw);
   RosPlayer::Pos          pos = pPtr->GetPosSpec ();
   Roster::RosterPlayer    *rpPtr = data->rs.qb;

   while (rpPtr < &data->rs.inj [Roster::NUM_INJ_SLOTS])
   {
      // only compare similar players
      RosPlayer   *psPtr = rpPtr->pPtr;
      if (pos == psPtr->GetPosSpec ())
      {
         pFound = TRUE;
         if (pAbrs > psPtr->SumAbrs (data->rw))
         {
            better = TRUE;
            break;
         }
      }

      rpPtr++;
   }

   // if there is no one on the team that plays this position, then he
   // is better than everyone else
   if (!pFound)
      better = TRUE;

   return better;
}
#endif

#if   (LCDT == 0)
/*
   ---------------------------------------------------------------------
   NAME:       RetirePlayer

   FUNCTION:   a player has called it quits, so fill his slot with 0

   INPUT:      tn = teamNews object to report retirement to
               rpPtr-> roster slot that player is vacating

   RETURNS:    slot freed
   ---------------------------------------------------------------------
*/

void NEAR
Roster::RetirePlayer (TeamNews &tn, RosterPlayer *rpPtr)
{
   vmAssert (rpPtr->pId);
   tn.AddRetire (data->id, rpPtr->pId);
   rpPtr->pId = 0;
   rpPtr->jersey = 0;
   rpPtr->pPtr->MakeNew ();
}

// GJW: 9-27-1995
/*
   ---------------------------------------------------------------------
   NAME:       GetInjCounts

   FUNCTION:   return the number of significantly injured players,
               in assigned slots (subtracting the number of non-injured
               players in open/inactive slots)
               i.e. the number of players that FORCE a team to play
               hurt

   INPUT:      injCounts-> array of values to fill

   RETURNS:    injCounts filled
   ---------------------------------------------------------------------
*/

void NEAR
Roster::GetInjCounts (DYN_U_16 *injCounts)
{
   RosterPlayer   *rpPtr = data->rs.qb;
   DYN_U_16       *iPtr = injCounts;

   // fill in on a general position basis
   for (SlotId sId = QB_SLOT; sId < OPEN_SLOT; sId = SlotId(sId + 1))
   {
      DYN_16   numSlots = getNumSlots (sId),
               numInj = 0;

      while (numSlots--)
      {
         if (rpPtr->pId)
         {
            if (rpPtr->pPtr->GetInjPts () >= RosPlayer::INJURY_QUESTION)
               numInj++;
         }

         rpPtr++;
      }

      *iPtr++ = numInj;
   }

   // subtract off any players from open & inactive
   while (rpPtr < data->rs.inj)
   {
      if (rpPtr->pId)
      {
         RosPlayer   *pPtr = rpPtr->pPtr;

         // if there are injured of this type
         iPtr = injCounts + posToSlot (pPtr->GetPosGen ());
         if (*iPtr)
         {
            // check if there are healthy 'waiting in the wings'
            if (pPtr->GetInjPts () < RosPlayer::INJURY_QUESTION)
               (*iPtr)--;
         }
      }
      rpPtr++;
   }
}
#endif







/*
   =====================================================================
                           LOCAL UTILITIES
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       getNumSlots

   FUNCTION:   return the number of slots for the given slot position

   INPUT:      sId = which slot

   RETURNS:    number of players needed to fill the slot
   ---------------------------------------------------------------------
*/

static DYN_16 NEAR
getNumSlots (const Roster::SlotId sId)
{
   DYN_16   numSlots;

   switch (sId)
   {
      case Roster::QB_SLOT:
         numSlots = Roster::NUM_QB_SLOTS;
         break;

      case Roster::RB_SLOT:
         numSlots = Roster::NUM_RB_SLOTS;
         break;

      case Roster::REC_SLOT:
         numSlots = Roster::NUM_REC_SLOTS;
         break;

      case Roster::OL_SLOT:
         numSlots = Roster::NUM_OL_SLOTS;
         break;

      case Roster::DL_SLOT:
         numSlots = Roster::NUM_DL_SLOTS;
         break;

      case Roster::LB_SLOT:
         numSlots = Roster::NUM_LB_SLOTS;
         break;

      case Roster::DB_SLOT:
         numSlots = Roster::NUM_DB_SLOTS;
         break;

      case Roster::K_SLOT:
         numSlots = Roster::NUM_K_SLOTS;
         break;

      case Roster::P_SLOT:
         numSlots = Roster::NUM_P_SLOTS;
         break;

      case Roster::OPEN_SLOT:
         numSlots = Roster::NUM_OPEN_SLOTS;
         break;

      case Roster::INACTIVE_SLOT:
         numSlots = Roster::NUM_INACTIVE_SLOTS;
         break;

      case Roster::INJ_SLOT:
         numSlots = Roster::NUM_INJ_SLOTS;
         break;
   }

   return numSlots;
}

/*
   ---------------------------------------------------------------------
   NAME:       posToSlot

   FUNCTION:   given a player's position (general or specific),
               return the slot id that matches

   INPUT:      pos = position

   RETURNS:    corresponding slot
   ---------------------------------------------------------------------
*/

static Roster::SlotId NEAR
posToSlot (const RosPlayer::Pos pos)
{
   Roster::SlotId    sId = Roster::MAX_SLOT;

   switch (pos)
   {
      case RosPlayer::POS_QB:
         sId = Roster::QB_SLOT;
         break;

      case RosPlayer::POS_RB:
      case RosPlayer::POS_FB:
      case RosPlayer::POS_HB:
         sId = Roster::RB_SLOT;
         break;

      case RosPlayer::POS_REC:
      case RosPlayer::POS_TE:
      case RosPlayer::POS_WR:
         sId = Roster::REC_SLOT;
         break;

      case RosPlayer::POS_OL:
      case RosPlayer::POS_C:
      case RosPlayer::POS_G:
      case RosPlayer::POS_T:
         sId = Roster::OL_SLOT;
         break;

      case RosPlayer::POS_DL:
      case RosPlayer::POS_DE:
      case RosPlayer::POS_DT:
         sId = Roster::DL_SLOT;
         break;

      case RosPlayer::POS_LB:
         sId = Roster::LB_SLOT;
         break;

      case RosPlayer::POS_DB:
      case RosPlayer::POS_CB:
      case RosPlayer::POS_S:
         sId = Roster::DB_SLOT;
         break;

      case RosPlayer::POS_K:
         sId = Roster::K_SLOT;
         break;

      case RosPlayer::POS_P:
         sId = Roster::P_SLOT;
         break;
   }

   return sId;
}

#if   (LCDT == 0) || (LCDT == 4)
/*
   ---------------------------------------------------------------------
   NAME:       verifyNumber

   FUNCTION:   given a pointer to a player, determine if the player
               is using the jersey number given (if this is NOT the
               player in question)

   INPUT:      rpPtr-> RosterPlayer
               num = jersey number to match
               id = id of player who is getting a new number
                     (if rpPtr points to him, don't bother)

   RETURNS:    TRUE/FALSE if number is used by other player
   ---------------------------------------------------------------------
*/

static DYN_16 NEAR
verifyNumber (const Roster::RosterPlayer *rpPtr, const DYN_16 num, const DYN_16 id)
{
   RosPlayer   *pPtr = rpPtr->pPtr;

   // if there is a player,
   // who isn't THIS player,
   // who has the wanted number,
   // return number already taken
   if (pPtr && (pPtr->GetPosGen () != RosPlayer::POS_NONE) &&
      (pPtr->GetId () != id) &&
      (rpPtr->jersey == num))
      return TRUE;

   return FALSE;
}
#endif

/*
   ---------------------------------------------------------------------
   NAME:       findRosterPlayer

   FUNCTION:   find a pointer to the RosterPlayer which has:
               a pointer to the given RosPlayer object
               the given player's id

   INPUT:      rpPtr-> player to find

   RETURNS:    -> RosterPlayer
   ---------------------------------------------------------------------
*/

static Roster::RosterPlayer * NEAR
findRosterPlayer (const Roster::RosterSlots *rsPtr, const RosPlayer *pPtr)
{
   Roster::RosterPlayer *rpPtr = (Roster::RosterPlayer *) rsPtr->qb;

   while (rpPtr <= &rsPtr->inj [Roster::NUM_INJ_SLOTS])
   {
      if (rpPtr->pPtr == pPtr)
         return rpPtr;

      rpPtr++;
   }

   return 0;
}

static Roster::RosterPlayer * NEAR
findRosterPlayer (const Roster::RosterSlots *rsPtr, const DYN_16 pId)
{
   Roster::RosterPlayer *rpPtr = (Roster::RosterPlayer *) rsPtr->qb;
   while (rpPtr <= &rsPtr->inj [Roster::NUM_INJ_SLOTS])
   {
      if (rpPtr->pId == pId)
         return rpPtr;

      rpPtr++;
   }

   return 0;
}

static Roster::RosterPlayer * NEAR
findRosterPlayer (Roster::RosterPlayer *rpPtr,
                  DYN_16 sId, const DYN_16 maxSlot,
                  const DYN_16 id)
{
   for (; sId < maxSlot; sId += 1)
   {
      DYN_16   numSlots = getNumSlots ((Roster::SlotId) sId);

      while (numSlots--)
      {
         if (rpPtr->pId == id)
         {
            RosPlayer   *pPtr = rpPtr->pPtr;

            if (pPtr)
               pPtr->SetStatus (sId);

            return rpPtr;
         }

         rpPtr++;
      }
   }

   return 0;
}

#if   (LCDT == 4)
/*
   ---------------------------------------------------------------------
   NAME:       findCEIPlayer

   FUNCTION:   find a pointer to the RosterPlayer who has a career
               ending injury

   INPUT:      data->rs. array of players to search

   RETURNS:    -> RosterPlayer
   ---------------------------------------------------------------------
*/

static Roster::RosterPlayer * NEAR
findCEIPlayer (const Roster::RosterSlots *rsPtr)
{
   Roster::RosterPlayer    *rpPtr = (Roster::RosterPlayer *) rsPtr->qb;
   while (rpPtr < &rsPtr->inj [Roster::NUM_INJ_SLOTS])
   {
      if (rpPtr->pPtr->GetInjPts () == RosPlayer::INJURY_OVER)
         return rpPtr;
      rpPtr++;
   }

   return 0;
}
#endif

#if   (LCDT == 0) || (LCDT == 4)
/*
   ---------------------------------------------------------------------
   NAME:       emptyRosterPlayer

   FUNCTION:   replace the player & id of the given slot with a
               brand new <empty> player

   INPUT:      rpPtr-> RosterPlayer slot

   RETURNS:    new <empty> RosPlayer created and put in lineup
   ---------------------------------------------------------------------
*/

static void NEAR
emptyRosterPlayer (Roster::RosterPlayer *rpPtr)
{
   rpPtr->pId = 0;
   rpPtr->pPtr = new RosPlayer ();
   rpPtr->jersey = 0;
}
#endif

/*
   ---------------------------------------------------------------------
   NAME:       compareFunc

   FUNCTION:   the function called by the generic shell sort to determine
               if the 2 list elements should be swapped

   INPUT:      iPtr, plusgPtr-> 2 players
               sortType = which sort is happening

   RETURNS:    TRUE/FALSE if 2 teams should be swapped
   ---------------------------------------------------------------------
*/

static DYN_16
compareFunc (void FAR *iPtr, void FAR *plusgPtr, DYN_16 sortType)
{
   DYN_16   doSwap = FALSE;

   switch (sortType)
   {
      case Roster::ROSTERSORT_ALLEN:
         doSwap = allenCompare ((RosPlayer *) iPtr, (RosPlayer *) plusgPtr);
         break;

      case Roster::ROSTERSORT_ALPHA:
         doSwap = alphaCompare ((RosPlayer *) iPtr, (RosPlayer *) plusgPtr);
         break;

      case Roster::ROSTERSORT_NUMBER:
         doSwap = numberCompare ((RosPlayer *) iPtr, (RosPlayer *) plusgPtr);
         break;

      case Roster::ROSTERSORT_POSITION:
         doSwap = positionCompare ((RosPlayer *) iPtr, (RosPlayer *) plusgPtr);
         break;

      case Roster::ROSTERSORT_ABR:
         doSwap = abrCompare ((RosPlayer *) iPtr, (RosPlayer *) plusgPtr);
         break;

      case Roster::ROSTERSORT_INJURY:
         doSwap = injyCompare ((RosPlayer *) iPtr, (RosPlayer *) plusgPtr);
         break;

      case Roster::ROSTERSORT_IRSELECT:
         doSwap = irSelectCompare ((RosPlayer *) iPtr, (RosPlayer *) plusgPtr);
         break;

      case Roster::ROSTERSORT_SPECABR:
         doSwap = abrSpecCompare ((RosPlayer *) iPtr, (RosPlayer *) plusgPtr);
         break;

      case Roster::ROSTERSORT_IRSPECABR:
         doSwap = irAbrSpecCompare ((RosPlayer *) iPtr, (RosPlayer *) plusgPtr);
         break;

      #if   (LCDT == 0)
      case Roster::ROSTERSORT_RATINGS:
         doSwap = ratingsCompare ((RosPlayer *) iPtr, (RosPlayer *) plusgPtr);
         break;

      case Roster::ROSTERSORT_POSDEPTH:
         doSwap = posDepthCompare ((RosPlayer *) iPtr, (RosPlayer *) plusgPtr);
         break;

      case Roster::ROSTERSORT_POSRATINGS:
         doSwap = posRatingsCompare ((RosPlayer *) iPtr, (RosPlayer *) plusgPtr);
         break;
      #endif
   }

   return doSwap;
}

/*
   ---------------------------------------------------------------------
   NAME:       allenCompare

   FUNCTION:   return which player comes first:
               assigned before open
               open before injured reserve

               empty positions are last per slot
               sort each group numerically

   INPUT:      p1, p2 -> RosPlayers

   RETURNS:    0 if p1 comes before p2
               !0 if p2 comes before p1
   ---------------------------------------------------------------------
*/

static DYN_16 NEAR
allenCompare (RosPlayer *p1, RosPlayer *p2)
{
   DYN_16      p1Empty =
                  p1->GetPosGen () == RosPlayer::POS_NONE,
               p2Empty =
                  p2->GetPosGen () == RosPlayer::POS_NONE;

   // first check slots
   Roster::SlotId   p1sId = (Roster::SlotId) p1->GetStatus ();
   Roster::SlotId   p2sId = (Roster::SlotId) p2->GetStatus ();

   if (p1sId != p2sId)
      return p1sId > p2sId;

   // if the same, check if one is empty, which comes last
   if (p2Empty && !p1Empty)
      return 0;

   else if (p1Empty && !p2Empty)
      return 1;

   // if both contain 'real' players, sort by number
   return numberCompare (p1, p2);
}

/*
   ---------------------------------------------------------------------
   NAME:       injyCompare

   FUNCTION:   return which player comes first:
               based on severity of injury
               if tied, based on time out of lineup

   INPUT:      p1, p2 -> RosPlayers
               (assume p1 & p2 have at least 1 injury point (meaning
               empty is impossible))

   RETURNS:    0 if p1 comes before p2
               !0 if p2 comes before p1
   ---------------------------------------------------------------------
*/

static DYN_16 NEAR
injyCompare (RosPlayer *p1, RosPlayer *p2)
{
   DYN_16   injPts1 = p1->GetInjPts (),
            injPts2 = p2->GetInjPts ();

   if (injPts1 != injPts2)
      return injPts1 < injPts2;

   // if same, base it on healing time
   return irSelectCompare (p1, p2);
}

/*
   ---------------------------------------------------------------------
   NAME:       irSelectCompare

   FUNCTION:   return which player comes first:
               based on time out of lineup

   INPUT:      p1, p2 -> RosPlayers

   RETURNS:    0 if p1 comes before p2
               !0 if p2 comes before p1
   ---------------------------------------------------------------------
*/

static DYN_16 NEAR
irSelectCompare (RosPlayer *p1, RosPlayer *p2)
{
   // gain an extra digit of significance by multiplying numerator by constant
   DYN_U_16 weeks1,
            weeks2,
            end = p1->GetABR (RosPlayer::RATING_EN);
   if (!end)
      end = 1;
   weeks1 = (16 * p1->GetInjPts () / end) + 1;
   if (p1->GetStatus () == Roster::INJ_SLOT)
      weeks1 += 8;

   end = p2->GetABR (RosPlayer::RATING_EN);
   if (!end)
      end = 1;
   weeks2 = (p2->GetInjPts () / end) + 1;
   if (p2->GetStatus () == Roster::INJ_SLOT)
      weeks2 += 8;

   return weeks1 < weeks2;
}

/*
   ---------------------------------------------------------------------
   NAME:       alphaCompare

   FUNCTION:   return which player comes first alphabeticaly

   INPUT:      p1, p2 -> RosPlayers

   RETURNS:    0 if p1 comes before p2
               !0 if p2 comes before p1
   ---------------------------------------------------------------------
*/

static DYN_16 NEAR
alphaCompare (RosPlayer *p1, RosPlayer *p2)
{
   DYN_16      p1Empty =
                  p1->GetPosGen () == RosPlayer::POS_NONE,
               p2Empty =
                  p2->GetPosGen () == RosPlayer::POS_NONE;

   // empty slots come last
   if (p2Empty && !p1Empty)
      return 0;

   else if (p1Empty && !p2Empty)
      return 1;

   else
      return strcmp (p1->GetLastName (),
                     p2->GetLastName ()) > 0;
}

/*
   ---------------------------------------------------------------------
   NAME:       numberCompare

   FUNCTION:   return which player comes first numerically by jersey

   INPUT:      p1, p2 -> RosPlayers

   RETURNS:    0 if p1 comes before p2
               !0 if p2 comes before p1
   ---------------------------------------------------------------------
*/

static DYN_16 NEAR
numberCompare (RosPlayer *p1, RosPlayer *p2)
{
   DYN_16      p1Empty = p1->GetPosGen () == RosPlayer::POS_NONE,
               p2Empty = p2->GetPosGen () == RosPlayer::POS_NONE;

   // empty slots come last
   if (p2Empty && !p1Empty)
      return 0;

   else if (p1Empty && !p2Empty)
      return 1;

   else
   {
      Roster::RosterPlayer   *rp1 = findRosterPlayer (rsSortPtr, p1),
                              *rp2 = findRosterPlayer (rsSortPtr, p2);
      return rp1->jersey > rp2->jersey;
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       positionCompare

   FUNCTION:   return which player comes first based on position
               if the same, then abr

   INPUT:      p1, p2 -> RosPlayers

   RETURNS:    0 if p1 comes before p2
               !0 if p2 comes before p1
   ---------------------------------------------------------------------
*/

static DYN_16 NEAR
positionCompare (RosPlayer *p1, RosPlayer *p2)
{
   RosPlayer::Pos p1Pos = p1->GetPosGen (),
                  p2Pos = p2->GetPosGen ();

   DYN_16      p1Empty = p1Pos == RosPlayer::POS_NONE,
               p2Empty = p2Pos == RosPlayer::POS_NONE;

   // empty slots come last
   if (p2Empty && !p1Empty)
      return 0;

   else if (p1Empty && !p2Empty)
      return 1;

   else
   {
      if (p1Pos == p2Pos)
         return abrCompare (p1, p2);

      else
         return p2Pos > p1Pos;
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       abrCompare

   FUNCTION:   return which player comes first based on total abrs

   INPUT:      p1, p2 -> RosPlayers
               (pointer may be NULL)

   RETURNS:    0 if p1 comes before p2
               !0 if p2 comes before p1
   ---------------------------------------------------------------------
*/

static DYN_16 NEAR
abrCompare (RosPlayer *p1, RosPlayer *p2)
{
   DYN_16      p1Empty = p1->GetPosGen () == RosPlayer::POS_NONE,
               p2Empty = p2->GetPosGen () == RosPlayer::POS_NONE;

   // empty slots come last
   if (p2Empty && !p1Empty)
      return 0;

   else if (p1Empty && !p2Empty)
      return 1;

   else
      return p2->SumAbrs (0) > p1->SumAbrs (0);
}

/*
   ---------------------------------------------------------------------
   NAME:       abrSpecCompare

   FUNCTION:   return which player comes first based on KEY RATING abrs

   INPUT:      p1, p2 -> RosPlayers with the same

   RETURNS:    0 if p1 comes before p2
               !0 if p2 comes before p1
   ---------------------------------------------------------------------
*/

static DYN_16 NEAR
abrSpecCompare (RosPlayer *p1, RosPlayer *p2)
{
   return p2->SumAbrs (rwSortPtr) > p1->SumAbrs (rwSortPtr);
}

/*
   ---------------------------------------------------------------------
   NAME:       irAbrSpecCompare

   FUNCTION:   return which player comes first based on KEY RATING abrs,
               modified by injury points

   INPUT:      p1, p2 -> RosPlayers with the same

   RETURNS:    0 if p1 comes before p2
               !0 if p2 comes before p1
   ---------------------------------------------------------------------
*/

static DYN_16 NEAR
irAbrSpecCompare (RosPlayer *p1, RosPlayer *p2)
{
   return p2->SumInjAbrs (rwSortPtr) > p1->SumInjAbrs (rwSortPtr);
}

#if   (LCDT == 0)
/*
   ---------------------------------------------------------------------
   NAME:       ratingsCompare

   FUNCTION:   return which player comes first based on user specified
               RATING abrs

   INPUT:      p1, p2 -> RosPlayers with the same

   RETURNS:    0 if p1 comes before p2
               !0 if p2 comes before p1
   ---------------------------------------------------------------------
*/

static DYN_16 NEAR
ratingsCompare (RosPlayer *p1, RosPlayer *p2)
{
   DYN_16   p1Sum = p1->SumAbrs (extRatWeights),
            p2Sum = p2->SumAbrs (extRatWeights);

   if (p1Sum == p2Sum)
   {
      p1Sum = p1->SumAbrs (NULL),
      p2Sum = p2->SumAbrs (NULL);
   }

   return p2Sum > p1Sum;
}

/*
   ---------------------------------------------------------------------
   NAME:       posDepthCompare

   FUNCTION:   return which player comes first based on position
               if the same, then depth

   INPUT:      p1, p2 -> RosPlayers

   RETURNS:    0 if p1 comes before p2
               !0 if p2 comes before p1
   ---------------------------------------------------------------------
*/

static DYN_16 NEAR
posDepthCompare (RosPlayer *p1, RosPlayer *p2)
{
   RosPlayer::Pos p1Pos = p1->GetPosSpec (),
                  p2Pos = p2->GetPosSpec ();

   DYN_16      p1Empty = p1Pos == RosPlayer::POS_NONE,
               p2Empty = p2Pos == RosPlayer::POS_NONE;

   // empty slots come last
   if (p2Empty && !p1Empty)
      return 0;

   else if (p1Empty && !p2Empty)
      return 1;

   else
   {
      if (p1Pos == p2Pos)
      {
         DYN_8 p1Healthy = (p1->GetStatus () != Roster::INJ_SLOT),
               p2Healthy = (p2->GetStatus () != Roster::INJ_SLOT);

         if (p1Healthy && !p2Healthy)
            return 0;

         else if (!p1Healthy && p2Healthy)
            return 1;

         else if (!p1Healthy && !p2Healthy)
            return abrCompare (p1, p2);

         else
         {
            DYN_16   p1Depth = rSortPtr->GetDepth (p1),
                     p2Depth = rSortPtr->GetDepth (p2);

            if (p1Depth == p2Depth)
               return abrCompare (p1, p2);
            else
               return p2Depth < p1Depth;
         }
      }

      else
         return posPriority [p2Pos] > posPriority [p1Pos];
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       posRatingsCompare

   FUNCTION:   return which player comes first based on position
               if the same, then user specified RATING abrs

   INPUT:      p1, p2 -> RosPlayers

   RETURNS:    0 if p1 comes before p2
               !0 if p2 comes before p1
   ---------------------------------------------------------------------
*/

static DYN_16 NEAR
posRatingsCompare (RosPlayer *p1, RosPlayer *p2)
{
   RosPlayer::Pos p1Pos = p1->GetPosSpec (),
                  p2Pos = p2->GetPosSpec ();

   DYN_16      p1Empty = p1Pos == RosPlayer::POS_NONE,
               p2Empty = p2Pos == RosPlayer::POS_NONE;

   // empty slots come last
   if (p2Empty && !p1Empty)
      return 0;

   else if (p1Empty && !p2Empty)
      return 1;

   else
   {
      if (p1Pos == p2Pos)
         return ratingsCompare (p1, p2);

      else
         return posPriority [p2Pos] > posPriority [p1Pos];
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       getSlot

   FUNCTION:   determine which slot this player is currently occupying

   INPUT:      rsPtr-> array of RosterPlayer structures
               pId = player to find

   RETURNS:    player's slot
   ---------------------------------------------------------------------
*/

static Roster::SlotId NEAR
getSlot (Roster::RosterSlots *rsPtr, const DYN_16 pId)
{
   Roster::SlotId       sId = Roster::NO_SLOT;
   Roster::RosterPlayer *rpPtr = rsPtr->qb;
   DYN_16               nP = Roster::NUM_ROSTER_PLAYERS;   

   while (nP--)
   {
      if (rpPtr->pId == pId)
         break;

      rpPtr++;
   }

   if (rpPtr < &rsPtr->qb [Roster::NUM_QB_SLOTS])
      sId = Roster::QB_SLOT;
   else if (rpPtr < &rsPtr->rb [Roster::NUM_RB_SLOTS])
      sId = Roster::RB_SLOT;
   else if (rpPtr < &rsPtr->rec [Roster::NUM_REC_SLOTS])
      sId = Roster::REC_SLOT;
   else if (rpPtr < &rsPtr->ol [Roster::NUM_OL_SLOTS])
      sId = Roster::OL_SLOT;
   else if (rpPtr < &rsPtr->dl [Roster::NUM_DL_SLOTS])
      sId = Roster::DL_SLOT;
   else if (rpPtr < &rsPtr->lb [Roster::NUM_LB_SLOTS])
      sId = Roster::LB_SLOT;
   else if (rpPtr < &rsPtr->db [Roster::NUM_DB_SLOTS])
      sId = Roster::DB_SLOT;
   else if (rpPtr < &rsPtr->k [Roster::NUM_K_SLOTS])
      sId = Roster::K_SLOT;
   else if (rpPtr < &rsPtr->p [Roster::NUM_P_SLOTS])
      sId = Roster::P_SLOT;
   else if (rpPtr < &rsPtr->open [Roster::NUM_OPEN_SLOTS])
      sId = Roster::OPEN_SLOT;
   else if (rpPtr < &rsPtr->inActive [Roster::NUM_INACTIVE_SLOTS])
      sId = Roster::INACTIVE_SLOT;
   else if (rpPtr < &rsPtr->inj [Roster::NUM_INJ_SLOTS])
      sId = Roster::INJ_SLOT;

   return sId;
}
#endif

/*
   ---------------------------------------------------------------------
   NAME:       swapDSL

   FUNCTION:   shift the given DraftStrategy up or down 1 in the list

   INPUT:      base-> start of array
               index = what player is going up/down
               offset = +/- 1

   RETURNS:    find the next player in array (either with index +/- 1) who
               is being swapped with the passed in player
   ---------------------------------------------------------------------
*/

static void NEAR
swapDSL (DYN_8 FAR *base, DYN_16 index, DYN_16 offset)
{
   DYN_8    FAR *dsPtr = base + index;
   DYN_8    oldRank = *dsPtr,
                  findRank = oldRank + offset;

   // don't search if the player is moving out of bounds
   // (i.e. up for the to spot, or down from the bottom)
   if ((findRank > 0) && (findRank < Roster::DRAFTSTRAT_MAX))
   {
      while (*base != findRank)
         base++;

      // swap this player, and whovever is moving up/down
      *base = oldRank;
      *dsPtr = findRank;
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       posToDraft/Release

   FUNCTION:   go through the array of positional weightings to determine
               what position to:
               take (highest score / what team sucks at)
               release (lowest score / what team does best)

   INPUT:      ptr-> array of positional weightings

   RETURNS:    index of array with highest value
   ---------------------------------------------------------------------
*/

static DYN_16 NEAR
posToDraft (DYN_U_16 *ptr, DYN_U_16 *end)
{
   DYN_16   maxValue = 0,
            maxSlot = -1,
            i = 0;

   while (ptr < end)
   {
      if (*ptr > maxValue)
      {
         maxSlot = i;
         maxValue = *ptr;
      }

      i++;
      ptr++;
   }

   return maxSlot;
}

static DYN_16 NEAR
posToRelease (DYN_U_16 *ptr, DYN_U_16 *end)
{
   DYN_16   maxValue = BIG_VALUE,
            maxSlot = -1,
            i = 0;

   while (ptr < end)
   {
      if (*ptr < maxValue)
      {
         maxSlot = i;
         maxValue = *ptr;
      }

      i++;
      ptr++;
   }

   return maxSlot;
}

/*
   ---------------------------------------------------------------------
   NAME:       fillPosDrafts

   FUNCTION:   determine what each draftable position (qb, rb, etc.)
               evaluates to, based on the values of draft strategies
               (qb uses passShort, runOutside, runInside & passLong to
               determine how urgent a qb is needed for this team)

               GJW 8-25-1995, changed 7 to 4 for punt, kick & trade

   INPUT:      posDrafts-> array to fill with point values
               stratRanks = array of points for each strategy

   RETURNS:    posDrafts values filled in
   ---------------------------------------------------------------------
*/

static void NEAR
fillPosDrafts (DYN_U_16 *posDrafts, DYN_8 FAR *dStratsPtr)
{
   // qb
   posDrafts [Roster::QB_SLOT] =
                           3 * (1 + dStratsPtr [Roster::DRAFTSTRAT_PASS_SHORT]) +
                           2 * (1 + dStratsPtr [Roster::DRAFTSTRAT_RUN_OUTSIDE]) +
                           1 * (1 + dStratsPtr [Roster::DRAFTSTRAT_RUN_INSIDE]) +
                           1 * (1 + dStratsPtr [Roster::DRAFTSTRAT_PASS_LONG]);

   // rb
   posDrafts [Roster::RB_SLOT] =
                           3 * (1 + dStratsPtr [Roster::DRAFTSTRAT_RUN_OUTSIDE]) +
                           2 * (1 + dStratsPtr [Roster::DRAFTSTRAT_RUN_INSIDE]) +
                           1 * (1 + dStratsPtr [Roster::DRAFTSTRAT_PASS_LONG]) +
                           1 * (1 + dStratsPtr [Roster::DRAFTSTRAT_PASS_SHORT]);

   // rec
   posDrafts [Roster::REC_SLOT] =
                           3 * (1 + dStratsPtr [Roster::DRAFTSTRAT_PASS_LONG]) +
                           2 * (1 + dStratsPtr [Roster::DRAFTSTRAT_PASS_SHORT]) +
                           1 * (1 + dStratsPtr [Roster::DRAFTSTRAT_RUN_INSIDE]) +
                           1 * (1 + dStratsPtr [Roster::DRAFTSTRAT_RUN_OUTSIDE]);

   // ol
   posDrafts [Roster::OL_SLOT] =
                           3 * (1 + dStratsPtr [Roster::DRAFTSTRAT_RUN_INSIDE]) +
                           2 * (1 + dStratsPtr [Roster::DRAFTSTRAT_PASS_LONG]) +
                           1 * (1 + dStratsPtr [Roster::DRAFTSTRAT_PASS_SHORT]) +
                           1 * (1 + dStratsPtr [Roster::DRAFTSTRAT_RUN_INSIDE]);

   // dl
   posDrafts [Roster::DL_SLOT] =
                           3 * (1 + dStratsPtr [Roster::DRAFTSTRAT_DEFEND_RUN]) +
                           5 * (1 + dStratsPtr [Roster::DRAFTSTRAT_RUSH_PASS] / 2) +
                           3 * (1 + dStratsPtr [Roster::DRAFTSTRAT_DEFEND_PASS] / 2);

   // lb
   posDrafts [Roster::LB_SLOT] =
                           3 * (1 + dStratsPtr [Roster::DRAFTSTRAT_RUSH_PASS]) +
                           2 * (1 + dStratsPtr [Roster::DRAFTSTRAT_DEFEND_PASS]) +
                           2 * (1 + dStratsPtr [Roster::DRAFTSTRAT_DEFEND_RUN]);

   // db
   posDrafts [Roster::DB_SLOT] =
                           3 * (1 + dStratsPtr [Roster::DRAFTSTRAT_DEFEND_PASS]) +
                           2 * (1 + dStratsPtr [Roster::DRAFTSTRAT_DEFEND_RUN]) +
                           2 * (1 + dStratsPtr [Roster::DRAFTSTRAT_RUSH_PASS]);

   // k
   posDrafts [Roster::K_SLOT] =
                           4 * (1 + dStratsPtr [Roster::DRAFTSTRAT_KICKING]);
//                           7 * (1 + dStratsPtr [Roster::DRAFTSTRAT_KICKING]);

   // p
   posDrafts [Roster::P_SLOT] =
                           4 * (1 + dStratsPtr [Roster::DRAFTSTRAT_PUNTING]);
//                           7 * (1 + dStratsPtr [Roster::DRAFTSTRAT_PUNTING]);

   // trade
   posDrafts [Roster::OPEN_SLOT] =
                           4 * (1 + dStratsPtr [Roster::DRAFTSTRAT_TRADE]);
//                           7 * (1 + dStratsPtr [Roster::DRAFTSTRAT_TRADE]);
}

/*
   ---------------------------------------------------------------------
   NAME:       tradeProof

   FUNCTION:   determine if the player being traded away is one of the
               best at his position on the team

   INPUT:      rpPtr-> entire set of players on this team
               pPtr-> player to compare against

   RETURNS:    TRUE/FALSE if player is untradeable
   ---------------------------------------------------------------------
*/

static DYN_16 NEAR
tradeProof (Roster::RosterSlots *rsPtr, RosPlayer *pPtr)
{
   RosPlayer::Pos          pos = pPtr->GetPosGen ();
   DYN_16                  rank = 0;
   Roster::RosterPlayer    *rpPtr = rsPtr->qb;

   while (rpPtr < &rsPtr->inj [Roster::NUM_INJ_SLOTS])
   {
      RosPlayer   *psPtr = rpPtr->pPtr;
      if (psPtr != pPtr)
      {
         // only compare similar players
         if (pos == psPtr->GetPosGen ())
         {
            if (abrCompare (pPtr, psPtr))
               rank++;
         }
      }
      rpPtr++;
   }

   // is player 'expendable'
   return rank < tradeProofCounts [posToSlot (pos)];
}

#if   (LCDT == 0)
/*
   ---------------------------------------------------------------------
   NAME:       setTrainAllocs

   FUNCTION:   determine which attributes should be trained for the most
               for the group of players

               10% for each attribute
               sum player PBR percentages
               the 3 lowest percentages receive addition 10, 5 and 5 %

               GJW: 7-26-1993, each rating get 5%,
               then the key ratings split up what's left (i.e. for QBs,
               in and st divide up the remaining 70% (6 * 5 = 30),
               but for OL, there are 4 key ratings (1 shared, the other 3
               specific), so the remaining 80% (4 * 5 = 20), is split up
               at 20% among the 4)

   INPUT:      taPtr-> group of training allocations
               pList-> list of players for some position

   RETURNS:    taPtr-> training allocations set
   ---------------------------------------------------------------------
*/

static void
setTrainAllocs (Roster::TrainAllocs *taPtr, const RosPlayer::Pos pos,
                  GenList<RosPlayer> *pList)
{
   DYN_8 maxPct,
         FAR *rat1Ptr = 0,
         FAR *rat2Ptr = 0,
         FAR *rat3Ptr = 0,
         FAR *rat4Ptr = 0;

   switch (pos)
   {
      case RosPlayer::POS_QB:
         rat1Ptr = &taPtr->in;
         rat2Ptr = &taPtr->st;
         maxPct = (100 - (6 * 5)) / 2;
         break;

      case RosPlayer::POS_RB:
         rat1Ptr = &taPtr->sp;   // shared
         rat2Ptr = &taPtr->st;   // fb
         rat3Ptr = &taPtr->ha;   // hb
         maxPct = (100 - (5 * 5)) / 3;
         break;

      case RosPlayer::POS_REC:
         rat1Ptr = &taPtr->ha;   // shared
         rat2Ptr = &taPtr->st;   // te
         rat3Ptr = &taPtr->sp;   // wr
         maxPct = (100 - (5 * 5)) / 3;
         break;

      case RosPlayer::POS_OL:
         rat1Ptr = &taPtr->st;   // shared
         rat2Ptr = &taPtr->ha;   // c
         rat3Ptr = &taPtr->ag;   // g
         rat4Ptr = &taPtr->ac;   // t
         maxPct = (100 - (4 * 5)) / 4;
         break;

      case RosPlayer::POS_DL:
         rat1Ptr = &taPtr->st;   // shared
         rat2Ptr = &taPtr->ac;   // de
         rat3Ptr = &taPtr->ag;   // dt
         maxPct = (100 - (5 * 5)) / 3;
         break;

      case RosPlayer::POS_LB:
         rat1Ptr = &taPtr->sp;
         rat2Ptr = &taPtr->st;
         maxPct = (100 - (6 * 5)) / 2;
         break;

      case RosPlayer::POS_DB:
         rat1Ptr = &taPtr->sp;   // shared
         rat2Ptr = &taPtr->ag;   // cb
         rat3Ptr = &taPtr->ha;   // s
         maxPct = (100 - (5 * 5)) / 3;
         break;

      case RosPlayer::POS_K:
      case RosPlayer::POS_P:
         rat1Ptr = &taPtr->st;
         rat2Ptr = &taPtr->di;
         maxPct = (100 - (6 * 5)) / 2;
         break;
   }

   // all ratings are given some minimum allocation time
   memset (taPtr, 5, sizeof (Roster::TrainAllocs));

   // then key ratings get a boost
   if (rat1Ptr)
      *rat1Ptr = maxPct;
   if (rat2Ptr)
      *rat2Ptr = maxPct;
   if (rat3Ptr)
      *rat3Ptr = maxPct;
   if (rat4Ptr)
      *rat4Ptr = maxPct;
}

/*
   ---------------------------------------------------------------------
   NAME:       executeTrainAllocs

   FUNCTION:   modify each player according to the amount of training
               time give to each rating

   INPUT:      taPtr-> percentage of time to devote
               pList-> list of players to action on

   RETURNS:    each player a little better than before
   ---------------------------------------------------------------------
*/

static void
executeTrainAllocs (Roster::TrainAllocs *taPtr, const RosPlayer::Pos pos,
                     GenList<RosPlayer> *pList)
{
   if (!pList->start ())
   {
      while (TRUE)
      {
         RosPlayer   *pPtr = (*pList) ();

         pPtr->Train ((RosPlayer::Ratings *) taPtr);

         if (++(*pList))
            break;
      }
   }
}
#endif

static void NEAR
encrypt (DYN_16 *ptr16)
{
   DYN_8    xor = 0x69 * (*ptr16++),
            *ptr8 = (DYN_8 *) ptr16;
   DYN_16   size = sizeof (RosterDiskData) - sizeof (DYN_16);

   while (size--)
   {
      *ptr8 = *ptr8 ^ xor++;
      ptr8++;
   }
}


// GJW: 3-15-1993, old way
#if   OBSOLETE
TradeStatus
Roster::TradeEval (DYN_16 p1Id, DYN_16 p2Id)
{
   // first, computer teams don't accept players with injuries
   if (p1Ptr->GetInjPts () < RosPlayer::INJURY_QUESTION)
   {
      // does player play a position needed by this club?
      DYN_8          needsWants = FALSE,
                     numPos = NUM_TRADE_SLOTS;
      DYN_16         ptd;
      RosPlayer::Pos pos;

      // this player must be what this team needs
      DYN_U_16 posDrafts [MAX_DRAFT_LIST + 1];
      fillPosDrafts (posDrafts, dsPtr->stratRanks);
      while (!needsWants && numPos--)
      {
         ptd = posToDraft (posDrafts, &posDrafts [MAX_DRAFT_LIST + 1]);
         posDrafts [ptd] = 0;

         pos = slotToPos [ptd];
         if ((pos == RosPlayer::POS_NONE) ||
            (pos == p1Ptr->GetPosGen ()))
            needsWants = TRUE;
      }

      if (needsWants)
      {
         // does outgoing player fill a position team does well in?
         needsWants = FALSE;
         numPos = NUM_TRADE_SLOTS;
         fillPosDrafts (posDrafts, dsPtr->stratRanks);
         while (!needsWants && numPos--)
         {
            ptd = posToRelease (posDrafts, &posDrafts [MAX_DRAFT_LIST + 1]);
            pos = slotToPos [ptd];
            posDrafts [ptd] = BIG_VALUE;

            if ((pos == RosPlayer::POS_NONE) ||
               (pos == p2Ptr->GetPosGen ()))
               needsWants = TRUE;
         }
      }

      // if incoming position is needed
      // and outgoing position is not
      if (needsWants)
      {
         // compare the players involved (i.e. don't accept a crappy player,
         // or trade away a 'franchise' player)
         if (!tradeProof (rsPtr, p2Ptr, p2Id) &&
            playerBetter (rsPtr, p1Ptr))
            acceptTrade = TRUE;
      }
   }

   // if the trade passes scrutiny,
   // adjust rosters to allow trade to proceed
   if (acceptTrade)
   {
      ts = TRADE_ACCEPTED;

      // make sure player going out is in an open slot
      MoveToOpen (p2Ptr);
   }

   return ts;
}

Roster::WannaTrade (RosPlayer::Pos pos, RosPlayer *ptPtr)
{
   // does player play a position needed by THIS club?
   DYN_8          needsWants = FALSE,
                  numPos = NUM_TRADE_SLOTS;
   RosPlayer::Pos posGive = ptPtr->GetPosGen ();

   // player must be what THIS team needs
   DYN_16         ptd;
   RosPlayer::Pos posGT;
   DYN_U_16       posDrafts [MAX_DRAFT_LIST + 1];
   fillPosDrafts (posDrafts, dsPtr->stratRanks);

   while (!needsWants && numPos--)
   {
      ptd = posToDraft (posDrafts, &posDrafts [MAX_DRAFT_LIST + 1]);
      posGT = slotToPos [ptd];
      posDrafts [ptd] = 0;

      if ((posGT == RosPlayer::POS_NONE) || (posGT == posGive))
         needsWants = TRUE;
   }

   // if player is needed by this team, find someone to give in return
   if (needsWants)
   {
      // determine if THIS team can give up anybody the other team wants
      fillPosDrafts (posDrafts, dsPtr->stratRanks);
      ptd = posToRelease (posDrafts, &posDrafts [MAX_DRAFT_LIST + 1]);
      posGT = slotToPos [ptd];

      if ((posGT == RosPlayer::POS_NONE) || (posGT == pos))
      {
         // make a list of players that match position other team wants
         GenList<RosPlayer>   pList (NUM_DB_SLOTS +
                                       NUM_OPEN_SLOTS +
                                       NUM_INACTIVE_SLOTS, 0);
         if (BuildPlayerList (&pList, pos, FALSE, FALSE))
         {
            pList.start ();

            // give away the best non-trade proof player
            DYN_16   abrValue = 0;

            while (TRUE)
            {
               RosPlayer   *bpPtr = pList ();
               if (!bpPtr->IREligible () &&
                  !tradeProof (rsPtr, bpPtr, bpPtr->GetId ()) &&
                  playerBetter (rsPtr, ptPtr))
                  {

                  DYN_16   ratings = bpPtr->SumAbrs (0);
                  if (ratings > abrValue)
                  {
                     pPtr = bpPtr;
                     abrValue = ratings;
                  }
               }

               if (++pList)
                  break;
            }
         }
      }
   }
}

DYN_16
Roster::ManageSlots ()
{
   RosterPlayer   *rpPtr = data->rs.open;

   // just check every open player to see if he's better than an assigned one
   for (DYN_16 i = 0; i < NUM_OPEN_SLOTS + NUM_INACTIVE_SLOTS; i++, rpPtr++)
   {
      // get another open player
      // if not an empty slot, then try to make player assigned
      RosPlayer      *opPtr = rpPtr->pPtr;
      RosPlayer::Pos thisPos = opPtr->GetPosGen ();

      // don't move empty players from open slots
      if (thisPos != RosPlayer::POS_NONE)
      {
         SlotId   sId = posToSlot (thisPos);
         DYN_16   numSlots = getNumSlots (sId);
         RosterPlayer   *destPtr = GetRPPtr (sId);

         while (numSlots--)
         {
            // check every assigned player at this position
            RosPlayer   *apPtr = destPtr->pPtr;
            if (abrCompare (apPtr, opPtr))
            {
               SwapPlayers (destPtr, rpPtr);
               update = TRUE;
               break;
            }

            destPtr++;
         }
      }
   }

   return update;
}

DYN_16
Roster::ManagePostGame ()
{
   // first, check if there are any holes this week
   // (empty, or players with 0 injury points)
   RosterPlayer   *rpPtr = data->rs.inj;
   DYN_16         update = FALSE,
                  irHoles = 0,   // empty AND totally healed players
                  numSlots = getNumSlots (INJ_SLOT);

   while (numSlots--)
   {
      if (!rpPtr->pId || !rpPtr->pPtr->GetInjPts ())
         irHoles++;
      rpPtr++;
   }

   // don't bother if the 2 players there can't come off
   if (irHoles)
   {
      DYN_16               numIREligible = 0;
      GenList<RosPlayer>  irList (NUM_GAME_PLAYERS, 0);
      RosPlayer            *pPtr;

      // look at everybody, and check for IR Eligibles
      // count number of players who can get on
      rpPtr = data->rs.qb;
      for (SlotId sId = QB_SLOT; sId < INJ_SLOT; sId = SlotId(sId + 1))
      {
         numSlots = getNumSlots (sId);

         while (numSlots--)
         {
            // can this player go on IR?
            // all empty slots are 'eligible' so don't test them
            pPtr = rpPtr->pPtr;
            if (rpPtr->pId && pPtr->IREligible ())
            {

               // get a list of players at this position
               // and count the healthy ones
               // only need a list big enough to hold the worst case
               // (all open players are DBs, and the assigneds are taken)
               GenList<RosPlayer>      pList (NUM_DB_SLOTS +
                                                NUM_OPEN_SLOTS +
                                                NUM_INACTIVE_SLOTS, 0);
               RosPlayer::Pos          pos = pPtr->GetPosGen ();
               DYN_16                  wellCount = 0;

               if (BuildPlayerList (&pList, pos, FALSE, FALSE))
               {
                  pList.start ();
                  while (TRUE)
                  {
                     RosPlayer   *ppPtr = pList ();
                     if (ppPtr != pPtr)
                     {
                        DYN_16   injPts = ppPtr->GetInjPts ();
                        if (injPts < RosPlayer::INJURY_QUESTION)
                           wellCount++;
                     }

                     if (++pList)
                        break;
                  }

                  SlotId   sId = sId;
                  if ((sId == OPEN_SLOT) || (sId == INACTIVE_SLOT))
                     sId = posToSlot (pos);

                  // Less than 80% of ASSIGNED slots must have healthy players
                  // and THIS player must be tradeproof
                  // GJW: 3-15-1993, don't have to be tradeproof, just hurt
                  // if ((wellCount <= (getNumSlots (sId) * 8) / 10) &&
                  //   tradeProof (rsPtr, pPtr, pPtr->GetId ())) {

                  if (wellCount <= ((getNumSlots (sId) * 8) / 10))
                  {
                     numIREligible++;
                     irList.linkin (pPtr);
                  }
               }
            }

            rpPtr++;
         }
      }

      // if players are trying to get on,
      // do slot management
      if (numIREligible)
      {
         // put players into sickest order
         SortPlayerList (&irList, ROSTERSORT_IRSELECT);

         // any injured players will first have to be moved to an open slot
         // before transfer to injured slot
         // irHoles is sum of empty and healed players (each time a new player
         // goes into an IR slot, irHoles is decremented)
         irList.start ();
         while (irHoles)
         {
            // get pointer to player who wants to go on
            // make sure this player wasn't an open player let go via a
            // prior call to MoveToOpen () that moved an assigned player into
            // his slot
            pPtr = irList ();
            if (GetPlayer (pPtr->GetId ()))
            {
               // move him into an open slot
               MoveToOpen (pPtr);

               // get an IR slot for this player
               rpPtr = data->rs.inj;
               numSlots = getNumSlots (INJ_SLOT);
               RosPlayer   *ipPtr;
               while (numSlots--)
               {
                  ipPtr = rpPtr->pPtr;
                  if (!rpPtr->pId || !ipPtr->GetInjPts ())
                  {
                     irHoles--;
                     break;
                  }

                  rpPtr++;
               }

               // rpPtr-> IR slot for player to go to
               // pPtr-> player going there
               IRMove (ipPtr, pPtr);
            }

            if (++irList)
               break;
         }

         // move any healed players FROM IR to OPEN if there are holes for
         // them to plug
         rpPtr = data->rs.open;
         numSlots = getNumSlots (OPEN_SLOT) + getNumSlots (INACTIVE_SLOT);
         DYN_16   openHoles = 0;
         while (numSlots--)
         {
            if (!rpPtr->pId)
               openHoles++;
            rpPtr++;
         }

         if (openHoles)
         {
            rpPtr = data->rs.inj;
            numSlots = getNumSlots (INJ_SLOT);
            while (numSlots--)
            {
               // can this IR player come off?
               RosPlayer   *ipPtr = rpPtr->pPtr;
               if (rpPtr->pId && !ipPtr->GetInjPts ())
               {
                  // look for an EMPTY open slot
                  RosterPlayer   *orpPtr = FindOpenSlot (RosPlayer::POS_NONE);
                  if (!orpPtr->pId)
                  {
                     SwapPlayers (rpPtr, orpPtr);
                     update = TRUE;
                  }
               }
            }
         }
      }
   }

   return update;
}

/*
   ---------------------------------------------------------------------
   NAME:       FlushPlayerList

   FUNCTION:   remove all players from list, but don't delete list

   INPUT:      pList-> list to remove players from

   RETURNS:    pList size = 0
   ---------------------------------------------------------------------
*/

void
Roster::FlushPlayerList (GenList<RosPlayer> *pList)
{
   if (pList && !pList->start ())
   {
      while (TRUE)
      {
         if (!pList->linkout ())
            break;
      }
   }
}

static void NEAR
setTrainAllocs (Roster::TrainAllocs *taPtr, GenList<RosPlayer> *pList)
{
   DYN_U_8  FAR *ptr = (DYN_U_8 FAR *) taPtr,
            offsets [3] = { 0, 0, 0 };

   // first sum each of the pbrs & abrs for each player
   if (!pList->start ())
   {
      RosPlayer::Rating pRating;
      DYN_U_16          abrCounts [sizeof (Roster::TrainAllocs)], *abrPtr,
                        pbrCounts [sizeof (Roster::TrainAllocs)], *pbrPtr;
      memset (abrCounts, 0, sizeof (abrCounts));
      memset (pbrCounts, 0, sizeof (pbrCounts));

      while (TRUE)
      {
         RosPlayer   *pPtr = (*pList) ();
         abrPtr = abrCounts;
         pbrPtr = pbrCounts;

         for (pRating = RosPlayer::RATING_AC;
               pRating <= RosPlayer::RATING_ST;
               pRating = RosPLayer::Rating(pRating + 1), abrPtr++, pbrPtr++)
         {
            *abrPtr += pPtr->GetABR (pRating);
            *pbrPtr += pPtr->GetPBR (pRating);
         }

         if (++(*pList))
            break;
      }

      // convert absulute counts to percentages
      DYN_16   numPlayers = pList->size ();
      abrPtr = abrCounts;
      pbrPtr = pbrCounts;
      pRating = RosPlayer::RATING_AC;
      for (; pRating <= RosPlayer::RATING_ST; pRating = RosPlayer::Rating(pRating + 1), abrPtr++, pbrPtr++)
      {
         if (numPlayers != 1)
         {
            *pbrPtr /= numPlayers;
            *abrPtr /= numPlayers;
         }
         *abrPtr *= 100;
         *abrPtr /= *pbrPtr;
      }

      // find the 3 lowest percentages, when 1 is found, set it to 101%
      // so that it isn't found the next time also
      DYN_U_8  minOffset = findLowestValue (abrCounts, sizeof (Roster::TrainAllocs));
      offsets [0] = minOffset;
      abrCounts [minOffset] = 101;

      minOffset = findLowestValue (abrCounts, sizeof (Roster::TrainAllocs));
      offsets [1] = minOffset;
      abrCounts [minOffset] = 101;

      minOffset = findLowestValue (abrCounts, sizeof (Roster::TrainAllocs));
      offsets [2] = minOffset;
   }

   // all positions get 10
   taPtr->ac = 10;
   taPtr->ag = 10;
   taPtr->di = 10;
   taPtr->en = 10;
   taPtr->ha = 10;
   taPtr->in = 10;
   taPtr->sp = 10;
   taPtr->st = 10;

   // and 3 get a little extra
   *(ptr + offsets [0]) += 10;
   *(ptr + offsets [1]) += 5;
   *(ptr + offsets [2]) += 5;
}

/*
   ---------------------------------------------------------------------
   NAME:       findLowestValue

   FUNCTION:   given an array of values, find the index of the lowest

   INPUT:      ptr -> array of 16 bit unsigned values
               size = number of elements in array

   RETURNS:    offset to the lowest value
   ---------------------------------------------------------------------
*/

static DYN_U_8 NEAR
findLowestValue (DYN_U_16 *ptr, DYN_U_8 size)
{
   DYN_U_8     minOffset = 0,
               offset = 0;
   DYN_U_16    min = -1;

   while (size--)
   {
      if (*ptr < min)
      {
         min = *ptr;
         minOffset = offset;
      }

      ptr++;
      offset++;
   }

   return minOffset;
}
#endif
