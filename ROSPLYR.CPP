/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   rosplyr.cpp

   GJW: 2-29-1992, original creation
   GJW: 4-12-1992, added RosPlayer (FILE *) constructor (just get the next
                     one off disk in order)
   GJW: 6-3-1992, players' data is kept in one big file, so ID must be
                     passed. lseek to the correct offset based on id.
                     also, keep the last file & id info around so that if
                     the next sequential one is loaded, don't bother with
                     lseek
   GJW: 6-10-1992, added LCDT define for controlling what get compiled
                   for the various libraries:
                     shell (LCDT = 0)
                     arcade (LCDT = A_LCDT)
                     editor (LCDT = ED_LCDT)
                     halftime (LCDT = HT_LCDT)
   GJW: 7-8-1992, no more blockified info, all players are the same size
                     for easier access (plus a modified flag for writing data)
   GJW: 7-14-1992, player id's start at 100, to differentiate from teams
   GJW: 9-9-1992, more injury info needed, EBRs are not
   GJW: 10-3-1992, too many TEs are generated. Increased the strength value
                     from 37 to 51.
                  (ST ratings between 21 and 57 are generated for a
                  REC (a range of 36). 50 % of that range is 18,
                  so 18 + 21 = 39 (close to 37)
                  Instead take 85% of the range and get (36 * .85) + 21 = 51)
   GJW: 12-1-1992, as soon as QBs have been in the league 6 years, they
                  end up riding the bench, so make their aging factor
                  .8 rather than 1.0
   GJW: 3-11-1993, decrease the effects of aging across the board,
                     make rookies start off with less ability
                     i.e. AGE_EFFECT_FACTOR form 2 to 1,
                     abrRange array values modified downward
   GJW: 4-13-1993, PlayerMem functionality
   GJW: 7-7-1993, new table values
                  add 10 to player's pbr
   GJW: 8-2-1993, retire also based on depth, starters at 1/2 rate
   GJW: 12-1-1994, slowed down rookie receivers and dbs by adding index
                     3 to abrRange, and abrRangeTable referring to it
	CAM	18 DEC 96	Added SumAbrs that takes up to two additional players

컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   #includes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#define		DBGPOOL		0
#include       <string.h>

#include       <ports.h>
#include       "dyndefs.h"

#include       "assertvm.h"
//#include       "lcdtutil.h"
#include       "mono.h"
#include       "rosplyr.h"
#include       "poolaloc.h"
#include       "fbmu.h"
#include			"lcdtutil.h"

#if DBGPOOL
#include "wtypes.h"
#include "winbase.h"
#endif

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   consts
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#define  PRINT_PLAYERS_ARR    FALSE
#define  DEBUG_COUNT          FALSE
#define  DEBUG_SUPER          FALSE
#define  DEBUG_SMALL_ALLOC    FALSE
#define  DEBUG_VERIFY_NODES   FALSE

const MAX_RATING_POINTS = 99;
const AGE_EFFECT_YEARS = 5;
const AGE_POS_QB_FACTOR = 8;
const AGE_POS_RB_FACTOR = 11;
const AGE_POS_REC_FACTOR = 12;
const AGE_POS_K_FACTOR = 7;
const AGE_POS_P_FACTOR = 10;
const AGE_EFFECT_FACTOR = 6;  // in eights, original value = 16
const SUPERSTAR_CHANCH = 1024;
const INCPBR_CHANCE        = 5;
const INCPBR_MIN_THRESH    = 26;
const INCPBR_MIN_BOOST_A   = 25;
const INCPBR_MIN_BOOST_B   = 25;
const INCPBR_MAX_BOOST     = 25;
const INCPBR_TINY_RANGE    = 5;

enum GenToIndex
{
      QB_INDEX,
      RB_INDEX,
      REC_INDEX,
      OL_INDEX,
      DL_INDEX,
      LB_INDEX,
      DB_INDEX,
      PK_INDEX,
};

enum InjuryType
{
      INJURY_BRUISED_ANKLE,
      INJURY_STRAINED_ANKLE,
      INJURY_SPRAINED_ANKLE,
      INJURY_BROKEN_ANKLE,

      INJURY_BRUISED_ARM,
      INJURY_STRAINED_WRIST,
      INJURY_BROKEN_FINGER,
      INJURY_BROKEN_HAND,

      INJURY_SORE_BACK,
      INJURY_STRAINED_BACK,
      INJURY_RUPTURED_DISK,

      INJURY_STRAINED_CHEST,
      INJURY_FRACTURED_RIB,
      INJURY_DISLOCATED_RIB,

      INJURY_BRUISED_THIGH,
      INJURY_STRAINED_HAMSTRING,
      INJURY_FRACTURED_FEMUR,

      INJURY_STRAINED_HIP,
      INJURY_HIP_FRACTURE,
      INJURY_DISLOCATED_HIP,

      INJURY_SORE_KNEE,
      INJURY_BRUISED_KNEE,
      INJURY_KNEE_CARTILAGE,

      INJURY_BRUISED_CALF,
      INJURY_STRAINED_CALF,
      INJURY_BROKEN_LEG,

      INJURY_SORE_NECK,
      INJURY_NECK_STRAIN,
      INJURY_NECK_SPRAIN,
      INJURY_DISLOCATED_DISK,

      INJURY_BRUISED_SHOULDER,
      INJURY_STRAINED_SHOULDER,
      INJURY_DISLOCATED_SHOULDER,

      MAX_INJURIES,
};

struct FAR RosPlayerData
{
   #if   DEBUG_VERIFY_NODES
   static void             VerifyNodes ();
   #endif

   void                    FAR * operator new (size_t size);
   void                    operator delete (void FAR *t, size_t size);

   static PoolAlloc        *pa;              // which pool did new () come

   DYN_16                  id;
   DYN_8                   firstName [PlayerRdWr::NAME_LEN + 1],
                           lastName [PlayerRdWr::NAME_LEN + 1];
   RosPlayer::Ratings      pbr;    // potential basic ratings
   RosPlayer::InjuryData   injury;
   RosPlayer::Pos          posGen,
                           posSpec;
   RosPlayer::Ratings      abr;    // actual basic ratings (<= pbr)
   DYN_8                   years,
                           status,
                           modified;
   double                  proBowlValue;
};

struct LowHigh
{
   DYN_8 low,
         high;
};

struct PosSpecific
{
   RosPlayer::Pos    posA,
                     posB;
   RosPlayer::Rating rating;
   DYN_8             cutoff;
};

struct BonusPBR
{
   RosPlayer::Rating bless1,
                     bless2,
                     bless3;
};

struct RetireMult
{
   DYN_8 numYears,
         percent;
};

const PLAYERS_PER_TEAM = 53 + 10;
const NUM_TEAMS = 32 * 2;
const NUM_ALLOC = (PLAYERS_PER_TEAM * NUM_TEAMS + 3); //* 12; JDW - Tripled this value

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   utility prototypes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

static DYN_8            NEAR generatePBRs (RosPlayer::Ratings *pbrPtr,
                                             DYN_8 FAR *src);
static void             NEAR generateABRs (RosPlayer::Ratings *abrPtr,
                                             RosPlayer::Ratings *maxPtr,
                                             DYN_8 FAR *src);
static RosPlayer::Pos   NEAR generateSpecPos (const RosPlayer::Pos pos,
                                          const DYN_8 FAR *prPtr);
static void             NEAR blessPBRs (const RosPlayer::Pos pos, DYN_8 FAR *);
static void             NEAR ageBRs (RosPlayer::Pos, DYN_8, DYN_8 FAR *, DYN_8 FAR *);
static DYN_8            NEAR getYearMult (const DYN_8 yearsLeft, const DYN_8 nextCutOff);

#if   PRINT_PLAYERS_ARR
static void NEAR printRatings (Printer *prnPtr, RosPlayer::Ratings *prPtr);
#endif

#if   DEBUG_COUNT
static void NEAR monoDump ();
#endif

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   global data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   local data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

INCLUDE_ASSERT;

// where to get memory from
PoolAlloc      *RosPlayer::pa,
               *RosPlayerData::pa;

#if   (LCDT == 0) || (LCDT == 4)

static LowHigh FAR pbrRange [] =
               {
                  { 20, 60 },    //  0
                  { 30, 60 },    //  1
                  { 30, 80 },    //  2
                  { 30, 90 },    //  3
                  { 35, 70 },    //  4
                  { 35, 79 },    //  5
                  { 40, 70 },    //  6
                  { 40, 75 },    //  7
                  { 40, 80 },    //  8
                  { 40, 85 },    //  9
                  { 40, 90 },    // 10
                  { 45, 70 },    // 11
                  { 45, 80 },    // 12
                  { 45, 90 },    // 13
                  { 50, 80 },    // 14
                  { 50, 85 },    // 15
                  { 50, 90 },    // 16
                  { 55, 80 },    // 17
                  { 55, 85 },    // 18
                  { 55, 90 },    // 19
                  { 60, 85 },    // 20
                  { 60, 90 },    // 21
                  { 60, 95 },    // 22
                  { 60, 99 },    // 23
                  { 65, 95 },    // 24
                  { 65, 99 },    // 25
                  { 70, 95 },    // 26
                  { 75, 99 },    // 27
                  { 80, 99 },    // 28
};

static DYN_8   FAR pbrRangeTable [8] [sizeof (RosPlayer::Ratings)] =
               {
                  // ac  ag  di  en  ha  in  sp  st
                  {  17, 17, 23,  2, 19, 23, 14, 24   }, // qb
                  {  28, 27, 13, 19, 24, 10, 27,  9   }, // rb
                  {  28, 28,  8, 25, 27,  8, 28,  9   }, // r
                  {   4,  6, 21, 25, 14,  3,  5, 26   }, // ol
                  {  12, 12, 21, 25, 12,  8, 20, 25   }, // dl
                  {  19, 18, 16, 25, 18, 19, 18, 22   }, // lb
                  {  28, 26,  8, 25, 26, 13, 27, 10   }, // db
                  {   7,  1, 19,  0, 13,  7,  0, 27   }, // k/p
               };

static LowHigh FAR abrRange [] =
               {
                  { 40, 80 }, // 0
                  { 50, 90 }, // 1
                  { 60, 90 }, // 2
                  { 60, 80 }, // 3
               };

static DYN_8   FAR abrRangeTable [8] [sizeof (RosPlayer::Ratings)] =
               {
                  // ac  ag   di   en   ha   in   sp   st
                  {  1,   1,   1,   0,   1,   2,   1,   2   }, // qb
                  {  2,   2,   0,   1,   1,   2,   2,   0   }, // rb
                  {  2,   1,   0,   1,   2,   2,   3,   0   }, // r
                  {  0,   0,   0,   0,   0,   2,   1,   1   }, // ol
                  {  1,   0,   0,   0,   0,   2,   1,   1   }, // dl
                  {  1,   1,   1,   0,   0,   2,   1,   0   }, // lb
                  {  2,   2,   0,   1,   1,   2,   3,   0   }, // db
                  {  0,   0,   1,   0,   0,   2,   0,   1   }, // k/p
               };

#define  PBR_LOW(i, r)     pbrRange [pbrRangeTable [i] [r]].low
#define  PBR_HIGH(i, r)    pbrRange [pbrRangeTable [i] [r]].high
#define  PBR_PCT(i, r, p)  PBR_LOW(i,r) + \
                           (p * (PBR_HIGH(i, r) - PBR_LOW(i, r)) / 100)


static PosSpecific   FAR specifics [] =
                     {
                        {
                           RosPlayer::POS_FB,   RosPlayer::POS_HB,
                           RosPlayer::RATING_ST,
                           PBR_PCT(RB_INDEX, RosPlayer::RATING_ST, 60)
                        },
                        {  RosPlayer::POS_TE,   RosPlayer::POS_WR,
                           RosPlayer::RATING_ST,
                           PBR_PCT(REC_INDEX, RosPlayer::RATING_ST, 85)
                        },
                        {  RosPlayer::POS_C,    RosPlayer::POS_G,
                           RosPlayer::RATING_HA,
                           PBR_PCT(OL_INDEX, RosPlayer::RATING_HA, 80)
                        },
                        {  RosPlayer::POS_DE,   RosPlayer::POS_DT,
                           RosPlayer::RATING_SP,
                           PBR_PCT(DL_INDEX, RosPlayer::RATING_SP, 50)
                        },
                        {  RosPlayer::POS_CB,   RosPlayer::POS_S,
                           RosPlayer::RATING_ST,
                           PBR_PCT(DB_INDEX, RosPlayer::RATING_ST, 65)
                        },
                     };

static BonusPBR   FAR bonusPBR [] =
                  {
   {  RosPlayer::RATING_IN,   RosPlayer::RATING_HA,   RosPlayer::RATING_DI   }, // qb
   {  RosPlayer::RATING_ST,   RosPlayer::RATING_AC,   RosPlayer::RATING_AG   }, // fb
   {  RosPlayer::RATING_HA,   RosPlayer::RATING_SP,   RosPlayer::RATING_AG   }, // hb
   {  RosPlayer::RATING_ST,   RosPlayer::RATING_HA,   RosPlayer::RATING_AG   }, // te
   {  RosPlayer::RATING_SP,   RosPlayer::RATING_HA,   RosPlayer::RATING_AC   }, // wr
   {  RosPlayer::RATING_HA,   RosPlayer::RATING_ST,   RosPlayer::RATING_EN   }, // c
   {  RosPlayer::RATING_ST,   RosPlayer::RATING_EN,   RosPlayer::RATING_AC   }, // g
   {  RosPlayer::RATING_ST,   RosPlayer::RATING_EN,   RosPlayer::RATING_AC   }, // t
   {  RosPlayer::RATING_AC,   RosPlayer::RATING_ST,   RosPlayer::RATING_SP   }, // de
   {  RosPlayer::RATING_ST,   RosPlayer::RATING_EN,   RosPlayer::RATING_AG   }, // dt
   {  RosPlayer::RATING_SP,   RosPlayer::RATING_AG,   RosPlayer::RATING_IN   }, // lb
   {  RosPlayer::RATING_ST,   RosPlayer::RATING_SP,   RosPlayer::RATING_AC   }, // cb
   {  RosPlayer::RATING_SP,   RosPlayer::RATING_HA,   RosPlayer::RATING_AG   }, // s
   {  RosPlayer::RATING_ST,   RosPlayer::RATING_HA,   RosPlayer::RATING_AC   }, // k
   {  RosPlayer::RATING_ST,   RosPlayer::RATING_HA,   RosPlayer::RATING_AC   }  // p
                  };

#endif

#if   (LCDT == 0)

static RetireMult FAR retMult [] =
                  {
                     { 3,  2 },    // 3 *  2 =  6 =  3% at 3 yrs
                     { 2,  8 },    // 2 *  8 = 16 =  8 (+  3 =  11% at  5 yrs)
                     { 2, 20 },    // 2 * 20 = 40 = 20 (+ 11 =  31% at  7 yrs)
                     { 3, 24 },    // 3 * 24 = 72 = 36 (+ 31 =  67% at 10 yrs)
                     { 2, 14 },    // 2 * 14 = 28 = 14 (+ 67 =  81% at 12 yrs)
                     { 3, 12 }     // 3 * 13 = 39 = 19 (+ 81 = 100% at 15 yrs)
                                   // (guaranteed retirement after 15 years)
                  };
#define  NUM_RET_MULT   (sizeof (retMult) / sizeof (RetireMult))

#endif   // LCDT

#if   PRINT_PLAYERS_ARR
static DYN_8   *posText [] =
               {
                  "ROSPLAYERPOS_QB",
                  "ROSPLAYERPOS_FB", "ROSPLAYERPOS_HB",
                  "ROSPLAYERPOS_TE", "ROSPLAYERPOS_WR",
                  "ROSPLAYERPOS_C",  "ROSPLAYERPOS_G", "ROSPLAYERPOS_T",
                  "ROSPLAYERPOS_DE", "ROSPLAYERPOS_DT",
                  "ROSPLAYERPOS_LB",
                  "ROSPLAYERPOS_CB", "ROSPLAYERPOS_S",
                  "ROSPLAYERPOS_K",
                  "ROSPLAYERPOS_P",
                  "ROSPLAYERPOS_RB",
                  "ROSPLAYERPOS_REC",
                  "ROSPLAYERPOS_OL",
                  "ROSPLAYERPOS_DL",
                  "ROSPLAYERPOS_DB",
                  "Error!",
               };
#endif

#if   DEBUG_COUNT
static DYN_16 numCons, numDest;
#endif   // LCDT

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/*
   ---------------------------------------------------------------------
   NAME:          RosPlayer

   FUNCTION:      RosPlayer object CONSTRUCTOR

   INPUT:         fp -> vm_bfile with data to extract
                  pId = id of player to find in file
                  (if no parameters, create a blank player)

   RETURNS:       -> created RosPlayer object
   ---------------------------------------------------------------------
*/

RosPlayer::RosPlayer (PlayerRdWr &prw, DYN_16 pId)
{
   dataPtr = new RosPlayerData;

   if (pId >= PlayerRdWr::BASE_PLAYER_ID)
      Load (prw, pId);

   else
      dataPtr->posGen = dataPtr->posSpec = POS_NONE;

// debugPrintf ("new: %p, %d, %s, %s\n", this, dataPtr->id, dataPtr->firstName, dataPtr->lastName);
}

RosPlayer::RosPlayer ()
{
   dataPtr = new RosPlayerData;

   dataPtr->posGen = dataPtr->posSpec = POS_NONE;
// debugPrintf ("empty: %p, %d, %s, %s\n", this, dataPtr->id, dataPtr->firstName, dataPtr->lastName);
}

RosPlayer::RosPlayer (RosPlayer *pPtr)
{
   dataPtr = new RosPlayerData;

   Copy (pPtr);

// debugPrintf ("copy: %p, %d, %s, %s\n", this, dataPtr->id, dataPtr->firstName, dataPtr->lastName);
}

/*
   ---------------------------------------------------------------------
   NAME:          ~RosPlayer

   FUNCTION:      RosPlayer object DESTRUCTOR

   INPUT:         -> RosPlayer object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

RosPlayer::~RosPlayer ()
{
// debugPrintf ("delete: %p, %d, %s, %s\n", this, dataPtr->id, dataPtr->firstName, dataPtr->lastName);
   delete dataPtr;
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

//#pragma  argsused
#if DBGPOOL
DYN_U_16	cLastVal = 0;
BOOL		fGoingUp = FALSE;
#endif

void FAR *
RosPlayer::operator new (size_t size)
{
   #if   DEBUG_COUNT
   numCons++;
   monoDump ();
   #endif

   // make sure this isn't some derived class
   vmAssert (pa && (size == sizeof (RosPlayer)));
   void  FAR *t = pa->New ();
#if DBGPOOL
   if (fGoingUp == FALSE)
   {
		char szDBG[100];
		sprintf(szDBG, "DELETE: %d out of %d\n", cLastVal, pa->numNodes);
		OutputDebugString(szDBG);
		fGoingUp = TRUE;
   }
   cLastVal = pa->numAlloc;
#endif
  vmAssert (t);

   memset (t, 0, sizeof (RosPlayer));
   return t;
}

void
RosPlayer::operator delete (void FAR *t, size_t size)
{
   #if   DEBUG_COUNT
   numDest++;
   monoDump ();
   #endif

   // make sure this isn't some derived class
   vmAssert (pa && (size == sizeof (RosPlayer)));
   pa->Delete (t);
#if DBGPOOL
   if (fGoingUp == TRUE)
   {
		char szDBG[100];
		sprintf(szDBG, "NEW:    %d out of %d\n", cLastVal, pa->numNodes);
		OutputDebugString(szDBG);
		fGoingUp = FALSE;
   }
   cLastVal = pa->numAlloc;
#endif
}



void FAR *
RosPlayerData::operator new (size_t size)
{
   // if there is no allocation object, or the size if wrong (indicating
   // a derived class), allocate out of conventional memory
   // only process items that are of the expected size
   vmAssert (size == sizeof (RosPlayerData));

   // try all pools before giving up
   void  FAR *t = pa->New ();
   vmAssert (t);

   // return a 'clean slate' to the constructor
   memset (t, 0, sizeof (RosPlayerData));
   return t;
}

void
RosPlayerData::operator delete (void FAR *t, size_t size)
{
   // only process items that are of the expected size
   vmAssert (size == sizeof (RosPlayerData));
   pa->Delete (t);
}

#if   DEBUG_VERIFY_NODES
void
RosPlayerData::VerifyNodes ()
{
   pa->VerifyNodes ();
}
#endif






/*
   =====================================================================
                           THE METHODS
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       Init

   FUNCTION:   initialize the pointer that operators new & delete
               use for allocating Player object memory
               initialize pointer for PlayerData memory management

   INPUT:      none

   RETURNS:    PoolAlloc object for Player created
               PoolAlloc object for PlayerData created
   ---------------------------------------------------------------------
*/

void
RosPlayer::Init ()
{
   // allocate memory for a player (in near memory)
   pa = new PoolAlloc (sizeof (RosPlayer), NUM_ALLOC);
   vmAssert (pa);

   RosPlayerData::pa = new PoolAlloc (sizeof (RosPlayerData), NUM_ALLOC);
   vmAssert (RosPlayerData::pa);
}
void
RosPlayer::DeInit ()
{
   // allocate memory for a player (in near memory)
   delete pa;

   delete RosPlayerData::pa;
}

#if   (LCDT == 0) || (LCDT == 2) || (LCDT == 4)
/*
   ---------------------------------------------------------------------
   NAME:       Save

   FUNCTION:   write a player to disk
               (but only if the data has changed)
               id                sizeof (DYN_16)
               injuryInfo, Pts   2 * sizeof (DYN_16)
               pbrs, abrs        2 * sizeof (PLAYER_RATINGS)
               jersey            sizeof (DYN_8)
               years             sizeof (DYN_8)
               posGen, Spec      2 * sizeof (DYN_8)
               firstName         strlen (firstName)
               lastName          strlen (lastName)

   INPUT:      fp-> FILE object already opened for writing to
               RosPlayer object

   RETURNS:    file updated
   ---------------------------------------------------------------------
*/

void
RosPlayer::Save (PlayerRdWr &prw)
{
   if (dataPtr->modified)
   {
      dataPtr->modified = FALSE;
      if (dataPtr->id >= PlayerRdWr::BASE_PLAYER_ID)
      {
         prw.SetData (dataPtr);
         vmAssert (prw.Write ());
      }
   }
}
#endif

/*
   ---------------------------------------------------------------------
   NAME:       Get/SetData

   FUNCTION:   return or set some value in RosPlayer object
               (can return a particular rating if 2nd parameter is used)

   INPUT:      new??? value of data to set

   RETURNS:    data wanted
   ---------------------------------------------------------------------
*/

DYN_16
RosPlayer::GetId () const
{
   return dataPtr->id;
}

void
RosPlayer::SetId (const DYN_16 newValue)
{
   dataPtr->id = newValue;
   dataPtr->modified = TRUE;
}

DYN_16
RosPlayer::GetInjPts () const
{
   return dataPtr->injury.points;
}

void
RosPlayer::SetInjPts (const DYN_16 newValue)
{
   dataPtr->injury.points = newValue;
   dataPtr->modified = TRUE;
}

DYN_16
RosPlayer::GetInjType () const
{
   return dataPtr->injury.type;
}

void
RosPlayer::SetInjType (const DYN_16 newValue)
{
   dataPtr->injury.type = (DYN_8)newValue;
   dataPtr->modified = TRUE;
}

DYN_16
RosPlayer::GetInjWeek () const
{
   return dataPtr->injury.week;
}

void
RosPlayer::SetInjWeek (const DYN_16 newValue)
{
   dataPtr->injury.week = (DYN_8)newValue;
   dataPtr->modified = TRUE;
}

RosPlayer::Pos
RosPlayer::GetPosGen () const
{
   if (!this)
      return POS_NONE;

   return dataPtr->posGen;
}

void
RosPlayer::SetPosGen (const Pos newValue)
{
   dataPtr->posGen = newValue;
   dataPtr->modified = TRUE;
}

RosPlayer::Pos
RosPlayer::GetPosSpec () const
{
   if (!this)
      return POS_NONE;

   return dataPtr->posSpec;
}

void
RosPlayer::SetPosSpec (const Pos newValue)
{
   dataPtr->posSpec = newValue;
   dataPtr->modified = TRUE;
}

DYN_16
RosPlayer::GetYears () const
{
   return dataPtr->years;
}

void
RosPlayer::SetYears (const DYN_16 newValue)
{
   dataPtr->years = (DYN_8)newValue;
   dataPtr->modified = TRUE;
}

DYN_16
RosPlayer::GetStatus () const
{
   if (!this)
      return -1;

   return dataPtr->status;
}

void
RosPlayer::SetStatus (const DYN_16 newValue)
{
   dataPtr->status = (DYN_8)newValue;
}

DYN_16
RosPlayer::GetPBR (const Rating prData) const
{
   return GetBR (prData, &dataPtr->pbr);
}

void
RosPlayer::SetPBR (const Rating prData, const DYN_16 newValue)
{
   SetBR (prData, &dataPtr->pbr, newValue);
}

DYN_16
RosPlayer::GetABR (const Rating prData) const
{
   return GetBR (prData, &dataPtr->abr);
}

void
RosPlayer::SetABR (const Rating prData, const DYN_16 newValue)
{
   SetBR (prData, &dataPtr->abr, newValue);
}

DYN_8 FAR *
RosPlayer::GetFirstName () const
{
   return dataPtr->firstName;
}

void
RosPlayer::SetFirstName (const DYN_8 FAR *name)
{
   strncpy (dataPtr->firstName, name, PlayerRdWr::NAME_LEN);
   dataPtr->modified = TRUE;
}

DYN_8 FAR *
RosPlayer::GetLastName () const
{
   return dataPtr->lastName;
}

void
RosPlayer::SetLastName (const DYN_8 FAR *name)
{
   strncpy (dataPtr->lastName, name, PlayerRdWr::NAME_LEN);
   dataPtr->modified = TRUE;
}

#if   (LCDT == 0)
/*
   ---------------------------------------------------------------------
   NAME:       Get/SetProBowlId

   FUNCTION:   if user wants to play a pro bowl game,
               the teams are picked from the available players
               if no stats are loaded, the sort values (ids) are set
               by the player's abrs

   INPUT:      player to set id for

   RETURNS:    id = sum of some key ratings
   ---------------------------------------------------------------------
*/

double
RosPlayer::GetProBowlId () const
{

   return dataPtr->proBowlValue;
}

void
RosPlayer::SetProBowlId (const double value)
{

   dataPtr->proBowlValue = value;
}
#endif

/*
   ---------------------------------------------------------------------
   NAME:       SumA/Pbrs

   FUNCTION:   sum all the player's ratings (using key rating mulitplier
               if wanted)

   INPUT:      rwPtr-> weighting mulitplier

   RETURNS:    all the ratings added together
   ---------------------------------------------------------------------
*/

DYN_16
RosPlayer::SumAbrs (RatingWeights *rwPtr)
{
   return SumBrs ((DYN_8 FAR *) &dataPtr->abr, rwPtr);
}




DYN_16 RosPlayer::SumAbrs ( RosPlayer *p2, RosPlayer *p3 )
{
	DYN_16 iSum = SumAbrs( ratWeights );

	if ( p2 != NULL )
	{
		iSum += p2->SumAbrs( ratWeights );
	}
	if ( p3 != NULL )
	{
		iSum += p3->SumAbrs( ratWeights );
	}

	return( iSum );
}




DYN_16
RosPlayer::SumPbrs (RatingWeights *rwPtr)
{
   return SumBrs ((DYN_8 FAR *) &dataPtr->pbr, rwPtr);
}

/*
   ---------------------------------------------------------------------
   NAME:       SumInjAbrs

   FUNCTION:   sum all the player's ratings (using key rating mulitplier
               if wanted) and take into account injuries:
               Status         Mulitplier (/64)
               HEALTHY        64 (duh)
               PROBABLE       56
               QUESTION       38
               DOUBT          19
               OUT             0 (also duh)

   INPUT:      rwPtr-> weighting mulitplier

   RETURNS:    all the ratings added together,
               and multiplied by injury factor
   ---------------------------------------------------------------------
*/

DYN_16
RosPlayer::SumInjAbrs (RatingWeights *rwPtr)
{
   DYN_16   abrs = SumBrs ((DYN_8 FAR *) &dataPtr->abr, rwPtr),
            injPts = dataPtr->injury.points;

   if (injPts)
   {
      if (injPts > INJURY_OUT)
         abrs = 0;
      else
      {
         DYN_16   factor;
         if (injPts > INJURY_DOUBT)
            factor = 19;
         else if (injPts > INJURY_QUESTION)
            factor = 38;
         else
            factor = 56;
         abrs = ((DYN_32) abrs * factor) / 64;
      }
   }

   return abrs;
}

/*
   ---------------------------------------------------------------------
   NAME:       Generate

   FUNCTION:   generate pbrs & abrs for a player of the given position

   INPUT:      pos = player's GENERAL (or SPECIFIC) position
               newid = player's identification
               firstName, lastName-> player's name
               blessed = GENERAL position that gets a special bonus

   RETURNS:    pbrs, abrs & abrs
               name
               specific position
   ---------------------------------------------------------------------
*/

#if   (LCDT == 0) || (LCDT == 4)

void
RosPlayer::Generate (const Pos pos, const DYN_16 newId,
                           const DYN_8 FAR *fnPtr, const DYN_8 FAR *lnPtr,
                           const Pos blessed)
{
   Pos         posGen;
   GenToIndex  tableIndex;
   switch (pos)
   {
      case POS_QB:
         posGen = POS_QB;
         tableIndex = QB_INDEX;
         break;

      case POS_RB:
      case POS_FB:
      case POS_HB:
         posGen = POS_RB;
         tableIndex = RB_INDEX;
         break;

      case POS_REC:
      case POS_TE:
      case POS_WR:
         posGen = POS_REC;
         tableIndex = REC_INDEX;
         break;

      case POS_OL:
      case POS_C:
      case POS_G:
      case POS_T:
         posGen = POS_OL;
         tableIndex = OL_INDEX;
         break;

      case POS_DL:
      case POS_DE:
      case POS_DT:
         posGen = POS_DL;
         tableIndex = DL_INDEX;
         break;

      case POS_LB:
         posGen = POS_LB;
         tableIndex = LB_INDEX;
         break;

      case POS_DB:
      case POS_CB:
      case POS_S:
         posGen = POS_DB;
         tableIndex = DB_INDEX;
         break;

      case POS_K:
         posGen = POS_K;
         tableIndex = PK_INDEX;
         break;

      case POS_P:
         posGen = POS_P;
         tableIndex = PK_INDEX;
         break;
   }

   dataPtr->id = newId;
   dataPtr->posGen = posGen;
   strncpy (dataPtr->firstName, fnPtr, PlayerRdWr::NAME_LEN + 1);
   strncpy (dataPtr->lastName, lnPtr, PlayerRdWr::NAME_LEN + 1);

   #if DEBUG_SUPER
   if (generatePBRs (&dataPtr->pbr, pbrRangeTable [tableIndex]))
   {
      DYN_8 newName [PlayerRdWr::NAME_LEN + 1];

      strncpy (newName, "SS ", PlayerRdWr::NAME_LEN);
      strncat (newName, lnPtr, PlayerRdWr::NAME_LEN);

      strncpy (dataPtr->lastName, newName, PlayerRdWr::NAME_LEN);
      dataPtr->lastName [PlayerRdWr::NAME_LEN] = 0;
   }
   #else

   generatePBRs (&dataPtr->pbr, pbrRangeTable [tableIndex]);
   #endif

   generateABRs (&dataPtr->abr, &dataPtr->pbr, abrRangeTable [tableIndex]);

   // if user is controlling special position, so be it
   if (pos < POS_MAX_SPEC)
      dataPtr->posSpec = pos;
   else
      dataPtr->posSpec = generateSpecPos (posGen, (DYN_8 FAR *) &dataPtr->pbr);
   if (pos == blessed)
      blessPBRs (dataPtr->posSpec, (DYN_8 FAR *) &dataPtr->pbr);

   SuperStar ();

   dataPtr->modified = TRUE;
}
#endif

#if   (LCDT == 0)
/*
   ---------------------------------------------------------------------
   NAME:       NewPlayer

   FUNCTION:   When a new league is created, teams are copied from those
               that already exist. That means the players are copied too.
               These copied players must be reset to their 'pristine' values.

   INPUT:      newid = player's id in the new league

   RETURNS:    new player id
               players abrs reset
               injury status set to no injuries
   ---------------------------------------------------------------------
*/

void
RosPlayer::NewPlayer (const DYN_16 newId)
{
   dataPtr->id = newId;
   SeasonReset (TRUE);
   dataPtr->modified = TRUE;
}
#endif

#if   (LCDT == 0) || (LCDT == 4)
/*
   ---------------------------------------------------------------------
   NAME:       SeasonReset

   FUNCTION:   At the beginning of a new season, a player's ratings and
               injuries are reset to their starting values (they are
               then altered by aging and training camp)
               CEI players aren't healed

   INPUT:      force = MUST heal (even if player is CEI)
               -> RosPlayer object

   RETURNS:    players abrs reset
               injury status set to no injuries
   ---------------------------------------------------------------------
*/

void
RosPlayer::SeasonReset (const DYN_16 force)
{
   // CEI players in CONTINUING leagues, don't heal
   if (force || (dataPtr->injury.points != INJURY_OVER))
   {
      dataPtr->injury.points = INJURY_HEALTHY;
      dataPtr->injury.type = dataPtr->injury.week = 0;
      dataPtr->modified = TRUE;
   }
}
#endif

#if   (LCDT == 0) || (LCDT == 2)
/*
   ---------------------------------------------------------------------
   NAME:       Age

   FUNCTION:   at the start of each season, every body gets a little
               older, and their skills deteriorate after 6 years

   INPUT:      player object

   RETURNS:    player has 1 more year of experience, with
               possible degradation of skills
   ---------------------------------------------------------------------
*/

void
RosPlayer::Age ()
{
   if (++dataPtr->years > (AGE_EFFECT_YEARS + 1))
      ageBRs (dataPtr->posGen,
               dataPtr->years,
               (DYN_8 FAR *) &dataPtr->pbr,
               (DYN_8 FAR *) &dataPtr->abr);

   dataPtr->modified = TRUE;
}
#endif

#if   (LCDT == 0)
/*
   ---------------------------------------------------------------------
   NAME:       Retire

   FUNCTION:   determine if this player is going to call it quits
               for number of years in the league, add in a certain number
               of percentage points:
               number of years in league:
                1 -  3     1%
                4 -  5     4%
                6 -  7    10%
                8 - 10    12%
               11 - 12     7%
               13 - 15     6%

               so a player in the league 11 years has the following:
               (3 * 1) + (2 * 4) + (2 * 10) + (3 * 12) + (1 * 7) = 77% chanch
               automatic retirement occurs after 23 seasons (100% chanch)

               GJW: 9-14-1992, also retire if Career Ending Injury
               GJW: 9-23-1993, rosters handle their own retirement, so
                     depth is no longer needed, this is only called by
                     the free-agent list

   INPUT:      depth = depth chart (starter = 0, so retire at 1/2
                        the normal rate, except for the last 2 sets (the
                        last 5 years) where the retire at a rate that
                        catches them up)

   RETURNS:    TRUE/FALSE
   ---------------------------------------------------------------------
*/

DYN_16
RosPlayer::Retire ()
{
   DYN_16      retire = FALSE;
   DYN_8       yrs = dataPtr->years;

   if (dataPtr->injury.points == INJURY_OVER)
      retire = TRUE;

   else if (yrs)
   {
      RetireMult  FAR *rmPtr = retMult;
      DYN_16      chanch = 0;

      while (yrs)
      {
         DYN_8    yearMult = getYearMult (yrs, rmPtr->numYears),
                  pct = rmPtr->percent;

         chanch += yearMult * pct;
         yrs -= yearMult;
         rmPtr++;
      }

      if ((getPosRand () % 200) < chanch)
         retire = TRUE;
   }

   return retire;
}

/*
   ---------------------------------------------------------------------
   NAME:       Train

   FUNCTION:   all the abrs go up some percentage, based on the percent
               of time allocated to that rating, and how close the player
               is to being at 100% potential

               (abr * 100) / pbr    pct increase per 5% training time
                0 - 65              2.0
               66 - 70              1.5
               71 - 75              1.25
               76 - 80              1.0
               81 - 85               .75
               86 - 90               .5
               91 - 95               .25
               96 - 99               .15

               if no time has been allocated, players go down 2%
               GJW: 8-18-1993, loses by 15%

   INPUT:      prPtr-> percentages of time spent in training camp

   RETURNS:    players abrs improved
   ---------------------------------------------------------------------
*/

void
RosPlayer::Train (Ratings *prPtr)
{
   DYN_8  FAR *pbrPtr,
            FAR *abrPtr,
            FAR *pctPtr,
            i = RATING_MAX;

   pbrPtr = (DYN_8 FAR *) &dataPtr->pbr;
   abrPtr = (DYN_8 FAR *) &dataPtr->abr;
   pctPtr = (DYN_8 FAR *) prPtr;
   while (i--)
   {
      // first get players pbr %age
      DYN_8    pbr = *pbrPtr,
               pct = *pctPtr;
      if (!pbr)
         pbr = 1;
      DYN_32   pbrPct = ((DYN_U_8)(*abrPtr) * 1000L) / (DYN_U_16) pbr;
      DYN_16   pctIncrease;

      // if training time allocated, player gets better
      if (pct)
      {
         // determine increase per 5% of training time
         if (pbrPct < 660)
            pctIncrease = 20;
         else if (pbrPct < 710)
            pctIncrease = 15;
         else if (pbrPct < 760)
            pctIncrease = 13;
         else if (pbrPct < 810)
            pctIncrease = 10;
         else if (pbrPct < 860)
            pctIncrease = 8;
         else if (pbrPct < 910)
            pctIncrease = 5;
         else if (pbrPct < 960)
            pctIncrease = 3;
         else
            pctIncrease = 2;

         // determine number of 5% units allocated
         pctIncrease *= pct / 5;
      }

      // otherwise he gets worse by 15%
      else
         pctIncrease = -150;

      // adjust rating based on percent increase/decrease
      pbrPct += pctIncrease;
      DYN_8    newAbr = (DYN_8)(((DYN_32) pbr * pbrPct) / 1000L);

      if (newAbr > pbr)
         newAbr = pbr;
      else if (newAbr < 0)
         newAbr = 0;

      *abrPtr = newAbr;

      // move on to next rating
      pbrPtr++;
      abrPtr++;
      pctPtr++;
   }

   dataPtr->modified = TRUE;
}
#endif   // LCDT

#if   (LCDT == 0) || (LCDT == 2) || (LCDT == 4)
/*
   ---------------------------------------------------------------------
   NAME:       ModRatings

   FUNCTION:   player takes a pct hit or enhancement on their pbrs & abrs

   INPUT:      pbrPct = percentage of pbr ratings left after hit
               abrPct = percentage of abr ratings left after hit

   RETURNS:    each abr and pbr mulitplied to pct / 100
   ---------------------------------------------------------------------
*/

void
RosPlayer::ModRatings (const DYN_16 pbrPct, const DYN_16 abrPct)
{
   // first pbrs
   DYN_8    FAR *pbrPtr = (DYN_8 FAR *) &dataPtr->pbr,
            FAR *abrPtr = (DYN_8 FAR *) &dataPtr->abr,
            ratCount = RATING_MAX;

   while (ratCount--)
   {
      DYN_8    pbr = *pbrPtr,
               abr = *abrPtr;

      pbr = ((DYN_U_16) (pbr * pbrPct)) / 100;
      if (pbr > MAX_RATING_POINTS)
         pbr = MAX_RATING_POINTS;

      abr = ((DYN_U_16) (abr * abrPct)) / 100;
      if (abr > MAX_RATING_POINTS)
         abr = MAX_RATING_POINTS;

      // if actual exceed potential, increase potential
      if (abr > pbr)
         pbr = abr;

      *pbrPtr++ = pbr;
      *abrPtr++ = abr;
   }

   dataPtr->modified = TRUE;
}
#endif   // LCDT


/*
   ---------------------------------------------------------------------
   NAME:       Copy

   FUNCTION:   duplicate the info of another player into this one

   INPUT:      pPtr-> source info

   RETURNS:    this' info obtained from pPtr
   ---------------------------------------------------------------------
*/

void
RosPlayer::Copy (RosPlayer *pPtr)
{
   memcpy (dataPtr, pPtr->dataPtr, sizeof (RosPlayerData));
}

/*
   ---------------------------------------------------------------------
   NAME:       Compare

   FUNCTION:   determine if the 2 players are the same

   INPUT:      pPtr-> another player

   RETURNS:    TRUE/FALSE if players are the same
   ---------------------------------------------------------------------
*/

DYN_16
RosPlayer::Compare (RosPlayer *pPtr)
{
   return !memcmp (dataPtr, pPtr->dataPtr, sizeof (RosPlayerData));
}

/*
   ---------------------------------------------------------------------
   NAME:       WeeksOut

   FUNCTION:   how many weeks is the player expected to miss

   INPUT:      injury points,
               endurance

   RETURNS:    injury points / endurance
   ---------------------------------------------------------------------
*/

DYN_16
RosPlayer::WeeksOut ()
{
   DYN_16   out = 0;

   if (dataPtr->injury.points)
   {
      DYN_16   en = dataPtr->abr.ratings [RATING_EN];

      if (!en)
         en = 1;
      else
      {
         en *= 3;
         en /= 2;
      }
      out = 1 + (dataPtr->injury.points / en);
   }

   return out;
}

/*
   ---------------------------------------------------------------------
   NAME:       IREligible

   FUNCTION:   determine if player can go onto IR

   INPUT:      if POS == NONE yes (an empty player)
               otherwise WeeksOut () >= 6

   RETURNS:    TRUE/FALSE if player is ok to go on IR
   ---------------------------------------------------------------------
*/

DYN_16
RosPlayer::IREligible ()
{
   // if going on, his injury must place him out for at least 6 weeks
   return (dataPtr->posGen == POS_NONE) || (WeeksOut () >= MIN_IR_WEEKS);
}

/*
   ---------------------------------------------------------------------
   NAME:       Heal

   FUNCTION:   player gets back some injury points for this weeks game

   INPUT:      irFlag = player on injured reserve indicator (heals a little
               faster)

               // GJW: 9-14-1992, can't heal if out forever (Career
               Ending Injury)

   RETURNS:    abrs inched upward based on endurance
   ---------------------------------------------------------------------
*/

void
RosPlayer::Heal (DYN_16 irFlag)
{
   DYN_16   points = dataPtr->injury.points;
   if (points && (points != INJURY_OVER))
   {
      DYN_16   en = dataPtr->abr.ratings [RATING_EN],
               healPoints;

      // player heals based on endurance attribute
      if (en <= 30)
         healPoints = 30;
      else if (en <= 50)
         healPoints = 60;
      else if (en <= 70)
         healPoints = 90;
      else if (en <= 90)
         healPoints = 120;
      else
         healPoints = 150;

      if (irFlag)
         healPoints += 15;

      // can't heal to better than '0'
      if (healPoints > points)
         healPoints = points;

      // HEAL THYSELF!
      dataPtr->injury.points -= healPoints;
      dataPtr->modified = TRUE;
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       Injury

   FUNCTION:   player has accumulated some injury points for the
               game just played

   INPUT:      weekNum = when did injury occur
               points = severity of injury

   RETURNS:    player's injury.points, type & week set
               (if points is non-0, then the player just re-injures
               the same spot)
   ---------------------------------------------------------------------
*/

void
RosPlayer::Injury (DYN_16 weekNum, DYN_16 points)
{
   // first time player being injured?
   // (or injury is career ending)
   if (!dataPtr->injury.points || (points == INJURY_OVER))
   {
      dataPtr->injury.week = (DYN_8)weekNum;

      DYN_8 location = getRandPercent (),
            type;

      // ankle
      if (location <= 17)
      {
         if (points <= 75)
            type = INJURY_BRUISED_ANKLE;
         else if (points <= 300)
            type = INJURY_STRAINED_ANKLE;
         else if (points <= 525)
            type = INJURY_SPRAINED_ANKLE;
         else
            type = INJURY_BROKEN_ANKLE;
      }

      // arm/hand
      else if (location <= 24)
      {
         if (points <= 115)
            type = INJURY_BRUISED_ARM;
         else if (points <= 450)
            type = INJURY_STRAINED_WRIST;
         else if (points <= 600)
            type = INJURY_BROKEN_FINGER;
         else
            type = INJURY_BROKEN_HAND;
      }

      // back
      else if (location <= 28)
      {
         if (points <= 225)
            type = INJURY_SORE_BACK;
         else if (points <= 450)
            type = INJURY_STRAINED_BACK;
         else
            type = INJURY_RUPTURED_DISK;
      }

      // chest/ribs
      else if (location <= 32)
      {
         if (points <= 300)
            type = INJURY_STRAINED_CHEST;
         else if (points <= 500)
            type = INJURY_FRACTURED_RIB;
         else
            type = INJURY_DISLOCATED_RIB;
      }

      // thigh
      else if (location <= 51)
      {
         if (points <= 60)
            type = INJURY_BRUISED_THIGH;
         else if (points <= 415)
            type = INJURY_STRAINED_HAMSTRING;
         else
            type = INJURY_FRACTURED_FEMUR;
      }

      // hip
      else if (location <= 56)
      {
         if (points <= 360)
            type = INJURY_STRAINED_HIP;
         else if (points <= 600)
            type = INJURY_HIP_FRACTURE;
         else
            type = INJURY_DISLOCATED_HIP;
      }

      // knee
      else if (location <= 82)
      {
         if (points <= 105)
            type = INJURY_SORE_KNEE;
         else if (points <= 500)
            type = INJURY_BRUISED_KNEE;
         else
            type = INJURY_KNEE_CARTILAGE;
      }

      // leg
      else if (location <= 88)
      {
         if (points <= 115)
            type = INJURY_BRUISED_CALF;
         else if (points <= 325)
            type = INJURY_STRAINED_CALF;
         else
            type = INJURY_BROKEN_LEG;
      }

      // neck
      else if (location <= 91)
      {
         if (points <= 120)
            type = INJURY_SORE_NECK;
         else if (points <= 450)
            type = INJURY_NECK_STRAIN;
         else if (points <= 600)
            type = INJURY_NECK_SPRAIN;
         else
            type = INJURY_DISLOCATED_DISK;
      }

      // shoulder
      else 
      {
         if (points <= 240)
            type = INJURY_BRUISED_SHOULDER;
         else if (points <= 360)
            type = INJURY_STRAINED_SHOULDER;
         else
            type = INJURY_DISLOCATED_SHOULDER;
      }

      dataPtr->injury.type = type;
   }

   points += dataPtr->injury.points;
   if (points > INJURY_OVER)
      points = INJURY_OVER;

   dataPtr->injury.points = points;
   dataPtr->modified = TRUE;
}

/*
   ---------------------------------------------------------------------
   NAME:       MakeNew

   FUNCTION:   if a player is retiring from a team,
               rather than deleting, and allocating a new one,
               just have this player go to 0

   INPUT:      this-> player to set to a null player

   RETURNS:    this player's data set to 0
   ---------------------------------------------------------------------
*/

void
RosPlayer::MakeNew ()
{
   memset (dataPtr, 0, sizeof (RosPlayerData));
   dataPtr->posGen = dataPtr->posSpec = POS_NONE;
}

/*
   ---------------------------------------------------------------------
   NAME:       Print

   FUNCTION:   print player's data to printer

   INPUT:      prnPtr -> Printer object to write info to
               flag = .arr/text output desired
               -> RosPlayer object

   RETURNS:    string printed to screen
   ---------------------------------------------------------------------
*/

//#pragma  argsused
void
RosPlayer::Print (Printer *prnPtr, DYN_16 flag)
{
   #if   PRINT_PLAYERS_ARR
   if (!prnPtr->TestJob ())
      prnPtr->InitJob (0);

   if (flag)
   {
      prnPtr->Print ("word\n");
      prnPtr->Print ("%4d                            // id\n",
                        dataPtr->id);
      prnPtr->Print ("byte\n");
      prnPtr->Print ("\"%s\"\n", dataPtr->firstName);
      prnPtr->Print ("\"%s\"\n", dataPtr->lastName);
      printRatings (prnPtr, &dataPtr->pbr);
      prnPtr->Print ("         // pbr's\n");
      prnPtr->Print ("word\n");
      prnPtr->Print ("%4d %4d                       // injury\n",
                        dataPtr->injury.points,
                        (dataPtr->injury.week << 8) | dataPtr->injury.type);
      prnPtr->Print ("byte\n");
      prnPtr->Print ("%s\n", dataPtr->posGen < POS_MAX ?
                              posText [dataPtr->posGen] : posText [POS_MAX]);
      prnPtr->Print ("%s\n", dataPtr->posSpec < POS_MAX_SPEC ?
                              posText [dataPtr->posSpec] : posText [POS_MAX]);
      printRatings (prnPtr, &dataPtr->abr);
      prnPtr->Print ("         // abr's\n");
      prnPtr->Print ("%2d                              // years\n",
                        dataPtr->years);
   }

   else
   {
      prnPtr->Print ("id: %4d\n", dataPtr->id);
      prnPtr->Print ("%s %s\n", dataPtr->firstName, dataPtr->lastName);
      prnPtr->Print ("%s, %s\n", posText [dataPtr->posGen], posText [dataPtr->posSpec]);
      prnPtr->Print ("inj: %4d, %4d\n", dataPtr->injury.points, (dataPtr->injury.week << 8) | dataPtr->injury.type);
      printRatings (prnPtr, &dataPtr->pbr);
      prnPtr->Print ("\n");
      printRatings (prnPtr, &dataPtr->abr);
      prnPtr->Print ("\n");
      prnPtr->Print ("\n");
   }
   #endif   // PRINT_ARR
}






/*
   =====================================================================
                           PRIVATE METHODS
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       Load

   FUNCTION:   read in a RosPlayer's data from file

   INPUT:      RosPlayer object
               -> FILE
               pId = id of player to scan file for

   RETURNS:    player's info loaded
   ---------------------------------------------------------------------
*/

void NEAR
RosPlayer::Load (PlayerRdWr &prw, DYN_16 pId)
{
   vmAssert (prw.Read (pId));
   prw.GetData (dataPtr);

   // GJW: 12-1-1994, an earlier bug may have overwritten a player's
   // position, so make sure general and specific positions match
   // (and are valid)
   DYN_16   mod = FALSE;
   if (dataPtr->posSpec >= POS_MAX_SPEC)
   {
      Pos   p;

      switch (dataPtr->posGen)
      {
         case POS_QB:
         case POS_LB:
         case POS_K:
         case POS_P:
            p = dataPtr->posGen;
            break;

         case POS_RB:
            p = POS_HB;
            break;

         case POS_REC:
            p = POS_WR;
            break;

         case POS_OL:
            p = POS_G;
            break;

         case POS_DL:
            p = POS_DE;
            break;

         case POS_DB:
            p = POS_S;
            break;

         default:
            vmAssert (0);
      }

      dataPtr->posSpec = p;
      mod = TRUE;
   }

   if (dataPtr->posGen >= POS_MAX)
   {
      Pos   p;

      switch (dataPtr->posSpec)
      {
         case POS_QB:
         case POS_LB:
         case POS_K:
         case POS_P:
            p = dataPtr->posSpec;
            break;
               
         case POS_FB:
         case POS_HB:
            p = POS_RB;
            break;

         case POS_TE:
         case POS_WR:
            p = POS_REC;
            break;

         case POS_C:
         case POS_G:
         case POS_T:
            p = POS_OL;
            break;

         case POS_DE:
         case POS_DT:
            p = POS_DL;
            break;

         case POS_CB:
         case POS_S:
            p = POS_DB;
            break;
      }

      dataPtr->posGen = p;
      mod = TRUE;
   }

   // GJW: 10-31-1995
   // quick hack, modify ratings of certain positions
   #if   0   
   if (dataPtr->posGen == POS_OL)
   {
      SetABR (RATING_ST, (dataPtr->abr.ratings [RATING_ST] * 107) / 100);
      SetABR (RATING_AG, (dataPtr->abr.ratings [RATING_AG] * 107) / 100);
      mod = TRUE;
   }

   else if (dataPtr->posGen == POS_DL)
   {
      SetABR (RATING_ST, (dataPtr->abr.ratings [RATING_ST] * 96) / 100);
      SetABR (RATING_AG, (dataPtr->abr.ratings [RATING_AG] * 96) / 100);
      mod = TRUE;
   }

   else if (dataPtr->posGen == POS_QB)
   {
      SetABR (RATING_AG, (dataPtr->abr.ratings [RATING_AG] * 105) / 100);
      SetABR (RATING_SP, (dataPtr->abr.ratings [RATING_SP] * 105) / 100);
      SetABR (RATING_DI, (dataPtr->abr.ratings [RATING_DI] * 95) / 100);
      mod = TRUE;
   }

   else if (dataPtr->posGen == POS_DB)
   {
      SetABR (RATING_HA, (dataPtr->abr.ratings [RATING_HA] * 104) / 100);
      mod = TRUE;
   }
   #endif

   // make any read-time modifications stick
   if (mod)
   {
      prw.SetData (dataPtr);
      prw.Write ();
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       Get/SetBR

   FUNCTION:   return or set a new ABR or PBR value

   INPUT:      prData = which rating to get/set
               prPtr-> abr or pbr set of ratings
               newValue = value to set rating to

   RETURNS:    value
   ---------------------------------------------------------------------
*/

DYN_16 NEAR
RosPlayer::GetBR (const Rating prData, const Ratings *prPtr) const
{
   DYN_16   value = 0;
   if (prData < RATING_MAX)
      value = prPtr->ratings [prData];

   return value;
}

void NEAR
RosPlayer::SetBR (const Rating prData, Ratings * const prPtr, DYN_16 newValue)
{
   Ratings  *abrPtr = &dataPtr->abr,
            *pbrPtr = &dataPtr->pbr;

   if (prData < RATING_MAX)
   {
      if (newValue > MAX_RATING_POINTS)
         newValue = MAX_RATING_POINTS;

      prPtr->ratings [prData] = (DYN_8)newValue;

      // if user makes the player better than potential,
      // set a new potential
      if (abrPtr->ratings [prData] > pbrPtr->ratings [prData])
         pbrPtr->ratings [prData] = abrPtr->ratings [prData];

      dataPtr->modified = TRUE;
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       SumBrs

   FUNCTION:   sum all the player's ratings (using key rating mulitplier
               if wanted)

   INPUT:      prPtr-> set of ratings to sum
               rwPtr-> multiplier table if wanted

   RETURNS:    sum of Abrs or Pbrs
   ---------------------------------------------------------------------
*/

DYN_16 NEAR
RosPlayer::SumBrs (DYN_8 FAR *prPtr, RatingWeights *rwPtr)
{
   // set rwPtr-> ratings mulitplier for THIS position
   // _NONE (empty) players have all ratings == 0 anyway
   if (rwPtr && (dataPtr->posSpec != POS_NONE))
         rwPtr = &rwPtr [dataPtr->posSpec];

   // then do each rating
   DYN_16   sum = 0,
            abrValue = *prPtr++,
            mult;

   if (rwPtr)
      mult = rwPtr->ac;
   else
      mult = 1;
   abrValue *= mult;
   sum += abrValue;

   abrValue = *prPtr++;
//   mult = 0;
   if (rwPtr)
      mult = rwPtr->ag;
   else
      mult = 1;
   abrValue *= mult;
   sum += abrValue;

   abrValue = *prPtr++;
//   mult = 0;
   if (rwPtr)
      mult = rwPtr->di;
   else
      mult = 1;
   abrValue *= mult;
   sum += abrValue;

   abrValue = *prPtr++;
//   mult = 0;
   if (rwPtr)
      mult = rwPtr->en;
   else
      mult = 1;
   abrValue *= mult;
   sum += abrValue;

   abrValue = *prPtr++;
//   mult = 0;
   if (rwPtr)
      mult = rwPtr->ha;
   else
      mult = 1;
   abrValue *= mult;
   sum += abrValue;

   abrValue = *prPtr++;
//   mult = 0;
   if (rwPtr)
      mult = rwPtr->in;
   else
      mult = 1;
   abrValue *= mult;
   sum += abrValue;

   abrValue = *prPtr++;
//   mult = 0;
   if (rwPtr)
      mult = rwPtr->sp;
   else
      mult = 1;
   abrValue *= mult;
   sum += abrValue;

   abrValue = *prPtr;
//   mult = 0;
   if (rwPtr)
      mult = rwPtr->st;
   else
      mult = 1;
   abrValue *= mult;
   sum += abrValue;

   return sum;
}

#if   (LCDT == 0) || (LCDT == 4)
/*
   ---------------------------------------------------------------------
   NAME:       SuperStar

   FUNCTION:   once in a while a player is REALLY good,
               right out of college
               1 in 1000 chanch

   INPUT:      random chanch

   RETURNS:    pbrs blessed 3 times (i.e. up by 15-30 pts, not 5-10)
   ---------------------------------------------------------------------
*/

void NEAR
RosPlayer::SuperStar ()
{
   if ((getPosRand () % SUPERSTAR_CHANCH) == 0)
   {
      // pbrs get a BIG jump
      blessPBRs (dataPtr->posSpec, (DYN_8 FAR *) &dataPtr->pbr);
      blessPBRs (dataPtr->posSpec, (DYN_8 FAR *) &dataPtr->pbr);
      blessPBRs (dataPtr->posSpec, (DYN_8 FAR *) &dataPtr->pbr);
   }
}
#endif






/*
   =====================================================================
                           LOCAL UTILITIES
   =====================================================================
*/

#if   (LCDT == 0) || (LCDT == 4)
/*
   ---------------------------------------------------------------------
   NAME:       generatePBRs

   FUNCTION:   generate a set of Potential Basic Ratings from the
               given tables

   INPUT:      dest-> player's PBR area
               src-> set of pbr indices for each rating
               use the indice for each rating to get a pointer into pbrRange,
               this will give the high and low range that a player may have
               generate a value somewhere in between

   RETURNS:    TRUE if a rating range boost was performed,
               FALSE otherwise.
               player's PBRs set
   ---------------------------------------------------------------------
*/

static DYN_8 NEAR
generatePBRs (RosPlayer::Ratings *pbrPtr, DYN_8 FAR *src)
{
   DYN_16   boost;
   if (getRandPercent() <= INCPBR_CHANCE)
      boost = INCPBR_MAX_BOOST;
   else
      boost = 0;

   DYN_8    FAR *dest = (DYN_8 FAR *) pbrPtr;
   for (DYN_16 ratIndex = 0; ratIndex < sizeof (RosPlayer::Ratings); ratIndex++)
   {
      LowHigh  FAR *lhPtr = &pbrRange[*src++];
      DYN_16   low        = lhPtr->low,
               high       = lhPtr->high;
      if (boost)
      {
         if ((high - low) < INCPBR_MIN_THRESH)
            low += ((low * INCPBR_MIN_BOOST_A) / 100);
         else
            low += ((low * INCPBR_MIN_BOOST_B) / 100);

         high += ((high * boost) / 100);

         if (high > MAX_RATING_POINTS)
            high = MAX_RATING_POINTS;

         if (low >= high)
            low = high - INCPBR_TINY_RANGE;
      }

      DYN_16   value = 10 + low + (getRandPercent () % (high - low));
      if (value > MAX_RATING_POINTS)
         value = MAX_RATING_POINTS;
      *dest++ = (DYN_8)value;
   }

   return (DYN_8)boost;
}

/*
   ---------------------------------------------------------------------
   NAME:       generateABRs

   FUNCTION:   generate a set of Actual Basic Ratings from the
               given tables

   INPUT:      dest-> player's ABR area
               max-> player's PBR area (the best he can be)
               src-> set of indices into abrRange, which is the percentage
               of PBR the player will start out having

   RETURNS:    ABR's set
   ---------------------------------------------------------------------
*/

static void NEAR
generateABRs (RosPlayer::Ratings *abrPtr, RosPlayer::Ratings *maxPtr,
               DYN_8 FAR *src)
{
   DYN_8    FAR *dest = (DYN_8 FAR *) abrPtr,
            FAR *max = (DYN_8 FAR *) maxPtr;

   for (DYN_16 ratIndex = 0; ratIndex < sizeof (RosPlayer::Ratings); ratIndex++)
   {
      LowHigh  FAR *lhPtr = &abrRange [*src++];
      DYN_8    percent = lhPtr->low +
                  (getRandPercent () % (lhPtr->high - lhPtr->low));
      *dest++ = *max++ * percent / 100;
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       generateSpecPos

   FUNCTION:   players have a secondary position based on the attributes
               (some are the same like QB & QB)
               (others are based on some attribute like RB & HB/FB based
                  on strength)

   INPUT:      pos = General position
               prPtr-> this player's ratings

   RETURNS:    specific position
   ---------------------------------------------------------------------
*/

static RosPlayer::Pos NEAR
generateSpecPos (const RosPlayer::Pos pos, const DYN_8 FAR *prPtr)
{
   RosPlayer::Pos posSpec;

   // these players have no specific position
   if ((pos == RosPlayer::POS_QB) ||
         (pos == RosPlayer::POS_LB) ||
         (pos == RosPlayer::POS_K) ||
         (pos == RosPlayer::POS_P))
      posSpec = pos;

   else
   {
      PosSpecific FAR *psPtr;

      // otherwise get a pointer to the 2 positions the player may be, plus
      // what attribute is used, and what the minimum value is
      switch (pos)
      {
         case RosPlayer::POS_RB:
            psPtr = &specifics [0];
            break;

         case RosPlayer::POS_REC:
            psPtr = &specifics [1];
            break;

         case RosPlayer::POS_OL:
            psPtr = &specifics [2];
            break;

         case RosPlayer::POS_DL:
            psPtr = &specifics [3];
            break;

         case RosPlayer::POS_DB:
            psPtr = &specifics [4];
            break;
      }

      // if the player 'makes the grade':
      if (prPtr [psPtr->rating] > psPtr->cutoff)
         posSpec = psPtr->posA;

      else
         posSpec = psPtr->posB;

      // tackles/guards are OLs who aren't Centers, so generate them
      // randomly
      if (posSpec == RosPlayer::POS_G)
      {
         if (Random () & 1)
         posSpec = RosPlayer::POS_T;
      }
   }

   return posSpec;
}

/*
   ---------------------------------------------------------------------
   NAME:       blessPBRs

   FUNCTION:   one position in the draft is selected to be 'blessed',
               that is their PBRs are a little higher than normal

   INPUT:      pos of this player
               prPtr-> original PBRs

   RETURNS:    PBRs upped by 5 - 10 points
   ---------------------------------------------------------------------
*/

static void NEAR
blessPBRs (const RosPlayer::Pos pos, DYN_8 FAR *prPtr)
{
   for (DYN_8 i = 0,
         FAR *bPBRPtr = (DYN_8 FAR *) &bonusPBR [pos];
         i < sizeof (BonusPBR);
         i++, bPBRPtr++)
   {
      DYN_8 bonusValue = (getPosRand () % 6) + 5,  // 5 - 10 extra
            FAR *pbrPtr = &prPtr [*bPBRPtr],
            newPBR = *pbrPtr + bonusValue;

      if (newPBR > MAX_RATING_POINTS)
         newPBR = MAX_RATING_POINTS;

      *pbrPtr = newPBR;
   }
}
#endif

#if   (LCDT == 0) || (LCDT == 2)
/*
   ---------------------------------------------------------------------
   NAME:       ageBRs

   FUNCTION:   when players get older, their skills go down
               implement the following formula on the skills that are aged:
               ac, ag, ha, st, sp, en

               the player_ratings structure looks like this:
               ac, ag, di, en, ha, in, sp, st
               1   1   0   1   1   0   1   1 = bitwise map of aged skills
                                             = 0xdb

               (years - 5) * (posFactor) * (randomFactor) * .2 = value
               value is then rounded to the nearest point
               each position is given a new randomFactor

   INPUT:      pos = player's general position (for posFactor)
               years = time served in league
               pbrs, abrs-> data to modify

   RETURNS:    pbrs,
               abrs updated (first determine the current abr/pbr %age. shrink
               the pbrs, the modify the abrs to maintain the same %age)
   ---------------------------------------------------------------------
*/

static void NEAR
ageBRs (RosPlayer::Pos pos, DYN_8 years, DYN_8 FAR *pbrs, DYN_8 FAR *abrs)
{
   DYN_8    posFactor;

   switch (pos)
   {
      case RosPlayer::POS_K:
         posFactor = AGE_POS_K_FACTOR;
         break;

      case RosPlayer::POS_QB:
      case RosPlayer::POS_OL:
      case RosPlayer::POS_DL:
         posFactor = AGE_POS_QB_FACTOR;
         break;

      case RosPlayer::POS_P:
         posFactor = AGE_POS_P_FACTOR;
         break;

      case RosPlayer::POS_RB:
      case RosPlayer::POS_LB:
         posFactor = AGE_POS_RB_FACTOR;
         break;

      case RosPlayer::POS_REC:
      case RosPlayer::POS_DB:
         posFactor = AGE_POS_REC_FACTOR;
         break;
   }

   DYN_16   baseFactor = ((years - AGE_EFFECT_YEARS) * posFactor * AGE_EFFECT_FACTOR) / 8;
   DYN_8    affectedRatings = (DYN_8)0xdb,
            i = sizeof (RosPlayer::Ratings);

   while (i--)
   {
      if (affectedRatings & 1)
      {
         DYN_8    randomFactor = (getPosRand () % 5) + 1;
         DYN_16   value = (randomFactor * baseFactor);
         value = (value / 100) + ((value % 100) >= 50);

         DYN_8    oldValue = *pbrs;
         if (!oldValue)
            oldValue = 1;

         DYN_U_8  abrPct = (*abrs * 100) / oldValue;
         DYN_8    newValue = oldValue - value;

         if (newValue < 0)
            newValue = 0;

         *pbrs = newValue;
         *abrs = (newValue * abrPct) / 100;
      }

      affectedRatings >>= 1;
      pbrs++;
      abrs++;
   }
}
#endif

#if   (LCDT == 0)
/*
   ---------------------------------------------------------------------
   NAME:       getYearMult

   FUNCTION:   determine how many years are to be mulitplied at the
               current retirement rate

               e.g. if a player has 6 years left, and the current cut off
               is 4, return 4 (he has 6 years so the full block can be
               removed)
               however, the next time this is called, the player will have
               2 years, so if the cut off is again 4, 2 is returned since
               that's all the player has left to give

   INPUT:      yearsLeft = number of years in the league not yet
                  accounted for
               nextCutOff = number of years at the current rate

   RETURNS:    number of years to use at the current rate
   ---------------------------------------------------------------------
*/

static DYN_8 NEAR
getYearMult (const DYN_8 yearsLeft, const DYN_8 nextCutOff)
{
   if (yearsLeft > nextCutOff)
      return nextCutOff;

   else
      return yearsLeft;
}
#endif

#if   PRINT_PLAYERS_ARR
/*
   ---------------------------------------------------------------------
   NAME:       printRatings

   FUNCTION:   print a player's ratings

   INPUT:      prnPtr-> printer object
               prPtr-> group of ratings

   RETURNS:    ratings formated and sent to printer object
   ---------------------------------------------------------------------
*/

static void NEAR
printRatings (Printer *prnPtr, RosPlayer::Ratings *prPtr)
{
   prnPtr->Print ("%2d %2d %2d %2d %2d %2d %2d %2d",
      prPtr->ratings [RosPlayer::RATING_AC],
      prPtr->ratings [RosPlayer::RATING_AG],
      prPtr->ratings [RosPlayer::RATING_DI],
      prPtr->ratings [RosPlayer::RATING_EN],
      prPtr->ratings [RosPlayer::RATING_HA],
      prPtr->ratings [RosPlayer::RATING_IN],
      prPtr->ratings [RosPlayer::RATING_SP],
      prPtr->ratings [RosPlayer::RATING_ST]);
}
#endif

#if   DEBUG_COUNT
static void NEAR
monoDump ()
{
   mono.printf (6, 7, "% 5d", numCons);
   mono.printf (6, 8, "% 5d", numDest);
   mono.printf (6, 9, "% 5d", numCons - numDest);
}
#endif
